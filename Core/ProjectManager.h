#pragma once
// ============================================================================
// ProjectManager.h - Gerenciador de Salvar/Carregar Projetos
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Formato: JSON (.ecproj)
// Versão do formato: 1.0
// ============================================================================
#include "EngineDataManager.h"
#include <string>

// Gerenciador de projetos (Save/Load)
class ProjectManager {
private:
    // Converte projeto para JSON string
    std::wstring ProjectToJSON(const EngineProject& project) const;
    
    // Converte JSON string para projeto
    bool JSONToProject(const std::wstring& json, EngineProject& project) const;
    
    // Helpers para escape de strings JSON
    std::wstring EscapeJSON(const std::wstring& str) const;
    std::wstring UnescapeJSON(const std::wstring& str) const;
    
    // Parse de valores JSON simples
    bool ParseJSONString(const std::wstring& json, const std::wstring& key, std::wstring& value) const;
    bool ParseJSONDouble(const std::wstring& json, const std::wstring& key, double& value) const;
    bool ParseJSONInt(const std::wstring& json, const std::wstring& key, int& value) const;
    bool ParseJSONBool(const std::wstring& json, const std::wstring& key, bool& value) const;
    
public:
    ProjectManager();
    ~ProjectManager();
    
    // Salvar projeto
    bool SaveProject(const std::wstring& filename, const EngineProject& project);
    
    // Carregar projeto
    bool LoadProject(const std::wstring& filename, EngineProject& project);
    
    // Verificar se arquivo é válido
    bool IsValidProjectFile(const std::wstring& filename) const;
    
    // Obter último erro
    std::wstring GetLastError() const { return lastError; }
    
private:
    std::wstring lastError;
};
