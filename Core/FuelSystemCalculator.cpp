#define _USE_MATH_DEFINES
#include "FuelSystemCalculator.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

FuelSystemCalculator::FuelSystemCalculator(const EngineCore* eng) 
    : engine(eng) {
}

FuelSystemCalculator::~FuelSystemCalculator() {
}

// ========== CÁLCULOS DE CARBURADOR ==========

double FuelSystemCalculator::CalculateRequiredCFM(double displacement, double maxRPM, 
                                                   double volumetricEfficiency) const {
    // CFM = (CID × RPM × VE%) / 3456
    // 
    // DERIVAÇÃO DA FÓRMULA (Vizard - "How to Build Horsepower", Chapter 3):
    // 1. Volume por rotação = CID
    // 2. Volume por minuto = CID × RPM
    // 3. Para 4 tempos: motor admite a cada 2 rotações
    //    Volume efetivo = (CID × RPM) / 2
    // 4. Conversão in³ para ft³: dividir por 1728 (12³)
    //    CFM = (CID × RPM / 2) / 1728
    // 5. Simplificando: CFM = (CID × RPM) / (2 × 1728) = (CID × RPM) / 3456
    //
    // NOTA: 3456 já incorpora o fator de 4 tempos (2 rotações)!
    //
    // Referências:
    // - Vizard, David - "How to Build Horsepower", Chapter 3
    // - Larew Performance - "Carburetor Sizing Calculator"
    
    // Converte CC para CI se necessário
    double displacementCI = displacement * 0.0610237; // cc para ci
    
    // CFM = (CID × RPM × VE%) / 3456
    double cfm = (displacementCI * maxRPM * (volumetricEfficiency / 100.0)) / 3456.0;
    
    return cfm;
}

double FuelSystemCalculator::CalculateCFMWithBoost(double baseCFM, double boostPSI) const {
    // CFM com boost = CFM × (1 + boost/14.7)
    return baseCFM * (1.0 + (boostPSI / 14.7));
}

int FuelSystemCalculator::SuggestCarburetorSize(double requiredCFM) const {
    std::vector<int> sizes = GetCommonCarburetorSizes();
    
    // Adiciona 10-15% de margem
    double targetCFM = requiredCFM * 1.10;
    
    // Encontra o tamanho comercial mais próximo (arredonda para cima)
    for (int size : sizes) {
        if (size >= targetCFM) {
            return size;
        }
    }
    
    return sizes.back(); // Retorna o maior se nenhum for suficiente
}

double FuelSystemCalculator::CalculateMainJetSize(double cfm, double targetAFR) const {
    // Fórmula simplificada: jet size é proporcional ao CFM e inversamente proporcional ao AFR
    // Tamanho base para AFR 14.7 e 600 CFM = 70
    double baseJet = 70.0;
    double baseCFM = 600.0;
    double baseAFR = 14.7;
    
    double jetSize = baseJet * (cfm / baseCFM) * (baseAFR / targetAFR);
    
    return jetSize;
}

double FuelSystemCalculator::CalculateVenturiVelocity(double cfm, double venturiDiameter) const {
    // V = Q / A
    // Q = CFM * 0.000471947 m^3/min (conversao exata)
    // A = pi * (D/2)^2 em m^2

    double radiusMM = venturiDiameter / 2.0;
    double areaM2 = (M_PI * radiusMM * radiusMM) / 1000000.0;

    // Converte CFM para m^3/s: CFM * 0.000471947 / 60
    double velocity = (cfm * 0.000471947) / (areaM2 * 60.0); // m/s

    return velocity;
}

// ========== CÁLCULOS DE INJEÇÃO ==========

double FuelSystemCalculator::CalculateRequiredInjectorFlow(double targetHP, double afr, 
                                                           double dutyCycle, int numInjectors) const {
    // Flow (lb/hr) = (HP × BSFC) / (num_injectors × (duty_cycle/100))
    // Usa BSFC_NA_STREET como padrão (conservador)
    double bsfc = BSFC_NA_STREET; 
    
    double flowLbHr = (targetHP * bsfc) / (numInjectors * (dutyCycle / 100.0));
    
    // Converte para cc/min
    return LbHrToCCMin(flowLbHr);
}

double FuelSystemCalculator::LbHrToCCMin(double lbhr) const {
    // Conversão EXATA lb/hr para cc/min para gasolina
    // 
    // DERIVAÇÃO (RC Engineering - "Fuel Injector Flow Calculator"):
    // 1 lb/hr = 453.6 g/hr
    // Densidade gasolina = 0.72 g/cc (SG = 0.72)
    // Volume (cc/hr) = massa (g/hr) / densidade (g/cc)
    // Volume (cc/min) = Volume (cc/hr) / 60
    //
    // Fórmula: cc/min = (lb/hr × 453.6 g/lb) / (0.72 g/cc) / 60 min
    //          cc/min = lb/hr × (453.6 / 0.72 / 60)
    //          cc/min = lb/hr × 10.5
    //
    // Referência: RC Engineering, ASNU Injector Data Book
    
    const double GASOLINE_DENSITY_G_CC = 0.72; // SG gasolina
    const double GRAMS_PER_POUND = 453.6;
    const double MINUTES_PER_HOUR = 60.0;
    
    return lbhr * (GRAMS_PER_POUND / GASOLINE_DENSITY_G_CC / MINUTES_PER_HOUR);
}

double FuelSystemCalculator::CCMinToLbHr(double ccmin) const {
    // Conversão inversa: cc/min para lb/hr
    const double GASOLINE_DENSITY_G_CC = 0.72;
    const double GRAMS_PER_POUND = 453.6;
    const double MINUTES_PER_HOUR = 60.0;
    
    return ccmin / (GRAMS_PER_POUND / GASOLINE_DENSITY_G_CC / MINUTES_PER_HOUR);
}

double FuelSystemCalculator::CalculateRequiredDutyCycle(double targetHP, double injectorFlow, 
                                                        double afr, int numInjectors) const {
    double bsfc = BSFC_NA_STREET; // Padrão conservador
    
    // Duty = (HP × BSFC) / (injector_flow_lb/hr × num_injectors) × 100
    double flowLbHr = CCMinToLbHr(injectorFlow);
    
    double dutyCycle = ((targetHP * bsfc) / (flowLbHr * numInjectors)) * 100.0;
    
    return dutyCycle;
}

double FuelSystemCalculator::CorrectFlowForPressure(double baseFlow, double basePressure, 
                                                    double newPressure) const {
    // Flow_new = Flow_base * sqrt(P_new / P_base)
    return baseFlow * sqrt(newPressure / basePressure);
}

double FuelSystemCalculator::SuggestFuelPressure(double boostPSI) const {
    // Base: 43.5 PSI (3 bar) para aspirado
    // Adiciona boost 1:1 para manter pressão diferencial
    return 43.5 + boostPSI;
}

// ========== BOMBA DE COMBUSTÍVEL ==========

double FuelSystemCalculator::CalculateRequiredFuelPumpFlow(double targetHP, double afr) const {
    double bsfc = BSFC_NA_STREET; // Padrão conservador
    
    // Flow (L/h) = (HP × BSFC × 1.3) × conversion
    // 1.3 = fator de segurança
    double flowLbHr = targetHP * bsfc * 1.3;
    
    // Converte lb/hr para L/h
    // 1 lb/hr ~ 0.454 kg/hr, gasolina ~ 0.75 kg/L
    double flowLh = (flowLbHr * 0.454) / GASOLINE_DENSITY;
    
    return flowLh;
}

bool FuelSystemCalculator::IsPumpAdequate(double pumpFlow, double requiredFlow, 
                                          double pumpPressure, double requiredPressure) const {
    return (pumpFlow >= requiredFlow * 1.2) && (pumpPressure >= requiredPressure);
}

// ========== CONSUMO ==========

double FuelSystemCalculator::CalculateBSFC(double power, bool isTurbo, bool isRace) const {
    // Seleciona BSFC baseado no tipo de motor
    // Referência: Heywood - "ICE Fundamentals" Figura 3.17
    //            SAE 2007-01-1464 - "BSFC Analysis in SI Engines"
    
    if (isRace) {
        // Motores de competição: CR alta, tuning otimizado
        if (isTurbo) {
            return BSFC_TURBO_RACE;  // 0.50 lb/HP/hr (235 g/kWh)
        } else {
            return BSFC_NA_RACE;     // 0.45 lb/HP/hr (212 g/kWh)
        }
    } else {
        // Motores de rua: CR moderada, tuning conservador
        if (isTurbo) {
            return BSFC_TURBO_STREET; // 0.55 lb/HP/hr (259 g/kWh)
        } else {
            return BSFC_NA_STREET;    // 0.50 lb/HP/hr (235 g/kWh)
        }
    }
}

double FuelSystemCalculator::CalculateFuelFlowRate(double power, double bsfc) const {
    // Flow (L/h) = (power_HP × BSFC_lb/hr × 0.454_kg/lb) / density_kg/L
    double flowKgH = power * bsfc * 0.454;
    return flowKgH / GASOLINE_DENSITY;
}

double FuelSystemCalculator::EstimateRange(double fuelFlow, double tankSize, double avgSpeed) const {
    if (fuelFlow <= 0.0) return 0.0;
    
    // Autonomia (km) = (tank_L / flow_L/h) × speed_km/h
    double timeHours = tankSize / fuelFlow;
    return timeHours * avgSpeed;
}

double FuelSystemCalculator::CalculateFuelEconomy(double fuelFlow, double avgSpeed, bool mpg) const {
    if (fuelFlow <= 0.0) return 0.0;
    
    // km/L = speed (km/h) / flow (L/h)
    double kmPerLiter = avgSpeed / fuelFlow;
    
    if (mpg) {
        // Converte para MPG (USA)
        return kmPerLiter * 2.352;
    }
    
    return kmPerLiter;
}

// ========== ANÁLISE COMPLETA ==========

FuelSystemAnalysis FuelSystemCalculator::AnalyzeFuelSystem(const FuelSystemConfig& config) const {
    FuelSystemAnalysis analysis = { 0 };
    
    if (!engine || !engine->IsValid()) {
        return analysis;
    }
    
    double displacement = engine->CalculateDisplacement();
    
    // CFM necessário
    analysis.requiredCFM = CalculateRequiredCFM(displacement, config.maxRPM, 85.0);
    if (config.boostPressure > 0) {
        analysis.requiredCFM = CalculateCFMWithBoost(analysis.requiredCFM, config.boostPressure);
    }
    
    // Injetor necessário
    analysis.requiredInjectorFlow = CalculateRequiredInjectorFlow(
        config.targetHP, config.targetAFR, 80.0, config.numberOfInjectors);
    analysis.requiredInjectorFlowLbHr = CCMinToLbHr(analysis.requiredInjectorFlow);
    
    // Consumo
    double bsfc = CalculateBSFC(config.targetHP, config.boostPressure > 0, false);
    analysis.fuelFlowRate = CalculateFuelFlowRate(config.targetHP, bsfc);
    // Conversao lb/HP/hr para g/kWh:
    // 1 lb/HP/hr = 453.592 g / (0.7457 kW * 1 hr) = 608.3 g/kWh
    analysis.specificFuelConsumption = bsfc * (453.592 / 0.7457);
    
    // Autonomia estimada (tanque 50L, 80 km/h)
    analysis.estimatedRange = EstimateRange(analysis.fuelFlowRate, 50.0, 80.0);
    
    // Duty cycle máximo recomendado
    analysis.maxDutyCycle = 85.0; // Limite seguro
    
    return analysis;
}

// ========== AFR ==========

double FuelSystemCalculator::GetIdealAFR(bool isIdle) const {
    if (isIdle) return 13.5; // Rico para marcha lenta estável
    return 14.7; // Estequiométrico
}

double FuelSystemCalculator::CalculateLambda(double afr) const {
    return afr / STOICH_AFR_GASOLINE; // Lambda = AFR_real / AFR_stoich
}

// ========== ANÁLISE DE TIPO DE ADMISSÃO ==========

std::wstring FuelSystemCalculator::CompareITBvsPlenum(double targetRPM, bool isRaceApplication) const {
    std::wostringstream comparison;
    
    comparison << L"---------------------------------------------------------------------------\r\n";
    comparison << L"  ITB vs PLENUM - COMPARACAO\r\n";
    comparison << L"---------------------------------------------------------------------------\r\n\r\n";
    
    comparison << L"PLENUM (Entrada Unica + Runners):\r\n";
    comparison << L"  + Mais simples e barato\r\n";
    comparison << L"  + Melhor torque em baixa/media rotacao\r\n";
    comparison << L"  + Efeito ram mais pronunciado\r\n";
    comparison << L"  + Mais facil de ajustar\r\n";
    comparison << L"  + Melhor para uso diario\r\n";
    comparison << L"  - VE ligeiramente menor em alta rpm\r\n";
    comparison << L"  - Resposta do acelerador um pouco mais lenta\r\n\r\n";
    
    comparison << L"ITB (Individual Throttle Bodies):\r\n";
    comparison << L"  + Resposta do acelerador instantanea\r\n";
    comparison << L"  + VE 5-10% maior em alta rotacao\r\n";
    comparison << L"  + Melhor distribuicao de ar entre cilindros\r\n";
    comparison << L"  + Som caracteristico incrivel\r\n";
    comparison << L"  + Ideal para >8000 RPM\r\n";
    comparison << L"  - Mais caro (4x borboletas + 4x TPS)\r\n";
    comparison << L"  - Ajuste mais complexo (sincronizacao)\r\n";
    comparison << L"  - Perda de torque baixo (sem efeito ram)\r\n";
    comparison << L"  - Maior consumo em uso urbano\r\n\r\n";
    
    comparison << L"RECOMENDACAO PARA SEU CASO:\r\n";
    if (isRaceApplication || targetRPM > 8500) {
        comparison << L"  -> ITB - Melhor escolha para competicao/alta rotacao\r\n";
        comparison << L"  Ganho estimado de VE: 7-10%\r\n";
        comparison << L"  Ganho estimado de potencia: 5-8 HP em alta rpm\r\n";
    } else if (targetRPM > 7000) {
        comparison << L"  -> Ambos viaveis - Depende do orcamento\r\n";
        comparison << L"  ITB: +5% potencia alta rpm, -10% torque baixo\r\n";
        comparison << L"  Plenum: Melhor custo-beneficio\r\n";
    } else {
        comparison << L"  -> PLENUM - Melhor escolha para uso misto/diario\r\n";
        comparison << L"  ITB nao vale o custo para RPM <7000\r\n";
        comparison << L"  Plenum oferece melhor dirigibilidade\r\n";
    }
    
    comparison << L"\r\n---------------------------------------------------------------------------\r\n";
    
    return comparison.str();
}

double FuelSystemCalculator::EstimateVEGainWithITB(double baseVE, double rpm) const {
    // REFERÊNCIAS CIENTÍFICAS DISPONÍVEIS SOBRE ITBs:
    // 
    // 1. SAE 890243 - "The Effect of Intake System Design on Multi-Cylinder Engine Breathing"
    //    - Compara plenum vs entradas individuais
    //    - Resultado: VE melhora 3-8% acima de 6500 RPM (varia por motor)
    //
    // 2. SAE 2008-01-0398 - "Optimization of Individual Runner Intake Manifolds"
    //    - Estuda runners individuais vs plenum
    //    - Resultado: Melhoria de 4-6% em VE com configuração otimizada
    //
    // 3. Bell, A. Graham - "Four-Stroke Performance Tuning", Chapter 3
    //    - Menciona ITBs melhoram distribuição e resposta acima de 7000 RPM
    //    - Não quantifica valores específicos (qualitativo)
    //
    // 4. Blair - "Design and Simulation", Chapter 8
    //    - Explica perda de efeito RAM do plenum em baixa rotação
    //    - Ganho em alta por melhor distribuição e menor restrição
    //
    // 5. Dados empíricos de fabricantes (Jenvey, Weber):
    //    - Ganhos típicos: 5-10% em potência de pico
    //    - Altamente dependente de setup (sincronização, velocity stacks, etc.)
    //
    // VALORES IMPLEMENTADOS:
    // Conservadores, baseados em SAE 890243 e SAE 2008-01-0398
    // Margem inferior dos estudos para maior confiabilidade
    
    double gainPercent = 0.0;
    
    if (rpm < 6000) {
        // Baixa rotação: Perda por falta de efeito RAM
        // Blair Chapter 8: Plenum cria onda de pressão em baixa rpm
        gainPercent = -2.0; // Perda típica documentada
    } else if (rpm < 7000) {
        // Transição: Efeito RAM diminui, distribuição melhora
        gainPercent = 1.0; // Começa a compensar
    } else if (rpm < 8000) {
        // Alta: Melhor distribuição predomina
        // SAE 890243: 3-8% nesta faixa (usamos 3% conservador)
        gainPercent = 3.0;
    } else if (rpm < 9000) {
        // Muito alta: Ganho mais pronunciado
        // SAE 2008-01-0398: 4-6% (usamos 5% centro da faixa)
        gainPercent = 5.0;
    } else {
        // Extrema: Máximo ganho (limite superior SAE 890243)
        gainPercent = 7.0; // Conservador (papers reportam até 8%)
    }
    
    return baseVE + (baseVE * gainPercent / 100.0);
}

// ========== VALIDAÇÃO ==========

bool FuelSystemCalculator::IsValidCarburetorSize(double cfm, double engineCFM) const {
    // Carburador deve ser 10-20% maior que necessário
    return (cfm >= engineCFM * 1.05) && (cfm <= engineCFM * 1.30);
}

bool FuelSystemCalculator::IsValidInjectorSize(double flow, double requiredFlow) const {
    // Injetor deve suportar pelo menos 120% do requerido
    return flow >= requiredFlow * 1.20;
}

std::wstring FuelSystemCalculator::GetFuelSystemAdvice(const FuelSystemConfig& config, 
                                                       const FuelSystemAnalysis& analysis) const {
    std::wostringstream advice;
    
    if (config.systemType == FuelSystemType::CARBURETOR) {
        int suggestedSize = SuggestCarburetorSize(analysis.requiredCFM);
        advice << L"Carburador Recomendado: " << suggestedSize << L" CFM\n";
        advice << L"CFM Necessário: " << std::fixed << std::setprecision(1) << analysis.requiredCFM << L"\n";
    } else {
        advice << L"Injetores Recomendados: " << std::fixed << std::setprecision(0) 
               << analysis.requiredInjectorFlow << L" cc/min\n";
        advice << L"ou " << std::fixed << std::setprecision(1) 
               << analysis.requiredInjectorFlowLbHr << L" lb/hr\n";
    }
    
    advice << L"\nConsumo Estimado: " << std::fixed << std::setprecision(1) 
           << analysis.fuelFlowRate << L" L/h a " << config.targetHP << L" HP\n";
    advice << L"Autonomia Estimada: " << std::fixed << std::setprecision(0) 
           << analysis.estimatedRange << L" km (tanque 50L, 80 km/h)\n";
    
    return advice.str();
}

// ========== TABELAS ==========

std::vector<int> FuelSystemCalculator::GetCommonCarburetorSizes() const {
    return { 390, 450, 500, 600, 650, 700, 750, 800, 850, 950, 1050, 1150, 1250 };
}

std::vector<int> FuelSystemCalculator::GetCommonInjectorSizes() const {
    // cc/min
    return { 160, 200, 240, 280, 320, 360, 440, 550, 650, 750, 850, 1000, 1200, 1600, 2000 };
}
