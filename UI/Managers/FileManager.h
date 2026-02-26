#pragma once
// ============================================================================
// FileManager.h - Gerenciador de Arquivos e Dialogs
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// 
// Responsável por dialogs de Open/Save e gerenciamento de arquivos
// ============================================================================
#include <windows.h>
#include <string>

// Forward declaration
class MainWindow;

class FileManager {
private:
    MainWindow* mainWindow;
    HWND hwndMain;
    HINSTANCE hInstance;
    
public:
    FileManager(MainWindow* window, HWND hwnd, HINSTANCE hInst);
    ~FileManager();
    
    // Dialogs
    bool ShowOpenFileDialog(std::wstring& filepath);
    bool ShowSaveFileDialog(std::wstring& filepath);
    bool ConfirmUnsavedChanges();
    
    // Operações de arquivo
    bool LoadProject(const std::wstring& filepath);
    bool SaveProject(const std::wstring& filepath);

    // Exportar relatório
    bool ExportReport(const std::wstring& filepath);
    bool ShowExportDialog(std::wstring& filepath);

    // Validação
    bool FileExists(const std::wstring& filepath);
};
