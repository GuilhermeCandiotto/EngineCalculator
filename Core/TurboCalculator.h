#pragma once
// ============================================================================
// TurboCalculator.h - Turbocharger & Supercharger Sizing Calculator
// Engine Calculator v1.2.0 - Fevereiro 2026
//
// REFERENCES:
// - Heywood, J.B. - "ICE Fundamentals" Ch.6 (Supercharging)
// - Garrett by Honeywell - "Turbo Tech 101-103" Application Guidelines
// - BorgWarner Turbo Systems - "Turbocharger Matching Guide"
// - Precision Turbo & Engine - "Compressor Map Reading Guide"
// - SAE 2006-01-0038 - "Turbocharger Matching for SI Engines"
// - SAE 920044 - Watson & Janota - "Turbocharging the ICE"
// - Corky Bell - "Maximum Boost" (Robert Bentley Publishers)
// - SAE 2003-01-0732 - "Intercooler Effectiveness"
// - SAE J1826 - "Turbocharger Gas Stand Test Code"
// ============================================================================
#include "EngineCore.h"
#include <cmath>
#include <string>
#include <vector>

// ============================================================================
// CONSTANTS
// Ref: Heywood Table 6.1, standard atmosphere, ideal gas relations
// ============================================================================
namespace TurboConstants {
    constexpr double P_ATM_KPA = 101.325;       // Standard atmospheric pressure (kPa)
    constexpr double P_ATM_PSI = 14.696;        // Standard atmospheric pressure (PSI)
    constexpr double T_AMB_K = 298.15;          // Standard ambient temp (25C = 298.15K)
    constexpr double T_AMB_C = 25.0;
    constexpr double GAMMA_AIR = 1.4;           // Ratio of specific heats (cp/cv) for air
    constexpr double CP_AIR = 1005.0;           // Specific heat at constant pressure (J/kg·K)
    constexpr double R_AIR = 287.058;           // Specific gas constant for air (J/kg·K)
    constexpr double KPA_PER_PSI = 6.89476;
    constexpr double LB_PER_KG = 2.20462;
    constexpr double CFM_PER_M3S = 2118.88;
}

// ============================================================================
// TURBO TYPE ENUMS
// ============================================================================
enum class TurboType {
    SINGLE_TURBO,
    TWIN_TURBO,           // Parallel (one per bank)
    SEQUENTIAL_TURBO,     // Small + large, staged
    COMPOUND_TURBO,       // Series (LP → HP)
    TWIN_SCROLL,          // Single turbo, divided exhaust housing
    VGT                   // Variable Geometry Turbine
};

enum class SuperchargerType {
    ROOTS,                // Positive displacement (Eaton, Magnuson)
    TWIN_SCREW,           // Positive displacement (Whipple, Kenne Bell)
    CENTRIFUGAL           // Procharger, Vortech, Paxton
};

// ============================================================================
// COMPRESSOR MAP POINT
// Ref: Garrett Turbo Tech 101 - "Reading a Compressor Map"
// A compressor map plots Pressure Ratio vs Corrected Airflow
// with islands of constant efficiency and speed lines.
// ============================================================================
struct CompressorMapPoint {
    double correctedFlow;       // Corrected airflow (lb/min)
    double pressureRatio;       // Total-to-total pressure ratio (P2/P1)
    double efficiency;          // Isentropic compressor efficiency (0-1)
    double correctedSpeed;      // Corrected shaft speed (RPM)
};

// ============================================================================
// TURBO SIZING INPUT
// ============================================================================
struct TurboSizingInput {
    // Engine parameters
    double displacementCC;      // Engine displacement (cc)
    double targetHP;            // Target flywheel HP
    double maxRPM;              // Max engine RPM
    double numCylinders;        // Number of cylinders
    double volumetricEfficiency; // VE at target RPM (fraction, e.g. 0.85)

    // Boost parameters
    double targetBoostPSI;      // Target boost pressure (PSI gauge)
    double ambientTempC;        // Ambient temperature (C)
    double ambientPressureKPA;  // Ambient pressure (kPa), default 101.325

    // System losses
    double intercoolerPressDrop; // Intercooler pressure drop (PSI), typical 1-2
    double pipingPressDrop;      // Piping losses (PSI), typical 0.5-1.5
    double filterPressDrop;      // Air filter pressure drop (PSI), typical 0.3-0.5

    // Turbo config
    TurboType turboType;
    int numberOfTurbos;         // 1 or 2

    TurboSizingInput()
        : displacementCC(0), targetHP(0), maxRPM(0), numCylinders(4),
          volumetricEfficiency(0.85), targetBoostPSI(0),
          ambientTempC(TurboConstants::T_AMB_C),
          ambientPressureKPA(TurboConstants::P_ATM_KPA),
          intercoolerPressDrop(1.5), pipingPressDrop(1.0),
          filterPressDrop(0.3), turboType(TurboType::SINGLE_TURBO),
          numberOfTurbos(1) {}
};

// ============================================================================
// INTERCOOLER INPUT
// Ref: SAE 2003-01-0732, Garrett Application Guidelines
// ============================================================================
struct IntercoolerInput {
    double boostPressurePSI;
    double compressorEfficiency;   // (0-1)
    double ambientTempC;
    double targetIATempC;          // Target intake air temp after IC
    double intercoolerEfficiency;  // Typical 0.60-0.85 (air-air), 0.80-0.95 (air-water)
    bool isAirToWater;

    IntercoolerInput()
        : boostPressurePSI(0), compressorEfficiency(0.72),
          ambientTempC(TurboConstants::T_AMB_C), targetIATempC(40.0),
          intercoolerEfficiency(0.70), isAirToWater(false) {}
};

// ============================================================================
// TURBO SIZING RESULTS
// ============================================================================
struct TurboSizingResult {
    // Airflow requirements
    double requiredAirflowKGS;  // Required mass airflow (kg/s)
    double requiredAirflowLBM;  // Required mass airflow (lb/min)
    double requiredAirflowCFM;  // Volume airflow at inlet (CFM)
    double correctedAirflow;    // Corrected airflow for compressor map (lb/min)

    // Pressure
    double pressureRatioTotal;  // Total pressure ratio required (P2c/P1c)
    double absoluteBoostKPA;    // Absolute boost pressure (kPa)
    double boostPSIG;           // Gauge boost pressure (PSI)

    // Temperature
    double compressorOutletTempC;  // T2 - Temperature after compressor (C)
    double tempRiseAcrossCompressor; // Delta-T compressor (C)

    // Power
    double compressorPowerKW;   // Power consumed by compressor (kW)
    double compressorPowerHP;   // Power consumed by compressor (HP)

    // Sizing recommendation
    double inducer_mm;          // Recommended compressor inducer diameter (mm)
    double exducer_mm;          // Recommended compressor exducer diameter (mm)
    double turbineWheelDia_mm;  // Recommended turbine wheel diameter (mm)
    std::wstring sizeClass;     // "Small", "Medium", "Large", "Very Large"

    // Turbo recommendations (Garrett frame sizes)
    std::wstring garrettFrame;      // e.g. "GT2860R", "GTX3076R"
    std::wstring borgWarnerFrame;   // e.g. "EFR 6758", "EFR 7670"
    std::wstring precisionFrame;    // e.g. "PT5858", "PT6266"

    // Efficiency
    double estimatedCompressorEff;  // Estimated efficiency at operating point
    double estimatedTurbineEff;     // Estimated turbine efficiency

    // Intercooler
    double heatRejectionKW;     // Heat to be rejected by intercooler
    double icCoreSizeEstimate;  // Estimated IC core volume (liters)

    // Warnings
    std::wstring warnings;
    std::wstring recommendations;

    // Hot side (turbine) results
    double exhaustMassFlowKGS;      // Exhaust mass flow (air + fuel)
    double turbineInletTempC;       // T3 - Turbine inlet temperature (C)
    double turbineOutletTempC;      // T4 - Turbine outlet temperature (C)
    double turbinePressureRatio;    // Turbine expansion ratio
    double turbinePowerKW;          // Power extracted by turbine
    double exhaustBackpressureKPA;  // Backpressure at exhaust ports
    double estimatedTurboSpeedRPM;  // Estimated turbo shaft speed
    double suggestedTurbineAR;      // Recommended A/R
    double boostThresholdRPM;       // RPM where boost begins
    std::wstring egtWarning;        // EGT material limit warnings
    std::wstring turbineHousingRec; // Housing material recommendation
};

// ============================================================================
// SUPERCHARGER RESULTS
// ============================================================================
struct SuperchargerResult {
    double requiredDisplacementCID; // Required SC displacement (CID per revolution)
    double parasticLossHP;         // Power consumed by supercharger
    double thermalEfficiency;      // Isentropic efficiency
    double outletTempC;            // Charge temperature out
    double netHPGain;              // HP gained minus parasitic loss
    double boostAtRPM;             // Boost at given RPM

    // Pulley sizing
    double driveRatio;             // Required crank:SC pulley ratio
    double crankPulleyDiaMM;       // Crank pulley diameter (mm)
    double scPulleyDiaMM;          // SC pulley diameter (mm)
    double scSpeedAtMaxRPM;        // SC rotor speed at engine max RPM
    double beltSpeedMS;            // Belt linear speed (m/s)
    double beltLoadN;              // Belt tension (N)
    double minBeltWidth_mm;        // Minimum belt width recommendation

    std::wstring typeRecommendation;
    std::wstring pulleyRecommendation;
    std::wstring warnings;
};

// ============================================================================
// WASTEGATE SIZING
// Ref: Garrett Tech - "Wastegate Sizing", Turbonetics Application Guide
// ============================================================================
struct WastegateResult {
    double requiredFlowArea_mm2;   // Minimum flow area (mm²)
    double recommendedDiameter_mm; // Recommended WG valve diameter (mm)
    double springPressurePSI;      // WG spring base pressure
    bool internalSufficient;       // Can internal wastegate handle it?
    std::wstring recommendation;
};

// ============================================================================
// TurboCalculator CLASS
// ============================================================================
class TurboCalculator {
private:
    const EngineCore* engine;

public:
    explicit TurboCalculator(const EngineCore* eng = nullptr);
    ~TurboCalculator();

    // ========== PRIMARY CALCULATIONS ==========

    // Calculate required airflow for target power
    // Ref: Heywood eq. 6.2 - Mass airflow for SI engine
    //   ma = (Vd * N * ηv * ρa) / (nR)
    //   where nR = 2 for 4-stroke
    // Alternative (from power target):
    //   ma = P_target / (ηf * Qfuel / AFR)
    //   Simplified: HP = (airflow_lb/min × 10.42) for gasoline (Garrett method)
    double CalculateRequiredAirflow(const TurboSizingInput& input) const;

    // Calculate compressor pressure ratio
    // Ref: Garrett Turbo Tech 103
    //   PR = (P_boost + P_atm + ΔP_losses) / (P_atm - ΔP_filter)
    //   Must account for intercooler drop, piping losses, filter restriction
    double CalculatePressureRatio(const TurboSizingInput& input) const;

    // Calculate corrected airflow for compressor map lookup
    // Ref: SAE J1826, Garrett - Corrected to standard conditions
    //   W_corrected = W_actual × √(T_inlet/T_ref) / (P_inlet/P_ref)
    //   T_ref = 545°R (85°F = 302.6K), P_ref = 14.696 PSI (Garrett standard)
    double CalculateCorrectedAirflow(double massFlowLBM,
                                      double inletTempK,
                                      double inletPressureKPA) const;

    // Complete turbo sizing analysis
    TurboSizingResult CalculateTurboSizing(const TurboSizingInput& input) const;

    // ========== COMPRESSOR CALCULATIONS ==========

    // Compressor outlet temperature (isentropic + efficiency)
    // Ref: Heywood eq. 6.7
    //   T2 = T1 × [1 + (PR^((γ-1)/γ) - 1) / ηc]
    //   where ηc = isentropic compressor efficiency (typically 0.65-0.78)
    double CalculateCompressorOutletTemp(double inletTempK,
                                          double pressureRatio,
                                          double compressorEfficiency) const;

    // Compressor power consumption
    // Ref: Heywood eq. 6.8
    //   Wc = ma × cp × T1 × (PR^((γ-1)/γ) - 1) / ηc
    double CalculateCompressorPower(double massFlowKGS,
                                     double inletTempK,
                                     double pressureRatio,
                                     double compressorEfficiency) const;

    // Estimate compressor efficiency at operating point
    // Ref: Garrett compressor maps empirical data
    //   Peak efficiency typically at 65-75% of surge-to-choke range
    //   Efficiency islands roughly follow: η = η_peak × f(distance_from_peak)
    double EstimateCompressorEfficiency(double correctedFlow,
                                         double pressureRatio) const;

    // Compressor inducer diameter estimation
    // Ref: BorgWarner matching guide, empirical correlation
    //   D_inducer ∝ (airflow)^0.5 with corrections for PR and speed
    double EstimateInducerDiameter(double correctedFlowLBM) const;

    // ========== TURBINE CALCULATIONS ==========

    // Turbine A/R selection guide
    // Ref: Garrett Turbo Tech 102 - "A/R and its effects"
    //   Smaller A/R = faster spool, higher backpressure, lower peak power
    //   Larger A/R = slower spool, lower backpressure, higher peak power
    //   Rule of thumb: A/R ≈ displacement_liters × 0.04-0.06 (street)
    //                  A/R ≈ displacement_liters × 0.06-0.10 (race)
    double SuggestTurbineAR(double displacementLiters,
                             double targetBoostPSI,
                             bool isRaceApplication) const;

    // Turbine wheel diameter estimation
    // Ref: BorgWarner empirical data
    //   Turbine wheel ≈ 80-90% of compressor exducer for matched turbo
    double EstimateTurbineWheelDiameter(double compressorExducerMM) const;

    // ========== INTERCOOLER ==========

    // Intercooler outlet temperature
    // Ref: SAE 2003-01-0732
    //   T_out = T_hot - ε × (T_hot - T_cold)
    //   ε = intercooler effectiveness (0.60-0.85 air/air, 0.80-0.95 air/water)
    double CalculateIntercoolerOutletTemp(double compressorOutTempC,
                                           double ambientTempC,
                                           double effectiveness) const;

    // Heat rejection required
    // Ref: Thermodynamics - Q = ma × cp × ΔT
    double CalculateHeatRejection(double massFlowKGS,
                                   double tempDropC) const;

    // Intercooler core volume estimate
    // Ref: Garrett application data, Bell "Maximum Boost"
    //   Rule of thumb: ~1L core per 100hp of heat rejection
    double EstimateICCoreVolume(double heatRejectionKW) const;

    // ========== WASTEGATE ==========

    // Wastegate flow area required
    // Ref: Garrett wastegate sizing, Turbonetics tech bulletin
    //   Flow area must bypass enough exhaust to prevent overboosting
    WastegateResult CalculateWastegate(double targetBoostPSI,
                                        double maxExhaustFlowKGS,
                                        double turbineEfficiency) const;

    // ========== SUPERCHARGER ==========

    // Roots/Twin-Screw displacement sizing
    // Ref: Eaton supercharger application guide
    //   SC_displacement = engine_displacement × PR × (1/ηv_sc)
    //   Parasitic loss ≈ 15-20% of boost power (Roots), 10-15% (Twin-Screw)
    SuperchargerResult CalculateSupercharger(double displacementCC,
                                              double targetBoostPSI,
                                              double maxRPM,
                                              double driveRatio,
                                              double crankPulleyDiaMM,
                                              SuperchargerType type) const;

    // ========== PULLEY SIZING ==========

    // Calculate SC pulley diameter for target boost
    // Ref: Eaton/Whipple application manuals
    //   boost ∝ (SC_speed × SC_displacement) / engine_displacement
    //   SC_speed = engine_RPM × (crank_pulley_dia / sc_pulley_dia)
    //   Smaller SC pulley = more boost (higher overdrive ratio)
    //
    // Typical crank pulley sizes:
    //   GM LS: 190-200mm, Ford Mod/Coyote: 170-180mm
    //   Mopar Hemi: 195-205mm, Honda K-series: 130-140mm
    //
    // SC max rotor speed limits:
    //   Eaton M90/M112: 14,000-16,000 RPM
    //   Eaton TVS1900/2300: 14,000-18,000 RPM
    //   Whipple 2.3L/2.9L: 14,000-16,000 RPM
    //   Kenne Bell 2.8L/3.6L: 14,000-17,000 RPM
    //   Procharger P-1SC: 40,000-65,000 RPM (centrifugal)
    //   Vortech V-3 Si: 45,000-65,000 RPM (centrifugal)
    double CalculateSCPulleyDiameter(double crankPulleyDiaMM,
                                      double targetDriveRatio) const;

    // Calculate drive ratio for target boost
    // Ref: Eaton TVS application guide, Whipple tech notes
    //   For PD blowers: ratio = (target_airflow) / (engine_airflow × VE_sc)
    //   Typical overdrive: 1.8:1 to 3.2:1
    double CalculateRequiredDriveRatio(double displacementCC,
                                        double scDisplacementCID,
                                        double targetBoostPSI,
                                        SuperchargerType type) const;

    // Belt speed and load
    // Ref: Gates belt engineering manual
    //   Max belt speed: 40 m/s (standard V-belt), 60 m/s (multi-rib)
    //   Belt tension depends on transmitted power
    double CalculateBeltSpeed(double pulleyDiaMM, double pulleyRPM) const;
    double CalculateBeltLoad(double powerKW, double beltSpeedMS) const;

    // ========== TURBO MATCHING ==========

    // Recommend Garrett frame size based on airflow
    // Ref: Garrett product catalog 2024
    //   GT/GTX series sizing by corrected airflow range
    std::wstring RecommendGarrettFrame(double correctedFlowLBM,
                                        double pressureRatio) const;

    // Recommend BorgWarner EFR frame
    // Ref: BorgWarner EFR product line specifications
    std::wstring RecommendBorgWarnerFrame(double correctedFlowLBM,
                                           double pressureRatio) const;

    // Recommend Precision Turbo frame
    // Ref: Precision Turbo & Engine product catalog
    std::wstring RecommendPrecisionFrame(double correctedFlowLBM,
                                          double pressureRatio) const;

    // ========== BOOST THRESHOLD ==========

    // Estimate boost threshold RPM (RPM where turbo starts making boost)
    // Ref: Garrett application data
    //   Depends on: turbine A/R, turbo inertia, exhaust energy
    //   Empirical: threshold ≈ 2000-3500 RPM (street), 4000-5000 (race)
    double EstimateBoostThresholdRPM(double turbineAR,
                                      double displacementLiters,
                                      double numCylinders) const;

    // ========== HOT SIDE (TURBINE) ANALYSIS ==========

    // Exhaust Gas Temperature estimation
    // Ref: Heywood Ch.4 Table 4.9, Garrett "Turbine Inlet Temperature Guidelines"
    //   Gasoline N/A: 750-900°C, Turbo: 850-1050°C
    //   Diesel: 500-700°C, Race gas: 900-1100°C
    //   T3 = f(AFR, ignition timing, boost, RPM)
    double EstimateEGT(double targetHP, double displacementCC,
                        double boostPSI, bool isRace) const;

    // Turbine outlet temperature
    // Ref: Heywood eq. 6.10 (isentropic expansion)
    //   T4 = T3 × [1 - ηt × (1 - 1/PR_t^((γ-1)/γ))]
    //   γ_exhaust ≈ 1.35 (combustion products, Heywood Table 4.7)
    double CalculateTurbineOutletTemp(double turbineInletTempK,
                                       double turbinePR,
                                       double turbineEfficiency) const;

    // Turbine power (must balance compressor power + bearing friction)
    // Ref: Heywood eq. 6.9
    //   Wt = m_exhaust × cp_exhaust × T3 × ηt × [1 - 1/PR_t^((γe-1)/γe)]
    double CalculateTurbinePower(double exhaustFlowKGS,
                                  double turbineInletTempK,
                                  double turbinePR,
                                  double turbineEfficiency) const;

    // Exhaust backpressure estimation
    // Ref: SAE 920044 Watson & Janota, Garrett turbine A/R flow data
    //   Backpressure depends on turbine A/R, flow rate, and exhaust temp
    //   Excessive backpressure reduces VE (pumping losses)
    double EstimateBackpressure(double exhaustFlowKGS,
                                 double turbineAR,
                                 double exhaustTempK) const;

    // Turbo shaft speed estimation
    // Ref: BorgWarner EFR specs, Garrett GT/GTX max speed data
    //   Speed ∝ tip_speed / (π × inducer_dia)
    //   Tip speed limit: ~500-520 m/s (aluminum), ~550 m/s (titanium)
    double EstimateTurboSpeed(double correctedFlowLBM,
                               double pressureRatio,
                               double inducerDiaMM) const;

    // Housing material recommendation based on EGT
    // Ref: Industry data - Garrett, BorgWarner material specs
    //   Cast iron (SiMo): up to 760°C continuous
    //   Stainless 304: up to 870°C continuous
    //   Inconel 713C: up to 950°C continuous
    //   Mar-M: up to 1050°C continuous
    std::wstring RecommendHousingMaterial(double egtC) const;

    // ========== COMPRESSOR MAP MODEL ==========

    // Generate compressor map data points for rendering
    // Models surge line, speed lines, efficiency islands based on turbo size
    // Ref: Garrett published compressor maps (GT/GTX series)
    struct CompressorMapData {
        // Surge line: vector of (flow, PR) points
        std::vector<std::pair<float, float>> surgeLine;
        // Choke line
        std::vector<std::pair<float, float>> chokeLine;
        // Speed lines: each is a vector of (flow, PR) at constant corrected RPM
        struct SpeedLine {
            double correctedRPM;
            std::vector<std::pair<float, float>> points;
        };
        std::vector<SpeedLine> speedLines;
        // Efficiency islands: each contour at a given efficiency level
        struct EfficiencyIsland {
            double efficiency;  // e.g. 0.60, 0.65, 0.70, 0.75, 0.78
            std::vector<std::pair<float, float>> contour;
        };
        std::vector<EfficiencyIsland> efficiencyIslands;
        // Map extents
        float maxFlow;
        float maxPR;
    };

    CompressorMapData GenerateCompressorMap(double inducerDiaMM,
                                             double maxCorrectedFlow) const;

    // ========== UTILITY ==========

    double PSItoKPA(double psi) const { return psi * TurboConstants::KPA_PER_PSI; }
    double KPAtoPSI(double kpa) const { return kpa / TurboConstants::KPA_PER_PSI; }
    double CelsiusToKelvin(double c) const { return c + 273.15; }
    double KelvinToCelsius(double k) const { return k - 273.15; }
    double HPtoKW(double hp) const { return hp * 0.7457; }
    double KWtoHP(double kw) const { return kw / 0.7457; }
};
