#define _USE_MATH_DEFINES
#include "CamshaftCalculator.h"
#include <cmath>
#include <algorithm>
#include <sstream>

CamshaftCalculator::CamshaftCalculator(const EngineCore* eng) 
    : engine(eng) {
}

CamshaftCalculator::~CamshaftCalculator() {
}

// ========== CÁLCULOS DE EVENTOS DE VÁLVULAS ==========

// Calcula todos os eventos de abertura/fechamento
// Método: usa LSA como base e aplica duração e avanço
// Referência: Blair - "Design and Simulation of Four-Stroke Engines", Chapter 7
ValveEvents CamshaftCalculator::CalculateValveEvents(const CamshaftSpec& spec) const {
    ValveEvents events = { 0 };
    
    // Calcula centerlines
    events.intakeCenterline = CalculateIntakeCenterline(spec.lsa, spec.advanceRetard);
    events.exhaustCenterline = CalculateExhaustCenterline(spec.lsa, spec.advanceRetard);
    
    // Calcula eventos de admissão
    // IVO (Intake Valve Open) = ICL - (duração/2)
    events.intakeOpen = events.intakeCenterline - (spec.intakeDuration / 2.0);
    // IVC (Intake Valve Close) = ICL + (duração/2) - 180
    events.intakeClose = events.intakeCenterline + (spec.intakeDuration / 2.0) - 180.0;
    
    // Calcula eventos de escape
    // EVO (Exhaust Valve Open) = ECL + (duração/2) - 180
    events.exhaustOpen = events.exhaustCenterline + (spec.exhaustDuration / 2.0) - 180.0;
    // EVC (Exhaust Valve Close) = ECL - (duração/2)
    events.exhaustClose = events.exhaustCenterline - (spec.exhaustDuration / 2.0);
    
    // Ajusta para valores convencionais (BTDC/ATDC/BBDC/ABDC)
    // IVO: negativo = BTDC, positivo = ATDC
    // IVC: sempre ABDC
    // EVO: sempre BBDC  
    // EVC: negativo = BTDC, positivo = ATDC
    
    // Calcula overlap
    events.overlap = CalculateOverlap(events);
    
    return events;
}

double CamshaftCalculator::CalculateOverlap(const ValveEvents& events) const {
    // Overlap = IVO (graus BTDC) + EVC (graus ATDC)
    // Quando IVO é negativo (BTDC) e EVC é positivo (ATDC)
    double ivoAbs = std::abs(events.intakeOpen);
    double evcAbs = std::abs(events.exhaustClose);
    
    return ivoAbs + evcAbs;
}

double CamshaftCalculator::CalculateIntakeCenterline(double lsa, double advanceRetard) const {
    // ICL (graus ATDC) = LSA - advance
    // Convenção: advanceRetard > 0 = avançado (ICL menor, mais perto do TDC)
    // Referência: Blair Chapter 7, Bell "Performance Tuning" Chapter 6
    // Exemplo: LSA=110, advance=4 -> ICL=106° ATDC
    return lsa - advanceRetard;
}

double CamshaftCalculator::CalculateExhaustCenterline(double lsa, double advanceRetard) const {
    // ECL (graus BTDC) = LSA + advance
    // Exemplo: LSA=110, advance=4 -> ECL=114° BTDC
    return lsa + advanceRetard;
}

// ========== ANÁLISE DE DURAÇÃO ==========

// Converte duração @ 0.050" para advertised (@ 0.004" ou 0.006")
// Baseado em dados empíricos
double CamshaftCalculator::CalculateAdvertisedDuration(double durationAt050, double liftRate) const {
    // Aproximação: adiciona 50-70 graus dependendo da taxa de levantamento
    // Taxa alta (came agressivo) = menos graus adicionados
    // Taxa baixa (came suave) = mais graus adicionados
    
    double additionalDegrees = 60.0; // valor médio padrão
    
    if (liftRate > 0.08) additionalDegrees = 50.0;      // Muito agressivo
    else if (liftRate > 0.06) additionalDegrees = 55.0; // Agressivo
    else if (liftRate > 0.04) additionalDegrees = 60.0; // Médio
    else additionalDegrees = 70.0;                       // Suave
    
    return durationAt050 + additionalDegrees;
}

double CamshaftCalculator::CalculateDurationAtLift(double maxLift, double targetLift, 
                                                    double baseDuration) const {
    if (maxLift <= 0.0 || targetLift <= 0.0 || targetLift > maxLift) return 0.0;
    
    // Modelo senoidal: lift(theta) = maxLift × sin(theta*pi/duracao)
    // Resolver para theta quando lift = targetLift
    double ratio = targetLift / maxLift;
    double angleRatio = asin(ratio) / (M_PI / 2.0);
    
    return baseDuration * angleRatio;
}

double CamshaftCalculator::CalculateDurationToLiftRatio(double duration, double lift) const {
    if (lift <= 0.0) return 0.0;
    return duration / lift;
}

// ========== ANÁLISE DE LEVANTAMENTO ==========

double CamshaftCalculator::CalculateCamLift(double valveLift, double rockerRatio) const {
    if (rockerRatio <= 0.0) return 0.0;
    return valveLift / rockerRatio;
}

double CamshaftCalculator::CalculateLiftRate(double lift, double duration) const {
    if (duration <= 0.0) return 0.0;
    // Taxa = lift / (duração/2)
    return lift / (duration / 2.0);
}

// Calcula o lift em um ângulo específico do virabrequim
// Modelo senoidal simplificado
double CamshaftCalculator::CalculateLiftAtAngle(double angle, double maxLift, double duration, 
                                                 double openingPoint) const {
    if (angle < openingPoint || angle > (openingPoint + duration)) return 0.0;
    
    // Ângulo relativo desde a abertura
    double relativeAngle = angle - openingPoint;
    
    // Perfil senoidal: L(theta) = Lmax × sin(pi * theta / duracao)
    double lift = maxLift * sin(M_PI * relativeAngle / duration);
    
    return lift;
}

// ========== ANÁLISE DE ÁREA E FLUXO ==========

// Calcula a área sob a curva do lobe (aproximação)
// Representa o fluxo total potencial
double CamshaftCalculator::CalculateLobeLiftArea(double duration, double maxLift) const {
    if (duration <= 0.0 || maxLift <= 0.0) return 0.0;
    
    // Para perfil senoidal: Area = (2/pi) × Lmax × duração
    // Área em mm·graus
    return (2.0 / M_PI) * maxLift * duration;
}

CamAreaAnalysis CamshaftCalculator::CalculateCamAreaAnalysis(const CamshaftSpec& spec) const {
    CamAreaAnalysis analysis = { 0 };
    
    // Calcula lift no came (antes do rocker)
    double intakeCamLift = CalculateCamLift(spec.intakeLift, spec.rockerRatio);
    double exhaustCamLift = CalculateCamLift(spec.exhaustLift, spec.rockerRatio);
    
    // Área sob a curva
    analysis.intakeArea = CalculateLobeLiftArea(spec.intakeDuration, intakeCamLift);
    analysis.exhaustArea = CalculateLobeLiftArea(spec.exhaustDuration, exhaustCamLift);
    
    // Velocidade média de abertura (mm/grau)
    analysis.intakeVelocity = CalculateLiftRate(intakeCamLift, spec.intakeDuration);
    analysis.exhaustVelocity = CalculateLiftRate(exhaustCamLift, spec.exhaustDuration);
    
    // Aceleração média aproximada (mm/grau²)
    // a ~ V / (duracao/4)
    analysis.intakeAcceleration = analysis.intakeVelocity / (spec.intakeDuration / 4.0);
    analysis.exhaustAcceleration = analysis.exhaustVelocity / (spec.exhaustDuration / 4.0);
    
    return analysis;
}

double CamshaftCalculator::CalculateMeanVelocity(double lift, double duration, double rpm) const {
    if (duration <= 0.0 || rpm <= 0.0) return 0.0;
    
    // Converte duração de graus para tempo (segundos)
    // Para 4T: came gira a rpm/2
    double camRPM = rpm / 2.0;
    double timePerDegree = 60.0 / (camRPM * 360.0); // segundos por grau
    double totalTime = duration * timePerDegree;
    
    // Velocidade média (mm/s)
    return lift / totalTime;
}

double CamshaftCalculator::CalculateMaxCamAcceleration(double lift, double duration, double rpm) const {
    if (duration <= 0.0 || rpm <= 0.0) return 0.0;
    
    // Para perfil senoidal: a_max = (pi/duracao)^2 × lift × (rpm/60)²
    double camRPM = rpm / 2.0; // 4T
    double omega = (2.0 * M_PI * camRPM) / 60.0; // rad/s
    double durationRad = duration * M_PI / 180.0;
    
    // Aceleração máxima (mm/s²)
    double acceleration = (M_PI / durationRad) * (M_PI / durationRad) * lift * omega * omega;
    
    return acceleration;
}

// ========== ANÁLISE DE FAIXA DE RPM ==========

// Calcula o RPM de pico baseado na duração
// Fórmula empírica baseada em dados de fabricantes (Comp Cams, Crane, Isky)
// Referência: Bell - "Four-Stroke Performance Tuning", Chapter 6
// Correlação: RPM_peak ~ duration_at_050 * 25 (4T)
double CamshaftCalculator::CalculatePeakRPMFromDuration(double duration, EngineType type) const {
    // =========================================================================
    // Estimativa de RPM de pico de potencia baseada no comando e geometria
    //
    // FUNDAMENTACAO:
    // O RPM de pico de potencia depende de dois fatores independentes:
    //
    // 1. COMANDO (duracao): Define QUANDO a VE maxima ocorre.
    //    Maior duracao = pico em RPM mais alto.
    //    Ref: A. Graham Bell, "Four-Stroke Performance Tuning", 3rd ed., Ch.5
    //    Ref: David Vizard, "How to Build Horsepower", Vol.2
    //
    // 2. STROKE (geometria): Define o LIMITE FISICO de RPM via MPS.
    //    Mean Piston Speed = 2 * stroke * RPM / 60
    //    Ref: Heywood, "IC Engine Fundamentals", eq. 2.11
    //    MPS maximo tipico:
    //      Street:      18-22 m/s
    //      Performance: 22-25 m/s
    //      Race:        25-27 m/s  (NASCAR, Le Mans)
    //      F1/Moto GP:  26-28 m/s  (materiais especiais)
    //
    // VALIDACAO com dados reais:
    //   Stock Civic (200°, 89mm):   ~5500  RPM (MPS=16.3)
    //   K20A Type R (250°, 86mm):   ~8000  RPM (MPS=22.9)
    //   S2000 F20C  (248°, 84mm):   ~8300  RPM (MPS=23.2)
    //   Ferrari 458 (260°, 81mm):   ~9000  RPM (MPS=24.3)
    //   BMW S1000RR (260°, 49.7mm): ~14200 RPM (MPS=23.5)
    //   F1 V10      (272°, 39.7mm): ~18500 RPM (MPS=24.5)
    // =========================================================================

    if (type == EngineType::TWO_STROKE) {
        return std::clamp(duration * 32.0, 2000.0, 22000.0);
    }

    // Passo 1: MPS de projeto baseado na agressividade do comando
    // Comandos mais agressivos (maior duracao) implicam trem de valvulas
    // capaz de suportar MPS mais alto (molas mais fortes, materiais melhores)
    // Interpolacao linear de dados empiricos (Bell, Vizard):
    //   200° -> 16.5 m/s (stock)
    //   240° -> 20.5 m/s (street performance)
    //   260° -> 23.0 m/s (race)
    //   280° -> 25.0 m/s (pro race)
    //   310° -> 27.0 m/s (F1/unlimited)
    double mpsDesign;
    if (duration <= 200.0) {
        mpsDesign = 16.5;
    } else if (duration <= 310.0) {
        // Interpolacao linear: 200->16.5, 310->27.0
        mpsDesign = 16.5 + (duration - 200.0) * (27.0 - 16.5) / (310.0 - 200.0);
    } else {
        mpsDesign = 27.0;
    }

    // Passo 2: Calcula RPM de pico via MPS
    // MPS = 2 * stroke(m) * RPM / 60
    // RPM = MPS * 30 / stroke(m) = MPS * 30000 / stroke(mm)
    double stroke_mm = 86.0; // default se nao tiver dados do motor
    if (engine && engine->GetStroke() > 0) {
        stroke_mm = engine->GetStroke();
    }

    double peakRPM = (mpsDesign * 30000.0) / stroke_mm;

    return std::clamp(peakRPM, 2000.0, 22000.0);
}

CamshaftCalculator::PowerBand CamshaftCalculator::CalculatePowerBand(double duration) const {
    PowerBand band;
    
    EngineType type = engine ? engine->GetEngineType() : EngineType::FOUR_STROKE;
    band.peakRPM = CalculatePeakRPMFromDuration(duration, type);
    
    // Faixa útil: ±20% do pico
    band.minRPM = band.peakRPM * 0.70;
    band.maxRPM = band.peakRPM * 1.15;
    
    return band;
}

// ========== RECOMENDAÇÕES ==========

std::wstring CamshaftCalculator::GetLSAAdvice(double lsa) const {
    if (lsa < 106.0) return L"[!] LSA muito pequeno - Overlap extremo, marcha lenta instável";
    else if (lsa < 110.0) return L"[!] LSA pequeno - Alta rpm, competição, requer convergência";
    else if (lsa < 114.0) return L"[OK] LSA medio - Balanceado, melhor escolha all-around";
    else if (lsa < 118.0) return L"[OK] LSA grande - Torque baixo, boa marcha lenta, economia";
    else return L"[!] LSA muito grande - Perda de potência de pico";
}

std::wstring CamshaftCalculator::GetOverlapAdvice(double overlap) const {
    if (overlap < 10.0) return L"[OK] Overlap minimo - Ótimo para turbo/SC, boa marcha lenta";
    else if (overlap < 30.0) return L"[OK] Overlap baixo - Bom para uso diário, resposta rápida";
    else if (overlap < 50.0) return L"[OK] Overlap moderado - Performance balanceada";
    else if (overlap < 70.0) return L"[!] Overlap alto - Marcha lenta irregular, potência alta rpm";
    else return L"[!!] Overlap muito alto - Apenas para competição, requer setup especial";
}

// ========== COMPATIBILIDADE E VALIDAÇÃO ==========

bool CamshaftCalculator::IsCompatibleWithEngine(const CamshaftSpec& spec) const {
    if (!engine || !engine->IsValid()) return false;
    
    // Verifica se o lift não é excessivo para o bore
    double maxLift = std::max(spec.intakeLift, spec.exhaustLift);
    double bore = engine->GetBore();
    
    // Lift não deve exceder ~35% do bore (regra empírica)
    if (maxLift > bore * 0.35) return false;
    
    return true;
}

double CamshaftCalculator::RecommendedCompressionRatio(double duration) const {
    // Comandos longos (alta duração) precisam de MENOS compressão
    // para evitar detonação devido ao overlap
    
    if (duration < 200.0) return 11.0; // Comando curto - alta CR possível
    else if (duration < 220.0) return 10.5;
    else if (duration < 240.0) return 10.0;
    else if (duration < 260.0) return 9.5;
    else if (duration < 280.0) return 9.0;
    else return 8.5; // Comando muito longo - CR baixa
}

double CamshaftCalculator::CalculateMinimumPistonToValveClearance(const CamshaftSpec& spec) const {
    // Folga mínima recomendada baseada no lift
    // Margem de segurança: 1.0 mm base + 10% do lift
    
    double maxLift = std::max(spec.intakeLift, spec.exhaustLift);
    double clearance = 1.0 + (maxLift * 0.10);
    
    return clearance;
}

bool CamshaftCalculator::IsValidCamshaftSpec(const CamshaftSpec& spec) const {
    return spec.intakeDuration > 0.0 && 
           spec.exhaustDuration > 0.0 &&
           spec.intakeLift > 0.0 &&
           spec.exhaustLift > 0.0 &&
           spec.lsa > 90.0 && spec.lsa < 130.0 &&
           spec.rockerRatio > 0.0;
}

std::wstring CamshaftCalculator::GetCamshaftRecommendation(const CamshaftSpec& spec) const {
    std::wostringstream result;
    
    double avgDuration = (spec.intakeDuration + spec.exhaustDuration) / 2.0;
    ValveEvents events = CalculateValveEvents(spec);
    
    result << L"ANÁLISE DO COMANDO:\r\n";
    
    // Classificação por duração
    if (avgDuration < 210.0) result << L"[OK] Comando SUAVE - Ótimo para uso diário, economia\r\n";
    else if (avgDuration < 230.0) result << L"[OK] Comando STREET - Balanceado, boa dirigibilidade\r\n";
    else if (avgDuration < 250.0) result << L"[OK] Comando PERFORMANCE - Street/strip, resposta rápida\r\n";
    else if (avgDuration < 270.0) result << L"[!] Comando RACE - Competição, marcha lenta irregular\r\n";
    else result << L"[!!] Comando EXTREMO - Apenas pista, requer setup completo\r\n";
    
    // RPM recomendado
    PowerBand band = CalculatePowerBand(avgDuration);
    result << L"Faixa útil: " << static_cast<int>(band.minRPM) << L"-" 
           << static_cast<int>(band.maxRPM) << L" RPM\r\n";
    result << L"Pico estimado: " << static_cast<int>(band.peakRPM) << L" RPM\r\n";
    
    // Compressão recomendada
    double recCR = RecommendedCompressionRatio(avgDuration);
    result << L"CR recomendada: " << recCR << L":1 ou menos\r\n";
    
    return result.str();
}

double CamshaftCalculator::CalculateAggressivenessIndex(const CamshaftSpec& spec) const {
    // Índice de 0-100 baseado em múltiplos fatores
    
    double durationScore = 0.0;
    double liftScore = 0.0;
    double lsaScore = 0.0;
    
    double avgDuration = (spec.intakeDuration + spec.exhaustDuration) / 2.0;
    double avgLift = (spec.intakeLift + spec.exhaustLift) / 2.0;
    
    // Score de duração (200° = 0, 280° = 100)
    durationScore = ((avgDuration - 200.0) / 80.0) * 100.0;
    durationScore = std::max(0.0, std::min(100.0, durationScore));
    
    // Score de lift (8mm = 0, 14mm = 100)
    liftScore = ((avgLift - 8.0) / 6.0) * 100.0;
    liftScore = std::max(0.0, std::min(100.0, liftScore));
    
    // Score de LSA (114° = 0, 106° = 100)
    lsaScore = ((114.0 - spec.lsa) / 8.0) * 100.0;
    lsaScore = std::max(0.0, std::min(100.0, lsaScore));
    
    // Média ponderada (duração tem mais peso)
    double index = (durationScore * 0.5) + (liftScore * 0.3) + (lsaScore * 0.2);
    
    return index;
}

std::wstring CamshaftCalculator::CompareCamshafts(const CamshaftSpec& cam1, const CamshaftSpec& cam2) const {
    std::wostringstream result;
    
    double dur1 = (cam1.intakeDuration + cam1.exhaustDuration) / 2.0;
    double dur2 = (cam2.intakeDuration + cam2.exhaustDuration) / 2.0;
    
    double lift1 = (cam1.intakeLift + cam1.exhaustLift) / 2.0;
    double lift2 = (cam2.intakeLift + cam2.exhaustLift) / 2.0;
    
    ValveEvents events1 = CalculateValveEvents(cam1);
    ValveEvents events2 = CalculateValveEvents(cam2);
    
    result << L"COMPARAÇÃO:\r\n";
    result << L"Comando 1 vs Comando 2\r\n\r\n";
    
    result << L"Duração: " << dur1 << L"° vs " << dur2 << L"° ";
    if (dur1 > dur2) result << L"(Cmd1 mais longo - pico mais alto)\r\n";
    else result << L"(Cmd2 mais longo - pico mais alto)\r\n";
    
    result << L"Lift: " << lift1 << L"mm vs " << lift2 << L"mm ";
    if (lift1 > lift2) result << L"(Cmd1 mais fluxo)\r\n";
    else result << L"(Cmd2 mais fluxo)\r\n";
    
    result << L"Overlap: " << events1.overlap << L"° vs " << events2.overlap << L"° ";
    if (events1.overlap > events2.overlap) result << L"(Cmd1 mais agressivo)\r\n";
    else result << L"(Cmd2 mais agressivo)\r\n";
    
    return result.str();
}
