#pragma once
// ============================================================================
// EngineDataManager.h - Gerenciador Centralizado de Dados do Projeto
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Implementa padrão Singleton e Observer para gerenciar o estado do projeto
// ============================================================================
#include "EngineCore.h"
#include "CompressionCalculator.h"
#include "ValveTrainCalculator.h"
#include "CamshaftCalculator.h"
#include "IntakeExhaustCalculator.h"
#include <string>
#include <vector>
#include <functional>

// Estrutura completa do projeto de motor
struct EngineProject {
    // Nome do projeto
    std::wstring projectName;
    std::wstring description;
    std::wstring author;
    
    // Dados básicos do motor
    struct BasicData {
        double bore;
        double stroke;
        int cylinders;
        EngineType engineType;
        double rodLength;
        double maxRPM;
    } basicData;
    
    // Dados de compressão
    struct CompressionData {
        double chamberVolume;
        double pistonDomeVolume;
        double gasketThickness;
        double deckHeight;
        double valveRelief;
        double ivcAngle;
    } compressionData;
    
    // Dados do trem de válvulas
    struct ValveTrainData {
        double intakeValveDiameter;
        double exhaustValveDiameter;
        int numberOfIntakeValves;
        int numberOfExhaustValves;
        double seatAngle;
        double maxLift;
        double springRate;
        double springPreload;
        double springInstalledHeight;
        double springCoilBindHeight;
        double retainerWeight;
        double valveWeight;
        double rockerRatio;
    } valveTrainData;
    
    // Dados do comando
    struct CamshaftData {
        double intakeDuration;
        double exhaustDuration;
        double intakeLift;
        double exhaustLift;
        double lsa;
        double advanceRetard;
        double rockerRatio;
    } camshaftData;
    
    // Dados de admissão
    struct IntakeData {
        double runnerLength;
        double runnerDiameter;
        double plenumVolume;
        double trumpetLength;
        double trumpetDiameter;
        double targetRPM;
        double boostPressure;
    } intakeData;
    
    // Dados de escape
    struct ExhaustData {
        double primaryLength;
        double primaryDiameter;
        double secondaryLength;
        double secondaryDiameter;
        double collectorDiameter;
        double targetRPM;
        double exhaustDuration;
        bool isFourIntoOne;
    } exhaustData;
    
    // Dados de sistema de combustível
    struct FuelData {
        int systemType;          // 0=Carb, 1=EFI Plenum, 2=EFI ITB, 3=Direct Injection
        double targetHP;
        double boostPressure;
        double fuelPressure;
        double targetAFR;
        int numberOfInjectors;
    } fuelData;
    
    // Dados de combustíveis alternativos
    struct AlternativeFuelsData {
        int nitrousSystemType;   // 0=Dry, 1=Wet, 2=Direct Port
        double nitrousHP;
        double nitrousBottlePressure;
        double nitrometanePercent;
        double methanolPercent;
        double waterMethRatio;
        double waterMethStartBoost;
    } alternativeFuelsData;
    
    // Construtor
    EngineProject();
    
    // Reset
    void Reset();
    
    // Validação
    bool IsBasicDataComplete() const;
};

// Observer pattern para notificar abas de mudanças
class IProjectObserver {
public:
    virtual ~IProjectObserver() {}
    virtual void OnProjectDataChanged(const EngineProject& project) = 0;
    virtual void OnProjectLoaded(const EngineProject& project) = 0;
};

// Gerenciador central de dados do projeto
class EngineDataManager {
private:
    static EngineDataManager* instance;
    EngineProject currentProject;
    std::vector<IProjectObserver*> observers;
    bool dataChanged;
    
    EngineDataManager();
    
public:
    ~EngineDataManager();
    
    // Singleton
    static EngineDataManager* GetInstance();
    static void DestroyInstance();
    
    // Acesso ao projeto
    EngineProject& GetProject() { return currentProject; }
    const EngineProject& GetProject() const { return currentProject; }
    
    // Atualização de dados
    void UpdateBasicData(const EngineProject::BasicData& data);
    void UpdateCompressionData(const EngineProject::CompressionData& data);
    void UpdateValveTrainData(const EngineProject::ValveTrainData& data);
    void UpdateCamshaftData(const EngineProject::CamshaftData& data);
    void UpdateIntakeData(const EngineProject::IntakeData& data);
    void UpdateExhaustData(const EngineProject::ExhaustData& data);
    void UpdateFuelData(const EngineProject::FuelData& data);
    void UpdateAlternativeFuelsData(const EngineProject::AlternativeFuelsData& data);
    
    // Novo projeto
    void NewProject();
    
    // Notificação de mudanças
    void NotifyDataChanged();
    void NotifyProjectLoaded();
    
    // Observer pattern
    void RegisterObserver(IProjectObserver* observer);
    void UnregisterObserver(IProjectObserver* observer);
    
    // Status
    bool HasUnsavedChanges() const { return dataChanged; }
    void SetSaved() { dataChanged = false; }
    void SetChanged() { dataChanged = true; }
    
    // Criar EngineCore a partir dos dados atuais
    EngineCore CreateEngineCore() const;
};
