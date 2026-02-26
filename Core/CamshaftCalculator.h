#pragma once
// ============================================================================
// CamshaftCalculator.h - Cálculos de Comando de Válvulas
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Referências:
// - Blair - "Design and Simulation of Four-Stroke Engines", Chapter 7
// - SAE Paper 2005-01-1688 - Camshaft Design and LSA
// - A. Graham Bell - "Four-Stroke Performance Tuning"
// - John Baechtel - "Competition Engine Building"
// ============================================================================
#include "EngineCore.h"
#include <cmath>
#include <string>

// Estrutura para especificações do comando de válvulas
struct CamshaftSpec {
    double intakeDuration;          // Duração admissão @ 0.050" (graus)
    double exhaustDuration;         // Duração escape @ 0.050" (graus)
    double intakeLift;              // Levantamento admissão (mm)
    double exhaustLift;             // Levantamento escape (mm)
    double lsa;                     // Lobe Separation Angle (graus)
    double advanceRetard;           // Avanço/Retardo instalação (graus)
    double rockerRatio;             // Relação do balanceiro
};

// Estrutura para eventos do comando (valve timing)
struct ValveEvents {
    double intakeOpen;              // Abertura admissão (graus BTDC)
    double intakeClose;             // Fechamento admissão (graus ABDC)
    double exhaustOpen;             // Abertura escape (graus BBDC)
    double exhaustClose;            // Fechamento escape (graus ATDC)
    double overlap;                 // Overlap (graus)
    double intakeCenterline;        // Centerline admissão (graus ATDC)
    double exhaustCenterline;       // Centerline escape (graus BTDC)
};

// Estrutura para análise de área do came
struct CamAreaAnalysis {
    double intakeArea;              // Área sob a curva - admissão
    double exhaustArea;             // Área sob a curva - escape
    double intakeVelocity;          // Velocidade média de abertura (mm/grau)
    double exhaustVelocity;         // Velocidade média de abertura (mm/grau)
    double intakeAcceleration;      // Aceleração média (mm/grau²)
    double exhaustAcceleration;     // Aceleração média (mm/grau²)
};

// Classe para cálculos de comando de válvulas
// Referências principais:
// - "Design and Simulation of Four-Stroke Engines" - Gordon P. Blair
// - "Camshaft Design and Lobe Separation Angle" - SAE Paper 2005-01-1688
// - "Four-Stroke Performance Tuning" - A. Graham Bell
// - "Competition Engine Building" - John Baechtel
class CamshaftCalculator {
private:
    const EngineCore* engine;
    
    // Calcula a área sob a curva do came (aproximação)
    double CalculateLobeLiftArea(double duration, double maxLift) const;
    
public:
    CamshaftCalculator(const EngineCore* eng);
    ~CamshaftCalculator();
    
    // ========== CÁLCULOS DE EVENTOS DE VÁLVULAS ==========
    
    // Calcula todos os eventos de abertura/fechamento das válvulas
    // Baseado em LSA, duração e avanço/retardo
    // Referência: Blair - Chapter 7, "Valve Timing"
    ValveEvents CalculateValveEvents(const CamshaftSpec& spec) const;
    
    // Calcula o overlap (graus em que ambas válvulas estão abertas)
    // Overlap = (Adm. abre BTDC) + (Esc. fecha ATDC)
    double CalculateOverlap(const ValveEvents& events) const;
    
    // Calcula a centerline da admissão
    // ICL = LSA + avanço/retardo
    double CalculateIntakeCenterline(double lsa, double advanceRetard) const;
    
    // Calcula a centerline do escape
    // ECL = LSA - avanço/retardo
    double CalculateExhaustCenterline(double lsa, double advanceRetard) const;
    
    // ========== ANÁLISE DE DURAÇÃO ==========
    
    // Calcula a duração efetiva (advertised duration)
    // Conversão de duração @ 0.050" para @ 0.004" (advertised)
    // Aproximação: Adv = Dur@0.050 + 50° a 70°
    double CalculateAdvertisedDuration(double durationAt050, double liftRate = 60.0) const;
    
    // Calcula a duração em graus de virabrequim @ lift específico
    double CalculateDurationAtLift(double maxLift, double targetLift, 
                                   double baseDuration) const;
    
    // Calcula a relação duração/levantamento
    // Indica agressividade do perfil
    // Valores baixos = perfil agressivo (racing)
    // Valores altos = perfil suave (economia)
    double CalculateDurationToLiftRatio(double duration, double lift) const;
    
    // ========== ANÁLISE DE LEVANTAMENTO ==========
    
    // Calcula o lift do came (no came, antes do rocker)
    double CalculateCamLift(double valveLift, double rockerRatio) const;
    
    // Calcula a taxa de levantamento (lift rate)
    // Taxa = Lift / (Duração/2)
    // Indica velocidade de abertura
    double CalculateLiftRate(double lift, double duration) const;
    
    // Calcula o lift em função do ângulo do virabrequim
    // Usando modelo senoidal simplificado
    double CalculateLiftAtAngle(double angle, double maxLift, double duration, 
                                double openingPoint) const;
    
    // ========== ANÁLISE DE ÁREA E FLUXO ==========
    
    // Calcula a análise completa de área do came
    // Área representa o fluxo total possível
    // Referência: Blair - "Volumetric Efficiency Analysis"
    CamAreaAnalysis CalculateCamAreaAnalysis(const CamshaftSpec& spec) const;
    
    // Calcula o coeficiente de velocidade média
    // Vmédia = (lift × rpm) / duração
    // Importante para estresse do trem de válvulas
    double CalculateMeanVelocity(double lift, double duration, double rpm) const;
    
    // Calcula a aceleração máxima do came
    // Para perfil senoidal: a = (2*pi/T)^2 * L
    double CalculateMaxCamAcceleration(double lift, double duration, double rpm) const;
    
    // ========== ANÁLISE DE FAIXA DE RPM ==========
    
    // Calcula o RPM ótimo baseado na duração
    // RPM_peak ~ (duracao - 100) × fator
    // Referência: Empirical data from Blair
    double CalculatePeakRPMFromDuration(double duration, EngineType type) const;
    
    // Calcula a faixa de potência útil
    struct PowerBand {
        double minRPM;
        double maxRPM;
        double peakRPM;
    };
    PowerBand CalculatePowerBand(double duration) const;
    
    // ========== RECOMENDAÇÕES DE LSA ==========
    
    // LSA afeta overlap, torque e potência de pico
    // LSA pequeno (106-110°): + overlap, + potência alta rpm, - torque baixo
    // LSA médio (110-114°): Balanceado
    // LSA grande (114-118°): - overlap, + torque baixo rpm, + economia
    std::wstring GetLSAAdvice(double lsa) const;
    
    // ========== RECOMENDAÇÕES DE OVERLAP ==========
    
    // Overlap afeta marcha lenta e resposta
    std::wstring GetOverlapAdvice(double overlap) const;
    
    // ========== COMPATIBILIDADE E VALIDAÇÃO ==========
    
    // Verifica se o comando é compatível com o motor
    bool IsCompatibleWithEngine(const CamshaftSpec& spec) const;
    
    // Verifica se a taxa de compressão é adequada para o comando
    // Comandos longos precisam de menos compressão
    double RecommendedCompressionRatio(double duration) const;
    
    // Verifica folga pistão-válvula necessária
    // Baseada no lift e tempo de abertura
    double CalculateMinimumPistonToValveClearance(const CamshaftSpec& spec) const;
    
    // Validação
    bool IsValidCamshaftSpec(const CamshaftSpec& spec) const;
    
    // Recomendação geral
    std::wstring GetCamshaftRecommendation(const CamshaftSpec& spec) const;
    
    // ========== COMPARAÇÃO DE PERFIS ==========
    
    // Compara dois comandos e retorna análise
    std::wstring CompareCamshafts(const CamshaftSpec& cam1, const CamshaftSpec& cam2) const;
    
    // Calcula o "índice de agressividade" do comando
    // Baseado em duração, lift e LSA
    // 0-100: 0 = economia, 100 = corrida extrema
    double CalculateAggressivenessIndex(const CamshaftSpec& spec) const;
};
