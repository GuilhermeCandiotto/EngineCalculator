#include "AlternativeFuelsCalculator.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

AlternativeFuelsCalculator::AlternativeFuelsCalculator(const EngineCore* eng)
    : engine(eng) {
}

AlternativeFuelsCalculator::~AlternativeFuelsCalculator() {
}

// ============================================================================
// ÓXIDO NITROSO (N2O/NOS)
// ============================================================================

double AlternativeFuelsCalculator::CalculateNitrousJetSize(double hpGain, double cylinderPressure,
                                                           int numCylinders) const {
    // SAE 2003-01-3242: Relação entre jet size e HP
    // Flow (lb/hr) = HP_gain × 0.125 (aprox.)
    // Jet size (inches) = sqrt(Flow / (K × Pressure))
    // K = constante de fluxo ~ 0.13
    
    double flowLbHr = hpGain * 0.125;
    double flowPerCylinder = flowLbHr / numCylinders;
    
    // Correção por pressão (referência: 900 PSI)
    double pressureRatio = sqrt(cylinderPressure / 900.0);
    
    double jetSize = sqrt(flowPerCylinder / (0.13 * pressureRatio));
    
    return jetSize; // polegadas
}

double AlternativeFuelsCalculator::CalculateFuelJetSize(double nitrousJetSize, double fuelType) const {
    // Ratio típico: 1 parte combustível para 6-8 partes nitro (massa)
    // Wet shot precisa adicionar combustível proporcional
    double ratio = 7.0; // 1:7 (conservador)
    
    return nitrousJetSize / ratio;
}

double AlternativeFuelsCalculator::EstimateNitrousHPGain(double jetSize, double pressure) const {
    // SAE 2005-01-0547: HP gain baseado em flow rate
    // Flow (lb/hr) ~ jet_size² × K * sqrt(pressure)
    
    double flowRate = jetSize * jetSize * 0.13 * sqrt(pressure / 900.0);
    double hpGain = flowRate / 0.125; // Aproximação inversa
    
    return hpGain;
}

double AlternativeFuelsCalculator::MaxSafeCRWithNitrous(double shotSize, bool hasIntercooler) const {
    // Recomendações baseadas em experiência da indústria:
    // - Sem nitro: CR até 11:1 (aspirado)
    // - 50 HP shot: CR <= 10:1
    // - 75 HP shot: CR <= 9.5:1
    // - 100+ HP shot: CR <= 9.0:1
    // - 150+ HP shot: CR <= 8.5:1
    
    double maxCR = 11.0;
    
    if (shotSize >= 150) {
        maxCR = 8.5;
    } else if (shotSize >= 100) {
        maxCR = 9.0;
    } else if (shotSize >= 75) {
        maxCR = 9.5;
    } else if (shotSize >= 50) {
        maxCR = 10.0;
    }
    
    // Intercooler permite +0.5 CR
    if (hasIntercooler) {
        maxCR += 0.5;
    }
    
    return maxCR;
}

double AlternativeFuelsCalculator::CalculateIntakeTempDrop(double shotSize) const {
    // N2O tem efeito endotérmico (absorve calor ao evaporar)
    // Queda típica: 60-80°F para 100 HP shot
    return N2O_TEMP_DROP * (shotSize / 100.0);
}

NitrousSystemData AlternativeFuelsCalculator::AnalyzeNitrousSystem(const NitrousConfig& config) const {
    NitrousSystemData data = { 0 };
    
    data.nitrousJetSize = CalculateNitrousJetSize(config.desiredGain, 
                                                  config.cylinderPressure,
                                                  config.numberOfCylinders);
    
    if (config.systemType == NitrousSystemType::WET || 
        config.systemType == NitrousSystemType::DIRECT_PORT) {
        data.fuelJetSize = CalculateFuelJetSize(data.nitrousJetSize);
    }
    
    data.expectedHPGain = config.desiredGain;
    data.expectedTorqueGain = config.desiredGain * 0.9; // Torque similar
    data.maxSafeCR = MaxSafeCRWithNitrous(config.desiredGain, config.hasIntercooler);
    data.intakeTemperatureDrop = CalculateIntakeTempDrop(config.desiredGain);
    data.oxygenEnrichment = N2O_OXYGEN_BOOST;
    
    // Avisos de segurança
    std::wostringstream warnings;
    warnings << L"[!!] AVISOS DE SEGURANÇA:\n\n";
    
    if (config.desiredGain > 150) {
        warnings << L"[!!] Shot >150 HP é EXTREMO! Risco de dano ao motor.\n";
        warnings << L"   - CR máxima: " << data.maxSafeCR << L":1\n";
        warnings << L"   - Use pistões forjados obrigatório\n";
        warnings << L"   - Bielas forjadas ou H-beam\n";
        warnings << L"   - Velas 2-3 graus mais frias\n\n";
    } else if (config.desiredGain > 100) {
        warnings << L"[!!] Shot >100 HP requer componentes reforçados:\n";
        warnings << L"   - CR máxima: " << data.maxSafeCR << L":1\n";
        warnings << L"   - Pistões forjados recomendado\n";
        warnings << L"   - Velas 1-2 graus mais frias\n\n";
    } else if (config.desiredGain > 75) {
        warnings << L"[!!] Shot de 75-100 HP:\n";
        warnings << L"   - CR máxima: " << data.maxSafeCR << L":1\n";
        warnings << L"   - Velas 1 grau mais fria\n\n";
    }
    
    if (config.systemType == NitrousSystemType::DRY) {
        warnings << L"[!!] Sistema DRY:\n";
        warnings << L"   - Enriquecimento de combustível OBRIGATÓRIO\n";
        warnings << L"   - AFR target: 11.5:1 com nitro ativo\n";
        warnings << L"   - Risco de lean-out se não configurar!\n\n";
    }
    
    warnings << L"[OK] Recomendacoes:\n";
    warnings << L"  - Ativar nitro apenas >3000 RPM\n";
    warnings << L"  - Nunca em marcha lenta ou neutro\n";
    warnings << L"  - Aquecer motor antes de usar\n";
    warnings << L"  - Purgar sistema antes de correr\n";
    warnings << L"  - Verificar pressão do cilindro regularmente\n";
    
    data.safetyWarnings = warnings.str();
    
    return data;
}

// ============================================================================
// NITROMETANO (CH3NO2)
// ============================================================================

NitromethaneProperties AlternativeFuelsCalculator::CalculateNitroProperties(
    const NitromethaneBlend& blend) const {
    
    NitromethaneProperties props = { 0 };
    
    // Interpolação linear entre gasolina e nitro puro
    double nitroFraction = blend.percentNitro / 100.0;
    
    // AFR: gasolina 14.7:1, nitro 1.7:1
    props.stoichAFR = GASOLINE_STOICH_AFR * (1.0 - nitroFraction) + 
                      NITRO_STOICH_AFR * nitroFraction;
    
    // Densidade energética (MJ/kg)
    props.energyDensity = GASOLINE_ENERGY_DENSITY * (1.0 - nitroFraction) +
                          NITRO_ENERGY_DENSITY * nitroFraction;
    
    // Octanagem (RON)
    double gasolineOctane = 95.0; // Regular premium
    props.octaneRating = gasolineOctane * (1.0 - nitroFraction) +
                         NITRO_OCTANE * nitroFraction;
    
    // CR máxima
    props.maxCR = 11.0 + (NITRO_MAX_CR - 11.0) * nitroFraction;
    
    // BSFC (consumo específico) - nitro consome MUITO
    props.bsfc = 0.50 + (1.2 - 0.50) * nitroFraction; // lb/HP/hr
    
    return props;
}

double AlternativeFuelsCalculator::CalculateNitroAFR(double percentNitro) const {
    double nitroFraction = percentNitro / 100.0;
    return GASOLINE_STOICH_AFR * (1.0 - nitroFraction) + 
           NITRO_STOICH_AFR * nitroFraction;
}

double AlternativeFuelsCalculator::MaxCRForNitro(double percentNitro) const {
    double nitroFraction = percentNitro / 100.0;
    return 11.0 + (NITRO_MAX_CR - 11.0) * nitroFraction;
}

double AlternativeFuelsCalculator::CalculateNitroConsumption(double hp, double percentNitro) const {
    NitromethaneBlend blend = { percentNitro, 0, 100.0 - percentNitro };
    auto props = CalculateNitroProperties(blend);
    
    // Flow (gal/hr) = HP × BSFC / density
    double flowLbHr = hp * props.bsfc;
    double densityLbGal = 7.5; // Aproximado para mistura
    
    return flowLbHr / densityLbGal; // gal/hr
}

double AlternativeFuelsCalculator::EstimateNitroPowerGain(double baseHP, double percentNitro) const {
    // Nitro permite CR muito alta e queima mais rápido
    // Ganho estimado: 15-30% dependendo da %
    double gainPercent = (percentNitro / 100.0) * 25.0; // Até 25% ganho
    return baseHP * (1.0 + gainPercent / 100.0) - baseHP;
}

// ============================================================================
// METANOL (CH3OH)
// ============================================================================

double AlternativeFuelsCalculator::CalculateMethanolAFR(double percentMethanol) const {
    double methanolFraction = percentMethanol / 100.0;
    return GASOLINE_STOICH_AFR * (1.0 - methanolFraction) +
           METHANOL_STOICH_AFR * methanolFraction;
}

double AlternativeFuelsCalculator::CalculateOctaneBoost(double percentMethanol) const {
    // Metanol = 113 RON vs gasolina premium = 95 RON
    double methanolFraction = percentMethanol / 100.0;
    double baseOctane = 95.0;
    double methanolOctane = METHANOL_OCTANE;
    
    return (methanolOctane - baseOctane) * methanolFraction;
}

double AlternativeFuelsCalculator::CalculateCoolingEffect(double percentMethanol, 
                                                          double injectionRate) const {
    // Metanol tem latent heat alto = resfriamento
    // Queda típica: 30-50°C com injeção ativa
    double coolingFactor = (percentMethanol / 100.0) * (injectionRate / 500.0);
    return 40.0 * coolingFactor; // °C
}

// ========== WATER/METHANOL INJECTION ==========

double AlternativeFuelsCalculator::CalculateWaterMethFlow(double hp, double boostPSI) const {
    // SAE 2006-01-3334: Recomendação típica
    // Flow (ml/min) ~ HP * 1.5 para boost >10 PSI
    // Aumenta com boost
    
    if (boostPSI < 5.0) return 0.0; // Não precisa
    
    double baseFlow = hp * 1.5;
    double boostMultiplier = 1.0 + ((boostPSI - 5.0) / 10.0) * 0.3;
    
    return baseFlow * boostMultiplier; // ml/min
}

double AlternativeFuelsCalculator::EstimateWaterMethPowerGain(double baseHP, 
                                                              double boostIncrease) const {
    // Water/Meth permite mais boost e timing
    // Ganho típico: 5-15% dependendo do boost adicional
    double gainPercent = boostIncrease * 0.8; // 0.8% por PSI adicional
    return baseHP * (gainPercent / 100.0);
}

double AlternativeFuelsCalculator::CalculateIATReduction(double flowRate) const {
    // Water/Meth 50/50 evapora = resfriamento
    // Típico: 40-70°F de redução @ 500 ml/min
    return (flowRate / 500.0) * 50.0; // °F
}

MethanolData AlternativeFuelsCalculator::AnalyzeMethanolSystem(const MethanolConfig& config) const {
    MethanolData data = { 0 };
    
    data.requiredAFR = CalculateMethanolAFR(config.methanolPercent);
    data.octaneBoost = CalculateOctaneBoost(config.methanolPercent);
    
    if (config.systemType == MethanolSystemType::WATER_METH_INJ) {
        // Sistema de injeção para turbo
        double baseHP = engine ? engine->CalculateDisplacement() * 0.5 : 200.0;
        data.injectionFlow = CalculateWaterMethFlow(baseHP, config.maxBoost);
        data.coolingEffect = CalculateCoolingEffect(config.methanolPercent, data.injectionFlow);
        data.powerGain = EstimateWaterMethPowerGain(baseHP, 3.0); // +3 PSI típico
    } else {
        // Combustível puro
        data.coolingEffect = CalculateCoolingEffect(config.methanolPercent, 0);
        data.powerGain = config.methanolPercent * 0.1; // Estimativa simples
    }
    
    std::wostringstream rec;
    rec << L"Recomendações para " << config.methanolPercent << L"% Metanol:\n\n";
    rec << L"AFR Target: " << std::fixed << std::setprecision(1) << data.requiredAFR << L":1\n";
    rec << L"Ganho de Octanagem: +" << data.octaneBoost << L" RON\n\n";
    
    if (config.systemType == MethanolSystemType::WATER_METH_INJ) {
        rec << L"Sistema Water/Meth:\n";
        rec << L"  - Vazão necessária: " << data.injectionFlow << L" ml/min\n";
        rec << L"  - Ativação em: " << config.injectionStartBoost << L" PSI\n";
        rec << L"  - Redução IAT: ~" << data.coolingEffect << L"°C\n";
    }
    
    data.recommendations = rec.str();
    
    return data;
}

// ========== TABELAS ==========

std::vector<double> AlternativeFuelsCalculator::GetCommonNitrousJetSizes() const {
    // Tamanhos comerciais de jets (polegadas)
    return { 0.028, 0.032, 0.036, 0.040, 0.044, 0.047, 0.052, 0.056, 0.063, 0.071 };
}

std::vector<AlternativeFuelsCalculator::FuelSpecs> 
AlternativeFuelsCalculator::GetFuelSpecifications() const {
    return {
        { L"Gasolina Premium 95", 14.7, 95.0, 43.4, 11.0, L"Uso diário" },
        { L"Gasolina Premium 98", 14.7, 98.0, 43.4, 11.5, L"Alta performance" },
        { L"E85 (Etanol 85%)", 9.8, 105.0, 26.8, 12.5, L"Flex fuel USA" },
        { L"Etanol E100 (Brasil)", 9.0, 110.0, 21.2, 13.5, L"Hidratado BR" },
        { L"Metanol M100", 6.4, 113.0, 19.9, 14.0, L"Racing/Indy" },
        { L"AvGas 100LL", 14.7, 100.0, 43.5, 12.0, L"Aviação c/ chumbo" },
        { L"VP C12", 14.7, 108.0, 42.8, 12.5, L"Racing sem chumbo" },
        { L"VP C16", 14.7, 117.0, 42.5, 14.0, L"Racing c/ chumbo" },
        { L"VP Q16", 14.7, 116.0, 41.0, 13.5, L"Oxygenated racing" },
        { L"VP MS109", 14.7, 109.0, 43.0, 12.5, L"Street legal racing" },
        { L"VP T4", 14.7, 105.0, 42.8, 12.0, L"Turbo racing" },
        { L"Sunoco Maximal", 14.7, 116.0, 42.5, 13.5, L"Racing premium" },
        { L"Sunoco GT Plus", 14.7, 100.0, 43.2, 11.5, L"Street racing" },
        { L"Nitrometano 20%", 10.5, 105.0, 38.0, 12.0, L"Drag blend" },
        { L"Nitrometano 90%", 2.2, 125.0, 15.0, 16.0, L"Top Fuel" },
        { L"LPG/GNV Propano", 15.5, 108.0, 46.0, 11.5, L"Gás GLP" }
    };
}

// ========== COMBUSTÍVEIS ESPECIAIS ==========

double AlternativeFuelsCalculator::CalculateAvGasProperties(double leadContent) const {
    // AvGas 100LL contém 0.56 g/L de tetraetil de chumbo (TEL)
    // Octanagem: 100 MON mínimo
    // Usado em aviões e alguns carros antigos de corrida
    return 100.0; // MON rating
}

std::vector<AlternativeFuelsCalculator::VPRacingFuel> 
AlternativeFuelsCalculator::GetVPRacingFuels() const {
    return {
        { L"C12", 108.0, 0.710, 0.0, false },      // Sem chumbo
        { L"C16", 117.0, 0.735, 0.0, true },       // Com chumbo
        { L"Q16", 116.0, 0.720, 3.5, false },      // Oxygenated (MTBE)
        { L"MS109", 109.0, 0.730, 0.0, false },    // Street legal
        { L"T4", 105.0, 0.715, 2.0, false },       // Para turbo
        { L"MR12", 112.0, 0.725, 0.0, false },     // Endurance racing
        { L"Import", 100.0, 0.720, 0.0, false }    // Street import
    };
}

double AlternativeFuelsCalculator::CalculateEthanolBlendAFR(double percentEthanol) const {
    // Interpolação entre gasolina (14.7:1) e etanol (9.0:1)
    double ethanolFraction = percentEthanol / 100.0;
    double ethanolAFR = 9.0;
    
    return GASOLINE_STOICH_AFR * (1.0 - ethanolFraction) + 
           ethanolAFR * ethanolFraction;
}

double AlternativeFuelsCalculator::CalculateEthanolOctane(double percentEthanol) const {
    // Etanol puro: ~110 RON
    // Gasolina: ~95 RON
    // Não é linear, mas aproximação é aceitável
    double ethanolFraction = percentEthanol / 100.0;
    double baseOctane = 95.0;
    double ethanolOctane = 110.0;
    
    return baseOctane + (ethanolOctane - baseOctane) * ethanolFraction;
}

std::wstring AlternativeFuelsCalculator::CompareAllFuels() const {
    std::wostringstream comparison;
    
    comparison << L"\n";
    comparison << L"===============================================================================\n";
    comparison << L"                      COMPARADOR DE COMBUSTIVEIS                               \n";
    comparison << L"===============================================================================\n\n";
    
    auto fuels = GetFuelSpecifications();
    
    comparison << L"Combustivel                AFR    OCT   Energia  CR Max   Notas\n";
    comparison << L"-------------------------------------------------------------------------------\n";
    
    for (const auto& fuel : fuels) {
        comparison << std::left << std::setw(26) << fuel.name;
        comparison << std::fixed << std::setprecision(1);
        comparison << std::setw(7) << fuel.stoichAFR;
        comparison << std::setw(6) << fuel.octane;
        comparison << std::setw(9) << fuel.energyDensity;
        comparison << std::setw(9) << fuel.maxCR << L":1";
        comparison << fuel.notes << L"\n";
    }
    
    comparison << L"\n===============================================================================\n";
    comparison << L"\nLEGENDA:\n";
    comparison << L"  AFR = Air-Fuel Ratio (estequiometrico)\n";
    comparison << L"  OCT = Octanagem (RON ou equivalente)\n";
    comparison << L"  Energia = MJ/kg\n";
    comparison << L"  CR Max = Taxa de compressao maxima segura\n\n";
    
    comparison << L"RECOMENDACOES POR APLICACAO:\n\n";
    comparison << L"[DRAG RACING]\n";
    comparison << L"  - Nitrometano 90% (Top Fuel)\n";
    comparison << L"  - VP C16 + Nitro (Pro Stock)\n";
    comparison << L"  - E85 + Turbo (Street class)\n\n";
    
    comparison << L"[CIRCUIT RACING]\n";
    comparison << L"  - VP C12 ou Sunoco Maximal\n";
    comparison << L"  - Metanol (endurance)\n";
    comparison << L"  - E85 (custo-beneficio)\n\n";
    
    comparison << L"[STREET PERFORMANCE]\n";
    comparison << L"  - Premium 98 (stock)\n";
    comparison << L"  - E85 (flex fuel)\n";
    comparison << L"  - VP MS109 (racing legal)\n\n";
    
    comparison << L"[AVIACAO]\n";
    comparison << L"  - AvGas 100LL obrigatorio\n\n";
    
    return comparison.str();
}
