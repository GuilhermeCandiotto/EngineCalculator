#include "EngineDataManager.h"
#include <algorithm>

// Inicialização do singleton
EngineDataManager* EngineDataManager::instance = nullptr;

// ============================================================================
// EngineProject
// ============================================================================

EngineProject::EngineProject() {
    Reset();
}

void EngineProject::Reset() {
    projectName = L"Novo Projeto";
    description = L"";
    author = L"";
    
    // Dados básicos
    basicData.bore = 0.0;
    basicData.stroke = 0.0;
    basicData.cylinders = 4;
    basicData.engineType = EngineType::FOUR_STROKE;
    basicData.rodLength = 0.0;
    basicData.maxRPM = 7000.0;
    
    // Compressão
    compressionData.chamberVolume = 0.0;
    compressionData.pistonDomeVolume = 0.0;
    compressionData.gasketThickness = 1.0;
    compressionData.deckHeight = 0.0;
    compressionData.valveRelief = 0.0;
    compressionData.ivcAngle = 45.0;
    
    // Trem de válvulas
    valveTrainData.intakeValveDiameter = 0.0;
    valveTrainData.exhaustValveDiameter = 0.0;
    valveTrainData.numberOfIntakeValves = 2;
    valveTrainData.numberOfExhaustValves = 2;
    valveTrainData.seatAngle = 45.0;
    valveTrainData.maxLift = 0.0;
    valveTrainData.springRate = 0.0;
    valveTrainData.springPreload = 0.0;
    valveTrainData.springInstalledHeight = 0.0;
    valveTrainData.springCoilBindHeight = 0.0;
    valveTrainData.retainerWeight = 0.0;
    valveTrainData.valveWeight = 0.0;
    valveTrainData.rockerRatio = 1.5;
    
    // Comando
    camshaftData.intakeDuration = 0.0;
    camshaftData.exhaustDuration = 0.0;
    camshaftData.intakeLift = 0.0;
    camshaftData.exhaustLift = 0.0;
    camshaftData.lsa = 110.0;
    camshaftData.advanceRetard = 0.0;
    camshaftData.rockerRatio = 1.5;
    
    // Admissão
    intakeData.runnerLength = 0.0;
    intakeData.runnerDiameter = 0.0;
    intakeData.plenumVolume = 0.0;
    intakeData.trumpetLength = 75.0;
    intakeData.trumpetDiameter = 60.0;
    intakeData.targetRPM = 6000.0;
    intakeData.boostPressure = 0.0;
    
    // Escape
    exhaustData.primaryLength = 0.0;
    exhaustData.primaryDiameter = 0.0;
    exhaustData.secondaryLength = 0.0;
    exhaustData.secondaryDiameter = 0.0;
    exhaustData.collectorDiameter = 0.0;
    exhaustData.targetRPM = 6000.0;
    exhaustData.exhaustDuration = 220.0;
    exhaustData.isFourIntoOne = false;

    // Turbo/Supercharger
    turboData.forcedInductionType = 0;
    turboData.turboType = 0;
    turboData.superchargerType = 0;
    turboData.numberOfTurbos = 1;
    turboData.targetBoostPSI = 0.0;
    turboData.targetHP = 0.0;
    turboData.intercoolerEfficiency = 0.70;
    turboData.intercoolerPressDrop = 1.5;
    turboData.pipingPressDrop = 1.0;
    turboData.filterPressDrop = 0.3;
    turboData.driveRatio = 1.0;
}

bool EngineProject::IsBasicDataComplete() const {
    return basicData.bore > 0.0 && 
           basicData.stroke > 0.0 && 
           basicData.cylinders > 0;
}

// ============================================================================
// EngineDataManager
// ============================================================================

EngineDataManager::EngineDataManager() : dataChanged(false) {
}

EngineDataManager::~EngineDataManager() {
}

EngineDataManager* EngineDataManager::GetInstance() {
    if (instance == nullptr) {
        instance = new EngineDataManager();
    }
    return instance;
}

void EngineDataManager::DestroyInstance() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

void EngineDataManager::UpdateBasicData(const EngineProject::BasicData& data) {
    currentProject.basicData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateCompressionData(const EngineProject::CompressionData& data) {
    currentProject.compressionData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateValveTrainData(const EngineProject::ValveTrainData& data) {
    currentProject.valveTrainData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateCamshaftData(const EngineProject::CamshaftData& data) {
    currentProject.camshaftData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateIntakeData(const EngineProject::IntakeData& data) {
    currentProject.intakeData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateExhaustData(const EngineProject::ExhaustData& data) {
    currentProject.exhaustData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateFuelData(const EngineProject::FuelData& data) {
    currentProject.fuelData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateAlternativeFuelsData(const EngineProject::AlternativeFuelsData& data) {
    currentProject.alternativeFuelsData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::UpdateTurboData(const EngineProject::TurboData& data) {
    currentProject.turboData = data;
    dataChanged = true;
    NotifyDataChanged();
}

void EngineDataManager::NewProject() {
    currentProject.Reset();
    dataChanged = false;
    NotifyProjectLoaded();
}

void EngineDataManager::NotifyDataChanged() {
    for (auto observer : observers) {
        observer->OnProjectDataChanged(currentProject);
    }
}

void EngineDataManager::NotifyProjectLoaded() {
    for (auto observer : observers) {
        observer->OnProjectLoaded(currentProject);
    }
}

void EngineDataManager::RegisterObserver(IProjectObserver* observer) {
    if (observer && std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    }
}

void EngineDataManager::UnregisterObserver(IProjectObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

EngineCore EngineDataManager::CreateEngineCore() const {
    EngineCore engine;
    engine.SetBore(currentProject.basicData.bore);
    engine.SetStroke(currentProject.basicData.stroke);
    engine.SetCylinders(currentProject.basicData.cylinders);
    engine.SetEngineType(currentProject.basicData.engineType);
    engine.SetDeckHeight(currentProject.compressionData.deckHeight);
    return engine;
}
