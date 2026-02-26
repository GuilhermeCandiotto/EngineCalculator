// ============================================================================
// TurboCalculator.cpp - Turbocharger & Supercharger Calculations
// Engine Calculator v1.2.0 - Fevereiro 2026
//
// SCIENTIFIC FOUNDATION:
// All formulas from Heywood Ch.6, Garrett Tech publications, BorgWarner
// matching guides, and Corky Bell "Maximum Boost".
//
// MANUFACTURER DATA:
// - Garrett by Honeywell: GT/GTX series compressor maps & A/R guidelines
// - BorgWarner: EFR series specifications & matching procedures
// - Precision Turbo & Engine: PT series sizing charts
// - Eaton: TVS/R-series supercharger displacement data
// - Whipple: Twin-screw efficiency curves
// ============================================================================
#include "TurboCalculator.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace TurboConstants;

TurboCalculator::TurboCalculator(const EngineCore* eng)
    : engine(eng) {
}

TurboCalculator::~TurboCalculator() {
}

// ============================================================================
// REQUIRED AIRFLOW
// Ref: Heywood eq. 6.2, Garrett "Turbo Tech 103"
//
// Method 1 (from displacement):
//   ma = (Vd × N × ηv × ρ_intake) / (nR × 60)
//   where ρ_intake = P_intake / (R × T_intake) [boosted density]
//
// Method 2 (from power target, Garrett simplified):
//   For gasoline: HP ≈ airflow_lb/min × 10.42 (at λ=0.85, ηth=0.30)
//   More precise: ma = HP × BSFC / (AFR × 60)
//     where BSFC ≈ 0.50 lb/hp/hr (turbo gasoline)
//           AFR ≈ 12.5:1 (power enrichment)
//
// We use BOTH methods and take the larger value for safety margin.
// ============================================================================
double TurboCalculator::CalculateRequiredAirflow(const TurboSizingInput& input) const {
    // Method 1: From displacement at boosted conditions
    double dispM3 = input.displacementCC * 1e-6;  // cc -> m³
    double rpmRPS = input.maxRPM / 60.0;           // RPM -> rev/s
    double nR = 2.0;  // 4-stroke = 2 revolutions per power stroke

    // Intake manifold absolute pressure (boosted)
    double P_intake = input.ambientPressureKPA + (input.targetBoostPSI * KPA_PER_PSI);
    double T_intake = CelsiusToKelvin(input.ambientTempC);

    // Boosted air density: ρ = P / (R × T)
    double rho_intake = (P_intake * 1000.0) / (R_AIR * T_intake);  // kPa->Pa

    double ma_disp = (dispM3 * rpmRPS * input.volumetricEfficiency * rho_intake) / nR;

    // Method 2: From target power (Garrett method)
    // HP = (airflow_lb/min) / 10.42 × (AFR correction)
    // More precise: ma = HP × BSFC / (AFR)
    double bsfc_kgs = 0.50 * 0.000126;  // 0.50 lb/hp/hr -> kg/s per HP
    double afr = 12.5;                    // Power enrichment AFR
    double ma_power = (input.targetHP * bsfc_kgs * (afr + 1.0)) / afr;

    // Use the larger of both for safety
    return std::max(ma_disp, ma_power);
}

// ============================================================================
// PRESSURE RATIO
// Ref: Garrett Turbo Tech 103
//   PR = P2_total / P1_total
//   P2_total = P_atm + P_boost + ΔP_intercooler + ΔP_piping
//   P1_total = P_atm - ΔP_filter
// ============================================================================
double TurboCalculator::CalculatePressureRatio(const TurboSizingInput& input) const {
    double P_atm = input.ambientPressureKPA / KPA_PER_PSI;  // kPa -> PSI

    // Compressor outlet must overcome all downstream losses
    double P2 = P_atm + input.targetBoostPSI
              + input.intercoolerPressDrop
              + input.pipingPressDrop;

    // Compressor inlet sees filter restriction
    double P1 = P_atm - input.filterPressDrop;

    if (P1 <= 0) P1 = 0.1;  // Safety

    return P2 / P1;
}

// ============================================================================
// CORRECTED AIRFLOW
// Ref: SAE J1826, Garrett standard correction
//   W_corr = W_actual × √(T_inlet / T_ref) / (P_inlet / P_ref)
//   T_ref = 545°R (85°F = 302.6K) - Garrett convention
//   P_ref = 14.696 PSI = 101.325 kPa
// ============================================================================
double TurboCalculator::CalculateCorrectedAirflow(double massFlowLBM,
                                                   double inletTempK,
                                                   double inletPressureKPA) const {
    const double T_REF = 302.6;   // Garrett reference: 545°R = 302.6K
    const double P_REF = 101.325; // Standard atmosphere kPa

    double tempCorrection = std::sqrt(inletTempK / T_REF);
    double pressCorrection = inletPressureKPA / P_REF;

    if (pressCorrection <= 0) pressCorrection = 0.01;

    return massFlowLBM * tempCorrection / pressCorrection;
}

// ============================================================================
// COMPRESSOR OUTLET TEMPERATURE
// Ref: Heywood eq. 6.7 (isentropic compression with efficiency)
//   T2 = T1 × [1 + (PR^((γ-1)/γ) - 1) / ηc]
//
// This is THE fundamental equation for turbo charge air temperature.
// Without intercooling, this determines intake air density and knock tendency.
//
// Example: T1=25°C, PR=2.0, ηc=0.72
//   T2 = 298.15 × [1 + (2.0^0.2857 - 1) / 0.72]
//   T2 = 298.15 × [1 + 0.2195 / 0.72] = 298.15 × 1.3049 = 389K = 116°C
// ============================================================================
double TurboCalculator::CalculateCompressorOutletTemp(double inletTempK,
                                                       double pressureRatio,
                                                       double compressorEfficiency) const {
    if (compressorEfficiency <= 0) compressorEfficiency = 0.60;
    if (pressureRatio <= 1.0) return inletTempK;

    double gamma_ratio = (GAMMA_AIR - 1.0) / GAMMA_AIR;  // 0.2857
    double isentropicTempRise = std::pow(pressureRatio, gamma_ratio) - 1.0;

    return inletTempK * (1.0 + isentropicTempRise / compressorEfficiency);
}

// ============================================================================
// COMPRESSOR POWER
// Ref: Heywood eq. 6.8
//   Wc = ma × cp × T1 × (PR^((γ-1)/γ) - 1) / ηc
//   Result in Watts when ma in kg/s, cp in J/(kg·K)
// ============================================================================
double TurboCalculator::CalculateCompressorPower(double massFlowKGS,
                                                  double inletTempK,
                                                  double pressureRatio,
                                                  double compressorEfficiency) const {
    if (compressorEfficiency <= 0) compressorEfficiency = 0.60;
    if (pressureRatio <= 1.0) return 0;

    double gamma_ratio = (GAMMA_AIR - 1.0) / GAMMA_AIR;
    double isentropicWork = std::pow(pressureRatio, gamma_ratio) - 1.0;

    // Power in Watts
    double Wc = massFlowKGS * CP_AIR * inletTempK * isentropicWork / compressorEfficiency;
    return Wc / 1000.0;  // Return kW
}

// ============================================================================
// COMPRESSOR EFFICIENCY ESTIMATION
// Ref: Garrett compressor map empirical analysis
//
// Peak efficiency regions by frame size (Garrett published data):
//   GT2554R: peak 72% at 25 lb/min, PR 2.0
//   GT2860RS: peak 76% at 35 lb/min, PR 2.2
//   GTX3071R: peak 78% at 48 lb/min, PR 2.8
//   GTX3576R: peak 78% at 65 lb/min, PR 2.5
//   GTX4088R: peak 80% at 85 lb/min, PR 3.0
//
// Efficiency degrades ~3-5% per 0.5 PR away from peak
// Efficiency degrades ~2-3% per 10 lb/min away from peak flow
// ============================================================================
double TurboCalculator::EstimateCompressorEfficiency(double correctedFlow,
                                                      double pressureRatio) const {
    // Base efficiency depends on turbo quality/size
    double peakEff = 0.76;  // Modern turbo average

    // Better efficiency for larger, modern turbos
    if (correctedFlow > 60) peakEff = 0.78;
    if (correctedFlow > 80) peakEff = 0.80;

    // Efficiency penalty for extreme PR
    double prPenalty = 0;
    if (pressureRatio > 3.0) prPenalty = (pressureRatio - 3.0) * 0.04;
    if (pressureRatio < 1.5) prPenalty = (1.5 - pressureRatio) * 0.06;

    // Operating near surge or choke reduces efficiency
    // Approximate: best at 50-70% of flow range
    double effEstimate = peakEff - prPenalty;

    return std::clamp(effEstimate, 0.55, 0.82);
}

// ============================================================================
// INDUCER DIAMETER ESTIMATION
// Ref: BorgWarner matching guide, empirical correlation from
//      published compressor maps across GT/GTX/EFR product lines
//
// Empirical data points (inducer dia vs peak corrected flow):
//   GT2554R:  41mm inducer,  ~30 lb/min max
//   GT2860RS: 46mm inducer,  ~42 lb/min max
//   GTX3071R: 54mm inducer,  ~58 lb/min max
//   GTX3576R: 60mm inducer,  ~76 lb/min max
//   GTX3582R: 62mm inducer,  ~82 lb/min max
//   GTX4088R: 68mm inducer, ~105 lb/min max
//   GTX4294R: 72mm inducer, ~118 lb/min max
//   GTX5008R: 80mm inducer, ~142 lb/min max
//
// Correlation: D_inducer ≈ 22 + 0.52 × W_corrected^0.82
// ============================================================================
double TurboCalculator::EstimateInducerDiameter(double correctedFlowLBM) const {
    if (correctedFlowLBM <= 0) return 30.0;
    return 22.0 + 0.52 * std::pow(correctedFlowLBM, 0.82);
}

// ============================================================================
// TURBINE A/R SELECTION
// Ref: Garrett Turbo Tech 102 - "The effects of turbine A/R"
//
// A/R = cross-sectional Area / Radius to centroid of that area
// Controls exhaust gas velocity entering turbine wheel
//
// Guidelines (Garrett published):
//   Street/daily:    A/R = 0.48-0.63 (4-cyl), 0.63-0.82 (6-cyl), 0.82-1.01 (V8)
//   Sport/fast spool: A/R = 0.63-0.82 (4-cyl), 0.82-1.01 (6-cyl), 1.01-1.16 (V8)
//   Race/top end:    A/R = 0.82-1.06 (4-cyl), 1.01-1.21 (6-cyl), 1.16-1.41 (V8)
//
// More boost = larger A/R (reduces backpressure at high RPM)
// Faster spool = smaller A/R (more turbine energy at low RPM)
// ============================================================================
double TurboCalculator::SuggestTurbineAR(double displacementLiters,
                                          double targetBoostPSI,
                                          bool isRaceApplication) const {
    // Base A/R from displacement
    double baseAR = displacementLiters * 0.045;  // Street baseline

    if (isRaceApplication) {
        baseAR = displacementLiters * 0.075;
    }

    // Adjust for boost level (higher boost = larger A/R for less backpressure)
    if (targetBoostPSI > 20) baseAR *= 1.15;
    else if (targetBoostPSI > 12) baseAR *= 1.08;

    return std::clamp(baseAR, 0.35, 1.50);
}

// ============================================================================
// TURBINE WHEEL DIAMETER
// Ref: BorgWarner design guidelines
//   For matched turbo: turbine wheel ≈ 82-90% of compressor exducer
//   Exducer ≈ inducer × 1.35-1.45 (typical centrifugal compressor)
// ============================================================================
double TurboCalculator::EstimateTurbineWheelDiameter(double compressorExducerMM) const {
    return compressorExducerMM * 0.86;  // 86% of exducer
}

// ============================================================================
// INTERCOOLER OUTLET TEMPERATURE
// Ref: SAE 2003-01-0732, heat exchanger effectiveness method
//   T_out = T_hot - ε × (T_hot - T_cold)
//   ε = effectiveness (0.60-0.85 air/air, 0.80-0.95 water/air)
//
// Typical values (Garrett application data):
//   Bar & plate air/air IC: ε = 0.65-0.80
//   Tube & fin air/air IC:  ε = 0.60-0.75
//   Air-water (Chargecooler): ε = 0.80-0.95
// ============================================================================
double TurboCalculator::CalculateIntercoolerOutletTemp(double compressorOutTempC,
                                                        double ambientTempC,
                                                        double effectiveness) const {
    return compressorOutTempC - effectiveness * (compressorOutTempC - ambientTempC);
}

// ============================================================================
// HEAT REJECTION
// Ref: Basic thermodynamics, Q = ma × cp × ΔT
// ============================================================================
double TurboCalculator::CalculateHeatRejection(double massFlowKGS,
                                                double tempDropC) const {
    return massFlowKGS * CP_AIR * tempDropC / 1000.0;  // kW
}

// ============================================================================
// INTERCOOLER CORE VOLUME ESTIMATE
// Ref: Bell "Maximum Boost" Ch.8, Garrett application data
//   Rule of thumb: ~0.8-1.2L of core per 100hp of heat rejection
//   More precisely depends on face area, depth, and fin density
// ============================================================================
double TurboCalculator::EstimateICCoreVolume(double heatRejectionKW) const {
    // 1L per ~7-10 kW of heat rejection
    return heatRejectionKW / 8.0;
}

// ============================================================================
// WASTEGATE SIZING
// Ref: Garrett wastegate tech bulletin, Turbonetics application guide
//   WG must bypass enough exhaust to prevent overboosting at max RPM
//   Flow area ≈ 15-25% of turbine inlet area for street
//   Flow area ≈ 25-40% of turbine inlet area for high boost race
// ============================================================================
WastegateResult TurboCalculator::CalculateWastegate(double targetBoostPSI,
                                                     double maxExhaustFlowKGS,
                                                     double turbineEfficiency) const {
    WastegateResult result;

    // Exhaust flow that must bypass turbine at max RPM
    // Typically 20-40% of total exhaust needs bypass
    double bypassFraction = 0.25;
    if (targetBoostPSI > 20) bypassFraction = 0.35;
    if (targetBoostPSI > 30) bypassFraction = 0.45;

    double bypassFlow = maxExhaustFlowKGS * bypassFraction;

    // Required flow area (from orifice flow equation)
    // A = m_dot / (Cd × ρ × V)
    // Simplified: area_mm² ≈ bypassFlow_kgs × 1200 (empirical for exhaust gas)
    result.requiredFlowArea_mm2 = bypassFlow * 1200.0;
    result.recommendedDiameter_mm = 2.0 * std::sqrt(result.requiredFlowArea_mm2 / 3.14159);

    result.springPressurePSI = targetBoostPSI * 0.5;  // Spring set at ~50% of target

    // Internal wastegate typically handles up to ~38mm valve
    result.internalSufficient = (result.recommendedDiameter_mm <= 38.0);

    if (!result.internalSufficient) {
        result.recommendation = L"External wastegate recomendado (Tial 44mm ou TurboSmart 45mm)";
    } else if (result.recommendedDiameter_mm > 30) {
        result.recommendation = L"Internal WG no limite. Considere external para melhor controle.";
    } else {
        result.recommendation = L"Internal wastegate adequado para esta aplicacao.";
    }

    return result;
}

// ============================================================================
// SUPERCHARGER CALCULATION
// Ref: Eaton TVS application guide, Whipple twin-screw data
//
// Roots/Twin-Screw sizing:
//   SC_displacement = Engine_disp × PR / ηv_sc
//   Parasitic loss (Roots): 15-22% of boost power
//   Parasitic loss (Twin-Screw): 10-15% of boost power
//   Parasitic loss (Centrifugal): 5-8% of boost power
//
// Thermal efficiency:
//   Roots: 45-55% isentropic
//   Twin-Screw: 65-75% isentropic
//   Centrifugal: 70-80% isentropic
// ============================================================================
SuperchargerResult TurboCalculator::CalculateSupercharger(double displacementCC,
                                                           double targetBoostPSI,
                                                           double maxRPM,
                                                           double driveRatio,
                                                           SuperchargerType type) const {
    SuperchargerResult result;

    double PR = 1.0 + (targetBoostPSI / P_ATM_PSI);
    double dispCID = displacementCC / 16.387;  // cc to cubic inches

    double scEfficiency, parasiticFraction;

    switch (type) {
    case SuperchargerType::ROOTS:
        scEfficiency = 0.50;
        parasiticFraction = 0.18;
        result.typeRecommendation = L"Eaton TVS ou Magnuson (Roots positivo)";
        break;
    case SuperchargerType::TWIN_SCREW:
        scEfficiency = 0.70;
        parasiticFraction = 0.12;
        result.typeRecommendation = L"Whipple ou Kenne Bell (Twin-Screw)";
        break;
    case SuperchargerType::CENTRIFUGAL:
        scEfficiency = 0.75;
        parasiticFraction = 0.06;
        result.typeRecommendation = L"Procharger, Vortech ou Paxton (Centrifugo)";
        break;
    }

    // Required SC displacement per revolution
    result.requiredDisplacementCID = dispCID * PR / (driveRatio * 0.90);

    // Outlet temperature
    double T1 = CelsiusToKelvin(T_AMB_C);
    double gamma_ratio = (GAMMA_AIR - 1.0) / GAMMA_AIR;
    result.outletTempC = KelvinToCelsius(T1 * (1.0 + (std::pow(PR, gamma_ratio) - 1.0) / scEfficiency));

    // Boost power and parasitic loss
    double boostPowerHP = (targetBoostPSI / P_ATM_PSI) * displacementCC * maxRPM / (2.0 * 60.0 * 745.7);
    result.parasticLossHP = boostPowerHP * parasiticFraction;
    result.netHPGain = boostPowerHP - result.parasticLossHP;
    result.thermalEfficiency = scEfficiency;
    result.boostAtRPM = targetBoostPSI;  // PD: boost ≈ constant across RPM range

    // Warnings
    std::wostringstream warn;
    if (targetBoostPSI > 15 && type == SuperchargerType::ROOTS) {
        warn << L"[!] Roots acima de 15 PSI: heat soak significativo\n";
        warn << L"    Considere Twin-Screw para melhor eficiencia termica\n";
    }
    if (result.outletTempC > 120) {
        warn << L"[!] Temp de carga " << (int)result.outletTempC << L"C - Intercooler OBRIGATORIO\n";
    }
    result.warnings = warn.str();

    return result;
}

// ============================================================================
// GARRETT FRAME RECOMMENDATION
// Ref: Garrett by Honeywell product catalog 2024
//
// Frame sizing by corrected airflow range (lb/min):
//   GT2554R:  15-32 lb/min   (150-280 HP)
//   GT2860RS: 20-42 lb/min   (200-375 HP)
//   GTX2867R: 22-45 lb/min   (250-400 HP)
//   GT3071R:  25-52 lb/min   (300-475 HP)
//   GTX3071R: 28-58 lb/min   (325-525 HP)
//   GT3076R:  30-60 lb/min   (350-550 HP)
//   GTX3076R: 32-65 lb/min   (375-600 HP)
//   GT3582R:  40-78 lb/min   (450-700 HP)
//   GTX3576R: 38-76 lb/min   (425-675 HP)
//   GTX3582R: 42-82 lb/min   (475-750 HP)
//   GTX4088R: 50-105 lb/min  (550-950 HP)
//   GTX4294R: 60-118 lb/min  (650-1050 HP)
//   GTX4508R: 70-135 lb/min  (750-1200 HP)
//   GTX5008R: 85-142 lb/min  (900-1350 HP)
//   GTX5533R: 100-175 lb/min (1100-1600 HP)
// ============================================================================
std::wstring TurboCalculator::RecommendGarrettFrame(double correctedFlowLBM,
                                                     double pressureRatio) const {
    struct FrameData { double minFlow; double maxFlow; const wchar_t* name; };
    const FrameData frames[] = {
        {15, 32,  L"GT2554R (41mm inducer)"},
        {20, 42,  L"GT2860RS (46mm inducer)"},
        {22, 45,  L"GTX2867R Gen II (47mm inducer)"},
        {28, 58,  L"GTX3071R Gen II (54mm inducer)"},
        {32, 65,  L"GTX3076R Gen II (58mm inducer)"},
        {38, 76,  L"GTX3576R Gen II (60mm inducer)"},
        {42, 82,  L"GTX3582R Gen II (62mm inducer)"},
        {50, 105, L"GTX4088R (68mm inducer)"},
        {60, 118, L"GTX4294R (72mm inducer)"},
        {70, 135, L"GTX4508R (76mm inducer)"},
        {85, 142, L"GTX5008R (80mm inducer)"},
        {100, 175, L"GTX5533R (88mm inducer)"},
    };

    std::wstring best;
    for (const auto& f : frames) {
        if (correctedFlowLBM >= f.minFlow * 0.85 && correctedFlowLBM <= f.maxFlow) {
            if (!best.empty()) best += L" | ";
            best += f.name;
        }
    }

    if (best.empty()) {
        if (correctedFlowLBM < 15) return L"GT2554R ou menor (low flow)";
        return L"GTX5533R+ ou turbo industrial custom";
    }

    return best;
}

// ============================================================================
// BORGWARNER EFR RECOMMENDATION
// Ref: BorgWarner EFR Product Line Specifications 2024
//
//   EFR 6258: 27-47 lb/min (300-425 HP)
//   EFR 6758: 30-55 lb/min (350-500 HP)
//   EFR 7064: 35-65 lb/min (400-575 HP)
//   EFR 7163: 38-68 lb/min (425-625 HP)
//   EFR 7670: 42-80 lb/min (475-700 HP)
//   EFR 8374: 55-100 lb/min (600-900 HP)
//   EFR 9180: 65-115 lb/min (700-1050 HP)
//   EFR 9280: 70-125 lb/min (750-1100 HP)
// ============================================================================
std::wstring TurboCalculator::RecommendBorgWarnerFrame(double correctedFlowLBM,
                                                        double pressureRatio) const {
    struct FrameData { double minFlow; double maxFlow; const wchar_t* name; };
    const FrameData frames[] = {
        {27, 47,  L"EFR 6258 (62/58mm)"},
        {30, 55,  L"EFR 6758 (67/58mm)"},
        {35, 65,  L"EFR 7064 (70/64mm)"},
        {38, 68,  L"EFR 7163 (71/63mm)"},
        {42, 80,  L"EFR 7670 (76/70mm)"},
        {55, 100, L"EFR 8374 (83/74mm)"},
        {65, 115, L"EFR 9180 (91/80mm)"},
        {70, 125, L"EFR 9280 (92/80mm)"},
    };

    std::wstring best;
    for (const auto& f : frames) {
        if (correctedFlowLBM >= f.minFlow * 0.85 && correctedFlowLBM <= f.maxFlow) {
            if (!best.empty()) best += L" | ";
            best += f.name;
        }
    }

    if (best.empty()) {
        if (correctedFlowLBM < 27) return L"Abaixo da faixa EFR";
        return L"Acima da faixa EFR - consulte S300/S400 series";
    }
    return best;
}

// ============================================================================
// PRECISION TURBO RECOMMENDATION
// Ref: Precision Turbo & Engine product catalog 2024
//
//   PT5431:  20-40 lb/min  (200-350 HP)
//   PT5858:  28-55 lb/min  (300-500 HP)
//   PT5862:  30-58 lb/min  (325-525 HP)
//   PT6062:  35-65 lb/min  (375-575 HP)
//   PT6266:  38-72 lb/min  (425-650 HP)
//   PT6466:  42-78 lb/min  (475-700 HP)
//   PT6766:  48-85 lb/min  (525-775 HP)
//   PT6870:  55-95 lb/min  (600-850 HP)
//   PT7275:  65-110 lb/min (700-1000 HP)
//   PT7685:  75-130 lb/min (800-1200 HP)
//   PT8085: 85-145 lb/min (900-1350 HP)
//   PT88:   100-165 lb/min (1050-1500 HP)
// ============================================================================
std::wstring TurboCalculator::RecommendPrecisionFrame(double correctedFlowLBM,
                                                       double pressureRatio) const {
    struct FrameData { double minFlow; double maxFlow; const wchar_t* name; };
    const FrameData frames[] = {
        {20, 40,  L"PT5431 (54/31mm)"},
        {28, 55,  L"PT5858 (58/58mm)"},
        {35, 65,  L"PT6062 (60/62mm)"},
        {38, 72,  L"PT6266 (62/66mm)"},
        {42, 78,  L"PT6466 (64/66mm)"},
        {48, 85,  L"PT6766 (67/66mm)"},
        {55, 95,  L"PT6870 (68/70mm)"},
        {65, 110, L"PT7275 (72/75mm)"},
        {75, 130, L"PT7685 (76/85mm)"},
        {85, 145, L"PT8085 (80/85mm)"},
        {100, 165, L"PT88 (88mm)"},
    };

    std::wstring best;
    for (const auto& f : frames) {
        if (correctedFlowLBM >= f.minFlow * 0.85 && correctedFlowLBM <= f.maxFlow) {
            if (!best.empty()) best += L" | ";
            best += f.name;
        }
    }

    if (best.empty()) {
        if (correctedFlowLBM < 20) return L"Abaixo da faixa PT";
        return L"PT88+ ou turbo custom";
    }
    return best;
}

// ============================================================================
// BOOST THRESHOLD RPM
// Ref: Garrett application data, empirical
//   Smaller A/R and lighter turbo = lower threshold
//   Threshold ≈ base_rpm / (displacement_effect × cylinder_effect)
// ============================================================================
double TurboCalculator::EstimateBoostThresholdRPM(double turbineAR,
                                                    double displacementLiters,
                                                    double numCylinders) const {
    // Base threshold for A/R=0.63, 2.0L, 4-cyl
    double base = 2800.0;

    // A/R effect: larger A/R = higher threshold
    double arFactor = turbineAR / 0.63;

    // Displacement effect: more displacement = more exhaust energy = lower threshold
    double dispFactor = 2.0 / displacementLiters;

    // More cylinders = more frequent exhaust pulses = faster spool
    double cylFactor = 4.0 / numCylinders;

    double threshold = base * arFactor * std::sqrt(dispFactor) * std::pow(cylFactor, 0.3);

    return std::clamp(threshold, 1500.0, 6000.0);
}

// ============================================================================
// COMPLETE TURBO SIZING
// Combines all calculations into a single comprehensive result
// ============================================================================
TurboSizingResult TurboCalculator::CalculateTurboSizing(const TurboSizingInput& input) const {
    TurboSizingResult r;

    // 1. Airflow requirements
    r.requiredAirflowKGS = CalculateRequiredAirflow(input);

    // Per-turbo airflow if twin setup
    double perTurboFlowKGS = r.requiredAirflowKGS / input.numberOfTurbos;

    r.requiredAirflowLBM = perTurboFlowKGS * LB_PER_KG * 60.0;  // kg/s -> lb/min
    r.requiredAirflowCFM = r.requiredAirflowKGS / 1.225 * CFM_PER_M3S;  // At inlet density

    // 2. Pressure ratio
    r.pressureRatioTotal = CalculatePressureRatio(input);
    r.absoluteBoostKPA = input.ambientPressureKPA + (input.targetBoostPSI * TurboConstants::KPA_PER_PSI);
    r.boostPSIG = input.targetBoostPSI;

    // 3. Corrected airflow (for map lookup)
    double inletTempK = CelsiusToKelvin(input.ambientTempC);
    r.correctedAirflow = CalculateCorrectedAirflow(
        r.requiredAirflowLBM, inletTempK, input.ambientPressureKPA);

    // 4. Compressor efficiency estimate
    r.estimatedCompressorEff = EstimateCompressorEfficiency(r.correctedAirflow, r.pressureRatioTotal);
    r.estimatedTurbineEff = 0.75;  // Typical turbine efficiency

    // 5. Temperature
    double T2_K = CalculateCompressorOutletTemp(inletTempK, r.pressureRatioTotal, r.estimatedCompressorEff);
    r.compressorOutletTempC = KelvinToCelsius(T2_K);
    r.tempRiseAcrossCompressor = r.compressorOutletTempC - input.ambientTempC;

    // 6. Compressor power
    r.compressorPowerKW = CalculateCompressorPower(
        perTurboFlowKGS, inletTempK, r.pressureRatioTotal, r.estimatedCompressorEff);
    r.compressorPowerHP = KWtoHP(r.compressorPowerKW);

    // 7. Sizing
    r.inducer_mm = EstimateInducerDiameter(r.correctedAirflow);
    r.exducer_mm = r.inducer_mm * 1.40;  // Typical exducer/inducer ratio
    r.turbineWheelDia_mm = EstimateTurbineWheelDiameter(r.exducer_mm);

    // Size class
    if (r.inducer_mm < 45) r.sizeClass = L"Small (Street/Daily)";
    else if (r.inducer_mm < 58) r.sizeClass = L"Medium (Sport/Street-Race)";
    else if (r.inducer_mm < 72) r.sizeClass = L"Large (Race/High Performance)";
    else r.sizeClass = L"Very Large (Pro Race/Drag)";

    // 8. Manufacturer recommendations
    r.garrettFrame = RecommendGarrettFrame(r.correctedAirflow, r.pressureRatioTotal);
    r.borgWarnerFrame = RecommendBorgWarnerFrame(r.correctedAirflow, r.pressureRatioTotal);
    r.precisionFrame = RecommendPrecisionFrame(r.correctedAirflow, r.pressureRatioTotal);

    // 9. Intercooler
    double icEffectiveness = 0.70;
    double icOutTemp = CalculateIntercoolerOutletTemp(r.compressorOutletTempC, input.ambientTempC, icEffectiveness);
    double tempDrop = r.compressorOutletTempC - icOutTemp;
    r.heatRejectionKW = CalculateHeatRejection(perTurboFlowKGS, tempDrop);
    r.icCoreSizeEstimate = EstimateICCoreVolume(r.heatRejectionKW);

    // 10. Warnings & recommendations
    std::wostringstream warn, rec;

    if (r.pressureRatioTotal > 3.5) {
        warn << L"[!!] PR > 3.5 - Compressor pode operar fora da faixa de eficiencia\n";
        warn << L"     Considere compound turbo (serie LP+HP)\n";
    }
    if (r.compressorOutletTempC > 200) {
        warn << L"[!!] Temp pos-compressor " << (int)r.compressorOutletTempC << L"C - Intercooler OBRIGATORIO\n";
    }
    if (r.compressorOutletTempC > 100 && r.compressorOutletTempC <= 200) {
        warn << L"[!] Temp pos-compressor " << (int)r.compressorOutletTempC << L"C - Intercooler recomendado\n";
    }

    double dispLiters = input.displacementCC / 1000.0;
    double suggestedAR = SuggestTurbineAR(dispLiters, input.targetBoostPSI, input.targetBoostPSI > 18);

    rec << L"A/R turbina sugerido: " << std::fixed << std::setprecision(2) << suggestedAR << L"\n";

    double threshold = EstimateBoostThresholdRPM(suggestedAR, dispLiters, input.numCylinders);
    rec << L"Boost threshold estimado: ~" << (int)threshold << L" RPM\n";

    if (input.numberOfTurbos == 2) {
        rec << L"Twin turbo: cada turbo dimensionado para " << std::setprecision(1)
            << r.requiredAirflowLBM << L" lb/min\n";
    }

    if (input.targetBoostPSI > 15) {
        rec << L"Combustivel: Premium 95+ octanas OBRIGATORIO\n";
        rec << L"CR recomendada: " << std::setprecision(1) << (9.5 - (input.targetBoostPSI - 15) * 0.15) << L":1 ou menor\n";
    }

    r.warnings = warn.str();
    r.recommendations = rec.str();

    return r;
}
