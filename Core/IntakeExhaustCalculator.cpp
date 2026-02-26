#define _USE_MATH_DEFINES
#include "IntakeExhaustCalculator.h"
#include <cmath>
#include <algorithm>
#include <sstream>

IntakeExhaustCalculator::IntakeExhaustCalculator(const EngineCore* eng) 
    : engine(eng) {
}

IntakeExhaustCalculator::~IntakeExhaustCalculator() {
}

// Calcula velocidade do som ajustada pela temperatura
// c = sqrt(gamma * R * T)
// Onde: gamma = 1.4 para ar, R = 287 J/(kg·K)
double IntakeExhaustCalculator::CalculateSpeedOfSound(double temperature) const {
    const double gamma = 1.4;  // Razão de calores específicos para ar
    const double R = 287.0;    // Constante do gás para ar (J/(kg·K))
    return sqrt(gamma * R * temperature);
}

double IntakeExhaustCalculator::CalculateDynamicPressure(double velocity, double density) const {
    // P_dinâmica = 0.5 * rho * v²
    return 0.5 * density * velocity * velocity;
}

// ========== CÁLCULOS DE ADMISSÃO ==========

// Calcula comprimento ideal do runner usando Teoria de Helmholtz
// Sistema de admissão age como ressonador de Helmholtz
// Referência: Blair - "Design and Simulation of Four-Stroke Engines", Chapter 8
double IntakeExhaustCalculator::CalculateIdealRunnerLength(double targetRPM, double runnerDiameter, 
                                                            double plenumVolume) const {
    if (targetRPM <= 0.0 || runnerDiameter <= 0.0 || plenumVolume <= 0.0) return 0.0;
    
    // Converte volume de litros para mm³
    double volumeMM3 = plenumVolume * 1e6;
    
    // Área do runner (mm²)
    double area = M_PI * (runnerDiameter / 2.0) * (runnerDiameter / 2.0);
    
    // Para motor 4T, cada cilindro induz uma vez por 2 revoluções
    // Frequência alvo (Hz) = RPM / 120
    double targetFrequency = targetRPM / 120.0;
    
    // Velocidade do som (mm/s)
    double c = CalculateSpeedOfSound(GAS_TEMP_INTAKE) * 1000.0;
    
    // Teoria de Helmholtz: f = (c/(2*pi)) * sqrt(A/(V*L))
    // Resolvendo para L: L = (c^2*A) / (4*pi^2*f^2*V)
    double length = (c * c * area) / (4.0 * M_PI * M_PI * targetFrequency * targetFrequency * volumeMM3);
    
    return length; // mm
}

double IntakeExhaustCalculator::CalculateHelmholtzFrequency(const IntakeManifoldConfig& config) const {
    if (config.runnerDiameter <= 0.0 || config.runnerLength <= 0.0 || config.plenumVolume <= 0.0) 
        return 0.0;
    
    // Área do runner (mm²)
    double area = M_PI * (config.runnerDiameter / 2.0) * (config.runnerDiameter / 2.0);
    
    // Volume (mm³)
    double volume = config.plenumVolume * 1e6;
    
    // Comprimento efetivo (inclui correção de extremidade)
    // L_eff = L + 0.85 × D (correção de extremidade aberta)
    double effectiveLength = config.runnerLength + (0.85 * config.runnerDiameter);
    
    // Velocidade do som (mm/s)
    double c = CalculateSpeedOfSound(GAS_TEMP_INTAKE) * 1000.0;
    
    // f = (c/(2*pi)) * sqrt(A/(V*L_eff))
    double frequency = (c / (2.0 * M_PI)) * sqrt(area / (volume * effectiveLength));
    
    return frequency; // Hz
}

double IntakeExhaustCalculator::CalculateTuningRPM(double frequency, int cylinders) const {
    if (frequency <= 0.0 || cylinders <= 0) return 0.0;
    
    // Para 4T: cada cilindro induz uma vez a cada 2 revoluções
    // RPM = f × 120
    return frequency * 120.0;
}

// Calcula diâmetro ideal baseado em velocidade de fluxo desejada
// Referência: SAE Paper 2003-01-0001 - "Intake System Design"
double IntakeExhaustCalculator::CalculateIdealRunnerDiameter(double displacement, double targetRPM,
                                                              double targetVelocity) const {
    if (displacement <= 0.0 || targetRPM <= 0.0 || targetVelocity <= 0.0) return 0.0;
    
    // Fluxo volumétrico por cilindro (m³/s)
    // Q = (displacement × rpm) / (120 × 1e6) para 4T
    double flowRate = (displacement * targetRPM) / (120.0 * 1e6);
    
    // Área necessária: A = Q / v
    double areaM2 = flowRate / targetVelocity;
    
    // Converter para mm²
    double areaMM2 = areaM2 * 1e6;
    
    // Diâmetro: D = 2 * sqrt(A/pi)
    double diameter = 2.0 * sqrt(areaMM2 / M_PI);
    
    return diameter; // mm
}

// Calcula volume ideal do plenum
// Regra empírica: 1.5-2.0 × cilindrada total para performance balanceada
// Referência: "Scientific Design of Intake Systems" - Smith & Morrison
double IntakeExhaustCalculator::CalculateIdealPlenumVolume(double totalDisplacement) const {
    if (totalDisplacement <= 0.0) return 0.0;
    
    // Fator de 1.75 é ótimo para maioria dos casos
    // Menor = melhor resposta transitória, pior topo
    // Maior = melhor topo, pior resposta
    double factor = 1.75;
    
    return (totalDisplacement / 1000.0) * factor; // litros
}

// Calcula comprimento ideal de corneta/velocity stack
// Referência: Blair - Chapter 8
double IntakeExhaustCalculator::CalculateIdealTrumpetLength(double runnerDiameter, double targetRPM) const {
    if (runnerDiameter <= 0.0 || targetRPM <= 0.0) return 0.0;
    
    // Comprimento típico: 0.75-1.5 × diâmetro do runner
    // Ajustado pelo RPM alvo
    double baseFactor = 1.0;
    
    // Alta rotação = corneta mais curta
    if (targetRPM > 8000.0) baseFactor = 0.75;
    else if (targetRPM > 6000.0) baseFactor = 1.0;
    else baseFactor = 1.25;
    
    return runnerDiameter * baseFactor;
}

IntakeAnalysis IntakeExhaustCalculator::CalculateIntakeAnalysis(const IntakeManifoldConfig& config,
                                                                 double rpm) const {
    IntakeAnalysis analysis = { 0 };
    
    if (!engine || !engine->IsValid()) return analysis;
    
    // Frequência de Helmholtz
    analysis.helmholtzFrequency = CalculateHelmholtzFrequency(config);
    
    // RPM de sintonia
    analysis.tuningRPM = CalculateTuningRPM(analysis.helmholtzFrequency, engine->GetCylinders());
    
    // Velocidade do fluxo
    double displacement = engine->CalculateDisplacement() / engine->GetCylinders();
    double area = M_PI * (config.runnerDiameter / 2.0) * (config.runnerDiameter / 2.0);
    double flowRate = (displacement * rpm) / (120.0 * 1e6); // m³/s
    analysis.flowVelocity = flowRate / (area / 1e6); // m/s
    
    // Número de Reynolds
    analysis.reynoldsNumber = CalculateReynoldsNumber(analysis.flowVelocity, 
                                                      config.runnerDiameter / 1000.0);
    
    // Efeito ram
    analysis.ramEffectPressure = CalculateRamEffect(analysis.flowVelocity) / 1000.0; // kPa
    
    // Eficiência volumétrica estimada
    analysis.volumetricEfficiency = CalculateVolumetricEfficiency(config, rpm);
    
    return analysis;
}

double IntakeExhaustCalculator::CalculateRamEffect(double velocity, double airDensity) const {
    // Pressão dinâmica = 0.5 * rho * v²
    return CalculateDynamicPressure(velocity, airDensity); // Pa
}

// Calcula eficiência volumétrica estimada
// Considera sintonia de comprimento e perdas de fricção
double IntakeExhaustCalculator::CalculateVolumetricEfficiency(const IntakeManifoldConfig& config,
                                                               double rpm) const {
    if (!engine || !engine->IsValid()) return 0.0;
    
    // Eficiência base: ~85%
    double baseEfficiency = 85.0;
    
    // Fator de sintonia (quão próximo do RPM ideal)
    double tuningRPM = CalculateTuningRPM(CalculateHelmholtzFrequency(config), 
                                          engine->GetCylinders());
    double rpmDifference = std::abs(rpm - tuningRPM);
    double tuningFactor = 1.0 - (rpmDifference / (tuningRPM * 2.0));
    tuningFactor = std::max(0.7, std::min(1.15, tuningFactor));
    
    // Fator de velocidade (velocidades ideais: 50-90 m/s)
    double displacement = engine->CalculateDisplacement() / engine->GetCylinders();
    double area = M_PI * (config.runnerDiameter / 2.0) * (config.runnerDiameter / 2.0);
    double velocity = ((displacement * rpm) / (120.0 * 1e6)) / (area / 1e6);
    
    double velocityFactor = 1.0;
    if (velocity < 50.0) velocityFactor = 0.90; // Muito lento
    else if (velocity > 90.0) velocityFactor = 0.85; // Muito rápido
    
    double efficiency = baseEfficiency * tuningFactor * velocityFactor;
    
    return std::max(70.0, std::min(115.0, efficiency)); // %
}

double IntakeExhaustCalculator::CalculateReynoldsNumber(double velocity, double diameter) const {
    // Re = (v * D) / nu
    // Onde: v = velocidade (m/s), D = diametro (m), nu = viscosidade cinemática
    return (velocity * diameter) / KINEMATIC_VISCOSITY;
}

// ========== CÁLCULOS DE ESCAPE ==========

// Calcula comprimento ideal do primário
// Baseado em reflexão de ondas de pressão
// Referência: Blair - "Design and Simulation", Chapter 9
double IntakeExhaustCalculator::CalculateIdealPrimaryLength(double exhaustDuration, double targetRPM) const {
    if (exhaustDuration <= 0.0 || targetRPM <= 0.0) return 0.0;
    
    // Velocidade do som nos gases de escape (mm/s)
    double c = CalculateSpeedOfSound(GAS_TEMP_EXHAUST) * 1000.0;
    
    // Tempo disponível para pulso (segundos)
    // Para 4T: tempo = (duração_graus / 360) × (60 / rpm)
    double pulseTime = (exhaustDuration / 360.0) * (60.0 / targetRPM);
    
    // Comprimento ideal: onda deve viajar ida e volta
    // L = (c × t) / 2
    double length = (c * pulseTime) / 2.0;
    
    // Fator de correção empírico (0.7-0.85 dependendo do design)
    length *= 0.75;
    
    return length; // mm
}

double IntakeExhaustCalculator::CalculateIdealPrimaryDiameter(double displacement, double targetRPM,
                                                               double targetVelocity) const {
    if (displacement <= 0.0 || targetRPM <= 0.0 || targetVelocity <= 0.0) return 0.0;
    
    // Gases de escape têm maior volume devido à temperatura
    // Fator de expansão: ~2.5-3.0×
    double expansionFactor = 2.7;
    
    // Fluxo volumétrico expandido (m³/s)
    double flowRate = ((displacement * targetRPM) / (120.0 * 1e6)) * expansionFactor;
    
    // Área necessária
    double areaM2 = flowRate / targetVelocity;
    double areaMM2 = areaM2 * 1e6;
    
    // Diâmetro
    double diameter = 2.0 * sqrt(areaMM2 / M_PI);
    
    return diameter; // mm
}

// Calcula comprimento do secundário (4-2-1)
// Deve ser sintonizado para segunda ordem de pulsos
double IntakeExhaustCalculator::CalculateIdealSecondaryLength(double primaryLength, 
                                                               double exhaustDuration) const {
    // Secundário tipicamente 1.5-2.0 × primário
    // Depende da duração e do overlap desejado
    double factor = 1.7; // valor médio otimizado
    
    return primaryLength * factor;
}

double IntakeExhaustCalculator::CalculateCollectorDiameter(double primaryDiameter, 
                                                            int numberOfPrimaries) const {
    if (primaryDiameter <= 0.0 || numberOfPrimaries <= 0) return 0.0;
    
    // Área total dos primários
    double primaryArea = M_PI * (primaryDiameter / 2.0) * (primaryDiameter / 2.0);
    double totalArea = primaryArea * numberOfPrimaries;
    
    // Diâmetro do coletor (área = soma das áreas dos primários)
    double collectorDiameter = 2.0 * sqrt(totalArea / M_PI);
    
    return collectorDiameter;
}

PulseAnalysis IntakeExhaustCalculator::CalculatePulseAnalysis(const ExhaustManifoldConfig& config,
                                                               double exhaustDuration, double rpm) const {
    PulseAnalysis analysis = { 0 };
    
    if (!engine || !engine->IsValid()) return analysis;
    
    // Velocidade do som nos gases de escape (m/s)
    double c = CalculateSpeedOfSound(GAS_TEMP_EXHAUST);
    
    // Frequência primária
    // f = c / (2 × L)
    analysis.primaryPulseFrequency = (c * 1000.0) / (2.0 * config.primaryLength);
    
    // Frequência secundária (para 4-2-1)
    if (!config.isFourIntoOne && config.secondaryLength > 0.0) {
        analysis.secondaryPulseFrequency = (c * 1000.0) / (2.0 * config.secondaryLength);
    }
    
    // RPM de sintonia
    // Baseado no tempo de reflexão do pulso
    analysis.tuningRPM = (c * 1000.0 * 60.0) / (4.0 * config.primaryLength);
    
    // Faixa efetiva: ±15% do RPM de sintonia
    analysis.effectiveRPMRange = analysis.tuningRPM * 0.15;
    
    // Tempo de reflexão (ms)
    analysis.reflectionTime = (2.0 * config.primaryLength) / (c * 1000.0) * 1000.0;
    
    // Eficiência de scavenging estimada
    analysis.scavengingEfficiency = CalculateScavengingEfficiency(config, exhaustDuration, 30.0);
    
    return analysis;
}

// Calcula eficiência de scavenging
// Referência: Heywood - "IC Engine Fundamentals", Chapter 6
double IntakeExhaustCalculator::CalculateScavengingEfficiency(const ExhaustManifoldConfig& config,
                                                               double exhaustDuration, double overlap) const {
    // Base efficiency: 70-85%
    double baseEfficiency = 75.0;
    
    // 4-2-1 tem melhor scavenging que 4-1 (5-10% ganho)
    double configFactor = config.isFourIntoOne ? 1.0 : 1.07;
    
    // Overlap maior = melhor scavenging (até certo ponto)
    double overlapFactor = 1.0 + (overlap / 200.0);
    overlapFactor = std::min(1.15, overlapFactor);
    
    // Diâmetro adequado ajuda scavenging
    double diameterFactor = 1.0;
    if (config.primaryDiameter < 30.0) diameterFactor = 0.90; // Muito pequeno
    else if (config.primaryDiameter > 60.0) diameterFactor = 0.95; // Muito grande
    
    double efficiency = baseEfficiency * configFactor * overlapFactor * diameterFactor;
    
    return std::max(60.0, std::min(95.0, efficiency)); // %
}

// Calcula backpressure estimada
double IntakeExhaustCalculator::CalculateBackpressure(const ExhaustManifoldConfig& config, 
                                                      double rpm, double displacement) const {
    // Modelo simplificado de backpressure
    // BP = f(fluxo, diâmetro, comprimento, rugosidade)
    
    // Fluxo de massa estimado (kg/s)
    double massFlow = (displacement * rpm * AIR_DENSITY) / (120.0 * 1e6);
    
    // Área do primário (m²)
    double area = M_PI * (config.primaryDiameter / 2000.0) * (config.primaryDiameter / 2000.0);
    
    // Velocidade (m/s)
    double velocity = massFlow / (AIR_DENSITY * area);
    
    // Backpressure (kPa) - aproximação
    double backpressure = (velocity * velocity) / 2000.0;
    
    // Fator de comprimento
    backpressure *= (config.primaryLength / 500.0);
    
    return backpressure; // kPa
}

std::wstring IntakeExhaustCalculator::Compare4Into1vs4Into2Into1(double targetRPM) const {
    std::wostringstream result;
    
    result << L"4-1 (Four-Into-One):\r\n";
    result << L"  + Mais simples e leve\r\n";
    result << L"  + Melhor para alta rotação (>" << static_cast<int>(targetRPM * 1.2) << L" rpm)\r\n";
    result << L"  + Menor custo\r\n";
    result << L"  + Scavenging moderado\r\n\r\n";
    
    result << L"4-2-1 (Four-Into-Two-Into-One):\r\n";
    result << L"  + Melhor scavenging (+5-10%)\r\n";
    result << L"  + Melhor torque médio\r\n";
    result << L"  + Faixa mais ampla\r\n";
    result << L"  - Mais complexo e pesado\r\n";
    
    return result.str();
}

// ========== CÁLCULOS DE TURBO/SUPERCHARGER ==========

double IntakeExhaustCalculator::CalculateMaxBoostPressure(double staticCR, double fuelOctane) const {
    // Regra empírica: CR_efetiva não deve exceder ~10:1 para gasolina premium
    // CR_eff = CR_static × (1 + boost/14.7)
    
    double maxEffectiveCR = 10.0; // Limite seguro para gasolina premium
    if (fuelOctane < 95) maxEffectiveCR = 9.0;
    else if (fuelOctane > 98) maxEffectiveCR = 11.0;
    
    // Boost máximo: boost = ((CR_max/CR_static) - 1) × 14.7
    double maxBoostPSI = ((maxEffectiveCR / staticCR) - 1.0) * 14.7;
    
    return std::max(0.0, maxBoostPSI); // PSI
}

double IntakeExhaustCalculator::CalculateEffectiveCompressionWithBoost(double staticCR, 
                                                                        double boostPSI) const {
    // CR_eff = CR_static * (P_intake / P_atm)
    // P_intake = P_atm + boost = P_atm * (1 + boost/14.7)
    // Logo: CR_eff = CR_static * (1 + boost/14.7)
    //
    // NAO se aplica expoente politropico aqui - a relacao e direta
    // pois estamos comparando a pressao no INICIO da compressao
    // Referencia: Heywood "ICE Fundamentals" Cap. 6
    //
    // Exemplo: CR=9:1, boost=14.7 PSI -> CR_eff = 9 * 2.0 = 18.0
    double boostRatio = 1.0 + (boostPSI / 14.7);
    return staticCR * boostRatio;
}

double IntakeExhaustCalculator::CalculateIntercoolerVolume(double displacement, 
                                                           double boostPSI) const {
    // Volume típico: 1.5-2.5 × cilindrada, aumentando com boost
    double baseFactor = 2.0;
    double boostFactor = 1.0 + (boostPSI / 20.0);
    
    return (displacement / 1000.0) * baseFactor * boostFactor; // litros
}

// ========== ANÁLISE DE FLUXO ==========

double IntakeExhaustCalculator::CalculateDischargeCoefficient(double diameter, double length, 
                                                               double velocity) const {
    // Cd varia com Reynolds e L/D ratio
    double ldRatio = length / diameter;
    
    // Cd típico: 0.85-0.95 para tubos retos
    double cd = 0.90;
    
    // Ajuste por L/D
    if (ldRatio < 5.0) cd -= 0.05;  // Muito curto
    if (ldRatio > 20.0) cd -= 0.03; // Muito longo (fricção)
    
    return cd;
}

// Calcula perda de carga usando equação de Darcy-Weisbach
double IntakeExhaustCalculator::CalculatePressureDrop(double length, double diameter, 
                                                      double velocity, double density) const {
    double re = CalculateReynoldsNumber(velocity, diameter / 1000.0);
    double f = CalculateFrictionFactor(re);
    
    // dP = f * (L/D) * (rho*v^2/2)
    double pressureDrop = f * (length / diameter) * CalculateDynamicPressure(velocity, density);
    
    return pressureDrop / 1000.0; // kPa
}

double IntakeExhaustCalculator::CalculateFrictionFactor(double reynoldsNumber, double roughness) const {
    // Equação de Colebrook-White simplificada
    if (reynoldsNumber < 2300.0) {
        // Fluxo laminar: f = 64/Re
        return 64.0 / reynoldsNumber;
    } else {
        // Fluxo turbulento: aproximação de Swamee-Jain
        // f = 0.25 / [log10(roughness/3.7 + 5.74/Re^0.9)]²
        double term = log10(roughness / 3.7 + 5.74 / pow(reynoldsNumber, 0.9));
        return 0.25 / (term * term);
    }
}

// ========== OTIMIZAÇÃO ==========

IntakeManifoldConfig IntakeExhaustCalculator::SuggestIntakeConfig(double targetRPM) const {
    IntakeManifoldConfig config = { 0 };
    
    if (!engine || !engine->IsValid()) return config;
    
    double displacement = engine->CalculateDisplacement() / engine->GetCylinders();
    double totalDisplacement = engine->CalculateDisplacement();
    
    // Sugere volume de plenum
    config.plenumVolume = CalculateIdealPlenumVolume(totalDisplacement);
    
    // Sugere diâmetro do runner
    config.runnerDiameter = CalculateIdealRunnerDiameter(displacement, targetRPM, 70.0);
    
    // Sugere comprimento do runner
    config.runnerLength = CalculateIdealRunnerLength(targetRPM, config.runnerDiameter, 
                                                     config.plenumVolume);
    
    // Sugere corneta
    config.trumpetLength = CalculateIdealTrumpetLength(config.runnerDiameter, targetRPM);
    config.trumpetDiameter = config.runnerDiameter * 1.5; // Expansão típica
    
    config.numberOfRunners = 1; // Individual por cilindro
    
    return config;
}

ExhaustManifoldConfig IntakeExhaustCalculator::SuggestExhaustConfig(double targetRPM, 
                                                                     double exhaustDuration) const {
    ExhaustManifoldConfig config = { 0 };
    
    if (!engine || !engine->IsValid()) return config;
    
    double displacement = engine->CalculateDisplacement() / engine->GetCylinders();
    
    // Sugere primário
    config.primaryLength = CalculateIdealPrimaryLength(exhaustDuration, targetRPM);
    config.primaryDiameter = CalculateIdealPrimaryDiameter(displacement, targetRPM, 100.0);
    
    // Decide entre 4-1 e 4-2-1
    if (targetRPM > 7000.0) {
        config.isFourIntoOne = true;
        config.secondaryLength = 0.0;
    } else {
        config.isFourIntoOne = false;
        config.secondaryLength = CalculateIdealSecondaryLength(config.primaryLength, exhaustDuration);
        config.secondaryDiameter = config.primaryDiameter * 1.3;
    }
    
    // Coletor final
    config.collectorDiameter = CalculateCollectorDiameter(config.primaryDiameter, 
                                                          engine->GetCylinders());
    config.mergeLengthRatio = 1.0;
    
    return config;
}

// ========== VALIDAÇÃO ==========

bool IntakeExhaustCalculator::IsValidIntakeConfig(const IntakeManifoldConfig& config) const {
    return config.runnerLength > 0.0 && 
           config.runnerDiameter > 0.0 &&
           config.plenumVolume > 0.0;
}

bool IntakeExhaustCalculator::IsValidExhaustConfig(const ExhaustManifoldConfig& config) const {
    return config.primaryLength > 0.0 && 
           config.primaryDiameter > 0.0;
}

std::wstring IntakeExhaustCalculator::GetIntakeAdvice(const IntakeManifoldConfig& config, 
                                                      double rpm) const {
    IntakeAnalysis analysis = CalculateIntakeAnalysis(config, rpm);
    
    std::wostringstream advice;
    
    if (analysis.flowVelocity < 50.0) {
        advice << L"[!] Velocidade muito baixa - Runner muito grande ou RPM baixo\r\n";
    } else if (analysis.flowVelocity > 90.0) {
        advice << L"[!] Velocidade muito alta - Aumentar diâmetro do runner\r\n";
    } else {
        advice << L"[OK] Velocidade de fluxo adequada\r\n";
    }
    
    if (std::abs(rpm - analysis.tuningRPM) < 1000.0) {
        advice << L"[OK] Sintonizado perfeitamente para este RPM\r\n";
    } else if (std::abs(rpm - analysis.tuningRPM) < 2000.0) {
        advice << L"[OK] Boa sintonia para esta faixa\r\n";
    } else {
        advice << L"[!] Fora da sintonia ideal - Ajustar comprimento do runner\r\n";
    }
    
    return advice.str();
}

std::wstring IntakeExhaustCalculator::GetExhaustAdvice(const ExhaustManifoldConfig& config, 
                                                       double rpm) const {
    std::wostringstream advice;
    
    if (config.primaryDiameter < 30.0) {
        advice << L"[!] Primario muito pequeno - Restrição, aumento de backpressure\r\n";
    } else if (config.primaryDiameter > 55.0) {
        advice << L"[!] Primario muito grande - Perda de velocidade e scavenging\r\n";
    } else {
        advice << L"[OK] Diametro do primario adequado\r\n";
    }
    
    if (config.isFourIntoOne) {
        advice << L"Configuração 4-1: Melhor para alta rotação\r\n";
    } else {
        advice << L"Configuração 4-2-1: Melhor torque médio e scavenging\r\n";
    }
    
    return advice.str();
}

std::wstring IntakeExhaustCalculator::AnalyzeForForcedInduction(const IntakeManifoldConfig& intake,
                                                                 double boostPressure) const {
    std::wostringstream analysis;
    
    analysis << L"ANÁLISE PARA INDUÇÃO FORÇADA:\r\n\r\n";
    
    if (boostPressure < 5.0) {
        analysis << L"Boost Baixo (<5 PSI):\r\n";
        analysis << L"  + Sistema de admissao original pode ser usado\r\n";
        analysis << L"  + Intercooler opcional\r\n";
    } else if (boostPressure < 10.0) {
        analysis << L"Boost Moderado (5-10 PSI):\r\n";
        analysis << L"  + Recomenda-se intercooler\r\n";
        analysis << L"  + Verificar runners quanto a vazamentos\r\n";
        analysis << L"  + Considerar reforco estrutural\r\n";
    } else {
        analysis << L"Boost Alto (>10 PSI):\r\n";
        analysis << L"  [!!] CRITICO: Intercooler obrigatório\r\n";
        analysis << L"  + Sistema de admissao reforçado necessário\r\n";
        analysis << L"  + Plenum maior recomendado\r\n";
        analysis << L"  + Runners encurtados podem ajudar resposta\r\n";
    }
    
    return analysis.str();
}
