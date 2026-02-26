#pragma once
// ============================================================================
// UnitConverter.h - Sistema de Conversão de Unidades
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Suporta conversão entre sistemas métrico e imperial
// ============================================================================
#include <string>
#include <map>

// Tipos de unidades
enum class UnitType {
    LENGTH,         // Distância (mm, inch)
    VOLUME,         // Volume (cc, ci)
    MASS,           // Massa (kg, lb)
    FORCE,          // Força (N, lbf)
    PRESSURE,       // Pressão (kPa, psi)
    TORQUE,         // Torque (Nm, lb-ft)
    POWER,          // Potência (kW, HP)
    VELOCITY,       // Velocidade (m/s, mph, km/h)
    TEMPERATURE     // Temperatura (°C, °F)
};

// Sistema de unidades
enum class UnitSystem {
    METRIC,         // Sistema métrico (SI)
    IMPERIAL        // Sistema imperial (USA/UK)
};

// Unidades específicas
enum class LengthUnit {
    MILLIMETER,     // mm
    INCH            // in
};

enum class VolumeUnit {
    CC,             // cm³ (cubic centimeters)
    CI,             // in³ (cubic inches)
    LITER           // L
};

enum class MassUnit {
    KILOGRAM,       // kg
    GRAM,           // g
    POUND           // lb
};

enum class ForceUnit {
    NEWTON,         // N
    POUND_FORCE     // lbf
};

enum class PressureUnit {
    KILOPASCAL,     // kPa
    PSI,            // lb/in² (pounds per square inch)
    BAR             // bar
};

enum class TorqueUnit {
    NEWTON_METER,   // Nm
    POUND_FOOT      // lb-ft
};

enum class PowerUnit {
    KILOWATT,       // kW
    HORSEPOWER,     // HP (mechanical)
    METRIC_HP       // cv (metric horsepower)
};

enum class VelocityUnit {
    METER_PER_SEC,  // m/s
    KM_PER_HOUR,    // km/h
    MILE_PER_HOUR   // mph
};

enum class TemperatureUnit {
    CELSIUS,        // °C
    FAHRENHEIT      // °F
};

// Conversor de unidades
class UnitConverter {
private:
    UnitSystem currentSystem;
    
    // Fatores de conversão
    static constexpr double MM_TO_INCH = 0.0393701;
    static constexpr double INCH_TO_MM = 25.4;
    
    static constexpr double CC_TO_CI = 0.0610237;
    static constexpr double CI_TO_CC = 16.3871;
    
    static constexpr double KG_TO_LB = 2.20462;
    static constexpr double LB_TO_KG = 0.453592;
    
    static constexpr double N_TO_LBF = 0.224809;
    static constexpr double LBF_TO_N = 4.44822;
    
    static constexpr double KPA_TO_PSI = 0.145038;
    static constexpr double PSI_TO_KPA = 6.89476;
    
    static constexpr double NM_TO_LBFT = 0.737562;
    static constexpr double LBFT_TO_NM = 1.35582;
    
    static constexpr double KW_TO_HP = 1.34102;
    static constexpr double HP_TO_KW = 0.745699;
    
    static constexpr double MPS_TO_MPH = 2.23694;
    static constexpr double MPH_TO_MPS = 0.44704;
    
public:
    UnitConverter(UnitSystem system = UnitSystem::METRIC);
    ~UnitConverter();
    
    // Configuração
    void SetSystem(UnitSystem system);
    UnitSystem GetSystem() const { return currentSystem; }
    
    // ========== CONVERSÕES DE COMPRIMENTO ==========
    
    double ConvertLength(double value, LengthUnit from, LengthUnit to) const;
    
    // Helpers específicos
    double MMToInch(double mm) const { return mm * MM_TO_INCH; }
    double InchToMM(double inch) const { return inch * INCH_TO_MM; }
    
    // ========== CONVERSÕES DE VOLUME ==========
    
    double ConvertVolume(double value, VolumeUnit from, VolumeUnit to) const;
    
    // Helpers específicos
    double CCToCI(double cc) const { return cc * CC_TO_CI; }
    double CIToCC(double ci) const { return ci * CI_TO_CC; }
    double CCToLiter(double cc) const { return cc / 1000.0; }
    double LiterToCC(double liter) const { return liter * 1000.0; }
    
    // ========== CONVERSÕES DE MASSA ==========
    
    double ConvertMass(double value, MassUnit from, MassUnit to) const;
    
    // Helpers específicos
    double KgToLb(double kg) const { return kg * KG_TO_LB; }
    double LbToKg(double lb) const { return lb * LB_TO_KG; }
    double GramToKg(double g) const { return g / 1000.0; }
    double KgToGram(double kg) const { return kg * 1000.0; }
    
    // ========== CONVERSÕES DE FORÇA ==========
    
    double ConvertForce(double value, ForceUnit from, ForceUnit to) const;
    
    // Helpers específicos
    double NToLbf(double n) const { return n * N_TO_LBF; }
    double LbfToN(double lbf) const { return lbf * LBF_TO_N; }
    
    // ========== CONVERSÕES DE PRESSÃO ==========
    
    double ConvertPressure(double value, PressureUnit from, PressureUnit to) const;
    
    // Helpers específicos
    double KPaToPSI(double kpa) const { return kpa * KPA_TO_PSI; }
    double PSIToKPa(double psi) const { return psi * PSI_TO_KPA; }
    double BarToKPa(double bar) const { return bar * 100.0; }
    double KPaToBar(double kpa) const { return kpa / 100.0; }
    
    // ========== CONVERSÕES DE TORQUE ==========
    
    double ConvertTorque(double value, TorqueUnit from, TorqueUnit to) const;
    
    // Helpers específicos
    double NmToLbFt(double nm) const { return nm * NM_TO_LBFT; }
    double LbFtToNm(double lbft) const { return lbft * LBFT_TO_NM; }
    
    // ========== CONVERSÕES DE POTÊNCIA ==========
    
    double ConvertPower(double value, PowerUnit from, PowerUnit to) const;
    
    // Helpers específicos
    double KwToHP(double kw) const { return kw * KW_TO_HP; }
    double HPToKw(double hp) const { return hp * HP_TO_KW; }
    double HPToMetricHP(double hp) const { return hp * 1.01387; }
    double MetricHPToHP(double cv) const { return cv * 0.98632; }
    
    // ========== CONVERSÕES DE VELOCIDADE ==========
    
    double ConvertVelocity(double value, VelocityUnit from, VelocityUnit to) const;
    
    // Helpers específicos
    double MpsToMph(double mps) const { return mps * MPS_TO_MPH; }
    double MphToMps(double mph) const { return mph * MPH_TO_MPS; }
    double MpsToKmh(double mps) const { return mps * 3.6; }
    double KmhToMps(double kmh) const { return kmh / 3.6; }
    
    // ========== CONVERSÕES DE TEMPERATURA ==========
    
    double ConvertTemperature(double value, TemperatureUnit from, TemperatureUnit to) const;
    
    // Helpers específicos
    double CelsiusToFahrenheit(double c) const { return (c * 9.0 / 5.0) + 32.0; }
    double FahrenheitToCelsius(double f) const { return (f - 32.0) * 5.0 / 9.0; }
    
    // ========== FORMATAÇÃO E LABELS ==========
    
    // Retorna o símbolo da unidade
    std::wstring GetUnitSymbol(LengthUnit unit) const;
    std::wstring GetUnitSymbol(VolumeUnit unit) const;
    std::wstring GetUnitSymbol(MassUnit unit) const;
    std::wstring GetUnitSymbol(ForceUnit unit) const;
    std::wstring GetUnitSymbol(PressureUnit unit) const;
    std::wstring GetUnitSymbol(TorqueUnit unit) const;
    std::wstring GetUnitSymbol(PowerUnit unit) const;
    std::wstring GetUnitSymbol(VelocityUnit unit) const;
    std::wstring GetUnitSymbol(TemperatureUnit unit) const;
    
    // Retorna unidades padrão do sistema atual
    LengthUnit GetDefaultLengthUnit() const;
    VolumeUnit GetDefaultVolumeUnit() const;
    PressureUnit GetDefaultPressureUnit() const;
    TorqueUnit GetDefaultTorqueUnit() const;
    PowerUnit GetDefaultPowerUnit() const;
    
    // Formata valor com unidade
    std::wstring FormatWithUnit(double value, LengthUnit unit, int precision = 2) const;
    std::wstring FormatWithUnit(double value, VolumeUnit unit, int precision = 2) const;
    std::wstring FormatWithUnit(double value, PressureUnit unit, int precision = 2) const;
    std::wstring FormatWithUnit(double value, PowerUnit unit, int precision = 2) const;
};

// Singleton global
class GlobalUnitConverter {
private:
    static UnitConverter* instance;
    
public:
    static UnitConverter* GetInstance();
    static void SetSystem(UnitSystem system);
    static void DestroyInstance();
};
