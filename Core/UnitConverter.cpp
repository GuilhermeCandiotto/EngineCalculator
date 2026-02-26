#include "UnitConverter.h"
#include <sstream>
#include <iomanip>

// Inicialização do singleton
UnitConverter* GlobalUnitConverter::instance = nullptr;

// ============================================================================
// UnitConverter
// ============================================================================

UnitConverter::UnitConverter(UnitSystem system) 
    : currentSystem(system) {
}

UnitConverter::~UnitConverter() {
}

void UnitConverter::SetSystem(UnitSystem system) {
    currentSystem = system;
}

// ========== CONVERSÕES DE COMPRIMENTO ==========

double UnitConverter::ConvertLength(double value, LengthUnit from, LengthUnit to) const {
    if (from == to) return value;
    
    if (from == LengthUnit::MILLIMETER && to == LengthUnit::INCH) {
        return MMToInch(value);
    } else if (from == LengthUnit::INCH && to == LengthUnit::MILLIMETER) {
        return InchToMM(value);
    }
    
    return value;
}

// ========== CONVERSÕES DE VOLUME ==========

double UnitConverter::ConvertVolume(double value, VolumeUnit from, VolumeUnit to) const {
    if (from == to) return value;
    
    // Converte tudo para CC primeiro, depois para unidade destino
    double valueInCC = value;
    
    switch (from) {
        case VolumeUnit::CI:
            valueInCC = CIToCC(value);
            break;
        case VolumeUnit::LITER:
            valueInCC = LiterToCC(value);
            break;
        case VolumeUnit::CC:
        default:
            valueInCC = value;
            break;
    }
    
    // Agora converte de CC para unidade destino
    switch (to) {
        case VolumeUnit::CI:
            return CCToCI(valueInCC);
        case VolumeUnit::LITER:
            return CCToLiter(valueInCC);
        case VolumeUnit::CC:
        default:
            return valueInCC;
    }
}

// ========== CONVERSÕES DE MASSA ==========

double UnitConverter::ConvertMass(double value, MassUnit from, MassUnit to) const {
    if (from == to) return value;
    
    // Converte tudo para KG primeiro
    double valueInKg = value;
    
    switch (from) {
        case MassUnit::POUND:
            valueInKg = LbToKg(value);
            break;
        case MassUnit::GRAM:
            valueInKg = GramToKg(value);
            break;
        case MassUnit::KILOGRAM:
        default:
            valueInKg = value;
            break;
    }
    
    // Converte de KG para unidade destino
    switch (to) {
        case MassUnit::POUND:
            return KgToLb(valueInKg);
        case MassUnit::GRAM:
            return KgToGram(valueInKg);
        case MassUnit::KILOGRAM:
        default:
            return valueInKg;
    }
}

// ========== CONVERSÕES DE FORÇA ==========

double UnitConverter::ConvertForce(double value, ForceUnit from, ForceUnit to) const {
    if (from == to) return value;
    
    if (from == ForceUnit::NEWTON && to == ForceUnit::POUND_FORCE) {
        return NToLbf(value);
    } else if (from == ForceUnit::POUND_FORCE && to == ForceUnit::NEWTON) {
        return LbfToN(value);
    }
    
    return value;
}

// ========== CONVERSÕES DE PRESSÃO ==========

double UnitConverter::ConvertPressure(double value, PressureUnit from, PressureUnit to) const {
    if (from == to) return value;
    
    // Converte tudo para kPa primeiro
    double valueInKPa = value;
    
    switch (from) {
        case PressureUnit::PSI:
            valueInKPa = PSIToKPa(value);
            break;
        case PressureUnit::BAR:
            valueInKPa = BarToKPa(value);
            break;
        case PressureUnit::KILOPASCAL:
        default:
            valueInKPa = value;
            break;
    }
    
    // Converte de kPa para unidade destino
    switch (to) {
        case PressureUnit::PSI:
            return KPaToPSI(valueInKPa);
        case PressureUnit::BAR:
            return KPaToBar(valueInKPa);
        case PressureUnit::KILOPASCAL:
        default:
            return valueInKPa;
    }
}

// ========== CONVERSÕES DE TORQUE ==========

double UnitConverter::ConvertTorque(double value, TorqueUnit from, TorqueUnit to) const {
    if (from == to) return value;
    
    if (from == TorqueUnit::NEWTON_METER && to == TorqueUnit::POUND_FOOT) {
        return NmToLbFt(value);
    } else if (from == TorqueUnit::POUND_FOOT && to == TorqueUnit::NEWTON_METER) {
        return LbFtToNm(value);
    }
    
    return value;
}

// ========== CONVERSÕES DE POTÊNCIA ==========

double UnitConverter::ConvertPower(double value, PowerUnit from, PowerUnit to) const {
    if (from == to) return value;
    
    // Converte tudo para kW primeiro
    double valueInKw = value;
    
    switch (from) {
        case PowerUnit::HORSEPOWER:
            valueInKw = HPToKw(value);
            break;
        case PowerUnit::METRIC_HP:
            valueInKw = HPToKw(MetricHPToHP(value));
            break;
        case PowerUnit::KILOWATT:
        default:
            valueInKw = value;
            break;
    }
    
    // Converte de kW para unidade destino
    switch (to) {
        case PowerUnit::HORSEPOWER:
            return KwToHP(valueInKw);
        case PowerUnit::METRIC_HP:
            return HPToMetricHP(KwToHP(valueInKw));
        case PowerUnit::KILOWATT:
        default:
            return valueInKw;
    }
}

// ========== CONVERSÕES DE VELOCIDADE ==========

double UnitConverter::ConvertVelocity(double value, VelocityUnit from, VelocityUnit to) const {
    if (from == to) return value;
    
    // Converte tudo para m/s primeiro
    double valueInMps = value;
    
    switch (from) {
        case VelocityUnit::KM_PER_HOUR:
            valueInMps = KmhToMps(value);
            break;
        case VelocityUnit::MILE_PER_HOUR:
            valueInMps = MphToMps(value);
            break;
        case VelocityUnit::METER_PER_SEC:
        default:
            valueInMps = value;
            break;
    }
    
    // Converte de m/s para unidade destino
    switch (to) {
        case VelocityUnit::KM_PER_HOUR:
            return MpsToKmh(valueInMps);
        case VelocityUnit::MILE_PER_HOUR:
            return MpsToMph(valueInMps);
        case VelocityUnit::METER_PER_SEC:
        default:
            return valueInMps;
    }
}

// ========== CONVERSÕES DE TEMPERATURA ==========

double UnitConverter::ConvertTemperature(double value, TemperatureUnit from, TemperatureUnit to) const {
    if (from == to) return value;
    
    if (from == TemperatureUnit::CELSIUS && to == TemperatureUnit::FAHRENHEIT) {
        return CelsiusToFahrenheit(value);
    } else if (from == TemperatureUnit::FAHRENHEIT && to == TemperatureUnit::CELSIUS) {
        return FahrenheitToCelsius(value);
    }
    
    return value;
}

// ========== SÍMBOLOS DE UNIDADES ==========

std::wstring UnitConverter::GetUnitSymbol(LengthUnit unit) const {
    switch (unit) {
        case LengthUnit::MILLIMETER: return L"mm";
        case LengthUnit::INCH: return L"in";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(VolumeUnit unit) const {
    switch (unit) {
        case VolumeUnit::CC: return L"cc";
        case VolumeUnit::CI: return L"ci";
        case VolumeUnit::LITER: return L"L";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(MassUnit unit) const {
    switch (unit) {
        case MassUnit::KILOGRAM: return L"kg";
        case MassUnit::GRAM: return L"g";
        case MassUnit::POUND: return L"lb";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(ForceUnit unit) const {
    switch (unit) {
        case ForceUnit::NEWTON: return L"N";
        case ForceUnit::POUND_FORCE: return L"lbf";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(PressureUnit unit) const {
    switch (unit) {
        case PressureUnit::KILOPASCAL: return L"kPa";
        case PressureUnit::PSI: return L"psi";
        case PressureUnit::BAR: return L"bar";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(TorqueUnit unit) const {
    switch (unit) {
        case TorqueUnit::NEWTON_METER: return L"Nm";
        case TorqueUnit::POUND_FOOT: return L"lb-ft";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(PowerUnit unit) const {
    switch (unit) {
        case PowerUnit::KILOWATT: return L"kW";
        case PowerUnit::HORSEPOWER: return L"HP";
        case PowerUnit::METRIC_HP: return L"cv";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(VelocityUnit unit) const {
    switch (unit) {
        case VelocityUnit::METER_PER_SEC: return L"m/s";
        case VelocityUnit::KM_PER_HOUR: return L"km/h";
        case VelocityUnit::MILE_PER_HOUR: return L"mph";
        default: return L"";
    }
}

std::wstring UnitConverter::GetUnitSymbol(TemperatureUnit unit) const {
    switch (unit) {
        case TemperatureUnit::CELSIUS: return L"°C";
        case TemperatureUnit::FAHRENHEIT: return L"°F";
        default: return L"";
    }
}

// ========== UNIDADES PADRÃO POR SISTEMA ==========

LengthUnit UnitConverter::GetDefaultLengthUnit() const {
    return (currentSystem == UnitSystem::METRIC) ? LengthUnit::MILLIMETER : LengthUnit::INCH;
}

VolumeUnit UnitConverter::GetDefaultVolumeUnit() const {
    return (currentSystem == UnitSystem::METRIC) ? VolumeUnit::CC : VolumeUnit::CI;
}

PressureUnit UnitConverter::GetDefaultPressureUnit() const {
    return (currentSystem == UnitSystem::METRIC) ? PressureUnit::KILOPASCAL : PressureUnit::PSI;
}

TorqueUnit UnitConverter::GetDefaultTorqueUnit() const {
    return (currentSystem == UnitSystem::METRIC) ? TorqueUnit::NEWTON_METER : TorqueUnit::POUND_FOOT;
}

PowerUnit UnitConverter::GetDefaultPowerUnit() const {
    return (currentSystem == UnitSystem::METRIC) ? PowerUnit::KILOWATT : PowerUnit::HORSEPOWER;
}

// ========== FORMATAÇÃO ==========

std::wstring UnitConverter::FormatWithUnit(double value, LengthUnit unit, int precision) const {
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value << L" " << GetUnitSymbol(unit);
    return oss.str();
}

std::wstring UnitConverter::FormatWithUnit(double value, VolumeUnit unit, int precision) const {
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value << L" " << GetUnitSymbol(unit);
    return oss.str();
}

std::wstring UnitConverter::FormatWithUnit(double value, PressureUnit unit, int precision) const {
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value << L" " << GetUnitSymbol(unit);
    return oss.str();
}

std::wstring UnitConverter::FormatWithUnit(double value, PowerUnit unit, int precision) const {
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value << L" " << GetUnitSymbol(unit);
    return oss.str();
}

// ============================================================================
// GlobalUnitConverter (Singleton)
// ============================================================================

UnitConverter* GlobalUnitConverter::GetInstance() {
    if (instance == nullptr) {
        instance = new UnitConverter(UnitSystem::METRIC);
    }
    return instance;
}

void GlobalUnitConverter::SetSystem(UnitSystem system) {
    GetInstance()->SetSystem(system);
}

void GlobalUnitConverter::DestroyInstance() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}
