#pragma once
// ============================================================================
// IntakeExhaustCalculator.h - Calculos de Admissao e Escape
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Referencias:
// - Blair - "Design and Simulation of Four-Stroke Engines", Chapter 8-9
// - SAE Paper 2003-01-0001 - Intake and Exhaust System Design
// - Pulkrabek - "Engineering Fundamentals of the ICE"
// - Smith & Morrison - "Scientific Design of Intake/Exhaust Systems"
// ============================================================================
#include "EngineCore.h"
#include <cmath>
#include <string>
#include <vector>

// Configuracao de coletor de admissao
struct IntakeManifoldConfig {
    double runnerLength;            // Comprimento do runner (mm)
    double runnerDiameter;          // Diametro interno do runner (mm)
    double plenumVolume;            // Volume do plenum (litros)
    double trumpetLength;           // Comprimento da corneta/velocity stack (mm)
    double trumpetDiameter;         // Diametro da corneta (mm)
    int numberOfRunners;            // Numero de runners (1-4 por cilindro)
};

// Configuracao de coletor de escape
struct ExhaustManifoldConfig {
    double primaryLength;           // Comprimento do primario (mm)
    double primaryDiameter;         // Diametro interno do primario (mm)
    double secondaryLength;         // Comprimento do secundario (mm)
    double secondaryDiameter;       // Diametro do secundario (mm)
    double collectorDiameter;       // Diametro do coletor final (mm)
    bool isFourIntoOne;             // true = 4-1, false = 4-2-1
    double mergeLengthRatio;        // Relacao de comprimento do merge (0.5-1.5)
};

// Resultados de analise de pulsos
struct PulseAnalysis {
    double primaryPulseFrequency;   // Frequencia do pulso primario (Hz)
    double secondaryPulseFrequency; // Frequencia do pulso secundario (Hz)
    double tuningRPM;               // RPM de sintonia (peak)
    double effectiveRPMRange;       // Faixa efetiva (+/-rpm)
    double reflectionTime;          // Tempo de reflexao (ms)
    double scavengingEfficiency;    // Eficiencia de scavenging (%)
};

// Resultados de analise de admissao
struct IntakeAnalysis {
    double helmholtzFrequency;      // Frequencia de Helmholtz (Hz)
    double tuningRPM;               // RPM de sintonia
    double ramEffectPressure;       // Pressao do efeito ram (kPa)
    double volumetricEfficiency;    // Eficiencia volumetrica (%)
    double flowVelocity;            // Velocidade do fluxo (m/s)
    double reynoldsNumber;          // Numero de Reynolds
};

// Classe para calculos de sistemas de admissao e escape
// Referencias principais:
// - "Design and Simulation of Four-Stroke Engines" - Gordon P. Blair
// - "Engineering Fundamentals of the Internal Combustion Engine" - Pulkrabek
// - "Intake and Exhaust System Design" - SAE Paper 2003-01-0001
// - "Performance Exhaust Systems" - Mike Mavrigian
// - "Scientific Design of Intake and Exhaust Systems" - Philip H. Smith & John C. Morrison
class IntakeExhaustCalculator {
private:
    const EngineCore* engine;
    
    // Constantes fisicas
    static constexpr double SPEED_OF_SOUND = 343.0;      // m/s a 20 C
    static constexpr double GAS_TEMP_INTAKE = 293.15;    // K (20 C)
    static constexpr double GAS_TEMP_EXHAUST = 1073.15;  // K (800 C)
    static constexpr double AIR_DENSITY = 1.225;         // kg/m^3
    static constexpr double KINEMATIC_VISCOSITY = 1.5e-5; // m^2/s
    
    // Calcula velocidade do som ajustada pela temperatura
    double CalculateSpeedOfSound(double temperature) const;
    
    // Calcula pressao dinamica do fluxo
    double CalculateDynamicPressure(double velocity, double density) const;
    
public:
    IntakeExhaustCalculator(const EngineCore* eng);
    ~IntakeExhaustCalculator();
    
    // ========== CALCULOS DE ADMISSAO ==========
    
    // Calcula o comprimento ideal do runner usando Teoria de Helmholtz
    // Formula: f = (c/(2*pi)) * sqrt(A/(V*L))
    // Onde: c = velocidade do som, A = area, V = volume, L = comprimento
    // Referencia: Blair - "Design and Simulation", Chapter 8
    double CalculateIdealRunnerLength(double targetRPM, double runnerDiameter, 
                                      double plenumVolume) const;
    
    // Calcula a frequencia de Helmholtz do sistema
    // Sistema se comporta como ressonador de Helmholtz
    double CalculateHelmholtzFrequency(const IntakeManifoldConfig& config) const;
    
    // Calcula o RPM de sintonia baseado na frequencia
    // Para motor 4T: RPM = (f * 60 * 2) / (cilindros)
    double CalculateTuningRPM(double frequency, int cylinders) const;
    
    // Calcula o diametro ideal do runner
    // Baseado na velocidade de fluxo desejada (tipicamente 50-90 m/s)
    // Referencia: SAE Paper 2003-01-0001
    double CalculateIdealRunnerDiameter(double displacement, double targetRPM,
                                        double targetVelocity = 70.0) const;
    
    // Calcula o volume ideal do plenum
    // Regra geral: 1.5-2.0 x cilindrada total
    // Referencia: "Scientific Design of Intake Systems" - Smith & Morrison
    double CalculateIdealPlenumVolume(double totalDisplacement) const;
    
    // Calcula o comprimento ideal da corneta (velocity stack/trumpet)
    // Melhora resposta transitoria e aumenta eficiencia volumetrica
    // Tipico: 50-150mm dependendo do RPM alvo
    double CalculateIdealTrumpetLength(double runnerDiameter, double targetRPM) const;
    
    // Calcula a analise completa de admissao
    IntakeAnalysis CalculateIntakeAnalysis(const IntakeManifoldConfig& config,
                                           double rpm) const;
    
    // Calcula o efeito ram (dynamic pressure rise)
    // Ganho de pressao devido a inercia do ar
    double CalculateRamEffect(double velocity, double airDensity = AIR_DENSITY) const;
    
    // Calcula eficiencia volumetrica estimada
    // Considera runner length, diameter, e rpm
    double CalculateVolumetricEfficiency(const IntakeManifoldConfig& config,
                                         double rpm) const;
    
    // Calcula o numero de Reynolds (regime de fluxo)
    // Re < 2300: laminar, 2300-4000: transicao, >4000: turbulento
    double CalculateReynoldsNumber(double velocity, double diameter) const;
    
    // ========== CALCULOS DE ESCAPE ==========
    
    // Calcula o comprimento ideal do primario
    // Baseado em pulsos de pressao e efeito de scavenging
    // Formula: L = (EO_duration * rpm * c) / (720 * temperature_factor)
    // Referencia: Blair - Chapter 9, "Exhaust System Design"
    double CalculateIdealPrimaryLength(double exhaustDuration, double targetRPM) const;
    
    // Calcula o diametro ideal do primario
    // Baseado em velocidade de gases e fluxo de massa
    // Velocidade tipica: 80-120 m/s
    double CalculateIdealPrimaryDiameter(double displacement, double targetRPM,
                                         double targetVelocity = 100.0) const;
    
    // Calcula o comprimento do secundario (para 4-2-1)
    // Deve estar em fase com pulsos primarios
    double CalculateIdealSecondaryLength(double primaryLength, double exhaustDuration) const;
    
    // Calcula o diametro do coletor final
    // Baseado em area combinada dos primarios
    double CalculateCollectorDiameter(double primaryDiameter, int numberOfPrimaries) const;
    
    // Calcula a analise de pulsos de pressao
    PulseAnalysis CalculatePulseAnalysis(const ExhaustManifoldConfig& config,
                                         double exhaustDuration, double rpm) const;
    
    // Calcula a eficiencia de scavenging (limpeza do cilindro)
    // Considera timing, comprimento e diametro
    // Referencia: Heywood - "IC Engine Fundamentals", Chapter 6
    double CalculateScavengingEfficiency(const ExhaustManifoldConfig& config,
                                         double exhaustDuration, double overlap) const;
    
    // Calcula backpressure (contrapressao) estimada
    // Baseado em diametro, comprimento e fluxo
    double CalculateBackpressure(const ExhaustManifoldConfig& config, 
                                 double rpm, double displacement) const;
    
    // Compara 4-1 vs 4-2-1
    // 4-1: Melhor alta rpm, mais simples
    // 4-2-1: Melhor torque medio, melhor scavenging
    std::wstring Compare4Into1vs4Into2Into1(double targetRPM) const;
    
    // ========== CALCULOS DE TURBO/SUPERCHARGER ==========
    
    // Calcula pressao de boost ideal
    // Baseado em CR estatica e octanagem
    double CalculateMaxBoostPressure(double staticCR, double fuelOctane = 95.0) const;
    
    // Calcula CR efetiva com boost
    // CR_eff = CR_static * (1 + boost/14.7)
    double CalculateEffectiveCompressionWithBoost(double staticCR, double boostPSI) const;
    
    // Calcula tamanho ideal do intercooler
    // Volume baseado em cilindrada e pressao de boost
    double CalculateIntercoolerVolume(double displacement, double boostPSI) const;
    
    // ========== ANALISE DE FLUXO ==========
    
    // Calcula coeficiente de descarga do sistema
    // Cd varia com geometria e velocidade
    double CalculateDischargeCoefficient(double diameter, double length, 
                                         double velocity) const;
    
    // Calcula perda de carga (pressure drop)
    // Devido a friccao e turbulencia
    double CalculatePressureDrop(double length, double diameter, 
                                 double velocity, double density) const;
    
    // Calcula o fator de friccao (Darcy-Weisbach)
    double CalculateFrictionFactor(double reynoldsNumber, double roughness = 0.0015) const;
    
    // ========== OTIMIZACAO ==========
    
    // Sugere configuracao de admissao ideal para RPM alvo
    IntakeManifoldConfig SuggestIntakeConfig(double targetRPM) const;
    
    // Sugere configuracao de escape ideal para RPM alvo
    ExhaustManifoldConfig SuggestExhaustConfig(double targetRPM, 
                                                double exhaustDuration) const;
    
    // ========== VALIDACAO E RECOMENDACOES ==========
    
    bool IsValidIntakeConfig(const IntakeManifoldConfig& config) const;
    bool IsValidExhaustConfig(const ExhaustManifoldConfig& config) const;
    
    std::wstring GetIntakeAdvice(const IntakeManifoldConfig& config, double rpm) const;
    std::wstring GetExhaustAdvice(const ExhaustManifoldConfig& config, double rpm) const;
    
    // Analisa se o sistema e adequado para aspirado ou forcado
    std::wstring AnalyzeForForcedInduction(const IntakeManifoldConfig& intake,
                                           double boostPressure) const;
};
