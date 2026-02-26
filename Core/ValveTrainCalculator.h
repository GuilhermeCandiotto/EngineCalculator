#pragma once
// ============================================================================
// ValveTrainCalculator.h - Cálculos do Trem de Válvulas
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Referências:
// - Blair - "Design and Simulation of Four-Stroke Engines", Chapter 5-6
// - SAE Paper 2001-01-0662 - Valve Sizing
// - SAE J1121 - Valve Spring Design
// - Heywood - "IC Engine Fundamentals", Chapter 6
// ============================================================================
#include "EngineCore.h"
#include <cmath>
#include <string>

// Estrutura para dados de válvulas
struct ValveData {
    double intakeValveDiameter;     // Diâmetro da válvula de admissão (mm)
    double exhaustValveDiameter;    // Diâmetro da válvula de escape (mm)
    int numberOfIntakeValves;       // Número de válvulas de admissão por cilindro
    int numberOfExhaustValves;      // Número de válvulas de escape por cilindro
    double seatAngle;               // Ângulo do assento da válvula (graus)
};

// Estrutura para dados do trem de válvulas
struct ValveTrainData {
    double maxLift;                 // Levantamento máximo da válvula (mm)
    double springRate;              // Taxa da mola (N/mm)
    double springPreload;           // Pré-carga da mola (N)
    double springInstalledHeight;   // Altura instalada da mola (mm)
    double springCoilBindHeight;    // Altura de encosto das espiras (mm)
    double retainerWeight;          // Peso do prato/trava (gramas)
    double valveWeight;             // Peso da válvula (gramas)
    double rockerRatio;             // Relação do balanceiro
};

// Estrutura para resultados do fluxo
struct FlowResults {
    double intakeFlowArea;          // Área de fluxo de admissão (mm²)
    double exhaustFlowArea;         // Área de fluxo de escape (mm²)
    double flowRatio;               // Relação admissão/escape
    double meanPistonSpeed;         // Velocidade média do pistão (m/s)
    double intakeVelocity;          // Velocidade do fluxo de admissão (m/s)
};

// Classe para cálculos do trem de válvulas
// Referências:
// - "Design and Simulation of Four-Stroke Engines" - Gordon P. Blair
// - "Race Engine Technology" - SAE International
// - "Competition Engine Building" - John Baechtel
class ValveTrainCalculator {
private:
    const EngineCore* engine;
    
    // Calcula a área de cortina (curtain area)
    double CalculateCurtainArea(double valveDiameter, double lift, double seatAngle) const;
    
public:
    ValveTrainCalculator(const EngineCore* eng);
    ~ValveTrainCalculator();
    
    // ========== CÁLCULOS DE DIMENSIONAMENTO DE VÁLVULAS ==========
    
    // Calcula o diâmetro ideal da válvula de admissão
    // Baseado na relação bore-to-valve ratio
    // Típico: 42-48% do diâmetro do cilindro para 2 válvulas
    //         35-40% para configuração de 4 válvulas
    // Referência: Blair - "Design and Simulation of Four-Stroke Engines", Chapter 5
    double CalculateIdealIntakeValveDiameter(int valvesPerCylinder = 2) const;
    
    // Calcula o diâmetro ideal da válvula de escape
    // Relação típica: 85-92% do diâmetro da admissão
    // Referência: SAE Paper 2001-01-0662
    double CalculateIdealExhaustValveDiameter(double intakeDiameter) const;
    
    // Calcula a área total de válvulas
    double CalculateTotalValveArea(const ValveData& data) const;
    
    // Calcula a relação entre área de válvula e área do pistão
    // Valores típicos: 0.25-0.35 para motores de alta performance
    double CalculateValveToPistonAreaRatio(const ValveData& data) const;
    
    // ========== CÁLCULOS DE FLUXO ==========
    
    // Calcula a área de fluxo efetiva no lift especificado
    // Usa o modelo de curtain area
    // Referência: Blair - Chapter 6, Flow Analysis
    double CalculateEffectiveFlowArea(double valveDiameter, double lift, 
                                      double seatAngle = 45.0) const;
    
    // Calcula o coeficiente de descarga (discharge coefficient)
    // Cd típico: 0.6-0.75 para portas bem desenvolvidas
    double CalculateDischargeCoefficient(double lift, double valveDiameter) const;
    
    // Calcula a velocidade do fluxo através da válvula
    // V = Q / A (velocidade = vazão / área)
    // Referência: Heywood - "Internal Combustion Engine Fundamentals"
    FlowResults CalculateFlowCharacteristics(const ValveData& data, 
                                             double lift, double rpm) const;
    
    // ========== CÁLCULOS DE MOLAS ==========
    
    // Calcula a força da mola no levantamento máximo
    // F = F_preload + k * lift
    double CalculateSpringForceAtMaxLift(const ValveTrainData& data) const;
    
    // Calcula a frequência natural da mola (natural frequency)
    // Para evitar surge (flutuação)
    // f_n deve ser > 3 × rpm_max / 60 para motores 4T
    // Referência: "Valve Spring Design" - SAE J1121
    double CalculateSpringNaturalFrequency(double springRate, double activeCoils,
                                           double wireDiameter, double meanDiameter) const;
    
    // Calcula a margem de coil bind (folga até o encosto)
    double CalculateCoilBindMargin(const ValveTrainData& data) const;
    
    // Calcula a força de fechamento da válvula
    // Deve ser suficiente para manter contato em alta rpm
    double CalculateValveSeatingForce(const ValveTrainData& data, double rpm) const;
    
    // ========== CÁLCULOS DINÂMICOS ==========
    
    // Calcula a aceleração máxima da válvula
    // Importante para determinar a força de inércia
    double CalculateMaxValveAcceleration(double maxLift, double camDuration, 
                                         double rpm) const;
    
    // Calcula a força de inércia do trem de válvulas
    // F = m × a (massa × aceleração)
    double CalculateInertiaForce(const ValveTrainData& data, double acceleration) const;
    
    // Calcula o RPM máximo antes do float (perda de contato)
    // Quando força de inércia > força da mola
    double CalculateValveFloatRPM(const ValveTrainData& data, 
                                  double camDuration) const;
    
    // ========== CÁLCULOS DE ROCKER RATIO ==========
    
    // Calcula o lift real da válvula dado o lift do came
    double CalculateActualValveLift(double camLift, double rockerRatio) const;
    
    // Calcula a carga no came considerando o rocker ratio
    double CalculateLoadOnCam(double valveForce, double rockerRatio) const;
    
    // ========== VALIDAÇÃO E RECOMENDAÇÕES ==========
    
    bool IsValidValveData(const ValveData& data) const;
    bool IsValidValveTrainData(const ValveTrainData& data) const;
    
    std::wstring GetValveSizingAdvice(const ValveData& data) const;
    std::wstring GetSpringAdvice(const ValveTrainData& data, double maxRPM) const;
};
