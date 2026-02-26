// ============================================================================
// StatusBarManager.cpp - Implementação do Gerenciador da Barra de Status
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// ============================================================================
#include "StatusBarManager.h"
#include "../MainWindow.h"
#include "../../Core/EngineDataManager.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

StatusBarManager::StatusBarManager(MainWindow* window, HWND hwnd, HINSTANCE hInst)
    : mainWindow(window), hwndMain(hwnd), hwndStatusBar(nullptr), hInstance(hInst) {
}

StatusBarManager::~StatusBarManager() {
    // Destruição automática via WM_DESTROY da janela pai
}

void StatusBarManager::CreateStatusBar() {
    hwndStatusBar = CreateWindowExW(
        0,
        STATUSCLASSNAMEW,
        nullptr,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwndMain,
        nullptr,
        hInstance,
        nullptr
    );
    
    UpdateStatusBar(L"Pronto");
}

void StatusBarManager::UpdateStatusBar(const wchar_t* text) {
    if (hwndStatusBar) {
        SendMessageW(hwndStatusBar, SB_SETTEXTW, 0, (LPARAM)text);
    }
}

void StatusBarManager::UpdateTitle(const std::wstring& filepath, bool hasUnsavedChanges) {
    std::wstring title = L"Engine Calculator - ";
    
    if (filepath.empty()) {
        title += EngineDataManager::GetInstance()->GetProject().projectName;
    } else {
        // Extrai só o nome do arquivo
        size_t pos = filepath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            title += filepath.substr(pos + 1);
        } else {
            title += filepath;
        }
    }
    
    if (hasUnsavedChanges) {
        title += L" *";
    }
    
    SetWindowTextW(hwndMain, title.c_str());
}

void StatusBarManager::MarkAsChanged() {
    UpdateStatusBar(L"Projeto modificado");
    // Nota: hasUnsavedChanges precisa ser atualizado no MainWindow
    // Este método é apenas para atualizar a UI
}
