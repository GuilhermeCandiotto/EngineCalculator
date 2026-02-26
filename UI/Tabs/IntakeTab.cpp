// ============================================================================
// IntakeTab.cpp - Implementação da Aba de Sistema de Admissão
// Engine Calculator v1.0.0 - Fevereiro 2026
// ============================================================================
#include "IntakeTab.h"
#include "../../Core/EngineCore.h"
#include "../../Core/IntakeExhaustCalculator.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

extern bool RegisterTabPageClass(HINSTANCE hInstance);

IntakeTab::IntakeTab(HWND parent, HINSTANCE instance)
    : TabPage(parent, instance), textResults(nullptr), hwndGraph(nullptr) {
}

IntakeTab::~IntakeTab() {
}

void IntakeTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
    HWND label = CreateWindowW(
        L"STATIC",
        text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, width, 20,
        hwndPage,
        nullptr,
        hInst,
        nullptr
    );

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(label, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void IntakeTab::CreateGroupBox(const wchar_t* text, int x, int y, int width, int height) {
    HWND groupBox = CreateWindowW(
        L"BUTTON",
        text,
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        x, y, width, height,
        hwndPage,
        nullptr,
        hInst,
        nullptr
    );

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(groupBox, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void IntakeTab::CreateControls() {
    int leftMargin = 10;
    int topMargin = 10;
    int labelWidth = GetLabelWidth(0.25f);
    int editWidth = GetEditWidth(0.12f);
    int rowHeight = 32;
    int currentY = topMargin;

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    CreateGroupBox(L"Sistema de Admissao", leftMargin, currentY, 850, 240);
    int groupY = currentY + 25;
    int col1X = leftMargin + 15;
    int col2X = 450;

    HWND lblRunnerLength = CreateWindowW(L"STATIC", L"Comprimento do Runner (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRunnerLength, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editRunnerLength.Create(hwndPage, IDC_EDIT_RUNNER_LENGTH,
        col1X + labelWidth, groupY - 3, editWidth, 22, 100.0, 800.0, 1);
    CreateTooltip(editRunnerLength.GetHandle(),
        L"O QUE E: Comprimento do tubo que leva ar do plenum ate a valvula.\n"
        L"COMO MEDIR: Trena/fita metrica do flange da borboleta ate a valvula.\n"
        L"Teoria de Helmholtz: sintoniza ressonancia em RPM especifico");
    groupY += rowHeight;

    HWND lblRunnerDiameter = CreateWindowW(L"STATIC", L"Diametro do Runner (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRunnerDiameter, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editRunnerDiameter.Create(hwndPage, IDC_EDIT_RUNNER_DIAMETER,
        col1X + labelWidth, groupY - 3, editWidth, 22, 30.0, 80.0, 1);
    CreateTooltip(editRunnerDiameter.GetHandle(),
        L"O QUE E: Diametro INTERNO do tubo de admissao.\n"
        L"COMO MEDIR: Paquimetro no diametro interno do tubo.\n"
        L"Afeta velocidade e volume de ar");
    groupY += rowHeight;

    HWND lblPlenumVolume = CreateWindowW(L"STATIC", L"Volume do Plenum (litros):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblPlenumVolume, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editPlenumVolume.Create(hwndPage, IDC_EDIT_PLENUM_VOLUME,
        col1X + labelWidth, groupY - 3, editWidth, 22, 1.0, 20.0, 2);
    CreateTooltip(editPlenumVolume.GetHandle(),
        L"O QUE E: Volume da camara central (caixa) que distribui ar.\n"
        L"COMO CALCULAR: Volume = Comprimento × Largura × Altura / 1000.\n"
        L"Regra basica: 1.5 a 2× a cilindrada do motor.\n"
        L"Exemplo: Motor 2.0L = plenum 3-4L ideal.\n"
        L"Muito pequeno = restricao, Muito grande = perda de pulso");
    groupY += rowHeight;

    HWND lblTrumpetLength = CreateWindowW(L"STATIC", L"Comprimento da Corneta (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblTrumpetLength, WM_SETFONT, (WPARAM)hFont, TRUE);
    CreateTooltip(lblTrumpetLength,
        L"O QUE E: Velocity stack - corneta na entrada do runner.\n"
        L"Suaviza fluxo e aumenta eficiencia volumetrica em 2-5%");
    
    editTrumpetLength.Create(hwndPage, IDC_EDIT_TRUMPET_LENGTH,
        col1X + labelWidth, groupY - 3, editWidth, 22, 0.0, 200.0, 1);
    editTrumpetLength.SetValue(75.0);
    CreateTooltip(editTrumpetLength.GetHandle(),
        L"O QUE E: Velocity stack - corneta na entrada do runner.\n"
        L"COMO MEDIR: Regua do topo do plenum ate a boca da corneta.\n"
        L"Suaviza fluxo e aumenta eficiencia volumetrica em 2-5%.");
    groupY += rowHeight;

    HWND lblTrumpetDiameter = CreateWindowW(L"STATIC", L"Diametro da Corneta (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblTrumpetDiameter, WM_SETFONT, (WPARAM)hFont, TRUE);
    CreateTooltip(lblTrumpetDiameter,
        L"O QUE E: Diametro da BOCA (abertura) da corneta.\n"
        L"Geralmente 10-20mm maior que o runner");
    
    editTrumpetDiameter.Create(hwndPage, IDC_EDIT_TRUMPET_DIAMETER,
        col1X + labelWidth, groupY - 3, editWidth, 22, 30.0, 120.0, 1);
    editTrumpetDiameter.SetValue(60.0);
    CreateTooltip(editTrumpetDiameter.GetHandle(),
        L"O QUE E: Diametro da BOCA (abertura) da corneta.\n"
        L"COMO MEDIR: Paquimetro no diametro externo da boca.\n"
        L"Geralmente 10-20mm maior que o runner.\n"
        L"Exemplo: Runner 50mm -> Corneta 60-70mm");

    groupY = currentY + 25;
    
    HWND lblTargetRPM = CreateWindowW(L"STATIC", L"RPM Alvo de Sintonia:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblTargetRPM, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editTargetRPM.Create(hwndPage, IDC_EDIT_TARGET_RPM_INTAKE,
        col2X + labelWidth, groupY - 3, editWidth, 22, 2000.0, 12000.0, 0);
    editTargetRPM.SetValue(6000.0);
    CreateTooltip(editTargetRPM.GetHandle(),
        L"O QUE E: RPM onde voce quer PICO de eficiencia volumetrica.\n"
        L"COMO ESCOLHER: Use o RPM de pico de torque desejado.\n"
        L"Helmholtz sintoniza ressonancia para maximizar potencia nesta RPM.\n"
        L"Software calcula comprimento ideal para esta RPM.\n"
        L"±500 RPM = bom, ±1000 RPM = aceitavel");
    groupY += rowHeight;

    HWND lblBoostPressure = CreateWindowW(L"STATIC", L"Pressao de Boost (PSI):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblBoostPressure, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    CreateLabel(L"(0 = Aspirado)", col2X + 10, groupY + 18, 150);
    editBoostPressure.Create(hwndPage, IDC_EDIT_BOOST_PRESSURE,
        col2X + labelWidth, groupY - 3, editWidth, 22, 0.0, 30.0, 1);
    editBoostPressure.SetValue(0.0);
    CreateTooltip(editBoostPressure.GetHandle(),
        L"O QUE E: Pressao POSITIVA gerada por turbo/supercharger.\n"
        L"COMO MEDIR: Manometro de boost no coletor de admissao.\n"
        L"0 PSI = Motor aspirado (sem turbo).");

    currentY += 250;

    CreateLabel(L"Analise do Sistema de Admissao:", leftMargin, currentY);
    currentY += 25;

    textResults = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        leftMargin, currentY, GetResultsWidth(0.97f), 220,
        hwndPage,
        (HMENU)IDC_RESULT_TEXT,
        hInst,
        nullptr
    );

    SendMessage(textResults, WM_SETFONT, (WPARAM)GetAppResultsFont(), TRUE);

    currentY += 225;
    WNDCLASSW wc = {};
    wc.lpfnWndProc = GraphWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"EngineCalcGraphIntake";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    hwndGraph = CreateWindowExW(WS_EX_CLIENTEDGE, L"EngineCalcGraphIntake", L"",
        WS_CHILD | WS_VISIBLE, leftMargin, currentY, GetResultsWidth(0.97f), 220,
        hwndPage, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);
}

void IntakeTab::Create() {
    RegisterTabPageClass(hInst);

    hwndPage = CreateWindowExW(
        0,
        L"EngineCalcTabPage",
        L"",
        WS_CHILD | WS_VSCROLL,
        0, 30, 880, 520,
        hwndParent,
        nullptr,
        hInst,
        nullptr
    );

    SetWindowLongPtr(hwndPage, GWLP_USERDATA, (LONG_PTR)this);
    CreateControls();
    SetContentHeight(800);
}

void IntakeTab::OnCalculate() {
    EngineDataManager* dataManager = EngineDataManager::GetInstance();
    if (dataManager) {
        const EngineProject& project = dataManager->GetProject();
        
        if (project.basicData.maxRPM > 0 && editTargetRPM.GetValue() == 6000.0) {
            editTargetRPM.SetValue(project.basicData.maxRPM);
        }
    }
    
    EngineCore engine;
    EngineDataManager* dm = EngineDataManager::GetInstance();
    if (dm && dm->GetProject().basicData.bore > 0) {
        const auto& bd = dm->GetProject().basicData;
        engine.SetBore(bd.bore);
        engine.SetStroke(bd.stroke);
        engine.SetCylinders(bd.cylinders);
    } else {
        engine.SetBore(86.0);
        engine.SetStroke(86.0);
        engine.SetCylinders(4);
    }
    engine.SetEngineType(EngineType::FOUR_STROKE);

    IntakeExhaustCalculator calc(&engine);

    IntakeManifoldConfig config;
    config.runnerLength = editRunnerLength.GetValue();
    config.runnerDiameter = editRunnerDiameter.GetValue();
    config.plenumVolume = editPlenumVolume.GetValue();
    config.trumpetLength = editTrumpetLength.GetValue();
    config.trumpetDiameter = editTrumpetDiameter.GetValue();
    config.numberOfRunners = 1;

    if (!calc.IsValidIntakeConfig(config)) {
        SetWindowTextW(textResults, L"Configuracao invalida. Verifique os valores inseridos.");
        return;
    }

    double targetRPM = editTargetRPM.GetValue();
    double boostPSI = editBoostPressure.GetValue();

    IntakeAnalysis analysis = calc.CalculateIntakeAnalysis(config, targetRPM);

    std::wostringstream results;
    results << std::fixed << std::setprecision(1);
    results << L"---------------------------------------------------------------------------\r\n";
    results << L"  ANALISE DO SISTEMA DE ADMISSAO\r\n";
    results << L"---------------------------------------------------------------------------\r\n\r\n";

    results << L"RESSONANCIA DE HELMHOLTZ:\r\n";
    results << L"  Frequencia: " << analysis.helmholtzFrequency << L" Hz\r\n";
    results << L"  RPM de Sintonia: " << analysis.tuningRPM << L" rpm\r\n";
    double rpmDiff = std::abs(targetRPM - analysis.tuningRPM);
    if (rpmDiff < 500) results << L"  -> PERFEITAMENTE SINTONIZADO!\r\n";
    else if (rpmDiff < 1000) results << L"  -> Bem sintonizado\r\n";
    else if (rpmDiff < 2000) results << L"  -> Sintonia aceitavel\r\n";
    else results << L"  -> Fora da sintonia ideal\r\n\r\n";

    results << L"CARACTERISTICAS DE FLUXO:\r\n";
    results << std::setprecision(1);
    results << L"  Velocidade do Fluxo: " << analysis.flowVelocity << L" m/s ";
    if (analysis.flowVelocity < 50) results << L"-> Muito lenta\r\n";
    else if (analysis.flowVelocity > 90) results << L"-> Muito rapida\r\n";
    else results << L"-> Ideal\r\n";

    results << L"  Numero de Reynolds: " << static_cast<int>(analysis.reynoldsNumber) << L" ";
    if (analysis.reynoldsNumber < 2300) results << L"(Laminar)\r\n";
    else if (analysis.reynoldsNumber < 4000) results << L"(Transicao)\r\n";
    else results << L"(Turbulento)\r\n";

    results << std::setprecision(2);
    results << L"  Efeito Ram: +" << analysis.ramEffectPressure << L" kPa\r\n";
    results << L"  Eficiencia Volumetrica: " << analysis.volumetricEfficiency << L" %\r\n\r\n";

    if (boostPSI > 0.0) {
        results << L"INDUCAO FORCADA:\r\n";
        results << calc.AnalyzeForForcedInduction(config, boostPSI);
        double intercoolerVol = calc.CalculateIntercoolerVolume(engine.CalculateDisplacement(), boostPSI);
        results << L"  Volume Intercooler Recomendado: " << intercoolerVol << L" litros\r\n\r\n";
    }

    results << L"RECOMENDACOES:\r\n";
    results << calc.GetIntakeAdvice(config, targetRPM);
    results << L"\r\n";
    results << L"---------------------------------------------------------------------------\r\n";
    results << L"Referencias: Blair - Design and Simulation of Four-Stroke Engines, Ch. 8\r\n";
    results << L"            SAE Paper 2003-01-0001 - Intake System Design";

    SetWindowTextW(textResults, results.str().c_str());

    UpdateGraph(analysis.tuningRPM, targetRPM);

    if (dataManager) {
        EngineProject::IntakeData intakeData;
        intakeData.runnerLength = config.runnerLength;
        intakeData.runnerDiameter = config.runnerDiameter;
        intakeData.plenumVolume = config.plenumVolume;
        intakeData.trumpetLength = config.trumpetLength;
        intakeData.trumpetDiameter = config.trumpetDiameter;
        intakeData.targetRPM = targetRPM;
        intakeData.boostPressure = boostPSI;
        
        dataManager->UpdateIntakeData(intakeData);
    }
}

void IntakeTab::OnClear() {
    editRunnerLength.SetValue(0.0);
    editRunnerDiameter.SetValue(0.0);
    editPlenumVolume.SetValue(0.0);
    editTrumpetLength.SetValue(75.0);
    editTrumpetDiameter.SetValue(60.0);
    editTargetRPM.SetValue(6000.0);
    editBoostPressure.SetValue(0.0);
    SetWindowTextW(textResults, L"");
}

void IntakeTab::RecalculateLayout() {
    if (!hwndPage) return;
    int newWidth = GetResultsWidth(0.97f);
    if (textResults) {
        RECT rc; GetWindowRect(textResults, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(textResults, NULL, rc.left, rc.top, newWidth, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    if (hwndGraph) {
        RECT rc; GetWindowRect(hwndGraph, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(hwndGraph, NULL, rc.left, rc.top, newWidth, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
        InvalidateRect(hwndGraph, NULL, TRUE);
    }
}

LRESULT CALLBACK IntakeTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        IntakeTab* tab = (IntakeTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (tab) {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc; GetClientRect(hwnd, &rc);
            tab->graphRenderer.SetArea(rc);
            tab->graphRenderer.Render(hdc);
            EndPaint(hwnd, &ps); return 0;
        }
    }
    if (msg == WM_ERASEBKGND) return 1;
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void IntakeTab::UpdateGraph(double tuningRPM, double targetRPM) {
    graphRenderer.ClearSeries();
    if (tuningRPM <= 0 || targetRPM <= 0) { if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE); return; }

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"VE Estimada vs RPM";
    cfg.xAxisLabel = L"RPM";
    cfg.yAxisLabel = L"VE (%)";
    float maxRPM = (float)(targetRPM * 1.5);
    cfg.xMin = 1000; cfg.xMax = maxRPM;
    cfg.yMin = 65; cfg.yMax = 105;
    cfg.xGridStep = maxRPM > 10000 ? 2000.0f : 1000.0f;
    cfg.yGridStep = 5;

    // Curva VE simplificada com pico na tuningRPM
    std::vector<float> xRPM, yVE;
    for (float rpm = 1000; rpm <= maxRPM; rpm += 100) {
        float diff = fabsf(rpm - (float)tuningRPM) / (float)tuningRPM;
        float ve = 85.0f + 15.0f * expf(-diff * diff * 8.0f); // Gaussiana centrada na tuningRPM
        if (ve > 100) ve = 100;
        xRPM.push_back(rpm); yVE.push_back(ve);
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xRPM, yVE,
        Gdiplus::Color(255, 0, 150, 255), L"VE Estimada", 2.5f));

    // Marcador RPM alvo
    std::vector<float> xM, yM;
    xM.push_back((float)targetRPM); yM.push_back(65);
    xM.push_back((float)targetRPM); yM.push_back(105);
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xM, yM,
        Gdiplus::Color(255, 255, 80, 80), L"RPM Alvo", 1.5f));

    // Marcador RPM sintonia
    std::vector<float> xT, yT;
    xT.push_back((float)tuningRPM); yT.push_back(65);
    xT.push_back((float)tuningRPM); yT.push_back(105);
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xT, yT,
        Gdiplus::Color(255, 0, 200, 80), L"RPM Sintonia", 1.5f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
