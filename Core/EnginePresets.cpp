#include "EnginePresets.h"
#include "JSONDataLoader.h"
#include <Windows.h>
#include <shlwapi.h>
#include <fstream>
#include <algorithm>
#include <set>

#pragma comment(lib, "Shlwapi.lib")

// ============================================================================
// CONSTRUTOR E INICIALIZAÇÃO
// ============================================================================

EnginePresetsManager::EnginePresetsManager() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        exeDir = exeDir.substr(0, pos + 1);
    }

    // Tenta encontrar Data/Engines/ com JSONs em varios locais
    // Prioriza o diretório com MAIS arquivos JSON (evita pegar dir vazio/exemplo)
    const wchar_t* suffixes[] = {
        L"Data\\Engines",
        L"..\\Data\\Engines",
        L"..\\..\\Data\\Engines",
        L"..\\..\\..\\Data\\Engines",
    };

    int bestCount = 0;
    std::wstring bestDir;
    std::wstring bestCompDir;

    for (const auto& suffix : suffixes) {
        wchar_t resolved[MAX_PATH];
        std::wstring candidate = exeDir + suffix;
        DWORD len = GetFullPathNameW(candidate.c_str(), MAX_PATH, resolved, NULL);
        if (len == 0 || len >= MAX_PATH) continue;

        std::wstring dir = resolved;
        if (dir.back() != L'\\') dir += L'\\';

        // Conta quantos JSONs tem neste diretorio
        int jsonCount = 0;
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW((dir + L"*.json").c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do { jsonCount++; } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }

        if (jsonCount > bestCount) {
            bestCount = jsonCount;
            bestDir = dir;

            size_t engPos = dir.rfind(L"\\Engines\\");
            if (engPos != std::wstring::npos) {
                bestCompDir = dir.substr(0, engPos) + L"\\Components\\";
            }
        }
    }

    if (bestCount > 0) {
        enginesDirectory = bestDir;
        componentsDirectory = bestCompDir;
    } else {
        enginesDirectory = exeDir + L"Data\\Engines\\";
        componentsDirectory = exeDir + L"Data\\Components\\";
    }

    InitializeEnginePresets();
    InitializeComponentDatabase();
}

EnginePresetsManager::EnginePresetsManager(const std::wstring& enginesDir, const std::wstring& componentsDir) 
    : enginesDirectory(enginesDir), componentsDirectory(componentsDir) {
    InitializeEnginePresets();
    InitializeComponentDatabase();
}

void EnginePresetsManager::InitializeEnginePresets() {
    enginePresets.clear();
    
    // Carrega todos os motores do banco de dados JSON
    enginePresets = JSONDataLoader::LoadAllEngines(enginesDirectory);
}

void EnginePresetsManager::InitializeComponentDatabase() {
    componentDatabase.clear();
    
    // Carrega componentes de JSON
    componentDatabase = JSONDataLoader::LoadAllComponents(componentsDirectory);
}

void EnginePresetsManager::ReloadData() {
    InitializeEnginePresets();
    InitializeComponentDatabase();
}

// ============================================================================
// MÉTODOS DE BUSCA E LISTAGEM
// ============================================================================

std::vector<EnginePreset> EnginePresetsManager::GetAllPresets() const {
    return enginePresets;
}

std::vector<EnginePreset> EnginePresetsManager::GetPresetsByCategory(const std::wstring& category) const {
    std::vector<EnginePreset> result;
    
    // Converte categoria de busca para lowercase para comparação case-insensitive
    std::wstring categoryLower = category;
    std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::towlower);
    
    for (const auto& preset : enginePresets) {
        // Converte categoria do preset para lowercase
        std::wstring presetCategoryLower = preset.category;
        std::transform(presetCategoryLower.begin(), presetCategoryLower.end(), presetCategoryLower.begin(), ::towlower);
        
        // Compara case-insensitive
        if (presetCategoryLower == categoryLower) {
            result.push_back(preset);
        }
    }
    
    return result;
}

EnginePreset EnginePresetsManager::GetPresetByName(const std::wstring& name) const {
    for (const auto& preset : enginePresets) {
        if (preset.name == name) {
            return preset;
        }
    }
    
    // Retorna preset vazio se não encontrar
    return EnginePreset();
}

std::vector<std::wstring> EnginePresetsManager::GetCategories() const {
    // Extrai categorias unicas dos presets carregados
    std::vector<std::wstring> categories;
    std::set<std::wstring> seen;

    for (const auto& preset : enginePresets) {
        if (!preset.category.empty() && seen.find(preset.category) == seen.end()) {
            seen.insert(preset.category);
            categories.push_back(preset.category);
        }
    }

    // Ordena alfabeticamente
    std::sort(categories.begin(), categories.end());

    return categories;
}

std::vector<ComponentPreset> EnginePresetsManager::GetComponentsByCategory(const std::wstring& category) const {
    auto it = componentDatabase.find(category);
    if (it != componentDatabase.end()) {
        return it->second;
    }
    
    return std::vector<ComponentPreset>();
}

std::vector<ComponentPreset> EnginePresetsManager::GetComponentsForEngine(const std::wstring& engineName) const {
    std::vector<ComponentPreset> result;
    
    for (const auto& pair : componentDatabase) {
        for (const auto& component : pair.second) {
            // Busca se o motor está na aplicação do componente
            if (component.application.find(engineName) != std::wstring::npos) {
                result.push_back(component);
            }
        }
    }
    
    return result;
}

bool EnginePresetsManager::ApplyPreset(const std::wstring& presetName, EngineProject& project) {
    EnginePreset preset = GetPresetByName(presetName);
    
    if (preset.name.empty()) {
        return false;  // Preset não encontrado
    }
    
    // Copia todos os dados do preset para o projeto
    project = preset.projectData;
    
    return true;
}

int EnginePresetsManager::GetComponentCount() const {
    int total = 0;
    for (const auto& pair : componentDatabase) {
        total += (int)pair.second.size();
    }
    return total;
}

// ============================================================================
// CRIAR ARQUIVO DE EXEMPLO
// ============================================================================

void EnginePresetsManager::CreateExampleEngineFile() const {
    std::wstring exampleFile = enginesDirectory + L"example_engines.json";
    
    // Cria um arquivo JSON de exemplo com alguns motores famosos
    std::wofstream file(exampleFile);
    if (!file.is_open()) return;
    
    file << L"{\n";
    file << L"  \"fileFormat\": \"Engine Presets Database\",\n";
    file << L"  \"version\": \"1.0.0\",\n";
    file << L"  \"lastUpdated\": \"2026-02-08\",\n";
    file << L"  \"engines\": [\n";
    file << L"    {\n";
    file << L"      \"name\": \"Honda K20A (Civic Type R EP3)\",\n";
    file << L"      \"description\": \"Motor aspirado de alta rotação com VTEC. Famoso pela confiabilidade e potencial de preparação.\",\n";
    file << L"      \"application\": \"Honda Civic Type R EP3 (2001-2005)\",\n";
    file << L"      \"category\": \"Japanese N/A\",\n";
    file << L"      \"factoryHP\": 200,\n";
    file << L"      \"factoryTorque\": 206,\n";
    file << L"      \"factoryRPM\": 7400,\n";
    file << L"      \"fuelType\": \"Gasolina 95 RON\",\n";
    file << L"      \"hasForcedInduction\": false,\n";
    file << L"      \"tags\": [\"Honda\", \"VTEC\", \"Aspirado\", \"Alta Rotação\"],\n";
    file << L"      \"basicData\": {\n";
    file << L"        \"bore\": 86.0,\n";
    file << L"        \"stroke\": 86.0,\n";
    file << L"        \"cylinders\": 4,\n";
    file << L"        \"engineType\": \"FOUR_STROKE\",\n";
    file << L"        \"rodLength\": 152.0,\n";
    file << L"        \"maxRPM\": 8600\n";
    file << L"      },\n";
    file << L"      \"compressionData\": {\n";
    file << L"        \"chamberVolume\": 44.0,\n";
    file << L"        \"pistonDomeVolume\": 0.0,\n";
    file << L"        \"gasketThickness\": 1.0,\n";
    file << L"        \"deckHeight\": 0.5,\n";
    file << L"        \"valveRelief\": 5.0,\n";
    file << L"        \"ivcAngle\": 45.0\n";
    file << L"      },\n";
    file << L"      \"camshaftData\": {\n";
    file << L"        \"intakeDuration\": 250.0,\n";
    file << L"        \"exhaustDuration\": 244.0,\n";
    file << L"        \"intakeLift\": 11.5,\n";
    file << L"        \"exhaustLift\": 10.8,\n";
    file << L"        \"lsa\": 110.0,\n";
    file << L"        \"advanceRetard\": 0.0,\n";
    file << L"        \"rockerRatio\": 1.4\n";
    file << L"      },\n";
    file << L"      \"intakeData\": {\n";
    file << L"        \"runnerLength\": 280.0,\n";
    file << L"        \"runnerDiameter\": 42.0,\n";
    file << L"        \"plenumVolume\": 3.5,\n";
    file << L"        \"trumpetLength\": 0.0,\n";
    file << L"        \"trumpetDiameter\": 0.0,\n";
    file << L"        \"targetRPM\": 7500,\n";
    file << L"        \"boostPressure\": 0.0\n";
    file << L"      },\n";
    file << L"      \"exhaustData\": {\n";
    file << L"        \"primaryLength\": 750.0,\n";
    file << L"        \"primaryDiameter\": 38.0,\n";
    file << L"        \"secondaryLength\": 450.0,\n";
    file << L"        \"secondaryDiameter\": 60.0,\n";
    file << L"        \"collectorDiameter\": 70.0,\n";
    file << L"        \"targetRPM\": 7500,\n";
    file << L"        \"exhaustDuration\": 244.0,\n";
    file << L"        \"isFourIntoOne\": true\n";
    file << L"      }\n";
    file << L"    },\n";
    file << L"    {\n";
    file << L"      \"name\": \"Toyota 2JZ-GTE (Supra MK4)\",\n";
    file << L"      \"description\": \"Lendário motor turbo inline-6. Conhecido por aguentar 1000+ HP com preparação.\",\n";
    file << L"      \"application\": \"Toyota Supra MK4 (1993-2002)\",\n";
    file << L"      \"category\": \"Japanese Turbo\",\n";
    file << L"      \"factoryHP\": 330,\n";
    file << L"      \"factoryTorque\": 441,\n";
    file << L"      \"factoryRPM\": 5600,\n";
    file << L"      \"fuelType\": \"Gasolina 95 RON\",\n";
    file << L"      \"hasForcedInduction\": true,\n";
    file << L"      \"tags\": [\"Toyota\", \"Turbo\", \"2JZ\", \"Supra\"],\n";
    file << L"      \"basicData\": {\n";
    file << L"        \"bore\": 86.0,\n";
    file << L"        \"stroke\": 86.0,\n";
    file << L"        \"cylinders\": 6,\n";
    file << L"        \"engineType\": \"FOUR_STROKE\",\n";
    file << L"        \"rodLength\": 142.0,\n";
    file << L"        \"maxRPM\": 7000\n";
    file << L"      },\n";
    file << L"      \"compressionData\": {\n";
    file << L"        \"chamberVolume\": 55.0,\n";
    file << L"        \"pistonDomeVolume\": 0.0,\n";
    file << L"        \"gasketThickness\": 1.2,\n";
    file << L"        \"deckHeight\": 1.0,\n";
    file << L"        \"valveRelief\": 3.0,\n";
    file << L"        \"ivcAngle\": 50.0\n";
    file << L"      },\n";
    file << L"      \"camshaftData\": {\n";
    file << L"        \"intakeDuration\": 232.0,\n";
    file << L"        \"exhaustDuration\": 236.0,\n";
    file << L"        \"intakeLift\": 9.7,\n";
    file << L"        \"exhaustLift\": 9.4,\n";
    file << L"        \"lsa\": 112.0,\n";
    file << L"        \"advanceRetard\": 0.0,\n";
    file << L"        \"rockerRatio\": 1.3\n";
    file << L"      },\n";
    file << L"      \"intakeData\": {\n";
    file << L"        \"runnerLength\": 350.0,\n";
    file << L"        \"runnerDiameter\": 48.0,\n";
    file << L"        \"plenumVolume\": 5.3,\n";
    file << L"        \"trumpetLength\": 0.0,\n";
    file << L"        \"trumpetDiameter\": 0.0,\n";
    file << L"        \"targetRPM\": 5500,\n";
    file << L"        \"boostPressure\": 96.5\n";
    file << L"      },\n";
    file << L"      \"exhaustData\": {\n";
    file << L"        \"primaryLength\": 650.0,\n";
    file << L"        \"primaryDiameter\": 42.0,\n";
    file << L"        \"secondaryLength\": 400.0,\n";
    file << L"        \"secondaryDiameter\": 65.0,\n";
    file << L"        \"collectorDiameter\": 76.0,\n";
    file << L"        \"targetRPM\": 5500,\n";
    file << L"        \"exhaustDuration\": 236.0,\n";
    file << L"        \"isFourIntoOne\": false\n";
    file << L"      }\n";
    file << L"    }\n";
    file << L"  ]\n";
    file << L"}\n";
    
    file.close();
}
