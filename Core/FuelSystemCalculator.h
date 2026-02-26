#pragma once
// ============================================================================
// FuelSystemCalculator.h - Calculadora de Sistema de Combustível
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Cálculos para carburadores, injetores, bombas e jatos
// Referências:
// - SAE J1832 - Fuel Injector Sizing
// - Holley Carburetor Technical Manual
// - Bosch Fuel Injection Systems
// ============================================================================
#include "EngineCore.h"
#include <string>
#include <vector>

// Tipos de sistema de combustível
enum class FuelSystemType {
    CARBURETOR,         // Carburador (mecânico)
    EFI_PLENUM,         // EFI com plenum (MPFI - Multi-Point Fuel Injection)
                        // Uma entrada de ar -> plenum -> runners -> cilindros
    EFI_ITB,            // EFI com ITBs (Individual Throttle Bodies)
                        // Borboletas individuais por cilindro (ex: Honda S2000, BMW M3)
    DIRECT_INJECTION    // Injeção direta na câmara (GDI/FSI)
};

// Tipo de admissão (independente do sistema de combustível)
enum class IntakeType {
    SINGLE_THROTTLE,    // Borboleta única + plenum + runners
                        // Ex: Maioria dos carros de produção
    INDIVIDUAL_THROTTLES // ITB - borboletas individuais por cilindro
                        // Ex: Motores de alta performance, motos esportivas
};

// Configuração do sistema de combustível
struct FuelSystemConfig {
    FuelSystemType systemType;
    double targetHP;                // Potência alvo (HP)
    double maxRPM;                  // RPM máximo
    double boostPressure;           // Pressão de boost (PSI) - 0 para aspirado
    double fuelPressure;            // Pressão de combustível (PSI)
    double targetAFR;               // Relação ar/combustível alvo
    int numberOfInjectors;          // Número de injetores (para injeção)
};

// Dados de carburador
struct CarburetorData {
    double cfmRating;               // Rating CFM do carburador
    double venturiDiameter;         // Diâmetro do venturi (mm)
    double mainJetSize;             // Tamanho do jato principal
    double pilotJetSize;            // Tamanho do jato piloto
    double needlePosition;          // Posição da agulha (1-5)
};

// Dados de injeção
struct InjectorData {
    double flowRate;                // Vazão do injetor (cc/min ou lb/hr)
    double dutyCycle;               // Duty cycle (%)
    int numberOfInjectors;          // Número de injetores
    double fuelPressure;            // Pressão de combustível (PSI)
};

// Resultados de análise
struct FuelSystemAnalysis {
    double requiredCFM;             // CFM necessário
    double requiredInjectorFlow;    // Vazão de injetor necessária (cc/min)
    double requiredInjectorFlowLbHr; // Vazão em lb/hr
    double fuelFlowRate;            // Vazão total de combustível (L/h)
    double specificFuelConsumption; // Consumo específico (g/kWh)
    double estimatedRange;          // Autonomia estimada (km) com tanque padrão
    double maxDutyCycle;            // Duty cycle máximo
};

// Classe para cálculos de sistema de combustível
class FuelSystemCalculator {
private:
    const EngineCore* engine;
    
    // Constantes BSFC (Brake Specific Fuel Consumption)
    // Referência: Heywood - "ICE Fundamentals" Figura 3.17
    // SAE 2007-01-1464 - "BSFC Analysis in SI Engines"
    
    // GASOLINA - ASPIRADO
    static constexpr double BSFC_NA_STREET = 0.50;      // lb/HP/hr (304 g/kWh)
                                                         // Motores de producao, CR 9-11:1
    static constexpr double BSFC_NA_RACE = 0.45;        // lb/HP/hr (274 g/kWh)
                                                         // CR alta 12-14:1, tuning otimizado

    // GASOLINA - TURBO/SUPERCHARGED
    static constexpr double BSFC_TURBO_STREET = 0.55;   // lb/HP/hr (335 g/kWh)
                                                         // AFR rico (11.5-12.0:1) para seguranca
    static constexpr double BSFC_TURBO_RACE = 0.50;     // lb/HP/hr (304 g/kWh)
                                                         // AFR controlado, lambda sensor

    // DIESEL
    static constexpr double BSFC_DIESEL_NA = 0.40;      // lb/HP/hr (243 g/kWh)
                                                         // Maior eficiencia termica
    static constexpr double BSFC_DIESEL_TURBO = 0.38;   // lb/HP/hr (231 g/kWh)
                                                         // Melhor eficiencia volumetrica

    // COMBUSTIVEIS ALTERNATIVOS
    static constexpr double BSFC_E85 = 0.68;            // lb/HP/hr (414 g/kWh)
                                                         // 30-40% mais consumo que gasolina
    static constexpr double BSFC_METHANOL = 0.80;       // lb/HP/hr (487 g/kWh)
                                                         // Dobro do consumo de gasolina
    
    // DENSIDADE DE COMBUSTÍVEIS (kg/L)
    static constexpr double GASOLINE_DENSITY = 0.75;    // Gasolina
    static constexpr double DIESEL_DENSITY = 0.85;      // Diesel
    static constexpr double E85_DENSITY = 0.78;         // E85
    static constexpr double METHANOL_DENSITY = 0.79;    // Metanol
    
    // AFR ESTEQUIOMÉTRICO
    static constexpr double STOICH_AFR_GASOLINE = 14.7;  // Gasolina
    static constexpr double STOICH_AFR_DIESEL = 14.5;    // Diesel
    static constexpr double STOICH_AFR_E85 = 9.8;        // E85
    static constexpr double STOICH_AFR_METHANOL = 6.4;   // Metanol
    
public:
    FuelSystemCalculator(const EngineCore* eng);
    ~FuelSystemCalculator();
    
    // ========== CÁLCULOS DE CARBURADOR ==========
    
    // Calcula CFM necessário para o motor
    // Fórmula: CFM = (Displacement × RPM × VE) / (3456 × 2) para 4T
    // Referência: Holley Carburetor Tech
    double CalculateRequiredCFM(double displacement, double maxRPM, 
                                double volumetricEfficiency = 85.0) const;
    
    // Calcula CFM corrigido para boost
    // CFM_boost = CFM_NA × (1 + boost/14.7)
    double CalculateCFMWithBoost(double baseCFM, double boostPSI) const;
    
    // Sugere tamanho de carburador
    // Retorna CFM rating comercial mais próximo
    int SuggestCarburetorSize(double requiredCFM) const;
    
    // Calcula tamanho de jato principal
    // Baseado em CFM e AFR alvo
    double CalculateMainJetSize(double cfm, double targetAFR) const;
    
    // Calcula velocidade do ar no venturi
    // V = (CFM × 16.67) / (área × 60)
    double CalculateVenturiVelocity(double cfm, double venturiDiameter) const;
    
    // ========== CÁLCULOS DE INJEÇÃO ==========
    
    // Calcula vazão de injetor necessária
    // Flow = (HP × BSFC) / (AFR × duty_cycle × num_injectors)
    // Referência: SAE J1832 - Fuel Injector Sizing
    double CalculateRequiredInjectorFlow(double targetHP, double afr, 
                                         double dutyCycle = 80.0, 
                                         int numInjectors = 4) const;
    
    // Converte lb/hr para cc/min
    double LbHrToCCMin(double lbhr) const;
    
    // Converte cc/min para lb/hr
    double CCMinToLbHr(double ccmin) const;
    
    // Calcula duty cycle necessário
    // Para verificar se injetor suporta a potência
    double CalculateRequiredDutyCycle(double targetHP, double injectorFlow, 
                                      double afr, int numInjectors) const;
    
    // Calcula flow corrigido por pressão
    // Flow_new = Flow_base * sqrt(P_new / P_base)
    double CorrectFlowForPressure(double baseFlow, double basePressure, 
                                   double newPressure) const;
    
    // Sugere pressão de combustível ideal
    // Aspirado: 43.5 PSI (3 bar)
    // Turbo: 58 PSI (4 bar) + boost
    double SuggestFuelPressure(double boostPSI) const;
    
    // ========== CÁLCULOS DE BOMBA DE COMBUSTÍVEL ==========
    
    // Calcula vazão de bomba necessária
    // Flow = (HP × BSFC × 1.3) / (AFR × densidade)
    // Fator 1.3 = margem de segurança
    double CalculateRequiredFuelPumpFlow(double targetHP, double afr) const;
    
    // Verifica se bomba suporta pressão + boost
    bool IsPumpAdequate(double pumpFlow, double requiredFlow, 
                        double pumpPressure, double requiredPressure) const;
    
    // ========== CONSUMO DE COMBUSTÍVEL ==========
    
    // Calcula consumo de combustível
    // BSFC = Brake Specific Fuel Consumption
    double CalculateBSFC(double power, bool isTurbo = false, bool isRace = false) const;
    
    // Calcula vazão de combustível (L/h)
    double CalculateFuelFlowRate(double power, double bsfc) const;
    
    // Calcula autonomia estimada
    // Range = (tank_size / fuel_flow) × avg_speed
    double EstimateRange(double fuelFlow, double tankSize = 50.0, 
                         double avgSpeed = 80.0) const;
    
    // Calcula consumo por km
    // km/L ou MPG
    double CalculateFuelEconomy(double fuelFlow, double avgSpeed, bool mpg = false) const;
    
    // ========== ANÁLISE COMPLETA ==========
    
    FuelSystemAnalysis AnalyzeFuelSystem(const FuelSystemConfig& config) const;
    
    // ========== AFR (AIR-FUEL RATIO) ==========
    
    // AFR ideal para diferentes condições
    double GetIdealAFR(bool isIdle) const;
    double GetIdealAFRForPower() const { return 12.5; }      // Máxima potência
    double GetIdealAFRForEconomy() const { return 15.5; }    // Economia
    double GetIdealAFRForCruise() const { return 14.7; }     // Cruzeiro (stoich)
    
    // Lambda (lambda = AFR_real / AFR_stoich)
    double CalculateLambda(double afr) const;
    
    // ========== ANÁLISE DE TIPO DE ADMISSÃO ==========
    
    // Compara ITB vs Plenum
    // ITB: Melhor resposta, maior VE em alta rpm, mais caro
    // Plenum: Melhor torque baixo, mais simples, mais barato
    std::wstring CompareITBvsPlenum(double targetRPM, bool isRaceApplication) const;
    
    // Estima ganho de VE com ITB
    // Baseado em SAE Papers científicos:
    // - SAE 890243: "Effect of Intake System Design" (3-8% ganho alta rpm)
    // - SAE 2008-01-0398: "Individual Runner Optimization" (4-6% ganho)
    // - Bell "Performance Tuning": Melhoria qualitativa >7000 RPM
    // - Blair Chapter 8: Perda de RAM em baixa, ganho alta
    // Valores implementados são CONSERVADORES (limite inferior dos estudos)
    double EstimateVEGainWithITB(double baseVE, double rpm) const;
    
    // ========== VALIDAÇÃO ==========
    
    bool IsValidCarburetorSize(double cfm, double engineCFM) const;
    bool IsValidInjectorSize(double flow, double requiredFlow) const;
    
    std::wstring GetFuelSystemAdvice(const FuelSystemConfig& config, 
                                     const FuelSystemAnalysis& analysis) const;
    
    // ========== TABELAS DE REFERÊNCIA ==========
    
    // Retorna lista de tamanhos comerciais de carburadores (CFM)
    std::vector<int> GetCommonCarburetorSizes() const;
    
    // Retorna lista de vazões comerciais de injetores (cc/min)
    std::vector<int> GetCommonInjectorSizes() const;
};
