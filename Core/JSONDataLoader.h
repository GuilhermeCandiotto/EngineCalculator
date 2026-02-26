#pragma once
// ============================================================================
// JSONDataLoader.h - Carregador de Dados JSON
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Carrega presets de motores e componentes de arquivos JSON
// ============================================================================
#include "EnginePresets.h"
#include <fstream>
#include <string>

// Usando nlohmann/json (header-only)
// Download: https://github.com/nlohmann/json/releases
// Apenas incluir json.hpp no projeto
#include "..\packages\nlohmann.json.3.12.0\build\native\include\nlohmann\json.hpp"

using json = nlohmann::json;

class JSONDataLoader {
public:
    // Carrega todos os motores de um arquivo JSON
    static std::vector<EnginePreset> LoadEnginesFromFile(const std::wstring& filepath);
    
    // Carrega componentes de um arquivo JSON
    static std::vector<ComponentPreset> LoadComponentsFromFile(const std::wstring& filepath);
    
    // Carrega todos os motores de um diretório
    static std::vector<EnginePreset> LoadAllEngines(const std::wstring& directory);
    
    // Carrega todos os componentes de um diretório
    static std::map<std::wstring, std::vector<ComponentPreset>> LoadAllComponents(const std::wstring& directory);
    
    // Salva motores em JSON
    static bool SaveEnginesToFile(const std::wstring& filepath, const std::vector<EnginePreset>& engines);
    
    // Salva componentes em JSON
    static bool SaveComponentsToFile(const std::wstring& filepath, const std::vector<ComponentPreset>& components);

private:
    // Helpers de conversão
    static std::string WStringToString(const std::wstring& wstr);
    static std::wstring StringToWString(const std::string& str);
    
    // Parse de um motor individual
    static EnginePreset ParseEngine(const json& engineJson);
    
    // Parse de um componente individual
    static ComponentPreset ParseComponent(const json& compJson);
    
    // Conversão de motor para JSON
    static json EngineToJson(const EnginePreset& preset);
    
    // Conversão de componente para JSON
    static json ComponentToJson(const ComponentPreset& component);
};
