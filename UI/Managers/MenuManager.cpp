// ============================================================================
// MenuManager.cpp - Implementação do Gerenciador de Menu
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// ============================================================================
#include "MenuManager.h"
#include "FileManager.h"
#include "TabManager.h"
#include "StatusBarManager.h"
#include "SettingsManager.h"
#include "../MainWindow.h"
#include "../EnginePresetsDialog.h"
#include "../../Core/EngineDataManager.h"
#include "../../Core/ProjectManager.h"
#include "../../Core/EngineValidator.h"
#include "../../Core/EnginePresets.h"
#include <commdlg.h>
#include <algorithm>

MenuManager::MenuManager(MainWindow* window, HWND hwnd, HINSTANCE hInst, EnginePresetsManager* presets)
    : mainWindow(window), hwndMain(hwnd), hInstance(hInst), presetsManager(presets) {
    LoadRecentFiles();
}

MenuManager::~MenuManager() {
}

void MenuManager::CreateAppMenu() {
    HMENU hMenuBar = ::CreateMenu();
    
    // Menu Arquivo
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_NEW, L"&Novo Projeto\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Abrir...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_SAVE, L"&Salvar\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_SAVEAS, L"Salvar &Como...");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_EXPORT, L"&Exportar Relatorio...\tCtrl+E");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"Sai&r\tCtrl+Q");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&Arquivo");
    
    // Menu Exibir
    HMENU hViewMenu = CreatePopupMenu();
    AppendMenuW(hViewMenu, MF_STRING, IDM_VIEW_UNITS_METRIC, L"Unidades &Métricas");
    AppendMenuW(hViewMenu, MF_STRING, IDM_VIEW_UNITS_IMPERIAL, L"Unidades &Imperiais (USA)");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hViewMenu, MF_STRING, IDM_VIEW_THEME_LIGHT, L"Tema &Claro");
    AppendMenuW(hViewMenu, MF_STRING, IDM_VIEW_THEME_DARK, L"Tema &Escuro");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hViewMenu, L"&Exibir");
    
    // Menu Validar
    HMENU hValidateMenu = CreatePopupMenu();
    AppendMenuW(hValidateMenu, MF_STRING, IDM_VALIDATE_PROJECT, L"&Validar Projeto Completo\tF5");
    AppendMenuW(hValidateMenu, MF_STRING, IDM_VALIDATE_CURRENT, L"Validar &Aba Atual");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hValidateMenu, L"&Validar");
    
    // Menu Database
    HMENU hDatabaseMenu = CreatePopupMenu();
    AppendMenuW(hDatabaseMenu, MF_STRING, IDM_PRESETS_LOAD, L"&Carregar Motor do Banco de Dados...");
    AppendMenuW(hDatabaseMenu, MF_STRING, IDM_COMPONENTS_BROWSE, L"&Navegador de Componentes...");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hDatabaseMenu, L"&Database");
    
    // Menu Ajuda
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, IDM_HELP_REFERENCES, L"&Referências Bibliográficas");
    AppendMenuW(hHelpMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, L"&Sobre...");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, L"A&juda");
    
    SetMenu(hwndMain, hMenuBar);
    
    // Atualiza menu de recentes
    UpdateRecentFilesMenu();
}

// ============================================================================
// HANDLERS - Arquivo
// ============================================================================

void MenuManager::OnFileNew() {
    // Verifica alterações não salvas
    if (mainWindow->HasUnsavedChanges()) {
        if (!mainWindow->GetFileManager()->ConfirmUnsavedChanges()) {
            return; // Usuário cancelou
        }
    }

    // Reseta o projeto
    EngineDataManager::GetInstance()->NewProject();

    // Limpa filepath
    mainWindow->SetCurrentFilePath(L"");
    mainWindow->SetUnsavedChanges(false);

    // Atualiza tabs com dados vazios
    mainWindow->GetTabManager()->RefreshAllTabsFromProject();

    // Atualiza título
    mainWindow->GetStatusBarManager()->UpdateTitle(L"", false);
    mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Novo projeto criado");
}

void MenuManager::OnFileOpen() {
    // Verifica alterações não salvas
    if (mainWindow->HasUnsavedChanges()) {
        if (!mainWindow->GetFileManager()->ConfirmUnsavedChanges()) {
            return;
        }
    }

    // Mostra dialog de abrir
    std::wstring filepath;
    if (mainWindow->GetFileManager()->ShowOpenFileDialog(filepath)) {
        if (mainWindow->GetFileManager()->LoadProject(filepath)) {
            mainWindow->SetCurrentFilePath(filepath);
            mainWindow->SetUnsavedChanges(false);

            // Atualiza tabs
            mainWindow->GetTabManager()->RefreshAllTabsFromProject();

            // Atualiza título e status
            mainWindow->GetStatusBarManager()->UpdateTitle(filepath, false);
            mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Projeto carregado com sucesso");

            // Adiciona aos recentes
            AddToRecentFiles(filepath);
        }
    }
}

void MenuManager::OnFileSave() {
    std::wstring filepath = mainWindow->GetCurrentFilePath();

    if (filepath.empty()) {
        // Sem filepath, faz SaveAs
        OnFileSaveAs();
        return;
    }

    if (mainWindow->GetFileManager()->SaveProject(filepath)) {
        mainWindow->SetUnsavedChanges(false);
        mainWindow->GetStatusBarManager()->UpdateTitle(filepath, false);
        mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Projeto salvo com sucesso");
    }
}

void MenuManager::OnFileSaveAs() {
    std::wstring filepath;
    if (mainWindow->GetFileManager()->ShowSaveFileDialog(filepath)) {
        if (mainWindow->GetFileManager()->SaveProject(filepath)) {
            mainWindow->SetCurrentFilePath(filepath);
            mainWindow->SetUnsavedChanges(false);
            mainWindow->GetStatusBarManager()->UpdateTitle(filepath, false);
            mainWindow->GetStatusBarManager()->UpdateStatusBar((L"Projeto salvo como: " + filepath).c_str());
            AddToRecentFiles(filepath);
        }
    }
}

void MenuManager::OnFileExit() {
    if (mainWindow->HasUnsavedChanges()) {
        if (!mainWindow->GetFileManager()->ConfirmUnsavedChanges()) {
            return; // Usuário cancelou
        }
    }
    DestroyWindow(hwndMain);
}

// ============================================================================
// HANDLERS - Exibir
// ============================================================================

void MenuManager::OnViewUnitsMetric() {
    HMENU hMenu = GetMenu(hwndMain);
    CheckMenuItem(hMenu, IDM_VIEW_UNITS_METRIC, MF_CHECKED);
    CheckMenuItem(hMenu, IDM_VIEW_UNITS_IMPERIAL, MF_UNCHECKED);
    TabPage::SetImperial(false);
    mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Unidades: Métrico (mm, cc, kPa)");
}

void MenuManager::OnViewUnitsImperial() {
    HMENU hMenu = GetMenu(hwndMain);
    CheckMenuItem(hMenu, IDM_VIEW_UNITS_METRIC, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_VIEW_UNITS_IMPERIAL, MF_CHECKED);
    TabPage::SetImperial(true);
    mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Unidades: Imperial (in, ci, PSI)");
}

void MenuManager::OnViewThemeLight() {
    mainWindow->GetSettingsManager()->ApplyTheme(Theme::LIGHT);
    mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Tema Claro ativado");
}

void MenuManager::OnViewThemeDark() {
    mainWindow->GetSettingsManager()->ApplyTheme(Theme::DARK);
    mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Tema Escuro ativado");
}

// ============================================================================
// HANDLERS - Validar
// ============================================================================

void MenuManager::OnValidateProject() {
    EngineValidator validator(&EngineDataManager::GetInstance()->GetProject());
    ValidationResult result = validator.ValidateComplete();
    
    std::wstring report = validator.GenerateReport(result);
    
    // Mostra em MessageBox (temporário - depois criar dialog customizado)
    MessageBoxW(hwndMain, report.c_str(), L"Validação do Projeto", 
                result.IsOK() ? MB_OK | MB_ICONINFORMATION : MB_OK | MB_ICONWARNING);
}

void MenuManager::OnValidateCurrent() {
    // Validação da aba atual (implementação simplificada)
    OnValidateProject(); // Por enquanto valida tudo
}

// ============================================================================
// HANDLERS - Database
// ============================================================================

void MenuManager::OnPresetsLoad() {
    if (!presetsManager) {
        MessageBoxW(hwndMain, L"Engine Presets Manager não foi inicializado.", L"Erro", MB_OK | MB_ICONERROR);
        return;
    }
    
    // Criar dialog de presets (SEM ARQUIVOS .RC - PROGRAMÁTICO)
    EnginePresetsDialog dialog(presetsManager, hInstance);
    
    if (dialog.Show(hwndMain)) {
        EnginePreset selected = dialog.GetSelectedPreset();
        
        // Aplica preset ao projeto atual
            EngineProject& project = EngineDataManager::GetInstance()->GetProject();
            if (presetsManager->ApplyPreset(selected.name, project)) {
                mainWindow->GetTabManager()->RefreshAllTabsFromProject();
                mainWindow->MarkAsChanged();
                mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Preset carregado com sucesso");
            }
    }
}

void MenuManager::OnComponentsBrowse() {
    MessageBoxW(hwndMain, 
        L"Components Browser será implementado em breve!\n\n"
        L"Funcionalidade:\n"
        L"• Navegar 330+ componentes\n"
        L"• Filtrar por categoria e fabricante\n"
        L"• Comparar especificações\n"
        L"• Calcular build total",
        L"Em Desenvolvimento",
        MB_OK | MB_ICONINFORMATION);
}

// ============================================================================
// HANDLERS - Ajuda
// ============================================================================

void MenuManager::OnHelpAbout() {
    std::wstring about = 
        L"=======================================================\n"
        L"  Engine Calculator v1.0.0\n"
        L"=======================================================\n\n"
        L"Calculadora Profissional de Motores a Combustao\n\n"
        L"Desenvolvido com C++ e Win32 API\n"
        L"2026\n\n"
        L"Baseado em literatura científica reconhecida:\n"
        L"• Gordon P. Blair - Design and Simulation of Four-Stroke Engines\n"
        L"• John Heywood - IC Engine Fundamentals\n"
        L"• SAE Technical Papers (diversos)\n"
        L"• Pulkrabek - Engineering Fundamentals of ICE\n"
        L"• A. Graham Bell - Four-Stroke Performance Tuning\n\n"
        L"Funcionalidades:\n"
        L"- Calculos basicos de motor (cilindrada, compressao, etc.)\n"
        L"- Analise completa de comando de valvulas e LSA\n"
        L"- Dimensionamento de sistemas de admissao e escape\n"
        L"- Validacao cruzada de componentes\n"
        L"- Salvar/Carregar projetos (.ecproj)\n\n"
        L"© 2026 - Todos os direitos reservados";
    
    MessageBoxW(hwndMain, about.c_str(), L"Sobre Engine Calculator", MB_OK | MB_ICONINFORMATION);
}

void MenuManager::OnHelpReferences() {
    std::wstring refs = 
        L"REFERÊNCIAS BIBLIOGRÁFICAS\n\n"
        L"Livros:\n"
        L"1. Blair, Gordon P. - \"Design and Simulation of Four-Stroke Engines\"\n"
        L"2. Blair, Gordon P. - \"Design and Simulation of Two-Stroke Engines\"\n"
        L"3. Heywood, John B. - \"Internal Combustion Engine Fundamentals\"\n"
        L"4. Pulkrabek, Willard W. - \"Engineering Fundamentals of the ICE\"\n"
        L"5. Bell, A. Graham - \"Four-Stroke Performance Tuning\"\n"
        L"6. Baechtel, John - \"Competition Engine Building\"\n\n"
        L"SAE Papers:\n"
        L"• SAE 2001-01-0662 - Valve Sizing\n"
        L"• SAE 2003-01-0001 - Intake System Design\n"
        L"• SAE 2004-01-1604 - Piston Speed\n"
        L"• SAE 2005-01-1688 - Camshaft Design & LSA\n"
        L"• SAE 2007-01-0148 - Dynamic Compression Ratio\n"
        L"• SAE J1121 - Valve Spring Design";
    
    MessageBoxW(hwndMain, refs.c_str(), L"Referências", MB_OK | MB_ICONINFORMATION);
}

// ============================================================================
// HISTÓRICO DE PROJETOS RECENTES
// ============================================================================

void MenuManager::AddToRecentFiles(const std::wstring& filepath) {
    if (filepath.empty()) return;
    
    // Remove se já existir (para movê-lo para o topo)
    auto it = std::find(recentFiles.begin(), recentFiles.end(), filepath);
    if (it != recentFiles.end()) {
        recentFiles.erase(it);
    }
    
    // Adiciona no início (mais recente)
    recentFiles.insert(recentFiles.begin(), filepath);
    
    // Limita a MAX_RECENT_FILES
    if (recentFiles.size() > MAX_RECENT_FILES) {
        recentFiles.resize(MAX_RECENT_FILES);
    }
    
    SaveRecentFiles();
    UpdateRecentFilesMenu();
}

void MenuManager::LoadRecentFiles() {
    recentFiles.clear();
    
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\EngineCalculator\\RecentFiles",
        0,
        KEY_READ,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        for (int i = 0; i < MAX_RECENT_FILES; i++) {
            wchar_t valueName[32];
            swprintf(valueName, 32, L"File%d", i);
            
            wchar_t buffer[MAX_PATH];
            DWORD bufferSize = sizeof(buffer);
            
            if (RegQueryValueExW(hKey, valueName, NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) {
                recentFiles.push_back(buffer);
            }
        }
        
        RegCloseKey(hKey);
    }
}

void MenuManager::SaveRecentFiles() {
    HKEY hKey;
    LONG result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\EngineCalculator\\RecentFiles",
        0, NULL, 0,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL
    );
    
    if (result == ERROR_SUCCESS) {
        // Salva cada arquivo
        for (size_t i = 0; i < recentFiles.size() && i < MAX_RECENT_FILES; i++) {
            wchar_t valueName[32];
            swprintf(valueName, 32, L"File%d", (int)i);
            
            RegSetValueExW(hKey, valueName, 0, REG_SZ, 
                          (BYTE*)recentFiles[i].c_str(), 
                          (DWORD)(recentFiles[i].length() + 1) * sizeof(wchar_t));
        }
        
        RegCloseKey(hKey);
    }
}

void MenuManager::UpdateRecentFilesMenu() {
    HMENU hMenu = GetMenu(hwndMain);
    if (!hMenu) return;
    
    // Encontra o menu File
    HMENU hFileMenu = GetSubMenu(hMenu, 0);
    if (!hFileMenu) return;
    
    // Remove itens antigos de recentes (se existirem)
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        RemoveMenu(hFileMenu, IDM_FILE_RECENT_1 + i, MF_BYCOMMAND);
    }
    RemoveMenu(hFileMenu, IDM_FILE_EXIT - 1, MF_BYCOMMAND); // Remove separador
    
    // Se há arquivos recentes, adiciona novo separador e itens
    if (!recentFiles.empty()) {
        // Encontra posição do "Sair" (último item)
        int itemCount = GetMenuItemCount(hFileMenu);
        int exitPosition = itemCount - 1;
        
        // Adiciona separador antes do "Sair"
        InsertMenuW(hFileMenu, exitPosition, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
        
        // Adiciona cada arquivo recente
        for (size_t i = 0; i < recentFiles.size(); i++) {
            // Extrai apenas nome do arquivo (sem caminho completo)
            std::wstring displayName = recentFiles[i];
            size_t pos = displayName.find_last_of(L"\\/");
            if (pos != std::wstring::npos) {
                displayName = displayName.substr(pos + 1);
            }
            
            // Formata: "&1 NomeDoArquivo.ecproj"
            std::wstring menuText = L"&" + std::to_wstring(i + 1) + L" " + displayName;
            
            InsertMenuW(hFileMenu, exitPosition + (int)i, 
                       MF_BYPOSITION | MF_STRING, 
                       IDM_FILE_RECENT_1 + i, 
                       menuText.c_str());
        }
    }
    
    DrawMenuBar(hwndMain);
}

void MenuManager::ClearRecentFiles() {
    recentFiles.clear();
    SaveRecentFiles();
    UpdateRecentFilesMenu();
}

void MenuManager::OnOpenRecentFile(int recentIndex) {
    if (recentIndex < 0 || recentIndex >= (int)recentFiles.size()) {
        return;
    }
    
    std::wstring filepath = recentFiles[recentIndex];
    
    // Verifica se arquivo ainda existe
    DWORD fileAttr = GetFileAttributesW(filepath.c_str());
    if (fileAttr == INVALID_FILE_ATTRIBUTES) {
        std::wstring msg = L"Arquivo não encontrado:\n\n" + filepath + 
                          L"\n\nDeseja remover da lista de projetos recentes?";
        int result = MessageBoxW(hwndMain, msg.c_str(), L"Arquivo Não Encontrado", 
                                MB_YESNO | MB_ICONWARNING);
        if (result == IDYES) {
            recentFiles.erase(recentFiles.begin() + recentIndex);
            SaveRecentFiles();
            UpdateRecentFilesMenu();
        }
        return;
    }
    
    // Verifica alterações não salvas
    if (mainWindow->HasUnsavedChanges()) {
        if (!mainWindow->GetFileManager()->ConfirmUnsavedChanges()) {
            return;
        }
    }

    // Carrega o projeto
    if (mainWindow->GetFileManager()->LoadProject(filepath)) {
        mainWindow->SetCurrentFilePath(filepath);
        mainWindow->SetUnsavedChanges(false);
        mainWindow->GetTabManager()->RefreshAllTabsFromProject();
        mainWindow->GetStatusBarManager()->UpdateTitle(filepath, false);
        mainWindow->GetStatusBarManager()->UpdateStatusBar(L"Projeto recente carregado");
        AddToRecentFiles(filepath);
    }
}
