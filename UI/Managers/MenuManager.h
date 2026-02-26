#pragma once
// ============================================================================
// MenuManager.h - Gerenciador de Menu da Aplicação
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// 
// Responsável por criar e gerenciar o menu principal e seus handlers
// ============================================================================
#include <windows.h>
#include <string>
#include <vector>

// IDs do Menu (movidos do MainWindow.h)
#define IDM_FILE_NEW            5001
#define IDM_FILE_OPEN           5002
#define IDM_FILE_SAVE           5003
#define IDM_FILE_SAVEAS         5004
#define IDM_FILE_EXIT           5005
#define IDM_FILE_RECENT_1       5006
#define IDM_FILE_RECENT_2       5007
#define IDM_FILE_RECENT_3       5008
#define IDM_FILE_RECENT_4       5009
#define IDM_FILE_RECENT_5       5010
#define IDM_VIEW_UNITS_METRIC   5011
#define IDM_VIEW_UNITS_IMPERIAL 5012
#define IDM_VIEW_THEME_LIGHT    5013
#define IDM_VIEW_THEME_DARK     5014
#define IDM_VALIDATE_PROJECT    5015
#define IDM_VALIDATE_CURRENT    5016
#define IDM_PRESETS_LOAD        5017
#define IDM_COMPONENTS_BROWSE   5018
#define IDM_HELP_ABOUT          5020
#define IDM_HELP_REFERENCES     5021

// Forward declarations
class MainWindow;
class EnginePresetsManager;

class MenuManager {
private:
    MainWindow* mainWindow;
    HWND hwndMain;
    HINSTANCE hInstance;
    EnginePresetsManager* presetsManager;
    
    // Histórico de arquivos recentes
    static constexpr int MAX_RECENT_FILES = 5;
    std::vector<std::wstring> recentFiles;
    
public:
    MenuManager(MainWindow* window, HWND hwnd, HINSTANCE hInst, EnginePresetsManager* presets);
    ~MenuManager();
    
    // Criação do menu
    void CreateAppMenu();
    void UpdateRecentFilesMenu();
    
    // Handlers de comando
    void OnFileNew();
    void OnFileOpen();
    void OnFileSave();
    void OnFileSaveAs();
    void OnFileExit();
    
    void OnViewUnitsMetric();
    void OnViewUnitsImperial();
    void OnViewThemeLight();
    void OnViewThemeDark();
    
    void OnValidateProject();
    void OnValidateCurrent();
    
    void OnPresetsLoad();
    void OnComponentsBrowse();
    
    void OnHelpAbout();
    void OnHelpReferences();
    
    void OnOpenRecentFile(int recentIndex);
    
    // Gerenciamento de arquivos recentes
    void AddToRecentFiles(const std::wstring& filepath);
    void LoadRecentFiles();
    void SaveRecentFiles();
    void ClearRecentFiles();
    
    // Getters
    const std::vector<std::wstring>& GetRecentFiles() const { return recentFiles; }
};

