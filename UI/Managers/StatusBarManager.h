#pragma once
// ============================================================================
// StatusBarManager.h - Gerenciador da Barra de Status
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// 
// Responsável por criar e atualizar a barra de status
// ============================================================================
#include <windows.h>
#include <string>

// Forward declaration
class MainWindow;

class StatusBarManager {
private:
    MainWindow* mainWindow;
    HWND hwndMain;
    HWND hwndStatusBar;
    HINSTANCE hInstance;
    
public:
    StatusBarManager(MainWindow* window, HWND hwnd, HINSTANCE hInst);
    ~StatusBarManager();
    
    // Criação da status bar
    void CreateStatusBar();
    
    // Atualização de mensagens
    void UpdateStatusBar(const wchar_t* text);
    void UpdateTitle(const std::wstring& filepath, bool hasUnsavedChanges);
    
    // Marca projeto como alterado
    void MarkAsChanged();
    
    // Getter
    HWND GetHandle() const { return hwndStatusBar; }
};
