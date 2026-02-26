#include "JSONDataLoader.h"
#include <Windows.h>
#include <codecvt>
#include <locale>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

// ============================================================================
// CONVERSÃO DE STRINGS
// ============================================================================

std::string JSONDataLoader::WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &result[0], size, NULL, NULL);
    return result;
}

std::wstring JSONDataLoader::StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
    return result;
}

// ============================================================================
// CARREGAMENTO DE MOTORES
// ============================================================================

std::vector<EnginePreset> JSONDataLoader::LoadEnginesFromFile(const std::wstring& filepath) {
    std::vector<EnginePreset> result;
    
    try {
        // Lê arquivo
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return result;
        }
        
        // Parse JSON
        json data = json::parse(file);
        
        // Pega categoria do arquivo (nível root)
        std::wstring fileCategory = L"";
        if (data.contains("category")) {
            fileCategory = StringToWString(data["category"].get<std::string>());
        }
        
        // Itera por motores
        if (data.contains("engines") && data["engines"].is_array()) {
            for (const auto& engineJson : data["engines"]) {
                EnginePreset preset = ParseEngine(engineJson);
                
                // Se o motor não tem categoria própria, usa a do arquivo
                if (preset.category.empty() && !fileCategory.empty()) {
                    preset.category = fileCategory;
                }
                
                result.push_back(preset);
            }
        }
        
        file.close();
    }
    catch (...) {
        // Erro ao carregar - retorna vetor vazio
    }
    
    return result;
}

EnginePreset JSONDataLoader::ParseEngine(const json& j) {
    EnginePreset preset;
    
    // Informações básicas
    preset.name = StringToWString(j.value("name", ""));
    preset.description = StringToWString(j.value("description", ""));
    preset.application = StringToWString(j.value("application", ""));
    preset.category = StringToWString(j.value("category", ""));
    preset.factoryHP = j.value("factoryHP", 0.0);
    preset.factoryTorque = j.value("factoryTorque", 0.0);
    preset.factoryRPM = j.value("factoryRPM", 0);
    preset.fuelType = StringToWString(j.value("fuelType", ""));
    preset.hasForcedInduction = j.value("hasForcedInduction", false);
    
    // Tags
    if (j.contains("tags") && j["tags"].is_array()) {
        for (const auto& tag : j["tags"]) {
            preset.tags.push_back(StringToWString(tag.get<std::string>()));
        }
    }
    
    // Dados do projeto
    preset.projectData.projectName = preset.name;
    
    // Basic Data
    if (j.contains("basicData")) {
        const auto& bd = j["basicData"];
        preset.projectData.basicData.bore = bd.value("bore", 0.0);
        preset.projectData.basicData.stroke = bd.value("stroke", 0.0);
        preset.projectData.basicData.cylinders = bd.value("cylinders", 4);
        preset.projectData.basicData.rodLength = bd.value("rodLength", 0.0);
        preset.projectData.basicData.maxRPM = bd.value("maxRPM", 0);
        
        std::string engineType = bd.value("engineType", "FOUR_STROKE");
        preset.projectData.basicData.engineType = (engineType == "TWO_STROKE") ? 
            EngineType::TWO_STROKE : EngineType::FOUR_STROKE;
    }
    
    // Compression Data
    if (j.contains("compressionData")) {
        const auto& cd = j["compressionData"];
        preset.projectData.compressionData.chamberVolume = cd.value("chamberVolume", 0.0);
        preset.projectData.compressionData.pistonDomeVolume = cd.value("pistonDomeVolume", 0.0);
        preset.projectData.compressionData.gasketThickness = cd.value("gasketThickness", 0.0);
        preset.projectData.compressionData.deckHeight = cd.value("deckHeight", 0.0);
        preset.projectData.compressionData.valveRelief = cd.value("valveRelief", 0.0);
        preset.projectData.compressionData.ivcAngle = cd.value("ivcAngle", 0.0);
    }
    
    // Camshaft Data
    if (j.contains("camshaftData")) {
        const auto& cam = j["camshaftData"];
        preset.projectData.camshaftData.intakeDuration = cam.value("intakeDuration", 0.0);
        preset.projectData.camshaftData.exhaustDuration = cam.value("exhaustDuration", 0.0);
        preset.projectData.camshaftData.intakeLift = cam.value("intakeLift", 0.0);
        preset.projectData.camshaftData.exhaustLift = cam.value("exhaustLift", 0.0);
        preset.projectData.camshaftData.lsa = cam.value("lsa", 0.0);
        preset.projectData.camshaftData.advanceRetard = cam.value("advanceRetard", 0.0);
        preset.projectData.camshaftData.rockerRatio = cam.value("rockerRatio", 0.0);
    }
    
    // Intake Data
    if (j.contains("intakeData")) {
        const auto& intake = j["intakeData"];
        preset.projectData.intakeData.runnerLength = intake.value("runnerLength", 0.0);
        preset.projectData.intakeData.runnerDiameter = intake.value("runnerDiameter", 0.0);
        preset.projectData.intakeData.plenumVolume = intake.value("plenumVolume", 0.0);
        preset.projectData.intakeData.trumpetLength = intake.value("trumpetLength", 0.0);
        preset.projectData.intakeData.trumpetDiameter = intake.value("trumpetDiameter", 0.0);
        preset.projectData.intakeData.targetRPM = intake.value("targetRPM", 0);
        preset.projectData.intakeData.boostPressure = intake.value("boostPressure", 0.0);
    }
    
    // Exhaust Data
    if (j.contains("exhaustData")) {
        const auto& exhaust = j["exhaustData"];
        preset.projectData.exhaustData.primaryLength = exhaust.value("primaryLength", 0.0);
        preset.projectData.exhaustData.primaryDiameter = exhaust.value("primaryDiameter", 0.0);
        preset.projectData.exhaustData.secondaryLength = exhaust.value("secondaryLength", 0.0);
        preset.projectData.exhaustData.secondaryDiameter = exhaust.value("secondaryDiameter", 0.0);
        preset.projectData.exhaustData.collectorDiameter = exhaust.value("collectorDiameter", 0.0);
        preset.projectData.exhaustData.targetRPM = exhaust.value("targetRPM", 0);
        preset.projectData.exhaustData.exhaustDuration = exhaust.value("exhaustDuration", 0.0);
        preset.projectData.exhaustData.isFourIntoOne = exhaust.value("isFourIntoOne", true);
    }
    
    return preset;
}

// ============================================================================
// CARREGAMENTO DE COMPONENTES
// ============================================================================

std::vector<ComponentPreset> JSONDataLoader::LoadComponentsFromFile(const std::wstring& filepath) {
    std::vector<ComponentPreset> result;
    
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return result;
        }
        
        json data = json::parse(file);
        
        if (data.contains("components") && data["components"].is_array()) {
            for (const auto& compJson : data["components"]) {
                result.push_back(ParseComponent(compJson));
            }
        }
        
        file.close();
    }
    catch (...) {
        // Erro - retorna vetor vazio
    }
    
    return result;
}

ComponentPreset JSONDataLoader::ParseComponent(const json& j) {
    ComponentPreset comp;
    
    comp.name = StringToWString(j.value("name", ""));
    comp.manufacturer = StringToWString(j.value("manufacturer", ""));
    comp.partNumber = StringToWString(j.value("partNumber", ""));
    comp.category = StringToWString(j.value("category", ""));
    comp.application = StringToWString(j.value("application", ""));
    comp.price = j.value("price", 0.0);
    
    // Specs (mapa de especificações)
    if (j.contains("specs") && j["specs"].is_object()) {
        for (auto& [key, value] : j["specs"].items()) {
            comp.specs[StringToWString(key)] = value.get<double>();
        }
    }
    
    return comp;
}

// ============================================================================
// CARREGAMENTO DE DIRETÓRIOS COMPLETOS
// ============================================================================

std::vector<EnginePreset> JSONDataLoader::LoadAllEngines(const std::wstring& directory) {
    std::vector<EnginePreset> allEngines;

    try {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            return allEngines;
        }

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.path().extension() == ".json") {
                auto engines = LoadEnginesFromFile(entry.path().wstring());
                allEngines.insert(allEngines.end(), engines.begin(), engines.end());
            }
        }
    }
    catch (const std::exception&) {
        // Parse ou filesystem error
    }
    catch (...) {
        // Erro desconhecido
    }

    return allEngines;
}

std::map<std::wstring, std::vector<ComponentPreset>> JSONDataLoader::LoadAllComponents(const std::wstring& directory) {
    std::map<std::wstring, std::vector<ComponentPreset>> allComponents;
    
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.path().extension() == ".json") {
                std::wstring filename = entry.path().stem().wstring();
                auto components = LoadComponentsFromFile(entry.path().wstring());
                
                if (!components.empty()) {
                    allComponents[filename] = components;
                }
            }
        }
    }
    catch (...) {
        // Diretório não existe ou erro
    }
    
    return allComponents;
}

// ============================================================================
// SALVAMENTO (EXPORT)
// ============================================================================

bool JSONDataLoader::SaveEnginesToFile(const std::wstring& filepath, const std::vector<EnginePreset>& engines) {
    try {
        json data;
        data["version"] = "1.0.0";
        data["lastUpdated"] = "2026-02-15";
        data["engines"] = json::array();
        
        for (const auto& engine : engines) {
            data["engines"].push_back(EngineToJson(engine));
        }
        
        std::ofstream file(filepath);
        file << data.dump(4); // Pretty print com indentação 4
        file.close();
        
        return true;
    }
    catch (...) {
        return false;
    }
}

json JSONDataLoader::EngineToJson(const EnginePreset& preset) {
    json j;
    
    j["name"] = WStringToString(preset.name);
    j["description"] = WStringToString(preset.description);
    j["application"] = WStringToString(preset.application);
    j["factoryHP"] = preset.factoryHP;
    j["factoryTorque"] = preset.factoryTorque;
    j["factoryRPM"] = preset.factoryRPM;
    j["fuelType"] = WStringToString(preset.fuelType);
    j["hasForcedInduction"] = preset.hasForcedInduction;
    
    // Tags
    j["tags"] = json::array();
    for (const auto& tag : preset.tags) {
        j["tags"].push_back(WStringToString(tag));
    }
    
    // Basic Data
    j["basicData"] = {
        {"bore", preset.projectData.basicData.bore},
        {"stroke", preset.projectData.basicData.stroke},
        {"cylinders", preset.projectData.basicData.cylinders},
        {"engineType", "FOUR_STROKE"},
        {"rodLength", preset.projectData.basicData.rodLength},
        {"maxRPM", preset.projectData.basicData.maxRPM}
    };
    
    // Compression Data
    j["compressionData"] = {
        {"chamberVolume", preset.projectData.compressionData.chamberVolume},
        {"pistonDomeVolume", preset.projectData.compressionData.pistonDomeVolume},
        {"gasketThickness", preset.projectData.compressionData.gasketThickness},
        {"deckHeight", preset.projectData.compressionData.deckHeight},
        {"valveRelief", preset.projectData.compressionData.valveRelief},
        {"ivcAngle", preset.projectData.compressionData.ivcAngle}
    };
    
    // Outros dados...
    
    return j;
}

json JSONDataLoader::ComponentToJson(const ComponentPreset& component) {
    json j;
    
    j["name"] = WStringToString(component.name);
    j["manufacturer"] = WStringToString(component.manufacturer);
    j["partNumber"] = WStringToString(component.partNumber);
    j["application"] = WStringToString(component.application);
    j["price"] = component.price;
    
    j["specs"] = json::object();
    for (const auto& [key, value] : component.specs) {
        j["specs"][WStringToString(key)] = value;
    }
    
    return j;
}
