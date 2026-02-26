// ============================================================================
// MainWindow.cpp - Implementação da Janela Principal (REFATORADA - FASE 2)
// Engine Calculator v1.0.0 - Março 2026
//
// REFATORAÇÃO: Código delegado para Managers especializados
// ============================================================================
#include "MainWindow.h"
#include "Managers/MenuManager.h"
#include "Managers/StatusBarManager.h"
#include "Managers/TabManager.h"
#include "Managers/FileManager.h"
#include "Managers/SettingsManager.h"
#include "EnginePresetsDialog.h"
#include "../Core/EngineDataManager.h"
#include "../Core/ProjectManager.h"
#include "../Core/EngineValidator.h"
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <sstream>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ============================================================================
// CONSTRUTOR E DESTRUTOR
// ============================================================================

MainWindow::MainWindow(HINSTANCE hInst) 
    : hwndMain(nullptr), hInstance(hInst), hAccelTable(nullptr),
      menuManager(nullptr), statusBarManager(nullptr), tabManager(nullptr),
      fileManager(nullptr), settingsManager(nullptr),
      presetsManager(nullptr),
      btnCalculate(nullptr), btnClear(nullptr), btnValidate(nullptr),
      hasUnsavedChanges(false), isMaximized(false),
      currentUnitSystem(UnitSystem::METRIC) {
    
    // Inicializa Engine Presets Manager
    presetsManager = new EnginePresetsManager();
}

MainWindow::~MainWindow() {
    // Deleta managers
    delete menuManager;
    delete statusBarManager;
    delete tabManager;
    delete fileManager;
    delete settingsManager;
    
    if (hAccelTable) {
        DestroyAcceleratorTable(hAccelTable);
    }
    
    if (presetsManager) {
        delete presetsManager;
        presetsManager = nullptr;
    }
}

// ============================================================================
// CRIAÇÃO DA JANELA
// ============================================================================

bool MainWindow::Create(int nCmdShow) {
    // Registra classe TabPage (necessário para as tabs)
    if (!RegisterTabPageClass(hInstance)) {
        MessageBoxW(nullptr, L"Erro ao registrar classe TabPage!", L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }
    
    // Carrega configurações do Registry (método estático - SEM criar objeto ainda)
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    int width = 900;
    int height = 700;
    bool maximized = false;
    
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\EngineCalculator", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD xVal = 0, yVal = 0, wVal = 900, hVal = 700, maxVal = 0;
        DWORD dataSize = sizeof(DWORD);
        
        RegQueryValueExW(hKey, L"WindowX", NULL, NULL, (BYTE*)&xVal, &dataSize);
        RegQueryValueExW(hKey, L"WindowY", NULL, NULL, (BYTE*)&yVal, &dataSize);
        RegQueryValueExW(hKey, L"WindowWidth", NULL, NULL, (BYTE*)&wVal, &dataSize);
        RegQueryValueExW(hKey, L"WindowHeight", NULL, NULL, (BYTE*)&hVal, &dataSize);
        RegQueryValueExW(hKey, L"Maximized", NULL, NULL, (BYTE*)&maxVal, &dataSize);
        
        RegCloseKey(hKey);
        
        // Valida valores
        RECT rcWork;
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &rcWork, 0);
        if (xVal >= rcWork.left && xVal <= rcWork.right - 100) x = xVal;
        if (yVal >= rcWork.top && yVal <= rcWork.bottom - 100) y = yVal;
        if (wVal >= 600 && wVal <= 3840) width = wVal;
        if (hVal >= 400 && hVal <= 2160) height = hVal;
        maximized = (maxVal == 1);
    }
    
    // Registra classe da janela
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(240, 240, 240)); // Brush temporário
    wcex.lpszClassName = GetWindowClassName();
    
    if (!RegisterClassExW(&wcex)) {
        return false;
    }
    
    // Cria janela
    hwndMain = CreateWindowExW(
        0,
        GetWindowClassName(),
        L"Engine Calculator - Calculadora Profissional de Motores a Combustão",
        WS_OVERLAPPEDWINDOW,
        x, y, width, height,
        nullptr,
        nullptr,
        hInstance,
        this
    );
    
    if (!hwndMain) {
        return false;
    }
    
    // AGORA cria os managers COM hwndMain válido (mas NÃO os inicializa ainda)
    // A inicialização será feita no WM_CREATE
    
    // Mostra janela
    if (maximized) {
        ShowWindow(hwndMain, SW_SHOWMAXIMIZED);
    } else {
        ShowWindow(hwndMain, nCmdShow);
    }
    
    UpdateWindow(hwndMain);
    return true;
}

// ============================================================================
// CRIAÇÃO DE CONTROLES (OnCreate)
// ============================================================================

void MainWindow::OnCreate() {
    // Cria os managers AGORA (hwndMain já existe e foi criado)
    settingsManager = new SettingsManager(this, hwndMain, hInstance);
    menuManager = new MenuManager(this, hwndMain, hInstance, presetsManager);
    statusBarManager = new StatusBarManager(this, hwndMain, hInstance);
    tabManager = new TabManager(this, hwndMain, hInstance);
    fileManager = new FileManager(this, hwndMain, hInstance);
    
    // Verifica se foram criados
    if (!menuManager || !statusBarManager || !tabManager || !settingsManager) {
        MessageBoxW(hwndMain, L"Erro crítico: Falha ao criar Managers!", L"Erro Fatal", MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
        return;
    }
    
    // Cria menu via MenuManager
    menuManager->CreateAppMenu();
    
    // Cria aceleradores
    ACCEL accels[] = {
        { FVIRTKEY | FCONTROL, 'N', IDM_FILE_NEW },
        { FVIRTKEY | FCONTROL, 'O', IDM_FILE_OPEN },
        { FVIRTKEY | FCONTROL, 'S', IDM_FILE_SAVE },
        { FVIRTKEY | FCONTROL, 'E', IDM_FILE_EXPORT },
        { FVIRTKEY,            VK_F5, IDM_VALIDATE_PROJECT },
        { FVIRTKEY | FCONTROL, 'Q', IDM_FILE_EXIT }
    };
    hAccelTable = CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(accels[0]));
    
    // Cria status bar via StatusBarManager
    statusBarManager->CreateStatusBar();
    
    // Cria toolbar
    CreateToolbar();
    
    // Cria tabs via TabManager
    tabManager->CreateTabControl();
    
    // Atualiza título
    statusBarManager->UpdateTitle(currentFilePath, hasUnsavedChanges);
    
    // Inicializa checkmark do sistema de unidades (padrão: Métrico)
    HMENU hMenu = GetMenu(hwndMain);
    if (hMenu) {
        CheckMenuItem(hMenu, IDM_VIEW_UNITS_METRIC, MF_CHECKED);
        CheckMenuItem(hMenu, IDM_VIEW_UNITS_IMPERIAL, MF_UNCHECKED);
    }
    
    // Carrega projetos recentes
    menuManager->LoadRecentFiles();
    menuManager->UpdateRecentFilesMenu();
    
    // Inicia auto-save
    settingsManager->StartAutoSaveTimer();
    
    // FORÇA LAYOUT INICIAL - Pega dimensões da janela e chama OnSize
    RECT rc;
    GetClientRect(hwndMain, &rc);
    OnSize(rc.right - rc.left, rc.bottom - rc.top);
    
    // Força redesenho completo
    InvalidateRect(hwndMain, NULL, TRUE);
    UpdateWindow(hwndMain);
}

// ============================================================================
// TOOLBAR
// ============================================================================

void MainWindow::CreateToolbar() {
    int margin = isMaximized ? 20 : 10;
    int buttonY = 12;
    int buttonWidth = 110;
    int buttonHeight = 34;
    int spacing = 120;

    btnCalculate = CreateWindowExW(
        0, L"BUTTON", L"Calcular",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        margin + spacing * 0, buttonY, buttonWidth, buttonHeight,
        hwndMain, (HMENU)1, hInstance, nullptr
    );

    btnClear = CreateWindowExW(
        0, L"BUTTON", L"Limpar",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        margin + spacing * 1, buttonY, buttonWidth, buttonHeight,
        hwndMain, (HMENU)2, hInstance, nullptr
    );

    btnValidate = CreateWindowExW(
        0, L"BUTTON", L"Validar",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        margin + spacing * 2, buttonY, buttonWidth, buttonHeight,
        hwndMain, (HMENU)3, hInstance, nullptr
    );
}

// ============================================================================
// EVENTOS
// ============================================================================

void MainWindow::OnSize(int width, int height) {
    // Detecta estado maximizado
    WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
    if (GetWindowPlacement(hwndMain, &wp)) {
        isMaximized = (wp.showCmd == SW_MAXIMIZE || wp.showCmd == SW_SHOWMAXIMIZED);
    }
    
    // Atualiza status bar (com verificação de null)
    if (statusBarManager) {
        HWND statusBar = statusBarManager->GetHandle();
        if (statusBar) {
            SendMessage(statusBar, WM_SIZE, 0, 0);
        }
    }
    
    // Atualiza tabs (com verificação de null)
    if (tabManager) {
        HWND tabCtrl = tabManager->GetTabControlHandle();
        if (tabCtrl) {
            RECT rcStatus = { 0 };
            if (statusBarManager) {
                HWND statusBar = statusBarManager->GetHandle();
                if (statusBar) {
                    GetWindowRect(statusBar, &rcStatus);
                }
            }
            int statusHeight = rcStatus.bottom - rcStatus.top;
            
            int leftMargin = isMaximized ? 20 : 10;
            int topMargin = 55;
            int sideMargin = isMaximized ? 40 : 30;
            
            int tabWidth = width - sideMargin;
            int tabHeight = height - topMargin - 20 - statusHeight;
            
            // Reposiciona Tab Control
            SetWindowPos(tabCtrl, nullptr, leftMargin, topMargin, 
                         tabWidth, tabHeight, SWP_NOZORDER);
            
            // AJUSTA AS PÁGINAS DAS TABS DENTRO DO TAB CONTROL
            RECT rcTab;
            GetClientRect(tabCtrl, &rcTab);
            TabCtrl_AdjustRect(tabCtrl, FALSE, &rcTab);
            
            // Reposiciona todas as páginas das tabs
            const auto& tabs = tabManager->GetTabs();
            for (auto tab : tabs) {
                HWND hwndPage = tab->GetHandle();
                if (hwndPage) {
                    SetWindowPos(hwndPage, nullptr, 
                                rcTab.left, rcTab.top,
                                rcTab.right - rcTab.left, 
                                rcTab.bottom - rcTab.top,
                                SWP_NOZORDER);
                }
            }
        }
    }
}

void MainWindow::OnTabChange() {
    if (tabManager) {
        tabManager->OnTabChange();
    }
}

void MainWindow::OnCalculate() {
    if (tabManager) {
        tabManager->OnCalculate();
    }
}

void MainWindow::OnClear() {
    if (tabManager) {
        tabManager->OnClear();
    }
}

void MainWindow::MarkAsChanged() {
    hasUnsavedChanges = true;
    if (statusBarManager) {
        statusBarManager->UpdateTitle(currentFilePath, hasUnsavedChanges);
        statusBarManager->UpdateStatusBar(L"Projeto modificado");
    }
}

// ============================================================================
// HANDLERS DE MENU (delegam para MenuManager)
// ============================================================================

void MainWindow::OnFileNew() {
    menuManager->OnFileNew();
}

void MainWindow::OnFileOpen() {
    menuManager->OnFileOpen();
}

void MainWindow::OnFileSave() {
    menuManager->OnFileSave();
}

void MainWindow::OnFileSaveAs() {
    menuManager->OnFileSaveAs();
}

void MainWindow::OnFileExport() {
    std::wstring filepath;
    if (fileManager->ShowExportDialog(filepath)) {
        if (fileManager->ExportReport(filepath)) {
            statusBarManager->UpdateStatusBar(L"Relatório exportado com sucesso");
            ShellExecuteW(hwndMain, L"open", filepath.c_str(), NULL, NULL, SW_SHOW);
        } else {
            MessageBoxW(hwndMain, L"Erro ao exportar relatório.", L"Erro", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::OnFileExit() {
    menuManager->OnFileExit();
}

void MainWindow::OnViewUnitsMetric() {
    menuManager->OnViewUnitsMetric();
}

void MainWindow::OnViewUnitsImperial() {
    menuManager->OnViewUnitsImperial();
}

void MainWindow::OnViewThemeLight() {
    TabPage::SetDarkTheme(false);
    settingsManager->ApplyTheme(Theme::LIGHT);

    // Atualiza fundo da janela principal
    SetClassLongPtr(hwndMain, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(240, 240, 240)));

    // Redesenha tudo
    if (tabManager) {
        const auto& tabs = tabManager->GetTabs();
        for (auto tab : tabs) {
            if (tab->GetHandle()) {
                InvalidateRect(tab->GetHandle(), NULL, TRUE);
                EnumChildWindows(tab->GetHandle(), [](HWND child, LPARAM) -> BOOL {
                    InvalidateRect(child, NULL, TRUE);
                    return TRUE;
                }, 0);
            }
        }
    }
    InvalidateRect(hwndMain, NULL, TRUE);
    statusBarManager->UpdateStatusBar(L"Tema Claro ativado");
}

void MainWindow::OnViewThemeDark() {
    TabPage::SetDarkTheme(true);
    settingsManager->ApplyTheme(Theme::DARK);

    // Atualiza fundo da janela principal
    SetClassLongPtr(hwndMain, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(45, 45, 48)));

    // Redesenha tudo
    if (tabManager) {
        const auto& tabs = tabManager->GetTabs();
        for (auto tab : tabs) {
            if (tab->GetHandle()) {
                InvalidateRect(tab->GetHandle(), NULL, TRUE);
                EnumChildWindows(tab->GetHandle(), [](HWND child, LPARAM) -> BOOL {
                    InvalidateRect(child, NULL, TRUE);
                    return TRUE;
                }, 0);
            }
        }
    }
    InvalidateRect(hwndMain, NULL, TRUE);
    statusBarManager->UpdateStatusBar(L"Tema Escuro ativado");
}

void MainWindow::OnValidateProject() {
    menuManager->OnValidateProject();
}

void MainWindow::OnValidateCurrent() {
    menuManager->OnValidateCurrent();
}

void MainWindow::OnPresetsLoad() {
    menuManager->OnPresetsLoad();
}

void MainWindow::OnComponentsBrowse() {
    menuManager->OnComponentsBrowse();
}

void MainWindow::OnHelpAbout() {
    menuManager->OnHelpAbout();
}

void MainWindow::OnHelpReferences() {
    menuManager->OnHelpReferences();
}

void MainWindow::OnOpenRecentFile(int recentIndex) {
    menuManager->OnOpenRecentFile(recentIndex);
}

// ============================================================================
// WINDOW PROCEDURE
// ============================================================================

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* window = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        window = (MainWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    } else {
        window = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (window) {
        switch (uMsg) {
            case WM_CREATE:
                // CORREÇÃO: hwndMain ainda não foi atribuído quando WM_CREATE chega!
                // Precisamos setar antes de chamar OnCreate()
                window->hwndMain = hwnd;
                window->OnCreate();
                return 0;
                
            case WM_SIZE:
                window->OnSize(LOWORD(lParam), HIWORD(lParam));
                return 0;
                
            case WM_NOTIFY:
                if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
                    window->OnTabChange();
                }
                return 0;
                
            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case 1: window->OnCalculate(); break;
                    case 2: window->OnClear(); break;
                    case 3: window->OnValidateProject(); break;
                    
                    case IDM_FILE_NEW: window->OnFileNew(); break;
                    case IDM_FILE_OPEN: window->OnFileOpen(); break;
                    case IDM_FILE_SAVE: window->OnFileSave(); break;
                    case IDM_FILE_SAVEAS: window->OnFileSaveAs(); break;
                    case IDM_FILE_EXPORT: window->OnFileExport(); break;
                    case IDM_FILE_EXIT: window->OnFileExit(); break;
                    
                    case IDM_VIEW_UNITS_METRIC: window->OnViewUnitsMetric(); break;
                    case IDM_VIEW_UNITS_IMPERIAL: window->OnViewUnitsImperial(); break;
                    case IDM_VIEW_THEME_LIGHT: window->OnViewThemeLight(); break;
                    case IDM_VIEW_THEME_DARK: window->OnViewThemeDark(); break;
                    
                    case IDM_VALIDATE_PROJECT: window->OnValidateProject(); break;
                    case IDM_VALIDATE_CURRENT: window->OnValidateCurrent(); break;
                    
                    case IDM_PRESETS_LOAD: window->OnPresetsLoad(); break;
                    case IDM_COMPONENTS_BROWSE: window->OnComponentsBrowse(); break;
                    
                    case IDM_HELP_ABOUT: window->OnHelpAbout(); break;
                    case IDM_HELP_REFERENCES: window->OnHelpReferences(); break;
                    
                    case IDM_FILE_RECENT_1:
                    case IDM_FILE_RECENT_2:
                    case IDM_FILE_RECENT_3:
                    case IDM_FILE_RECENT_4:
                    case IDM_FILE_RECENT_5:
                        window->OnOpenRecentFile(LOWORD(wParam) - IDM_FILE_RECENT_1);
                        break;
                }
                return 0;
                
            case WM_GETMINMAXINFO:
                {
                    MINMAXINFO* mmi = (MINMAXINFO*)lParam;
                    mmi->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
                    mmi->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
                }
                return 0;

            case WM_CTLCOLORSTATIC:
            case WM_CTLCOLOREDIT:
                if (TabPage::IsDarkTheme()) {
                    HDC hdcCtrl = (HDC)wParam;
                    SetTextColor(hdcCtrl, RGB(220, 220, 220));
                    if (uMsg == WM_CTLCOLOREDIT) {
                        SetBkColor(hdcCtrl, RGB(30, 30, 30));
                        static HBRUSH hEditBr = CreateSolidBrush(RGB(30, 30, 30));
                        return (LRESULT)hEditBr;
                    }
                    SetBkColor(hdcCtrl, RGB(45, 45, 48));
                    return (LRESULT)TabPage::GetThemeBgBrush();
                }
                break;

            case WM_ERASEBKGND:
                if (TabPage::IsDarkTheme()) {
                    HDC hdc = (HDC)wParam;
                    RECT rc;
                    GetClientRect(hwnd, &rc);
                    FillRect(hdc, &rc, TabPage::GetThemeBgBrush());
                    return 1;
                }
                break;
                
            case WM_TIMER:
                if (wParam == 1) {
                    window->settingsManager->PerformAutoSave();
                }
                return 0;

            case WM_DRAWITEM:
                {
                    LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
                    if (dis->CtlType == ODT_BUTTON) {
                        HDC hdc = dis->hDC;
                        RECT rc = dis->rcItem;
                        bool pressed = (dis->itemState & ODS_SELECTED);
                        bool focused = (dis->itemState & ODS_FOCUS);

                        // Cores por botao
                        COLORREF bgColor, textColor, borderColor;
                        if (dis->CtlID == 1) { // Calcular
                            bgColor = pressed ? RGB(0, 90, 180) : RGB(0, 120, 215);
                            textColor = RGB(255, 255, 255);
                            borderColor = RGB(0, 84, 166);
                        } else if (dis->CtlID == 3) { // Validar
                            bgColor = pressed ? RGB(16, 110, 50) : RGB(16, 137, 62);
                            textColor = RGB(255, 255, 255);
                            borderColor = RGB(14, 100, 44);
                        } else { // Limpar
                            bgColor = pressed ? RGB(190, 190, 190) : RGB(225, 225, 225);
                            textColor = RGB(30, 30, 30);
                            borderColor = RGB(170, 170, 170);
                        }

                        // Fundo com cantos arredondados
                        HBRUSH hBrush = CreateSolidBrush(bgColor);
                        HPEN hPen = CreatePen(PS_SOLID, 1, borderColor);
                        SelectObject(hdc, hBrush);
                        SelectObject(hdc, hPen);
                        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
                        DeleteObject(hBrush);
                        DeleteObject(hPen);

                        // Texto
                        SetBkMode(hdc, TRANSPARENT);
                        SetTextColor(hdc, textColor);
                        HFONT hFont = CreateFontW(15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
                        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

                        wchar_t text[64];
                        GetWindowTextW(dis->hwndItem, text, 64);
                        DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                        SelectObject(hdc, oldFont);
                        DeleteObject(hFont);

                        // Borda de foco
                        if (focused) {
                            InflateRect(&rc, -3, -3);
                            DrawFocusRect(hdc, &rc);
                        }
                        return TRUE;
                    }
                }
                return 0;

            case WM_DESTROY:
                if (window && window->settingsManager) {
                    window->settingsManager->SaveWindowSettings();
                }
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
