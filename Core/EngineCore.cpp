#define _USE_MATH_DEFINES
#include "EngineCore.h"
#include <cmath>

EngineCore::EngineCore() 
    : bore(0.0), stroke(0.0), cylinders(4), 
      type(EngineType::FOUR_STROKE), deckHeight(0.0) {
}

EngineCore::~EngineCore() {
}

// Calcula a área do cilindro (mm²)
double EngineCore::CalculateBoreArea() const {
    if (bore <= 0.0) return 0.0;
    double radius = bore / 2.0;
    return M_PI * radius * radius;
}

// Calcula a cilindrada unitária (cc)
// Formula: V = pi * (bore/2)² * stroke
double EngineCore::CalculateDisplacement() const {
    if (!IsValid()) return 0.0;
    
    double area = CalculateBoreArea();
    double unitaryDisplacement = area * stroke / 1000.0; // mm³ para cc
    
    return unitaryDisplacement * cylinders;
}

// Calcula a relação diâmetro/curso
// Oversquare (bore > stroke): motores de alta rotação
// Square (bore = stroke): motores equilibrados
// Undersquare (bore < stroke): motores de torque
double EngineCore::CalculateBoreStrokeRatio() const {
    if (stroke <= 0.0) return 0.0;
    return bore / stroke;
}

// Calcula a relação comprimento da biela / curso
// Valores típicos: 1.5 a 2.0
// Maior = menos estresse angular, menor velocidade lateral do pistão
// Referência: Pulkrabek - "Engineering Fundamentals of ICE"
double EngineCore::CalculateRodStrokeRatio(double rodLength) const {
    if (stroke <= 0.0 || rodLength <= 0.0) return 0.0;
    return rodLength / stroke;
}

// Calcula a velocidade média do pistão (m/s)
// Fórmula: Vp = 2 * stroke * rpm / 60000
// Limite típico para motores de rua: 20-25 m/s
// Motores de competição: até 30 m/s
// Referência: SAE Paper 2004-01-1604
double EngineCore::CalculatePistonSpeed(double rpm) const {
    if (stroke <= 0.0 || rpm <= 0.0) return 0.0;
    return (2.0 * stroke * rpm) / 60000.0;
}

bool EngineCore::IsValid() const {
    return bore > 0.0 && stroke > 0.0 && cylinders > 0;
}

std::wstring EngineCore::GetValidationError() const {
    if (bore <= 0.0) return L"Diâmetro do cilindro inválido";
    if (stroke <= 0.0) return L"Curso do pistão inválido";
    if (cylinders <= 0) return L"Número de cilindros inválido";
    return L"";
}

// ============================================================================
// CÁLCULOS DE MEP (MEAN EFFECTIVE PRESSURE)
// ============================================================================
// Referências Científicas:
// - Heywood, J.B. "Internal Combustion Engine Fundamentals" (1988)
//   Capítulo 2.6-2.7: Mean Effective Pressure and Engine Performance
// - SAE J1349 (2011): Engine Power Test Code - Spark Ignition and Compression Ignition
// - SAE 2007-01-1464: BMEP and Thermal Efficiency Analysis in SI Engines
// - Blair, G.P. "Design and Simulation of Four-Stroke Engines" (1999)
//   Capítulo 4: Performance Analysis
// ============================================================================

// BMEP - Brake Mean Effective Pressure (kPa)
// Pressao media efetiva no FREIO (saida real apos perdas mecanicas)
// 
// DEFINICAO (Heywood, p.50):
//   "BMEP e a pressao teorica constante que, atuando sobre o pistao durante
//    o curso de potencia, produziria o mesmo trabalho que o motor real"
//
// FORMULA (Heywood eq. 2.13):
//   BMEP = (nR * 2 * pi * T) / Vd
//   Onde:
//     - nR = revolucoes por ciclo (2 para 4T, 1 para 2T)
//     - T  = torque [Nm]
//     - Vd = cilindrada total [m^3]
//
// EM UNIDADES PRATICAS:
//   BMEP [kPa] = (nR * 2 * pi * T [Nm]) / Vd [litros]
//   Para 4T: BMEP [kPa] = (4 * pi * T) / Vd  (fator = 4*pi = 12.566)
//   Para 2T: BMEP [kPa] = (2 * pi * T) / Vd  (fator = 2*pi = 6.283)
//
// VERIFICACAO com Honda B18C (T=178 Nm, Vd=1.797 L):
//   BMEP = 4*pi * 178 / 1.797 = 1245 kPa (12.45 bar) - OK!
//
// VALORES TIPICOS (Heywood, Tabela 2.2):
//   - Aspirado gasolina rua:     850-1050 kPa (8.5-10.5 bar)
//   - Aspirado gasolina race:    1100-1400 kPa (11-14 bar)
//   - Turbo gasolina street:     1200-1700 kPa (12-17 bar)
//   - Turbo gasolina race:       1700-2500 kPa (17-25 bar)
//   - Diesel aspirado:           700-900 kPa (7-9 bar)
//   - Diesel turbo:              1000-1600 kPa (10-16 bar)
//   - F1 (V10 aspirado 2005):    ~1450 kPa (14.5 bar) @ 19000 RPM
//   - Top Fuel Dragster:         ~1900 kPa (19 bar)
//
double EngineCore::CalculateBMEP(double torqueNm, double displacementLiters) const {
    if (torqueNm <= 0.0 || displacementLiters <= 0.0) return 0.0;

    // nR = revolucoes por ciclo de potencia
    double nR = (type == EngineType::FOUR_STROKE) ? 2.0 : 1.0;

    // BMEP [kPa] = (nR * 2 * pi * T [Nm]) / Vd [litros]
    // Derivacao: Vd[m^3] = Vd[L] * 1e-3
    //   BMEP[Pa] = (nR * 2*pi * T) / (Vd[L] * 1e-3)
    //   BMEP[kPa] = BMEP[Pa] / 1000 = (nR * 2*pi * T) / Vd[L]
    double bmep = (nR * 2.0 * M_PI * torqueNm) / displacementLiters;

    return bmep;
}

// IMEP - Indicated Mean Effective Pressure (kPa)
// Pressão média INDICADA (antes das perdas por fricção)
//
// DEFINIÇÃO (Heywood, p.49):
//   "IMEP é a pressão média efetiva calculada a partir do diagrama PV indicado,
//    representando o trabalho realizado PELO GÁS sobre o pistão"
//
// RELAÇÃO COM BMEP:
//   IMEP = BMEP / eta_m
//   Onde eta_m = eficiencia mecanica
//
// EFICIÊNCIA MECÂNICA TÍPICA (Blair, p.142):
//   - Motores modernos 4T:      82-87% (0.82-0.87)
//   - Motores race 4T:          85-90% (0.85-0.90)
//   - Motores 2T:               75-82% (0.75-0.82)
//   - F1 (alta tecnologia):     ~88-92% (0.88-0.92)
//
// FATORES QUE AFETAM:
//   - Atrito pistão/cilindro (maior fator)
//   - Atrito mancais (crankshaft, camshaft)
//   - Bomba de óleo
//   - Acessórios (alternador, etc.)
//   - Trem de válvulas
//
double EngineCore::CalculateIMEP(double bmep, double mechanicalEfficiency) const {
    if (bmep <= 0.0 || mechanicalEfficiency <= 0.0 || mechanicalEfficiency > 1.0) return 0.0;
    
    // IMEP = BMEP / eta_m
    double imep = bmep / mechanicalEfficiency;
    
    return imep;
}

// FMEP - Friction Mean Effective Pressure (kPa)
// Pressão equivalente às PERDAS POR FRICÇÃO
//
// DEFINIÇÃO (Heywood, p.50):
//   "FMEP representa o trabalho perdido por atrito e bombeamento de óleo,
//    expresso como uma pressão média equivalente"
//
// RELAÇÃO:
//   FMEP = IMEP - BMEP
//   
// MODELO DE CHEN-FLYNN (SAE Paper 1980):
//   FMEP = C1 + C2 × (Vp) + C3 × (Vp²)
//   Onde Vp = velocidade média do pistão
//   Coeficientes típicos (kPa):
//     C1 = 97 kPa   (atrito estático)
//     C2 = 15 kPa/(m/s)
//     C3 = 5 kPa/(m/s)²
//
// VALORES TÍPICOS (Heywood, Figura 2.13):
//   @ 1500 RPM:  ~100-150 kPa
//   @ 3000 RPM:  ~150-250 kPa
//   @ 6000 RPM:  ~250-450 kPa
//   @ 9000 RPM:  ~400-700 kPa (race engines)
//
double EngineCore::CalculateFMEP(double imep, double bmep) const {
    if (imep < bmep) return 0.0;
    
    // FMEP = IMEP - BMEP
    double fmep = imep - bmep;
    
    return fmep;
}

// PMEP - Pumping Mean Effective Pressure (kPa)
// Pressão de BOMBEAMENTO (perdas de admissão/escape)
//
// DEFINIÇÃO (Heywood, p.172-173):
//   "PMEP é o trabalho negativo realizado durante os cursos de admissão e escape,
//    expresso como pressão média equivalente. Representa a energia necessária
//    para 'bombear' a mistura fresca para dentro e os gases queimados para fora"
//
// COMPONENTES (Blair, p.156):
//   1. Restrição no intake (filtro de ar, borboleta, runners)
//   2. Backpressure no escape (catalisador, silencioso)
//   3. Perdas de válvulas (flow restriction)
//
// VALORES TÍPICOS (Heywood, Figura 6.5):
//   WOT (Wide Open Throttle):
//     - Aspirado bem projetado:    20-40 kPa (0.2-0.4 bar)
//     - Aspirado restritivo:       40-70 kPa (0.4-0.7 bar)
//   Partial Throttle (50%):
//     - Borboleta fechada:         200-400 kPa (2-4 bar) NEGATIVO!
//   Turbo/Supercharged:
//     - Pode ser NEGATIVO (boost > backpressure)
//
// MODELO SIMPLIFICADO (baseado em Heywood Fig. 6.5 e SAE 2002-01-0483):
//   PMEP_WOT = 20 + 5 * RPM/1000  [kPa]
//   PMEP_PartThrottle = PMEP_WOT * (1 + 5*(1 - throttle)^2)
//
double EngineCore::CalculatePMEP(double rpm, double throttlePosition) const {
    if (rpm <= 0.0 || throttlePosition < 0.0 || throttlePosition > 1.0) return 0.0;
    
    // PMEP base (WOT - Wide Open Throttle)
    // Heywood Figura 6.5: PMEP WOT cresce significativamente com RPM
    // Valores tipicos WOT: 20-40 kPa @3000, 50+ kPa @6000, 65+ kPa @9000
    double pmep_wot = 20.0 + 5.0 * (rpm / 1000.0);

    // Correcao para throttle parcial
    // Quando throttle fecha, PMEP aumenta drasticamente (vacuum)
    double throttleFactor = 1.0 + 5.0 * (1.0 - throttlePosition) * (1.0 - throttlePosition);
    
    double pmep = pmep_wot * throttleFactor;
    
    return pmep;
}

// CLASSIFICAÇÃO DE BMEP
// Baseado em valores típicos da literatura
std::wstring EngineCore::ClassifyBMEP(double bmep) const {
    if (bmep <= 0.0) return L"Inválido";
    
    // Classificação baseada em Heywood e experiência prática
    if (bmep < 700.0)
        return L"Muito Baixo - Possível problema no motor";
    else if (bmep < 900.0)
        return L"Baixo - Motor aspirado de baixa performance ou diesel antigo";
    else if (bmep < 1100.0)
        return L"Normal - Motor aspirado de rua típico (8.5-11 bar)";
    else if (bmep < 1400.0)
        return L"Bom - Motor aspirado bem projetado ou mild turbo";
    else if (bmep < 1700.0)
        return L"Muito Bom - Motor turbo de rua ou aspirado race";
    else if (bmep < 2000.0)
        return L"Excelente - Motor turbo de alta performance";
    else if (bmep < 2500.0)
        return L"Extremo - Motor de competição (rally, time attack)";
    else
        return L"Extremo+ - Top Fuel, Pro Stock, F1";
}

// VALORES TÍPICOS POR TIPO DE MOTOR
// Referência: Heywood Tabela 2.2 + dados empíricos
double EngineCore::GetTypicalBMEP(const std::wstring& engineType) const {
    if (engineType == L"Aspirado Rua") return 950.0;          // 9.5 bar
    if (engineType == L"Aspirado Race") return 1250.0;        // 12.5 bar
    if (engineType == L"Turbo Rua") return 1450.0;            // 14.5 bar
    if (engineType == L"Turbo Race") return 2100.0;           // 21 bar
    if (engineType == L"Diesel Rua") return 800.0;            // 8 bar
    if (engineType == L"Diesel Turbo") return 1300.0;         // 13 bar
    if (engineType == L"F1 (V10 Aspirado)") return 1450.0;    // 14.5 bar
    if (engineType == L"Top Fuel Dragster") return 1900.0;    // 19 bar
    
    return 1000.0; // Default
}
