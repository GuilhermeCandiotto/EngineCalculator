// ============================================================================
// TabManager.cpp - Implementação do Gerenciador de Tabs
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// ============================================================================
#include "TabManager.h"
#include "../MainWindow.h"
#include "../TabPages.h"
#include "../Tabs/BasicTab.h"
#include "../Tabs/CompressionTab.h"
#include "../Tabs/ValveTrainTab.h"
#include "../Tabs/CamshaftTab.h"
#include "../Tabs/IntakeTab.h"
#include "../Tabs/ExhaustTab.h"
#include "../Tabs/FuelTab.h"
#include "../Tabs/AlternativeFuelsTab.h"
#include "../Tabs/TurboTab.h"
#include "../../Core/EngineDataManager.h"
#include "../../Core/EngineValidator.h"
#include "StatusBarManager.h"
#include <commctrl.h>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")

// Formata double sem zeros a direita (ex: 272.0 -> "272", 3.5 -> "3.5")
static std::wstring FmtVal(double v, int maxDecimals = 2) {
    if (v == (int)v) return std::to_wstring((int)v);
    std::wostringstream ss;
    ss << std::fixed << std::setprecision(maxDecimals) << v;
    std::wstring s = ss.str();
    size_t dot = s.find(L'.');
    if (dot != std::wstring::npos) {
        size_t last = s.find_last_not_of(L'0');
        if (last == dot) s = s.substr(0, dot);
        else s = s.substr(0, last + 1);
    }
    return s;
}

// IDs do Tab Control
#define ID_TAB_CONTROL 1001

TabManager::TabManager(MainWindow* window, HWND hwnd, HINSTANCE hInst)
    : mainWindow(window), hwndMain(hwnd), hwndTab(nullptr), hInstance(hInst),
      tabBasic(nullptr), tabCompression(nullptr), tabValveTrain(nullptr),
      tabCamshaft(nullptr), tabIntake(nullptr), tabExhaust(nullptr),
      tabFuel(nullptr), tabAlternativeFuels(nullptr), tabTurbo(nullptr),
      currentTab(0) {
}

TabManager::~TabManager() {
    // Deleta todas as tabs
    for (auto tab : tabs) {
        delete tab;
    }
    tabs.clear();
}

void TabManager::CreateTabControl() {
    // Inicializa Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Cria o controle de abas
    hwndTab = CreateWindowExW(
        0,
        WC_TABCONTROLW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_FIXEDWIDTH,
        10, 55, 870, 580,
        hwndMain,
        (HMENU)ID_TAB_CONTROL,
        hInstance,
        nullptr
    );
    
    if (!hwndTab) {
        MessageBoxW(hwndMain, L"Erro ao criar Tab Control!", L"Erro", MB_OK | MB_ICONERROR);
        return;
    }
    
    HFONT hFont = CreateFontW(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    SendMessage(hwndTab, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Adiciona as abas
    TCITEMW tie = { 0 };
    tie.mask = TCIF_TEXT;
    
    tie.pszText = (LPWSTR)L"Básico";
    TabCtrl_InsertItem(hwndTab, 0, &tie);
    
    tie.pszText = (LPWSTR)L"Compressão";
    TabCtrl_InsertItem(hwndTab, 1, &tie);
    
    tie.pszText = (LPWSTR)L"Trem de Válvulas";
    TabCtrl_InsertItem(hwndTab, 2, &tie);
    
    tie.pszText = (LPWSTR)L"Comando";
    TabCtrl_InsertItem(hwndTab, 3, &tie);
    
    tie.pszText = (LPWSTR)L"Admissão";
    TabCtrl_InsertItem(hwndTab, 4, &tie);
    
    tie.pszText = (LPWSTR)L"Escape";
    TabCtrl_InsertItem(hwndTab, 5, &tie);
    
    tie.pszText = (LPWSTR)L"Combustível";
    TabCtrl_InsertItem(hwndTab, 6, &tie);
    
    tie.pszText = (LPWSTR)L"Combust. Alternativos";
    TabCtrl_InsertItem(hwndTab, 7, &tie);

    tie.pszText = (LPWSTR)L"Turbo/SC";
    TabCtrl_InsertItem(hwndTab, 8, &tie);

    InitializeTabs();
}

void TabManager::InitializeTabs() {
    // Cria as páginas das abas
    tabBasic = new BasicTab(hwndTab, hInstance);
    tabBasic->Create();
    tabs.push_back(tabBasic);
    
    tabCompression = new CompressionTab(hwndTab, hInstance);
    tabCompression->Create();
    tabs.push_back(tabCompression);
    
    tabValveTrain = new ValveTrainTab(hwndTab, hInstance);
    tabValveTrain->Create();
    tabs.push_back(tabValveTrain);
    
    tabCamshaft = new CamshaftTab(hwndTab, hInstance);
    tabCamshaft->Create();
    tabs.push_back(tabCamshaft);
    
    tabIntake = new IntakeTab(hwndTab, hInstance);
    tabIntake->Create();
    tabs.push_back(tabIntake);
    
    tabExhaust = new ExhaustTab(hwndTab, hInstance);
    tabExhaust->Create();
    tabs.push_back(tabExhaust);
    
    tabFuel = new FuelTab(hwndTab, hInstance);
    tabFuel->Create();
    tabs.push_back(tabFuel);
    
    tabAlternativeFuels = new AlternativeFuelsTab(hwndTab, hInstance);
    tabAlternativeFuels->Create();
    tabs.push_back(tabAlternativeFuels);

    tabTurbo = new TurboTab(hwndTab, hInstance);
    tabTurbo->Create();
    tabs.push_back(tabTurbo);

    // Mostra a primeira aba
    if (!tabs.empty()) {
        tabs[0]->Show(true);
    }
}

void TabManager::OnTabChange() {
    int selectedTab = TabCtrl_GetCurSel(hwndTab);

    if (selectedTab >= 0 && selectedTab < static_cast<int>(tabs.size())) {
        // Esconde todas as abas
        for (auto tab : tabs) {
            tab->Show(false);
        }

        // Propaga dados basicos para a aba destino
        EngineDataManager* dm = EngineDataManager::GetInstance();
        if (dm) {
            const EngineProject& project = dm->GetProject();

            // Compressao: bore, stroke, cilindros
            if (selectedTab == TAB_COMPRESSION && tabCompression && project.basicData.bore > 0) {
                HWND h = tabCompression->GetHandle();
                SetWindowTextW(GetDlgItem(h, IDC_EDIT_BORE_COMP), FmtVal(project.basicData.bore).c_str());
                SetWindowTextW(GetDlgItem(h, IDC_EDIT_STROKE_COMP), FmtVal(project.basicData.stroke).c_str());
                SetWindowTextW(GetDlgItem(h, IDC_EDIT_CYLINDERS_COMP), std::to_wstring(project.basicData.cylinders).c_str());
            }

            // Comando: RPM de analise
            if (selectedTab == TAB_CAMSHAFT && tabCamshaft && project.basicData.maxRPM > 0) {
                HWND h = tabCamshaft->GetHandle();
                SetWindowTextW(GetDlgItem(h, IDC_EDIT_CAM_RPM), 
                    std::to_wstring((int)project.basicData.maxRPM).c_str());
            }

            // Admissao: RPM alvo
            if (selectedTab == TAB_INTAKE && tabIntake && project.basicData.maxRPM > 0) {
                HWND h = tabIntake->GetHandle();
                SetWindowTextW(GetDlgItem(h, IDC_EDIT_TARGET_RPM_INTAKE), 
                    std::to_wstring((int)project.basicData.maxRPM).c_str());
            }

            // Escape: RPM alvo e duracao escape do comando
            if (selectedTab == TAB_EXHAUST && tabExhaust) {
                HWND h = tabExhaust->GetHandle();
                if (project.basicData.maxRPM > 0) {
                    SetWindowTextW(GetDlgItem(h, IDC_EDIT_TARGET_RPM_EXHAUST), 
                        std::to_wstring((int)project.basicData.maxRPM).c_str());
                }
                if (project.camshaftData.exhaustDuration > 0) {
                    SetWindowTextW(GetDlgItem(h, IDC_EDIT_EXHAUST_DUR_CALC), 
                        FmtVal(project.camshaftData.exhaustDuration).c_str());
                }
            }
        }

        // Mostra a aba selecionada
        tabs[selectedTab]->Show(true);
        currentTab = selectedTab;

        // Validacao rapida: alerta na status bar se houver problemas criticos
        EngineDataManager* dmv = EngineDataManager::GetInstance();
        if (dmv && dmv->GetProject().basicData.bore > 0) {
            EngineValidator validator(&dmv->GetProject());
            ValidationResult vr = validator.ValidateComplete();
            StatusBarManager* sb = mainWindow->GetStatusBarManager();
            if (sb) {
                if (vr.criticalCount > 0) {
                    std::wstring msg = L"⚠ " + std::to_wstring(vr.criticalCount) + L" alerta(s) critico(s) - Pressione F5 para detalhes";
                    sb->UpdateStatusBar(msg.c_str());
                } else if (vr.warningCount > 0) {
                    std::wstring msg = std::to_wstring(vr.warningCount) + L" aviso(s) - Pressione F5 para detalhes";
                    sb->UpdateStatusBar(msg.c_str());
                } else {
                    sb->UpdateStatusBar(L"Pronto");
                }
            }
        }
    }
}

void TabManager::OnCalculate() {
    if (currentTab >= 0 && currentTab < static_cast<int>(tabs.size())) {
        tabs[currentTab]->OnCalculate();
    }
}

void TabManager::OnClear() {
    if (currentTab >= 0 && currentTab < static_cast<int>(tabs.size())) {
        tabs[currentTab]->OnClear();
    }
}

void TabManager::RefreshAllTabsFromProject() {
    // Pega referência do projeto
    const EngineProject& project = EngineDataManager::GetInstance()->GetProject();
    
    // BASIC TAB
    if (tabBasic && tabBasic->GetHandle()) {
        HWND h = tabBasic->GetHandle();
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_BORE), FmtVal(project.basicData.bore).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_STROKE), FmtVal(project.basicData.stroke).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_CYLINDERS), std::to_wstring(project.basicData.cylinders).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_ROD_LENGTH), FmtVal(project.basicData.rodLength).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_RPM), std::to_wstring((int)project.basicData.maxRPM).c_str());
    }

    // COMPRESSION TAB
    if (tabCompression && tabCompression->GetHandle()) {
        HWND h = tabCompression->GetHandle();
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_BORE_COMP), FmtVal(project.basicData.bore).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_STROKE_COMP), FmtVal(project.basicData.stroke).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_CYLINDERS_COMP), std::to_wstring(project.basicData.cylinders).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_CHAMBER_VOL), FmtVal(project.compressionData.chamberVolume).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_PISTON_DOME), FmtVal(project.compressionData.pistonDomeVolume).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_GASKET_THICK), FmtVal(project.compressionData.gasketThickness).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_DECK_HEIGHT), FmtVal(project.compressionData.deckHeight).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_VALVE_RELIEF), FmtVal(project.compressionData.valveRelief).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_IVC_ANGLE), FmtVal(project.compressionData.ivcAngle).c_str());
    }

    // CAMSHAFT TAB
    if (tabCamshaft && tabCamshaft->GetHandle()) {
        HWND h = tabCamshaft->GetHandle();
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_INTAKE_DURATION), FmtVal(project.camshaftData.intakeDuration).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_EXHAUST_DURATION), FmtVal(project.camshaftData.exhaustDuration).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_INTAKE_LIFT_CAM), FmtVal(project.camshaftData.intakeLift).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_EXHAUST_LIFT_CAM), FmtVal(project.camshaftData.exhaustLift).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_LSA), FmtVal(project.camshaftData.lsa).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_ADVANCE_RETARD), FmtVal(project.camshaftData.advanceRetard).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_ROCKER_RATIO_CAM), FmtVal(project.camshaftData.rockerRatio).c_str());
    }

    // INTAKE TAB
    if (tabIntake && tabIntake->GetHandle()) {
        HWND h = tabIntake->GetHandle();
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_RUNNER_LENGTH), FmtVal(project.intakeData.runnerLength).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_RUNNER_DIAMETER), FmtVal(project.intakeData.runnerDiameter).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_PLENUM_VOLUME), FmtVal(project.intakeData.plenumVolume).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_TRUMPET_LENGTH), FmtVal(project.intakeData.trumpetLength).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_TRUMPET_DIAMETER), FmtVal(project.intakeData.trumpetDiameter).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_TARGET_RPM_INTAKE), std::to_wstring((int)project.intakeData.targetRPM).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_BOOST_PRESSURE), FmtVal(project.intakeData.boostPressure).c_str());
    }

    // EXHAUST TAB
    if (tabExhaust && tabExhaust->GetHandle()) {
        HWND h = tabExhaust->GetHandle();
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_PRIMARY_LENGTH), FmtVal(project.exhaustData.primaryLength).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_PRIMARY_DIAMETER), FmtVal(project.exhaustData.primaryDiameter).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_SECONDARY_LENGTH), FmtVal(project.exhaustData.secondaryLength).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_SECONDARY_DIAMETER), FmtVal(project.exhaustData.secondaryDiameter).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_COLLECTOR_DIAMETER), FmtVal(project.exhaustData.collectorDiameter).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_TARGET_RPM_EXHAUST), std::to_wstring((int)project.exhaustData.targetRPM).c_str());
        SetWindowTextW(GetDlgItem(h, IDC_EDIT_EXHAUST_DUR_CALC), FmtVal(project.exhaustData.exhaustDuration).c_str());
        SendMessage(GetDlgItem(h, IDC_COMBO_EXHAUST_TYPE), CB_SETCURSEL, 
                   project.exhaustData.isFourIntoOne ? 0 : 1, 0);
    }
}
