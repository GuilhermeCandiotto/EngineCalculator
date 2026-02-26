// ============================================================================
// SettingsManager.cpp - Implementação do Gerenciador de Configurações
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// ============================================================================
#include "SettingsManager.h"
#include "../MainWindow.h"
#include "../../Core/ProjectManager.h"
#include "../../Core/EngineDataManager.h"

// Cores dos temas (constantes locais)
static constexpr COLORREF LIGHT_BG = RGB(240, 240, 240);
static constexpr COLORREF LIGHT_TEXT = RGB(30, 30, 30);
static constexpr COLORREF DARK_BG = RGB(45, 45, 48);
static constexpr COLORREF DARK_TEXT = RGB(220, 220, 220);

SettingsManager::SettingsManager(MainWindow* window, HWND hwnd, HINSTANCE hInst)
    : mainWindow(window), hwndMain(hwnd), hInstance(hInst),
      currentTheme(Theme::LIGHT), backgroundColor(LIGHT_BG), textColor(LIGHT_TEXT),
      hBackgroundBrush(nullptr) {
    
    // Cria brush inicial
    hBackgroundBrush = CreateSolidBrush(backgroundColor);
}

SettingsManager::~SettingsManager() {
    if (hBackgroundBrush) {
        DeleteObject(hBackgroundBrush);
    }
}

void SettingsManager::SaveWindowSettings() {
    if (!hwndMain) return;
    
    WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
    if (!GetWindowPlacement(hwndMain, &wp)) return;
    
    // Chave do Registry: HKEY_CURRENT_USER\Software\EngineCalculator
    HKEY hKey;
    LONG result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\EngineCalculator",
        0, NULL, 0,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL
    );
    
    if (result == ERROR_SUCCESS) {
        // Salva posição X, Y
        RegSetValueExW(hKey, L"WindowX", 0, REG_DWORD, 
                       (BYTE*)&wp.rcNormalPosition.left, sizeof(DWORD));
        RegSetValueExW(hKey, L"WindowY", 0, REG_DWORD, 
                       (BYTE*)&wp.rcNormalPosition.top, sizeof(DWORD));
        
        // Salva largura e altura
        DWORD width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
        DWORD height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
        RegSetValueExW(hKey, L"WindowWidth", 0, REG_DWORD, 
                       (BYTE*)&width, sizeof(DWORD));
        RegSetValueExW(hKey, L"WindowHeight", 0, REG_DWORD, 
                       (BYTE*)&height, sizeof(DWORD));
        
        // Salva se estava maximizada
        DWORD maximized = (wp.showCmd == SW_MAXIMIZE) ? 1 : 0;
        RegSetValueExW(hKey, L"Maximized", 0, REG_DWORD, 
                       (BYTE*)&maximized, sizeof(DWORD));
        
        RegCloseKey(hKey);
    }
}

void SettingsManager::LoadWindowSettings(int& x, int& y, int& width, int& height, bool& maximized, Theme& theme) {
    x = CW_USEDEFAULT;
    y = CW_USEDEFAULT;
    width = 900;
    height = 700;
    maximized = false;
    theme = Theme::LIGHT;
    
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\EngineCalculator",
        0,
        KEY_READ,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        DWORD xVal = 0, yVal = 0, widthVal = 900, heightVal = 700, maxVal = 0, themeValue = 0;
        DWORD dataSize = sizeof(DWORD);
        
        RegQueryValueExW(hKey, L"WindowX", NULL, NULL, (BYTE*)&xVal, &dataSize);
        RegQueryValueExW(hKey, L"WindowY", NULL, NULL, (BYTE*)&yVal, &dataSize);
        RegQueryValueExW(hKey, L"WindowWidth", NULL, NULL, (BYTE*)&widthVal, &dataSize);
        RegQueryValueExW(hKey, L"WindowHeight", NULL, NULL, (BYTE*)&heightVal, &dataSize);
        RegQueryValueExW(hKey, L"Maximized", NULL, NULL, (BYTE*)&maxVal, &dataSize);
        RegQueryValueExW(hKey, L"Theme", NULL, NULL, (BYTE*)&themeValue, &dataSize);
        
        RegCloseKey(hKey);
        
        // Valida se está na tela
        RECT rcWork;
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &rcWork, 0);
        
        if (xVal >= rcWork.left && xVal <= rcWork.right - 100 &&
            yVal >= rcWork.top && yVal <= rcWork.bottom - 100) {
            x = xVal;
            y = yVal;
        }
        
        if (widthVal >= 600 && widthVal <= 3840) {
            width = widthVal;
        }
        if (heightVal >= 400 && heightVal <= 2160) {
            height = heightVal;
        }
        
        maximized = (maxVal == 1);
        theme = (themeValue == 1) ? Theme::DARK : Theme::LIGHT;
    }
}

void SettingsManager::ApplyTheme(Theme theme) {
    currentTheme = theme;
    
    // Define cores baseadas no tema
    if (theme == Theme::LIGHT) {
        backgroundColor = LIGHT_BG;
        textColor = LIGHT_TEXT;
    } else {
        backgroundColor = DARK_BG;
        textColor = DARK_TEXT;
    }
    
    // Recria brush com nova cor
    if (hBackgroundBrush) {
        DeleteObject(hBackgroundBrush);
    }
    hBackgroundBrush = CreateSolidBrush(backgroundColor);
    
    // Salva preferência no Registry
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\EngineCalculator",
                        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD themeValue = (theme == Theme::DARK) ? 1 : 0;
        RegSetValueExW(hKey, L"Theme", 0, REG_DWORD, (BYTE*)&themeValue, sizeof(DWORD));
        RegCloseKey(hKey);
    }
    
    // Atualiza interface
    RefreshUI();
}

void SettingsManager::RefreshUI() {
    if (!hwndMain) return;
    
    // Força redesenho de toda a janela
    InvalidateRect(hwndMain, NULL, TRUE);
    UpdateWindow(hwndMain);
    
    // NOTA: Para redesenhar tabs e botões, precisa acessar membros do MainWindow
    // Isso será implementado via callbacks ou friend class
}

void SettingsManager::InitializeAutoSave() {
    // Implementação futura
}

void SettingsManager::StartAutoSaveTimer() {
    SetTimer(hwndMain, AUTOSAVE_TIMER_ID, AUTOSAVE_INTERVAL_MS, NULL);
}

void SettingsManager::StopAutoSaveTimer() {
    KillTimer(hwndMain, AUTOSAVE_TIMER_ID);
}

void SettingsManager::PerformAutoSave() {
    if (!mainWindow->HasUnsavedChanges()) return;

    // Salva em arquivo temporario de recovery
    std::wstring filepath = mainWindow->GetCurrentFilePath();
    if (filepath.empty()) {
        // Sem filepath definido, salva como recovery
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        filepath = std::wstring(tempPath) + L"EngineCalculator_autosave.ecproj";
    } else {
        // Cria backup ao lado do arquivo original
        filepath += L".autosave";
    }

    ProjectManager pm;
    pm.SaveProject(filepath, EngineDataManager::GetInstance()->GetProject());
}

void SettingsManager::CheckForAutosaveRecovery() {
    // Implementação futura - verificar se existe arquivo de recovery
}

void SettingsManager::CreateBackupFile() {
    // Implementação futura - criar backup antes de salvar
}

void SettingsManager::CleanupAutosaveFile() {
    // Implementação futura - limpar arquivo de auto-save ao salvar com sucesso
}
