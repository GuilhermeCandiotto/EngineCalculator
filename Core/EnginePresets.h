#pragma once
// ============================================================================
// EnginePresets.h - Presets de Motores Famosos e Banco de Componentes
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Database completo de motores icônicos e componentes reais
// ============================================================================
#include "EngineDataManager.h"
#include <vector>
#include <string>
#include <map>

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

struct EnginePreset {
    std::wstring name;              // Nome do motor (ex: "GM LS3")
    std::wstring description;       // Descrição completa
    std::wstring application;       // Aplicação (ex: "Corvette C6 2008-2013")
    std::wstring category;          // Categoria (Street, Race, Import, etc.)
    
    // Dados técnicos completos
    EngineProject projectData;      // Todos os dados do projeto
    
    // Especificações de fábrica
    double factoryHP;               // Potência de fábrica (HP)
    double factoryTorque;           // Torque de fábrica (lb-ft)
    int factoryRPM;                 // RPM de potência máxima
    std::wstring fuelType;          // Tipo de combustível
    bool hasForcedInduction;        // Turbo/Super?
    
    // Tags para busca
    std::vector<std::wstring> tags;
};

struct ComponentPreset {
    std::wstring name;              // Nome do componente
    std::wstring manufacturer;      // Fabricante
    std::wstring partNumber;        // Número de peça
    std::wstring category;          // Categoria
    std::wstring application;       // Motores compatíveis
    double price;                   // Preço estimado (USD)
    
    // Dados técnicos específicos por tipo
    std::map<std::wstring, double> specs;
};

// ============================================================================
// CLASSE GERENCIADORA DE PRESETS
// ============================================================================

class EnginePresetsManager {
private:
    std::vector<EnginePreset> enginePresets;
    std::map<std::wstring, std::vector<ComponentPreset>> componentDatabase;
    
    // Caminhos dos dados JSON
    std::wstring enginesDirectory;
    std::wstring componentsDirectory;
    
    void InitializeEnginePresets();
    void InitializeComponentDatabase();
    void CreateExampleEngineFile() const;
    
public:
    EnginePresetsManager();
    EnginePresetsManager(const std::wstring& enginesDir, const std::wstring& componentsDir);
    
    // Recarregar dados do disco
    void ReloadData();
    
    // Busca e listagem
    std::vector<EnginePreset> GetAllPresets() const;
    std::vector<EnginePreset> GetPresetsByCategory(const std::wstring& category) const;
    EnginePreset GetPresetByName(const std::wstring& name) const;
    std::vector<std::wstring> GetCategories() const;
    
    // Componentes
    std::vector<ComponentPreset> GetComponentsByCategory(const std::wstring& category) const;
    std::vector<ComponentPreset> GetComponentsForEngine(const std::wstring& engineName) const;
    
    // Aplicação de preset
    bool ApplyPreset(const std::wstring& presetName, EngineProject& project);
    
    // Estatísticas
    int GetEngineCount() const { return (int)enginePresets.size(); }
    int GetComponentCount() const;
    std::wstring GetEnginesDirectory() const { return enginesDirectory; }
};

// ============================================================================
// CATEGORIAS DE MOTORES
// ============================================================================

namespace PresetCategories {
    const wchar_t* const AMERICAN_V8 = L"American V8";
    const wchar_t* const JAPANESE_TURBO = L"Japanese Turbo";
    const wchar_t* const JAPANESE_NA = L"Japanese N/A";
    const wchar_t* const EUROPEAN_PERFORMANCE = L"European Performance";
    const wchar_t* const IMPORT_TUNER = L"Import Tuner";
    const wchar_t* const RACE_ENGINES = L"Race Engines";
}

// ============================================================================
// CATEGORIAS DE COMPONENTES
// ============================================================================

namespace ComponentCategories {
    const wchar_t* const CAMSHAFTS = L"Camshafts";
    const wchar_t* const VALVE_SPRINGS = L"Valve Springs";
    const wchar_t* const PISTONS = L"Pistons";
    const wchar_t* const CONNECTING_RODS = L"Connecting Rods";
    const wchar_t* const CYLINDER_HEADS = L"Cylinder Heads";
    const wchar_t* const INTAKE_MANIFOLDS = L"Intake Manifolds";
    const wchar_t* const EXHAUST_HEADERS = L"Exhaust Headers";
    const wchar_t* const TURBOCHARGERS = L"Turbochargers";
    const wchar_t* const FUEL_INJECTORS = L"Fuel Injectors";
    const wchar_t* const CARBURETORS = L"Carburetors";
}
