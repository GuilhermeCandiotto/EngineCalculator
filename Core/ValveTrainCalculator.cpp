#define _USE_MATH_DEFINES
#include "ValveTrainCalculator.h"
#include <cmath>
#include <algorithm>

ValveTrainCalculator::ValveTrainCalculator(const EngineCore* eng) 
    : engine(eng) {
}

ValveTrainCalculator::~ValveTrainCalculator() {
}

// ========== CÁLCULOS DE DIMENSIONAMENTO DE VÁLVULAS ==========

// Calcula o diâmetro ideal da válvula de admissão
// Baseado na relação bore-to-valve diameter
// Referência: Blair - "Design and Simulation of Four-Stroke Engines"
// Chapter 5: Porting and Valves
double ValveTrainCalculator::CalculateIdealIntakeValveDiameter(int valvesPerCylinder) const {
    if (!engine || !engine->IsValid()) return 0.0;
    
    double bore = engine->GetBore();
    double ratio = 0.0;
    
    // Relações típicas baseadas em dados empíricos
    switch (valvesPerCylinder) {
        case 2: // 2 válvulas por cilindro (1 adm, 1 esc)
            ratio = 0.45; // 45% do bore
            break;
        case 4: // 4 válvulas por cilindro (2 adm, 2 esc)
            ratio = 0.37; // 37% do bore por válvula
            break;
        case 5: // 5 válvulas (3 adm, 2 esc)
            ratio = 0.32; // 32% do bore por válvula
            break;
        default:
            ratio = 0.40;
    }
    
    return bore * ratio;
}

// Calcula o diâmetro ideal da válvula de escape
// A válvula de escape é menor porque os gases saem sob pressão
// Relação típica: 85-90% da admissão
// Referência: SAE Paper 2001-01-0662
double ValveTrainCalculator::CalculateIdealExhaustValveDiameter(double intakeDiameter) const {
    if (intakeDiameter <= 0.0) return 0.0;
    
    // 88% é um valor ótimo para a maioria dos motores
    return intakeDiameter * 0.88;
}

// Calcula a área total de válvulas
double ValveTrainCalculator::CalculateTotalValveArea(const ValveData& data) const {
    double intakeArea = M_PI * (data.intakeValveDiameter / 2.0) * (data.intakeValveDiameter / 2.0);
    double exhaustArea = M_PI * (data.exhaustValveDiameter / 2.0) * (data.exhaustValveDiameter / 2.0);
    
    double totalIntake = intakeArea * data.numberOfIntakeValves;
    double totalExhaust = exhaustArea * data.numberOfExhaustValves;
    
    return totalIntake + totalExhaust;
}

// Calcula a relação entre área de válvula e área do pistão
// Valores típicos:
// - Motores de produção: 0.20-0.25
// - Motores de performance: 0.25-0.32
// - Motores de corrida: 0.32-0.40
// Referência: Blair, Chapter 5
double ValveTrainCalculator::CalculateValveToPistonAreaRatio(const ValveData& data) const {
    if (!engine || !engine->IsValid()) return 0.0;
    
    double pistonArea = engine->CalculateBoreArea();
    double intakeArea = M_PI * (data.intakeValveDiameter / 2.0) * (data.intakeValveDiameter / 2.0);
    intakeArea *= data.numberOfIntakeValves;
    
    return intakeArea / pistonArea;
}

// ========== CÁLCULOS DE FLUXO ==========

// Calcula a área de cortina (curtain area)
// É a área anular entre a válvula e o assento
// A = pi * D * L * cos(alpha)
// Onde: D = diâmetro da válvula, L = lift, alpha = angulo do assento
// Referência: Blair, Chapter 6
double ValveTrainCalculator::CalculateCurtainArea(double valveDiameter, double lift, 
                                                   double seatAngle) const {
    if (valveDiameter <= 0.0 || lift <= 0.0) return 0.0;
    
    double angleRad = seatAngle * M_PI / 180.0;
    return M_PI * valveDiameter * lift * cos(angleRad);
}

// Calcula a área de fluxo efetiva
// Considera o menor entre: área de cortina e área da cabeça da válvula
double ValveTrainCalculator::CalculateEffectiveFlowArea(double valveDiameter, double lift, 
                                                         double seatAngle) const {
    if (valveDiameter <= 0.0 || lift <= 0.0) return 0.0;
    
    // Área da cabeça da válvula
    double valveHeadArea = M_PI * (valveDiameter / 2.0) * (valveDiameter / 2.0);
    
    // Área de cortina
    double curtainArea = CalculateCurtainArea(valveDiameter, lift, seatAngle);
    
    // A área efetiva é a menor das duas
    return std::min(valveHeadArea, curtainArea);
}

// Calcula o coeficiente de descarga
// Cd varia com L/D (relação lift/diâmetro)
// Valores típicos:
// - L/D < 0.1: Cd ~ 0.4-0.5
// - L/D = 0.25: Cd ~ 0.6-0.7 (ótimo)
// - L/D > 0.35: Cd ~ 0.6-0.65
// Referência: Heywood - "IC Engine Fundamentals", Chapter 6
double ValveTrainCalculator::CalculateDischargeCoefficient(double lift, double valveDiameter) const {
    if (valveDiameter <= 0.0) return 0.0;
    
    double liftToDiameter = lift / valveDiameter;
    
    // Modelo empírico baseado em dados experimentais
    if (liftToDiameter < 0.05) return 0.40;
    else if (liftToDiameter < 0.15) return 0.50 + (liftToDiameter - 0.05) * 2.0;
    else if (liftToDiameter < 0.25) return 0.65 + (liftToDiameter - 0.15) * 0.5;
    else if (liftToDiameter < 0.35) return 0.70;
    else return 0.65;
}

// Calcula as características de fluxo
FlowResults ValveTrainCalculator::CalculateFlowCharacteristics(const ValveData& data, 
                                                                double lift, double rpm) const {
    FlowResults results = { 0 };
    
    if (!engine || !engine->IsValid()) return results;
    
    // Área de fluxo de admissão
    double singleIntakeArea = CalculateEffectiveFlowArea(data.intakeValveDiameter, lift, data.seatAngle);
    results.intakeFlowArea = singleIntakeArea * data.numberOfIntakeValves;
    
    // Área de fluxo de escape
    double singleExhaustArea = CalculateEffectiveFlowArea(data.exhaustValveDiameter, lift, data.seatAngle);
    results.exhaustFlowArea = singleExhaustArea * data.numberOfExhaustValves;
    
    // Relação admissão/escape
    if (results.exhaustFlowArea > 0.0) {
        results.flowRatio = results.intakeFlowArea / results.exhaustFlowArea;
    }
    
    // Velocidade média do pistão
    results.meanPistonSpeed = engine->CalculatePistonSpeed(rpm);
    
    // Velocidade do fluxo de admissão (aproximação)
    // V = Vp × Ap / Av
    // Onde: Vp = velocidade do pistão, Ap = área do pistão, Av = área da válvula
    double pistonArea = engine->CalculateBoreArea();
    if (results.intakeFlowArea > 0.0) {
        results.intakeVelocity = (results.meanPistonSpeed * pistonArea) / results.intakeFlowArea;
    }
    
    return results;
}

// ========== CÁLCULOS DE MOLAS ==========

// Calcula a força da mola no levantamento máximo
// F = F_preload + k × lift
double ValveTrainCalculator::CalculateSpringForceAtMaxLift(const ValveTrainData& data) const {
    return data.springPreload + (data.springRate * data.maxLift);
}

// Calcula a frequência natural da mola
// Para evitar surge (ressonância)
// Referência: SAE J1121 - "Valve Spring Design"
double ValveTrainCalculator::CalculateSpringNaturalFrequency(double springRate, double activeCoils,
                                                              double wireDiameter, double meanDiameter) const {
    if (activeCoils <= 0.0 || wireDiameter <= 0.0 || meanDiameter <= 0.0) return 0.0;
    
    // Módulo de cisalhamento do aço (GPa)
    const double G = 79.3e9; // 79.3 GPa em Pa
    
    // Massa da mola por espira ativa
    double wireLengthPerCoil = M_PI * meanDiameter;
    double volumePerCoil = M_PI * (wireDiameter / 2.0) * (wireDiameter / 2.0) * wireLengthPerCoil;
    double densitySteel = 7850.0; // kg/m³
    double springMass = volumePerCoil * activeCoils * densitySteel / 1e9; // converter mm³ para m³
    
    // Frequência natural (Hz)
    // f = (1/(2*pi)) * sqrt(k/m)
    return (1.0 / (2.0 * M_PI)) * sqrt(springRate * 1000.0 / springMass); // k em N/mm
}

// Calcula a margem de coil bind
double ValveTrainCalculator::CalculateCoilBindMargin(const ValveTrainData& data) const {
    double compressionAtMaxLift = data.springInstalledHeight - data.maxLift;
    return compressionAtMaxLift - data.springCoilBindHeight;
}

// Calcula a força de fechamento da válvula
// Deve superar a inércia em alta rotação
double ValveTrainCalculator::CalculateValveSeatingForce(const ValveTrainData& data, double rpm) const {
    // Força residual da mola no assento (força de pré-carga)
    double springForce = data.springPreload;
    
    // Em alta rpm, precisa vencer a inércia
    // Esta é uma verificação simplificada
    double acceleration = CalculateMaxValveAcceleration(data.maxLift, 240.0, rpm); // assumindo 240° de duração
    double inertiaForce = CalculateInertiaForce(data, acceleration);
    
    return springForce - inertiaForce;
}

// ========== CÁLCULOS DINÂMICOS ==========

// Calcula a aceleração máxima da válvula
// Para um perfil senoidal simplificado:
// a_max ~ (omega^2 * L × rpm²) / (1800² × (duração/360)²)
// Referência: "Valve Train Design" - Harold Brookbank
double ValveTrainCalculator::CalculateMaxValveAcceleration(double maxLift, double camDuration, 
                                                           double rpm) const {
    if (maxLift <= 0.0 || camDuration <= 0.0 || rpm <= 0.0) return 0.0;
    
    // Converte duração de graus para radianos
    double durationRad = camDuration * M_PI / 180.0;
    
    // Velocidade angular (rad/s)
    double omega = (2.0 * M_PI * rpm) / 60.0;
    
    // Para motor 4T, o came gira à metade da rotação do motor
    if (engine && engine->GetEngineType() == EngineType::FOUR_STROKE) {
        omega /= 2.0;
    }
    
    // Aceleracao maxima aproximada (m/s^2)
    // Para perfil senoidal meio-ciclo: y(t) = Lmax * sin(pi*t/T)
    // y''(t) = -Lmax * (pi/T)^2 * sin(pi*t/T)
    // |a_max| = Lmax * (pi/T)^2
    // Referencia: Blair - "Design and Simulation", Chapter 5
    double T = durationRad / omega; // tempo de abertura (s)
    double acceleration = (M_PI / T) * (M_PI / T) * (maxLift / 1000.0); // converter mm para m

    return acceleration;
}

// Calcula a força de inércia
// F = m × a
double ValveTrainCalculator::CalculateInertiaForce(const ValveTrainData& data, double acceleration) const {
    // Massa total em movimento (kg)
    double totalMass = (data.valveWeight + data.retainerWeight) / 1000.0; // gramas para kg
    
    // F = m × a (Newton)
    return totalMass * acceleration;
}

// Calcula o RPM máximo antes do float
// Float ocorre quando força de inércia > força da mola
double ValveTrainCalculator::CalculateValveFloatRPM(const ValveTrainData& data, 
                                                    double camDuration) const {
    if (data.maxLift <= 0.0 || camDuration <= 0.0) return 0.0;
    
    // Iteração para encontrar RPM onde força de inércia = força da mola
    double rpm = 1000.0;
    double step = 100.0;
    
    for (int i = 0; i < 100; i++) {
        double acceleration = CalculateMaxValveAcceleration(data.maxLift, camDuration, rpm);
        double inertiaForce = CalculateInertiaForce(data, acceleration);
        // Forca da mola no LIFT MAXIMO (onde float realmente ocorre)
        // F_max = preload + springRate * maxLift
        // Referencia: SAE J1121 - "Valve Spring Design and Testing"
        double springForce = data.springPreload + (data.springRate * data.maxLift);
        
        if (inertiaForce >= springForce) {
            return rpm;
        }
        
        rpm += step;
        if (rpm > 20000.0) break; // limite de segurança
    }
    
    return rpm;
}

// ========== CÁLCULOS DE ROCKER RATIO ==========

double ValveTrainCalculator::CalculateActualValveLift(double camLift, double rockerRatio) const {
    return camLift * rockerRatio;
}

double ValveTrainCalculator::CalculateLoadOnCam(double valveForce, double rockerRatio) const {
    return valveForce / rockerRatio;
}

// ========== VALIDAÇÃO ==========

bool ValveTrainCalculator::IsValidValveData(const ValveData& data) const {
    return data.intakeValveDiameter > 0.0 && 
           data.exhaustValveDiameter > 0.0 &&
           data.numberOfIntakeValves > 0 &&
           data.numberOfExhaustValves > 0;
}

bool ValveTrainCalculator::IsValidValveTrainData(const ValveTrainData& data) const {
    return data.maxLift > 0.0 && 
           data.springRate > 0.0 &&
           data.springPreload > 0.0;
}

std::wstring ValveTrainCalculator::GetValveSizingAdvice(const ValveData& data) const {
    if (!engine || !engine->IsValid()) return L"";
    
    double ratio = CalculateValveToPistonAreaRatio(data);
    
    if (ratio < 0.20) return L"[!] Area de valvula muito pequena - Restrição de fluxo";
    else if (ratio < 0.25) return L"[OK] Adequado para uso normal";
    else if (ratio < 0.32) return L"[OK] Otimo para performance";
    else if (ratio < 0.40) return L"[OK] Excelente para competicao";
    else return L"[!] Valvulas muito grandes - Possível perda de velocidade";
}

std::wstring ValveTrainCalculator::GetSpringAdvice(const ValveTrainData& data, double maxRPM) const {
    double margin = CalculateCoilBindMargin(data);
    
    if (margin < 1.0) return L"[!!] CRITICO: Margem de coil bind insuficiente!";
    else if (margin < 2.0) return L"[!] Margem de coil bind baixa";
    else if (margin < 3.0) return L"[OK] Margem de coil bind adequada";
    else return L"[OK] Margem de coil bind otima";
}
