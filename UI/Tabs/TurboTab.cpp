// ============================================================================
// TurboTab.cpp - Turbo/Supercharger Calculator Tab Implementation
// Engine Calculator v1.2.0 - Fevereiro 2026
// ============================================================================
#include "TurboTab.h"
#include "../../Core/TurboCalculator.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

extern bool RegisterTabPageClass(HINSTANCE hInstance);

// Control IDs
#define IDC_COMBO_FI_TYPE       3001
#define IDC_COMBO_TURBO_CONFIG  3002
#define IDC_COMBO_SC_TYPE       3003
#define IDC_EDIT_TARGET_HP      3010
#define IDC_EDIT_TARGET_BOOST   3011
#define IDC_EDIT_MAX_RPM        3012
#define IDC_EDIT_IC_PRESS_DROP  3020
#define IDC_EDIT_PIPING_DROP    3021
#define IDC_EDIT_FILTER_DROP    3022
#define IDC_EDIT_IC_EFFICIENCY  3023
#define IDC_EDIT_DRIVE_RATIO    3030
#define IDC_RESULT_TURBO        3040

TurboTab::TurboTab(HWND parent, HINSTANCE instance)
    : TabPage(parent, instance), comboFIType(nullptr), comboTurboConfig(nullptr),
      comboSCType(nullptr), lblTurboConfig(nullptr), lblSCType(nullptr),
      lblDriveRatio(nullptr), textResults(nullptr), hwndGraph(nullptr) {
}

TurboTab::~TurboTab() {
}

void TurboTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
    HWND label = CreateWindowW(L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, width, 20, hwndPage, nullptr, hInst, nullptr);
    SendMessage(label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

HWND TurboTab::CreateLabelEx(const wchar_t* text, int x, int y, int width) {
    HWND label = CreateWindowW(L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, width, 20, hwndPage, nullptr, hInst, nullptr);
    SendMessage(label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    return label;
}

void TurboTab::UpdateVisibility() {
    int fiType = (int)SendMessage(comboFIType, CB_GETCURSEL, 0, 0);
    bool isTurbo = (fiType == 0);
    bool isSC = (fiType == 1);

    int showTurbo = isTurbo ? SW_SHOW : SW_HIDE;
    int showSC = isSC ? SW_SHOW : SW_HIDE;

    ShowWindow(lblTurboConfig, showTurbo);
    ShowWindow(comboTurboConfig, showTurbo);

    ShowWindow(lblSCType, showSC);
    ShowWindow(comboSCType, showSC);
    ShowWindow(lblDriveRatio, showSC);
    if (editDriveRatio.GetHandle())
        ShowWindow(editDriveRatio.GetHandle(), showSC);
}

void TurboTab::OnCommand(WPARAM wParam, LPARAM lParam) {
    if (HIWORD(wParam) == CBN_SELCHANGE) {
        int id = LOWORD(wParam);
        if (id == IDC_COMBO_FI_TYPE) {
            UpdateVisibility();
        }
    }
}

void TurboTab::CreateControls() {
    int leftMargin = GetMargin();
    int topMargin = GetMargin();
    int labelWidth = GetLabelWidth(0.35f);
    int editWidth = GetEditWidth(0.22f);
    int currentY = topMargin;

    // ========== SYSTEM TYPE ==========
    CreateLabel(L"TIPO DE INDUCAO FORCADA:", leftMargin, currentY, 300);
    currentY += 20;

    CreateLabel(L"Sistema:", leftMargin, currentY + 3, labelWidth);
    comboFIType = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        leftMargin + labelWidth, currentY, editWidth + 40, 120,
        hwndPage, (HMENU)IDC_COMBO_FI_TYPE, hInst, nullptr);
    SendMessage(comboFIType, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    SendMessageW(comboFIType, CB_ADDSTRING, 0, (LPARAM)L"Turbocharger");
    SendMessageW(comboFIType, CB_ADDSTRING, 0, (LPARAM)L"Supercharger");
    SendMessage(comboFIType, CB_SETCURSEL, 0, 0);
    currentY += 30;

    lblTurboConfig = CreateLabelEx(L"Configuracao Turbo:", leftMargin, currentY + 3, labelWidth);
    comboTurboConfig = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        leftMargin + labelWidth, currentY, editWidth + 40, 150,
        hwndPage, (HMENU)IDC_COMBO_TURBO_CONFIG, hInst, nullptr);
    SendMessage(comboTurboConfig, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    SendMessageW(comboTurboConfig, CB_ADDSTRING, 0, (LPARAM)L"Single Turbo");
    SendMessageW(comboTurboConfig, CB_ADDSTRING, 0, (LPARAM)L"Twin Turbo (Paralelo)");
    SendMessageW(comboTurboConfig, CB_ADDSTRING, 0, (LPARAM)L"Sequential Turbo");
    SendMessageW(comboTurboConfig, CB_ADDSTRING, 0, (LPARAM)L"Compound (Serie)");
    SendMessageW(comboTurboConfig, CB_ADDSTRING, 0, (LPARAM)L"Twin-Scroll");
    SendMessage(comboTurboConfig, CB_SETCURSEL, 0, 0);
    currentY += 30;

    lblSCType = CreateLabelEx(L"Tipo Supercharger:", leftMargin, currentY + 3, labelWidth);
    comboSCType = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        leftMargin + labelWidth, currentY, editWidth + 40, 120,
        hwndPage, (HMENU)IDC_COMBO_SC_TYPE, hInst, nullptr);
    SendMessage(comboSCType, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    SendMessageW(comboSCType, CB_ADDSTRING, 0, (LPARAM)L"Roots (Eaton, Magnuson)");
    SendMessageW(comboSCType, CB_ADDSTRING, 0, (LPARAM)L"Twin-Screw (Whipple, Kenne Bell)");
    SendMessageW(comboSCType, CB_ADDSTRING, 0, (LPARAM)L"Centrifugo (Procharger, Vortech)");
    SendMessage(comboSCType, CB_SETCURSEL, 0, 0);
    currentY += 35;

    // ========== MAIN PARAMETERS ==========
    CreateLabel(L"PARAMETROS:", leftMargin, currentY, 300);
    currentY += 20;

    CreateLabel(L"Potencia Alvo (HP):", leftMargin, currentY, labelWidth);
    editTargetHP.Create(hwndPage, IDC_EDIT_TARGET_HP,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 50.0, 3000.0, 0);
    CreateTooltip(editTargetHP.GetHandle(),
        L"Potencia desejada no virabrequim (flywheel HP).\n"
        L"O turbo sera dimensionado para suprir o airflow necessario.\n"
        L"Ref: Garrett Turbo Tech 103 - HP = airflow/10.42");
    currentY += 30;

    CreateLabel(L"Boost Alvo (PSI):", leftMargin, currentY, labelWidth);
    editTargetBoost.Create(hwndPage, IDC_EDIT_TARGET_BOOST,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 1.0, 60.0, 1);
    CreateTooltip(editTargetBoost.GetHandle(),
        L"Pressao de boost manometrica (gauge PSI).\n"
        L"Street: 7-15 PSI | Sport: 15-25 PSI | Race: 25-45 PSI\n"
        L"1 bar = 14.5 PSI | 1 kg/cm2 = 14.2 PSI");
    currentY += 30;

    CreateLabel(L"RPM Maximo:", leftMargin, currentY, labelWidth);
    editMaxRPM.Create(hwndPage, IDC_EDIT_MAX_RPM,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 3000.0, 15000.0, 0);
    CreateTooltip(editMaxRPM.GetHandle(),
        L"RPM maximo do motor.\n"
        L"O turbo deve suprir airflow suficiente neste RPM.");
    currentY += 35;

    // ========== SYSTEM LOSSES ==========
    CreateLabel(L"PERDAS DO SISTEMA:", leftMargin, currentY, 300);
    currentY += 20;

    CreateLabel(L"Queda IC (PSI):", leftMargin, currentY, labelWidth);
    editICPressDrop.Create(hwndPage, IDC_EDIT_IC_PRESS_DROP,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 0.0, 5.0, 1);
    editICPressDrop.SetValue(1.5);
    CreateTooltip(editICPressDrop.GetHandle(),
        L"Queda de pressao no intercooler.\n"
        L"Bar & plate: 1.0-2.0 PSI | Tube & fin: 0.5-1.5 PSI\n"
        L"Ref: Garrett application guidelines");
    currentY += 28;

    CreateLabel(L"Queda Tubulacao (PSI):", leftMargin, currentY, labelWidth);
    editPipingDrop.Create(hwndPage, IDC_EDIT_PIPING_DROP,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 0.0, 5.0, 1);
    editPipingDrop.SetValue(1.0);
    currentY += 28;

    CreateLabel(L"Queda Filtro (PSI):", leftMargin, currentY, labelWidth);
    editFilterDrop.Create(hwndPage, IDC_EDIT_FILTER_DROP,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 0.0, 3.0, 1);
    editFilterDrop.SetValue(0.3);
    currentY += 28;

    CreateLabel(L"Eficiencia IC (%):", leftMargin, currentY, labelWidth);
    editICEfficiency.Create(hwndPage, IDC_EDIT_IC_EFFICIENCY,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 40.0, 98.0, 0);
    editICEfficiency.SetValue(70.0);
    CreateTooltip(editICEfficiency.GetHandle(),
        L"Eficiencia do intercooler (effectiveness).\n"
        L"Air/Air bar&plate: 65-80% | Air/Water: 80-95%\n"
        L"Ref: SAE 2003-01-0732");
    currentY += 28;

    lblDriveRatio = CreateLabelEx(L"Drive Ratio (SC):", leftMargin, currentY, labelWidth);
    editDriveRatio.Create(hwndPage, IDC_EDIT_DRIVE_RATIO,
        leftMargin + labelWidth, currentY - 3, editWidth, 24, 0.5, 5.0, 2);
    editDriveRatio.SetValue(1.0);
    CreateTooltip(editDriveRatio.GetHandle(),
        L"Relacao de transmissao do supercharger.\n"
        L"Polia do SC / Polia do crank.\n"
        L"Tipico: 2.0-3.0 para Roots, 1.0-2.5 para centrifugo.");
    currentY += 35;

    // ========== RESULTS ==========
    CreateLabel(L"Resultados:", leftMargin, currentY);
    currentY += 22;

    textResults = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        leftMargin, currentY, GetResultsWidth(), 220,
        hwndPage, (HMENU)IDC_RESULT_TURBO, hInst, nullptr);
    SendMessage(textResults, WM_SETFONT, (WPARAM)GetAppResultsFont(), TRUE);
    currentY += 225;

    // ========== GRAPH ==========
    WNDCLASSW wc = {};
    wc.lpfnWndProc = GraphWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"EngineCalcGraphTurbo";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    hwndGraph = CreateWindowExW(WS_EX_CLIENTEDGE, L"EngineCalcGraphTurbo", L"",
        WS_CHILD | WS_VISIBLE,
        leftMargin, currentY, GetResultsWidth(), 220,
        hwndPage, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);

    // Initial visibility: Turbo selected by default, hide SC fields
    UpdateVisibility();
}

void TurboTab::Create() {
    RegisterTabPageClass(hInst);

    hwndPage = CreateWindowExW(0, L"EngineCalcTabPage", L"",
        WS_CHILD | WS_VSCROLL,
        0, 30, 800, 450, hwndParent, nullptr, hInst, nullptr);

    SetWindowLongPtr(hwndPage, GWLP_USERDATA, (LONG_PTR)this);
    CreateControls();
    SetContentHeight(1100);
}

void TurboTab::OnCalculate() {
    // Get project data
    EngineDataManager* dm = EngineDataManager::GetInstance();
    EngineCore engine;

    if (dm && dm->GetProject().basicData.bore > 0) {
        const auto& bd = dm->GetProject().basicData;
        engine.SetBore(bd.bore);
        engine.SetStroke(bd.stroke);
        engine.SetCylinders(bd.cylinders);

        if (editMaxRPM.GetValue() == 7000.0 && bd.maxRPM > 0) {
            editMaxRPM.SetValue(bd.maxRPM);
        }
    } else {
        engine.SetBore(86.0);
        engine.SetStroke(86.0);
        engine.SetCylinders(4);
    }
    engine.SetEngineType(EngineType::FOUR_STROKE);

    TurboCalculator calc(&engine);

    int fiType = (int)SendMessage(comboFIType, CB_GETCURSEL, 0, 0);
    int turboConfig = (int)SendMessage(comboTurboConfig, CB_GETCURSEL, 0, 0);
    int scType = (int)SendMessage(comboSCType, CB_GETCURSEL, 0, 0);

    double targetHP = editTargetHP.GetValue();
    double targetBoost = editTargetBoost.GetValue();
    double maxRPM = editMaxRPM.GetValue();

    if (targetHP <= 0 || targetBoost <= 0 || maxRPM <= 0) {
        SetWindowTextW(textResults,
            L"Preencha Potencia Alvo, Boost e RPM Maximo para calcular.\n\n"
            L"Dicas:\n"
            L"  - Va na aba Basica e preencha bore/stroke primeiro\n"
            L"  - Defina a potencia desejada no virabrequim (flywheel HP)\n"
            L"  - Defina o boost alvo em PSI (1 bar = 14.5 PSI)");
        return;
    }

    double displacement = engine.CalculateDisplacement();  // cc

    std::wostringstream results;
    results << std::fixed;

    if (fiType == 0) {
        // ========== TURBOCHARGER ==========
        TurboSizingInput input;
        input.displacementCC = displacement;
        input.targetHP = targetHP;
        input.maxRPM = maxRPM;
        input.numCylinders = engine.GetCylinders();
        input.volumetricEfficiency = 0.85;
        input.targetBoostPSI = targetBoost;
        input.intercoolerPressDrop = editICPressDrop.GetValue();
        input.pipingPressDrop = editPipingDrop.GetValue();
        input.filterPressDrop = editFilterDrop.GetValue();
        input.turboType = static_cast<TurboType>(turboConfig);
        input.numberOfTurbos = (turboConfig == 1 || turboConfig == 2) ? 2 : 1;

        TurboSizingResult r = calc.CalculateTurboSizing(input);

        results << L"===============================================================\r\n";
        results << L"  ANALISE DE TURBOCHARGER\r\n";
        results << L"===============================================================\r\n\r\n";

        results << L"DADOS DO MOTOR:\r\n";
        results << std::setprecision(0);
        results << L"  Cilindrada: " << displacement << L" cc (" << std::setprecision(2)
                << displacement / 1000.0 << L" L)\r\n";
        results << std::setprecision(0);
        results << L"  Potencia Alvo: " << targetHP << L" HP\r\n";
        results << L"  RPM Maximo: " << maxRPM << L"\r\n";
        results << std::setprecision(1);
        results << L"  Boost Alvo: " << targetBoost << L" PSI ("
                << targetBoost / 14.5 << L" bar)\r\n\r\n";

        results << L"AIRFLOW REQUERIDO:\r\n";
        results << std::setprecision(1);
        results << L"  Massa de ar: " << r.requiredAirflowLBM << L" lb/min";
        if (input.numberOfTurbos > 1)
            results << L" (por turbo)";
        results << L"\r\n";
        results << L"  Airflow corrigido: " << r.correctedAirflow << L" lb/min (para mapa)\r\n";
        results << std::setprecision(2);
        results << L"  Pressure Ratio: " << r.pressureRatioTotal << L":1\r\n\r\n";

        results << L"TEMPERATURAS:\r\n";
        results << std::setprecision(0);
        results << L"  Saida compressor: " << r.compressorOutletTempC << L" C\r\n";
        results << L"  Elevacao termica: +" << r.tempRiseAcrossCompressor << L" C\r\n";

        double icEff = editICEfficiency.GetValue() / 100.0;
        double icOut = calc.CalculateIntercoolerOutletTemp(r.compressorOutletTempC, 25.0, icEff);
        results << L"  Pos-intercooler (est.): " << (int)icOut << L" C (IC "
                << (int)(icEff * 100) << L"% eff)\r\n\r\n";

        results << L"DIMENSIONAMENTO:\r\n";
        results << std::setprecision(0);
        results << L"  Classe: " << r.sizeClass << L"\r\n";
        results << std::setprecision(1);
        results << L"  Inducer estimado: " << r.inducer_mm << L" mm\r\n";
        results << L"  Exducer estimado: " << r.exducer_mm << L" mm\r\n";
        results << L"  Roda turbina: " << r.turbineWheelDia_mm << L" mm\r\n";
        results << std::setprecision(0);
        results << L"  Eficiencia compressor est.: " << r.estimatedCompressorEff * 100 << L"%\r\n";
        results << L"  Potencia compressor: " << r.compressorPowerHP << L" HP\r\n\r\n";

        results << L"TURBO RECOMENDADO:\r\n";
        results << L"  Garrett:   " << r.garrettFrame << L"\r\n";
        results << L"  BorgWarner:" << r.borgWarnerFrame << L"\r\n";
        results << L"  Precision: " << r.precisionFrame << L"\r\n\r\n";

        results << L"INTERCOOLER:\r\n";
        results << std::setprecision(1);
        results << L"  Rejeicao termica: " << r.heatRejectionKW << L" kW\r\n";
        results << L"  Volume core est.: " << r.icCoreSizeEstimate << L" L\r\n\r\n";

        // Wastegate
        double exhaustFlowKGS = r.requiredAirflowKGS * 1.05;  // Exhaust ≈ air + fuel
        WastegateResult wg = calc.CalculateWastegate(targetBoost, exhaustFlowKGS, 0.75);
        results << L"WASTEGATE:\r\n";
        results << std::setprecision(0);
        results << L"  Area de fluxo necessaria: " << wg.requiredFlowArea_mm2 << L" mm2\r\n";
        results << std::setprecision(1);
        results << L"  Diametro recomendado: " << wg.recommendedDiameter_mm << L" mm\r\n";
        results << L"  " << wg.recommendation << L"\r\n\r\n";

        if (!r.warnings.empty()) {
            results << L"ALERTAS:\r\n" << r.warnings << L"\r\n";
        }
        if (!r.recommendations.empty()) {
            results << L"RECOMENDACOES:\r\n" << r.recommendations << L"\r\n";
        }

        results << L"===============================================================\r\n";
        results << L"Ref: Heywood Ch.6, Garrett Turbo Tech, BorgWarner Matching Guide\r\n";
        results << L"SAE 2006-01-0038, Bell \"Maximum Boost\"\r\n";

        UpdateGraph(r.correctedAirflow, r.pressureRatioTotal);

    } else {
        // ========== SUPERCHARGER ==========
        SuperchargerType sType = static_cast<SuperchargerType>(scType);
        double driveRatio = editDriveRatio.GetValue();

        SuperchargerResult sr = calc.CalculateSupercharger(
            displacement, targetBoost, maxRPM, driveRatio, sType);

        results << L"===============================================================\r\n";
        results << L"  ANALISE DE SUPERCHARGER\r\n";
        results << L"===============================================================\r\n\r\n";

        results << L"DADOS DO MOTOR:\r\n";
        results << std::setprecision(0);
        results << L"  Cilindrada: " << displacement << L" cc\r\n";
        results << L"  Potencia Alvo: " << targetHP << L" HP\r\n";
        results << std::setprecision(1);
        results << L"  Boost Alvo: " << targetBoost << L" PSI\r\n\r\n";

        results << L"DIMENSIONAMENTO:\r\n";
        results << L"  Tipo: " << sr.typeRecommendation << L"\r\n";
        results << std::setprecision(1);
        results << L"  Displacement SC: " << sr.requiredDisplacementCID << L" CID/rev\r\n";
        results << std::setprecision(0);
        results << L"  Eficiencia isentropica: " << sr.thermalEfficiency * 100 << L"%\r\n";
        results << L"  Temp saida: " << sr.outletTempC << L" C\r\n\r\n";

        results << L"POTENCIA:\r\n";
        results << std::setprecision(1);
        results << L"  Perda parasitica: " << sr.parasticLossHP << L" HP\r\n";
        results << L"  Ganho liquido: " << sr.netHPGain << L" HP\r\n\r\n";

        if (!sr.warnings.empty()) {
            results << L"ALERTAS:\r\n" << sr.warnings << L"\r\n";
        }

        results << L"===============================================================\r\n";
        results << L"Ref: Eaton TVS Guide, Whipple Engineering, Heywood Ch.6\r\n";
    }

    SetWindowTextW(textResults, results.str().c_str());

    // Save to project
    if (dm) {
        EngineProject::TurboData td;
        td.forcedInductionType = fiType + 1;
        td.turboType = turboConfig;
        td.superchargerType = scType;
        td.numberOfTurbos = (turboConfig == 1 || turboConfig == 2) ? 2 : 1;
        td.targetBoostPSI = targetBoost;
        td.targetHP = targetHP;
        td.intercoolerEfficiency = editICEfficiency.GetValue();
        td.intercoolerPressDrop = editICPressDrop.GetValue();
        td.pipingPressDrop = editPipingDrop.GetValue();
        td.filterPressDrop = editFilterDrop.GetValue();
        td.driveRatio = editDriveRatio.GetValue();
        dm->UpdateTurboData(td);
    }
}

void TurboTab::OnClear() {
    editTargetHP.SetValue(0.0);
    editTargetBoost.SetValue(0.0);
    editMaxRPM.SetValue(7000.0);
    editICPressDrop.SetValue(1.5);
    editPipingDrop.SetValue(1.0);
    editFilterDrop.SetValue(0.3);
    editICEfficiency.SetValue(70.0);
    editDriveRatio.SetValue(1.0);
    SendMessage(comboFIType, CB_SETCURSEL, 0, 0);
    SendMessage(comboTurboConfig, CB_SETCURSEL, 0, 0);
    SendMessage(comboSCType, CB_SETCURSEL, 0, 0);
    SetWindowTextW(textResults, L"");
    graphRenderer.ClearSeries();
    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}

void TurboTab::RecalculateLayout() {
    if (!hwndPage) return;
    int labelWidth = GetLabelWidth(0.35f);
    int editWidth = GetEditWidth(0.22f);
    int leftMargin = GetMargin();

    HWND hwndChild = GetWindow(hwndPage, GW_CHILD);
    while (hwndChild) {
        wchar_t className[256];
        GetClassNameW(hwndChild, className, 256);
        RECT rc;
        GetWindowRect(hwndChild, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);

        if (wcscmp(className, L"STATIC") == 0) {
            SetWindowPos(hwndChild, NULL, rc.left, rc.top, labelWidth, rc.bottom - rc.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        } else if (wcscmp(className, L"EDIT") == 0) {
            if (rc.bottom - rc.top > 100) {
                SetWindowPos(hwndChild, NULL, rc.left, rc.top, GetResultsWidth(), rc.bottom - rc.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);
            } else {
                SetWindowPos(hwndChild, NULL, leftMargin + labelWidth, rc.top, editWidth, rc.bottom - rc.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
    }

    if (hwndGraph) {
        RECT rc;
        GetWindowRect(hwndGraph, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(hwndGraph, NULL, rc.left, rc.top, GetResultsWidth(), rc.bottom - rc.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        InvalidateRect(hwndGraph, NULL, TRUE);
    }
    InvalidateRect(hwndPage, NULL, TRUE);
}

LRESULT CALLBACK TurboTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        TurboTab* tab = (TurboTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (tab) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            tab->graphRenderer.SetArea(rc);
            tab->graphRenderer.Render(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    if (msg == WM_ERASEBKGND) return 1;
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void TurboTab::UpdateGraph(double correctedFlow, double pressureRatio) {
    graphRenderer.ClearSeries();

    if (correctedFlow <= 0 || pressureRatio <= 1.0) {
        if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
        return;
    }

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Ponto de Operacao no Mapa do Compressor";
    cfg.xAxisLabel = L"Airflow Corrigido (lb/min)";
    cfg.yAxisLabel = L"Pressure Ratio";
    float maxFlow = (float)(correctedFlow * 2.0);
    cfg.xMin = 0; cfg.xMax = maxFlow;
    cfg.yMin = 1.0f; cfg.yMax = 4.0f;
    cfg.xGridStep = maxFlow > 80 ? 20.0f : 10.0f;
    cfg.yGridStep = 0.5f;

    // Surge line approximation (typical centrifugal compressor)
    std::vector<float> xSurge, ySurge;
    for (float pr = 1.0f; pr <= 4.0f; pr += 0.1f) {
        float surgePt = (pr - 1.0f) * maxFlow * 0.12f;
        xSurge.push_back(surgePt);
        ySurge.push_back(pr);
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xSurge, ySurge,
        Gdiplus::Color(180, 200, 0, 0), L"Surge Line", 2.0f));

    // Choke line approximation
    std::vector<float> xChoke, yChoke;
    for (float pr = 1.0f; pr <= 4.0f; pr += 0.1f) {
        float chokePt = maxFlow * (1.0f - (pr - 1.0f) * 0.08f);
        xChoke.push_back(chokePt);
        yChoke.push_back(pr);
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xChoke, yChoke,
        Gdiplus::Color(180, 0, 0, 200), L"Choke Line", 2.0f));

    // Peak efficiency island (approximate)
    std::vector<float> xEff, yEff;
    float centerFlow = maxFlow * 0.5f;
    for (float t = 0; t <= 6.30f; t += 0.15f) {
        float ex = centerFlow + cosf(t) * maxFlow * 0.18f;
        float ey = 2.2f + sinf(t) * 0.6f;
        xEff.push_back(ex);
        yEff.push_back(ey);
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xEff, yEff,
        Gdiplus::Color(100, 0, 180, 0), L"Peak Eff Zone", 1.5f));

    // Operating point
    std::vector<float> xOp, yOp;
    xOp.push_back((float)correctedFlow);
    yOp.push_back(1.0f);
    xOp.push_back((float)correctedFlow);
    yOp.push_back((float)pressureRatio);
    xOp.push_back(0);
    yOp.push_back((float)pressureRatio);
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xOp, yOp,
        Gdiplus::Color(255, 255, 80, 80), L"Ponto Operacao", 2.5f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
