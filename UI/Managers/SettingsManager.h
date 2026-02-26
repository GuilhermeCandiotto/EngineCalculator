#pragma once
// ============================================================================
// SettingsManager.h - Gerenciador de Configurações e Persistência
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// 
// Responsável por salvar/carregar configurações e gerenciar temas
// ============================================================================
#include <windows.h>

// Forward declarations
class MainWindow;
enum class Theme; // Forward declaration do Theme definido em MainWindow.h

class SettingsManager {
private:
    MainWindow* mainWindow;
    HWND hwndMain;
    HINSTANCE hInstance;
    
    // Cores dos temas
    static constexpr COLORREF LIGHT_BG = RGB(240, 240, 240);
    static constexpr COLORREF LIGHT_TEXT = RGB(30, 30, 30);
    static constexpr COLORREF DARK_BG = RGB(45, 45, 48);
    static constexpr COLORREF DARK_TEXT = RGB(220, 220, 220);
    
    // Auto-save
    static constexpr UINT_PTR AUTOSAVE_TIMER_ID = 1;
    static constexpr UINT AUTOSAVE_INTERVAL_MS = 5 * 60 * 1000; // 5 minutos
    
    Theme currentTheme;
    COLORREF backgroundColor;
    COLORREF textColor;
    HBRUSH hBackgroundBrush;
    
public:
    SettingsManager(MainWindow* window, HWND hwnd, HINSTANCE hInst);
    ~SettingsManager();
    
    // Persistência de janela
    void SaveWindowSettings();
    void LoadWindowSettings(int& x, int& y, int& width, int& height, bool& maximized, Theme& theme);
    
    // Temas
    void ApplyTheme(Theme theme);
    void RefreshUI();
    Theme GetCurrentTheme() const { return currentTheme; }
    COLORREF GetBackgroundColor() const { return backgroundColor; }
    COLORREF GetTextColor() const { return textColor; }
    HBRUSH GetBackgroundBrush() const { return hBackgroundBrush; }
    
    // Auto-Save
    void InitializeAutoSave();
    void StartAutoSaveTimer();
    void StopAutoSaveTimer();
    void PerformAutoSave();
    void CheckForAutosaveRecovery();
    void CreateBackupFile();
    void CleanupAutosaveFile();
};
