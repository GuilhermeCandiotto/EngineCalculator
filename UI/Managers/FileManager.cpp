// ============================================================================
// FileManager.cpp - Implementação do Gerenciador de Arquivos
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// ============================================================================
#include "FileManager.h"
#include "../MainWindow.h"
#include "../../Core/ProjectManager.h"
#include "../../Core/EngineDataManager.h"
#include "../../Core/EngineValidator.h"
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma comment(lib, "comdlg32.lib")

FileManager::FileManager(MainWindow* window, HWND hwnd, HINSTANCE hInst)
    : mainWindow(window), hwndMain(hwnd), hInstance(hInst) {
}

FileManager::~FileManager() {
}

bool FileManager::ShowOpenFileDialog(std::wstring& filepath) {
    OPENFILENAMEW ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMain;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Engine Projects (*.ecproj)\0*.ecproj\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameW(&ofn)) {
        filepath = szFile;
        return true;
    }
    
    return false;
}

bool FileManager::ShowSaveFileDialog(std::wstring& filepath) {
    OPENFILENAMEW ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMain;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Engine Projects (*.ecproj)\0*.ecproj\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.lpstrDefExt = L"ecproj";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameW(&ofn)) {
        filepath = szFile;
        return true;
    }
    
    return false;
}

bool FileManager::ConfirmUnsavedChanges() {
    int result = MessageBoxW(
        hwndMain,
        L"O projeto possui alterações não salvas.\n\nDeseja salvar antes de continuar?",
        L"Alterações Não Salvas",
        MB_YESNOCANCEL | MB_ICONQUESTION
    );

    if (result == IDYES) {
        // Salva o projeto
        std::wstring filepath = mainWindow->GetCurrentFilePath();
        if (filepath.empty()) {
            // Sem filepath, mostra dialog de salvar
            if (!ShowSaveFileDialog(filepath)) {
                return false; // Cancelou o dialog
            }
        }
        if (SaveProject(filepath)) {
            mainWindow->SetCurrentFilePath(filepath);
            mainWindow->SetUnsavedChanges(false);
            return true;
        }
        return false; // Falha ao salvar
    } else if (result == IDNO) {
        return true;
    } else {
        return false; // Cancelar
    }
}

bool FileManager::LoadProject(const std::wstring& filepath) {
    ProjectManager pm;
    EngineProject project;
    
    if (pm.LoadProject(filepath, project)) {
        EngineDataManager::GetInstance()->GetProject() = project;
        EngineDataManager::GetInstance()->NotifyProjectLoaded();
        return true;
    } else {
        std::wstring error = L"Erro ao carregar projeto:\n" + pm.GetLastError();
        MessageBoxW(hwndMain, error.c_str(), L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }
}

bool FileManager::SaveProject(const std::wstring& filepath) {
    ProjectManager pm;
    if (pm.SaveProject(filepath, EngineDataManager::GetInstance()->GetProject())) {
        return true;
    } else {
        std::wstring error = L"Erro ao salvar projeto:\n" + pm.GetLastError();
        MessageBoxW(hwndMain, error.c_str(), L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }
}

bool FileManager::FileExists(const std::wstring& filepath) {
    DWORD fileAttr = GetFileAttributesW(filepath.c_str());
    return (fileAttr != INVALID_FILE_ATTRIBUTES);
}

bool FileManager::ShowExportDialog(std::wstring& filepath) {
    OPENFILENAMEW ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMain;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Relatório Texto (*.txt)\0*.txt\0Todos (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn)) {
        filepath = szFile;
        return true;
    }
    return false;
}

bool FileManager::ExportReport(const std::wstring& filepath) {
    const EngineProject& p = EngineDataManager::GetInstance()->GetProject();

    std::wofstream file(filepath);
    if (!file.is_open()) return false;

    // Header
    file << L"================================================================\n";
    file << L"  ENGINE CALCULATOR - RELATORIO COMPLETO DO PROJETO\n";
    file << L"================================================================\n";
    file << L"  Projeto: " << p.projectName << L"\n";

    time_t now = time(nullptr);
    struct tm t;
    localtime_s(&t, &now);
    wchar_t dateBuf[64];
    wcsftime(dateBuf, 64, L"%d/%m/%Y %H:%M", &t);
    file << L"  Data: " << dateBuf << L"\n";
    file << L"================================================================\n\n";

    // Basico
    file << L"--- DADOS BASICOS ---\n";
    file << L"  Bore:       " << p.basicData.bore << L" mm\n";
    file << L"  Stroke:     " << p.basicData.stroke << L" mm\n";
    file << L"  Cilindros:  " << p.basicData.cylinders << L"\n";
    file << L"  Rod Length: " << p.basicData.rodLength << L" mm\n";
    file << L"  RPM Max:    " << (int)p.basicData.maxRPM << L"\n";
    if (p.basicData.bore > 0 && p.basicData.stroke > 0) {
        double disp = 3.14159265 / 4.0 * (p.basicData.bore/10.0) * (p.basicData.bore/10.0) 
                      * (p.basicData.stroke/10.0) * p.basicData.cylinders;
        file << L"  Cilindrada: " << std::fixed << std::setprecision(0) << disp << L" cc\n";
        file << L"  B/S Ratio:  " << std::setprecision(2) << (p.basicData.bore / p.basicData.stroke) << L"\n";
    }
    file << L"\n";

    // Compressao
    file << L"--- COMPRESSAO ---\n";
    file << L"  Vol. Camara:    " << p.compressionData.chamberVolume << L" cc\n";
    file << L"  Vol. Piston:    " << p.compressionData.pistonDomeVolume << L" cc\n";
    file << L"  Esp. Junta:     " << p.compressionData.gasketThickness << L" mm\n";
    file << L"  Deck Height:    " << p.compressionData.deckHeight << L" mm\n";
    file << L"  IVC Angle:      " << p.compressionData.ivcAngle << L" ABDC\n";
    file << L"\n";

    // Comando
    file << L"--- COMANDO DE VALVULAS ---\n";
    file << L"  Dur. Adm @0.050:  " << p.camshaftData.intakeDuration << L" graus\n";
    file << L"  Dur. Esc @0.050:  " << p.camshaftData.exhaustDuration << L" graus\n";
    file << L"  Lift Adm:         " << p.camshaftData.intakeLift << L" mm\n";
    file << L"  Lift Esc:         " << p.camshaftData.exhaustLift << L" mm\n";
    file << L"  LSA:              " << p.camshaftData.lsa << L" graus\n";
    file << L"  Rocker Ratio:     " << p.camshaftData.rockerRatio << L"\n";
    file << L"\n";

    // Admissao
    file << L"--- ADMISSAO ---\n";
    file << L"  Runner Length:    " << p.intakeData.runnerLength << L" mm\n";
    file << L"  Runner Diameter:  " << p.intakeData.runnerDiameter << L" mm\n";
    file << L"  Plenum Volume:    " << p.intakeData.plenumVolume << L" L\n";
    file << L"  RPM Alvo:         " << (int)p.intakeData.targetRPM << L"\n";
    file << L"  Boost:            " << p.intakeData.boostPressure << L" kPa\n";
    file << L"\n";

    // Escape
    file << L"--- ESCAPE ---\n";
    file << L"  Primario:  " << p.exhaustData.primaryLength << L" mm x " << p.exhaustData.primaryDiameter << L" mm\n";
    file << L"  Secundario:" << p.exhaustData.secondaryLength << L" mm x " << p.exhaustData.secondaryDiameter << L" mm\n";
    file << L"  Coletor:   " << p.exhaustData.collectorDiameter << L" mm\n";
    file << L"  Tipo:      " << (p.exhaustData.isFourIntoOne ? L"4-1" : L"4-2-1") << L"\n";
    file << L"\n";

    // Validacao
    file << L"--- VALIDACAO ---\n";
    EngineValidator validator(&p);
    ValidationResult result = validator.ValidateComplete();
    file << L"  Status: " << (result.IsOK() ? L"OK" : L"PROBLEMAS DETECTADOS") << L"\n";
    file << L"  Criticos: " << result.criticalCount 
         << L"  Erros: " << result.errorCount
         << L"  Avisos: " << result.warningCount << L"\n";
    for (size_t i = 0; i < result.issues.size(); i++) {
        const auto& issue = result.issues[i];
        const wchar_t* icon = (issue.severity == ValidationSeverity::CRITICAL) ? L"[!!]" :
                              (issue.severity == ValidationSeverity::VALIDATION_ERROR) ? L"[!]" :
                              (issue.severity == ValidationSeverity::WARNING) ? L"[?]" : L"[i]";
        file << L"  " << icon << L" [" << issue.category << L"] " << issue.message << L"\n";
        if (!issue.recommendation.empty())
            file << L"      -> " << issue.recommendation << L"\n";
    }

    file << L"\n================================================================\n";
    file << L"  Gerado por Engine Calculator v1.0.0\n";
    file << L"================================================================\n";

    file.close();
    return true;
}
