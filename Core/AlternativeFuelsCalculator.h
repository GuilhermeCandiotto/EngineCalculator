#pragma once
// ============================================================================
// AlternativeFuelsCalculator.h - Calculadora de Combustíveis Alternativos
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Cálculos para sistemas especiais de combustível:
// - Óxido Nitroso (N2O/NOS)
// - Nitrometano (CH3NO2)
// - Metanol (CH3OH)
// - Water/Methanol Injection
//
// Referências:
// - SAE 2003-01-3242 - Nitrous Oxide Injection Systems
// - SAE 2005-01-0547 - Performance Effects of Nitrous Oxide
// - SAE 2004-01-3514 - Nitromethane Combustion Characteristics
// - SAE 2002-01-2743 - Methanol as Racing Fuel
// - SAE 2006-01-3334 - Water-Methanol Injection
// - NHRA Technical Specifications
// ============================================================================
#include "EngineCore.h"
#include <string>
#include <vector>

// ============================================================================
// ÓXIDO NITROSO (N2O / NOS)
// ============================================================================

// Tipo de sistema de nitro
enum class NitrousSystemType {
    DRY,        // Dry shot - só nitro (precisa enriquecer combustível separado)
    WET,        // Wet shot - nitro + combustível (kit completo)
    DIRECT_PORT // Port injection - injeção direta por cilindro (melhor)
};

// Configuração do sistema de nitro
struct NitrousConfig {
    NitrousSystemType systemType;
    double baseHP;              // HP do motor sem nitro
    double desiredGain;         // Ganho desejado em HP (50, 75, 100, 150, 200+)
    double cylinderPressure;    // Pressão do cilindro de N2O (900-1000 PSI)
    double fuelPressure;        // Pressão da gasolina (6-8 PSI para wet)
    int numberOfCylinders;      // Número de cilindros
    bool hasIntercooler;        // Tem intercooler? (reduz risco)
};

// Dados calculados do sistema
struct NitrousSystemData {
    double nitrousJetSize;      // Tamanho do jet de nitro (ex: 0.040")
    double fuelJetSize;         // Tamanho do jet de combustível (wet only)
    double expectedHPGain;      // Ganho real esperado
    double expectedTorqueGain;  // Ganho de torque
    double maxSafeCR;           // CR máxima segura com nitro
    double intakeTemperatureDrop; // Queda de temperatura (°F)
    double oxygenEnrichment;    // % adicional de O2
    std::wstring safetyWarnings; // Avisos de segurança
};

// ============================================================================
// NITROMETANO (CH3NO2)
// ============================================================================

// Mistura de combustível
struct NitromethaneBlend {
    double percentNitro;        // % de nitrometano (0-100%)
    double percentMethanol;     // % de metanol
    double percentGasoline;     // % de gasolina
};

// Propriedades do nitrometano
struct NitromethaneProperties {
    double stoichAFR;           // AFR estequiométrico
    double energyDensity;       // MJ/kg
    double octaneRating;        // RON equivalente
    double flameSpeed;          // m/s
    double maxCR;               // CR máxima suportada
    double bsfc;                // Consumo específico
};

// ============================================================================
// METANOL (CH3OH) e WATER/METH INJECTION
// ============================================================================

// Tipo de sistema de metanol
enum class MethanolSystemType {
    PURE_METHANOL,      // 100% metanol como combustível principal
    E85_BLEND,          // Mistura etanol (similar ao metanol)
    WATER_METH_INJ      // Injeção de água/metanol (50/50) para turbo
};

// Configuração de metanol
struct MethanolConfig {
    MethanolSystemType systemType;
    double methanolPercent;     // % de metanol no combustível
    double waterPercent;        // % de água (para injeção)
    double injectionStartBoost; // PSI para começar injeção (water/meth)
    double maxBoost;            // Boost máximo
};

// Propriedades calculadas
struct MethanolData {
    double requiredAFR;         // AFR necessário
    double octaneBoost;         // Ganho de octanagem (números)
    double coolingEffect;       // Queda de temperatura (°C)
    double powerGain;           // Ganho de potência estimado (%)
    double injectionFlow;       // Vazão de injeção necessária (ml/min)
    std::wstring recommendations; // Recomendações
};

// ============================================================================
// CALCULADORA PRINCIPAL
// ============================================================================

class AlternativeFuelsCalculator {
private:
    const EngineCore* engine;
    
    // ========== CONSTANTES QUÍMICAS ==========
    
    // Óxido Nitroso
    static constexpr double N2O_DECOMP_TEMP = 300.0;    // °C (decompõe liberando O2)
    static constexpr double N2O_OXYGEN_BOOST = 36.0;    // % adicional de O2
    static constexpr double N2O_TEMP_DROP = 60.0;       // °F de resfriamento
    static constexpr double N2O_HP_PER_10PSI = 3.0;     // HP por 10 PSI de nitro
    
    // Nitrometano
    static constexpr double NITRO_STOICH_AFR = 1.7;     // AFR estequiométrico
    static constexpr double NITRO_ENERGY_DENSITY = 11.3; // MJ/kg
    static constexpr double NITRO_OCTANE = 130.0;       // RON equivalente
    static constexpr double NITRO_MAX_CR = 17.0;        // CR máxima
    
    // Metanol
    static constexpr double METHANOL_STOICH_AFR = 6.4;  // AFR estequiométrico
    static constexpr double METHANOL_ENERGY_DENSITY = 19.9; // MJ/kg
    static constexpr double METHANOL_OCTANE = 113.0;    // RON
    static constexpr double METHANOL_LATENT_HEAT = 1100.0; // kJ/kg (resfriamento)
    
    // Gasolina (referência)
    static constexpr double GASOLINE_STOICH_AFR = 14.7;
    static constexpr double GASOLINE_ENERGY_DENSITY = 43.4; // MJ/kg
    
public:
    AlternativeFuelsCalculator(const EngineCore* eng);
    ~AlternativeFuelsCalculator();
    
    // ========== ÓXIDO NITROSO (N2O/NOS) ==========
    
    // Calcula tamanho do jet de nitro
    // Baseado em HP desejado e pressão do cilindro
    // Referência: SAE 2003-01-3242
    double CalculateNitrousJetSize(double hpGain, double cylinderPressure, 
                                   int numCylinders = 1) const;
    
    // Calcula tamanho do jet de combustível (wet systems)
    // Ratio típico: 1 parte combustível para 6-8 partes nitro
    double CalculateFuelJetSize(double nitrousJetSize, double fuelType = 0) const;
    
    // Estima ganho real de HP com nitro
    // Fórmula: HP_gain = flow_rate × pressure × constant
    // SAE 2005-01-0547: Ganhos típicos 40-200 HP
    double EstimateNitrousHPGain(double jetSize, double pressure) const;
    
    // CR máxima segura com nitro
    // Nitro aumenta pressão - precisa reduzir CR
    // Recomendação: CR <= 9.5:1 para 75+ HP shot
    double MaxSafeCRWithNitrous(double shotSize, bool hasIntercooler) const;
    
    // Queda de temperatura de admissão
    // N2O evapora absorvendo calor (efeito endotérmico)
    double CalculateIntakeTempDrop(double shotSize) const;
    
    // Análise completa do sistema
    NitrousSystemData AnalyzeNitrousSystem(const NitrousConfig& config) const;
    
    // ========== NITROMETANO (CH3NO2) ==========
    
    // Calcula propriedades de uma mistura
    // Pure nitro (100%), blend (20-90%), ou gasoline (0%)
    NitromethaneProperties CalculateNitroProperties(const NitromethaneBlend& blend) const;
    
    // AFR necessário para mistura
    // Interpolação entre gasolina (14.7:1) e nitro puro (1.7:1)
    double CalculateNitroAFR(double percentNitro) const;
    
    // CR máxima suportada
    // Nitro suporta CR muito alta (até 17:1 em Top Fuel)
    double MaxCRForNitro(double percentNitro) const;
    
    // Consumo de combustível
    // Nitro queima muito mais rápido que gasolina
    // Top Fuel: 15 galões em 4 segundos!
    double CalculateNitroConsumption(double hp, double percentNitro) const;
    
    // Ganho de potência estimado
    // Baseado em % de nitro na mistura
    double EstimateNitroPowerGain(double baseHP, double percentNitro) const;
    
    // ========== METANOL (CH3OH) ==========
    
    // AFR ideal para metanol
    // Puro: 6.4:1, Misturas: interpolado
    double CalculateMethanolAFR(double percentMethanol) const;
    
    // Ganho de octanagem
    // Metanol = 113 RON, melhora detonação
    double CalculateOctaneBoost(double percentMethanol) const;
    
    // Efeito de resfriamento (latent heat)
    // Metanol evapora mais que gasolina = mais resfriamento
    double CalculateCoolingEffect(double percentMethanol, double injectionRate) const;
    
    // ========== WATER/METHANOL INJECTION ==========
    
    // Vazão de injeção necessária
    // Baseado em boost e HP
    // Referência: SAE 2006-01-3334
    double CalculateWaterMethFlow(double hp, double boostPSI) const;
    
    // Ganho de potência com injeção
    // Water/Meth permite mais boost e timing
    double EstimateWaterMethPowerGain(double baseHP, double boostIncrease) const;
    
    // Redução de temperatura (IAT)
    // Water/Meth 50/50 reduz temperatura significativamente
    double CalculateIATReduction(double flowRate) const;
    
    // Análise completa
    MethanolData AnalyzeMethanolSystem(const MethanolConfig& config) const;
    
    // ========== COMPARAÇÕES E VALIDAÇÕES ==========
    
    // Compara combustíveis
    std::wstring CompareFuels(bool includeNitrous, bool includeNitro, 
                              bool includeMethanol) const;
    
    // Verifica segurança da configuração
    std::wstring ValidateFuelConfiguration(double currentCR, double boost,
                                          bool hasNitrous, double percentNitro,
                                          double percentMethanol) const;
    
    // Recomendações de setup
    std::wstring GetFuelRecommendations(double targetHP, double currentHP,
                                       bool isStreet, bool isDrag) const;
    
    // ========== TABELAS DE REFERÊNCIA ==========
    
    // Tamanhos comerciais de jets de nitro (polegadas)
    std::vector<double> GetCommonNitrousJetSizes() const;
    
    // Misturas comuns de nitrometano
    std::vector<NitromethaneBlend> GetCommonNitroBlends() const;
    
    // Especificações de combustíveis por categoria
    struct FuelSpecs {
        std::wstring name;
        double stoichAFR;
        double octane;
        double energyDensity;
        double maxCR;
        std::wstring notes;
    };
    
    std::vector<FuelSpecs> GetFuelSpecifications() const;
    
    // ========== COMBUSTÍVEIS ESPECIAIS ==========
    
    // AvGas (Benzina de Aviação)
    double CalculateAvGasProperties(double leadContent = 0.56) const; // g/L de TEL
    
    // VP Racing Fuels
    struct VPRacingFuel {
        std::wstring model; // C12, C16, Q16, MS109, T4
        double octane;
        double specificGravity;
        double oxygenContent; // %
        bool isLeaded;
    };
    std::vector<VPRacingFuel> GetVPRacingFuels() const;
    
    // E85 / Etanol
    double CalculateEthanolBlendAFR(double percentEthanol) const;
    double CalculateEthanolOctane(double percentEthanol) const;
    
    // Comparação de combustíveis
    std::wstring CompareAllFuels() const;
};
