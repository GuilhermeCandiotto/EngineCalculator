#pragma once
// ============================================================================
// MainWindow.h - Janela Principal da Aplicação (REFATORADA - FASE 2)
// Engine Calculator v1.0.0 - Março 2026
// 
// Interface Win32 com delegação para Managers especializados
// ============================================================================
#include <windows.h>
#include <commctrl.h>
#include "TabPages.h"
#include "../Core/EngineCore.h"
#include "../Core/EnginePresets.h"
#include <vector>
#include <string>

// Forward declarations dos Managers
class MenuManager;
class StatusBarManager;
class TabManager;
class FileManager;
class SettingsManager;

// IDs do Menu (movidos para MenuManager.h, mas mantidos aqui para compatibilidade)
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
#define IDM_FILE_EXPORT         5022

// ========== ENUM DE TEMAS ==========
enum class Theme {
    LIGHT,
    DARK
};


class MainWindow {
private:
    HWND hwndMain;
    HINSTANCE hInstance;
    HACCEL hAccelTable;
    
    // ========== MANAGERS (FASE 2 Refatoração) ==========
    MenuManager* menuManager;
    StatusBarManager* statusBarManager;
    TabManager* tabManager;
    FileManager* fileManager;
    SettingsManager* settingsManager;
    
    // Engine Presets Manager
    EnginePresetsManager* presetsManager;
    
    // Controles da barra de ferramentas
    HWND btnCalculate;
    HWND btnClear;
    HWND btnValidate;
    
    // Estado do projeto
    std::wstring currentFilePath;
    bool hasUnsavedChanges;
    
    // Sistema de unidades
    UnitSystem currentUnitSystem;
    
    // Flag para detectar estado maximizado
    bool isMaximized;
    
    // ========== TAMANHOS MÍNIMOS E MÁXIMOS DA JANELA ==========
    static constexpr int MIN_WINDOW_WIDTH = 800;
    static constexpr int MIN_WINDOW_HEIGHT = 600;
    
    
    // Handlers do menu (delegam para MenuManager)
    void OnFileNew();
    void OnFileOpen();
    void OnFileSave();
    void OnFileSaveAs();
    void OnFileExport();
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
    
    // Eventos internos
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnCreate();
    void OnSize(int width, int height);
    void OnTabChange();
    void OnCalculate();
    void OnClear();
    void CreateToolbar();
    
public:
    MainWindow(HINSTANCE hInst);
    ~MainWindow();
    
    bool Create(int nCmdShow);
    static const wchar_t* GetWindowClassName() { return L"EngineCalculatorMainWindow"; }
    
    // Getters
    HWND GetHandle() const { return hwndMain; }
    HACCEL GetAcceleratorTable() const { return hAccelTable; }
    
    // Gerenciamento de estado
    void MarkAsChanged();
    bool HasUnsavedChanges() const { return hasUnsavedChanges; }
    void SetUnsavedChanges(bool value) { hasUnsavedChanges = value; }
    
    // Acesso aos managers (para callbacks)
    MenuManager* GetMenuManager() { return menuManager; }
    StatusBarManager* GetStatusBarManager() { return statusBarManager; }
    TabManager* GetTabManager() { return tabManager; }
    FileManager* GetFileManager() { return fileManager; }
    SettingsManager* GetSettingsManager() { return settingsManager; }
    
    // Acesso ao filepath atual
    const std::wstring& GetCurrentFilePath() const { return currentFilePath; }
    void SetCurrentFilePath(const std::wstring& path) { currentFilePath = path; }
};
