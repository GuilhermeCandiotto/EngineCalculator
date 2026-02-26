// ============================================================================
// ExhaustTab.cpp - Implementação da Aba de Sistema de Escape
// Engine Calculator v1.0.0 - Fevereiro 2026
// ============================================================================
#include "ExhaustTab.h"
#include "../../Core/EngineCore.h"
#include "../../Core/IntakeExhaustCalculator.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

extern bool RegisterTabPageClass(HINSTANCE hInstance);

ExhaustTab::ExhaustTab(HWND parent, HINSTANCE instance)
    : TabPage(parent, instance), comboExhaustType(nullptr), textResults(nullptr), hwndGraph(nullptr) {
}

ExhaustTab::~ExhaustTab() {
}

void ExhaustTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
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

void ExhaustTab::CreateGroupBox(const wchar_t* text, int x, int y, int width, int height) {
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

void ExhaustTab::CreateControls() {
    int leftMargin = 10;
    int topMargin = 10;
    int labelWidth = GetLabelWidth(0.26f);
    int editWidth = GetEditWidth(0.12f);
    int rowHeight = 32;
    int currentY = topMargin;

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    CreateGroupBox(L"Sistema de Escape", leftMargin, currentY, 850, 240);
    int groupY = currentY + 25;
    int col1X = leftMargin + 15;
    int col2X = 450;

    HWND lblPrimaryLength = CreateWindowW(L"STATIC", L"Comprimento Primario (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblPrimaryLength, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editPrimaryLength.Create(hwndPage, IDC_EDIT_PRIMARY_LENGTH,
        col1X + labelWidth, groupY - 3, editWidth, 22, 200.0, 1200.0, 1);
    CreateTooltip(editPrimaryLength.GetHandle(),
        L"O QUE E: Comprimento do TUBO PRIMARIO (individual de cada cilindro).\n"
        L"Do FLANGE (porta de escape do cabecote) ate a JUNCAO DOS TUBOS.\n\n"
        L"COMO MEDIR: Trena seguindo o tubo do flange ate onde junta os tubos do coletor.\n\n"
        L"Teoria de pulsos: Onda negativa retorna e ajuda scavenging (limpeza).\n"
        L"Tipico: 600-900mm para 5000-7000 RPM");
    groupY += rowHeight;

    HWND lblPrimaryDiameter = CreateWindowW(L"STATIC", L"Diametro Primario (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblPrimaryDiameter, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editPrimaryDiameter.Create(hwndPage, IDC_EDIT_PRIMARY_DIAMETER,
        col1X + labelWidth, groupY - 3, editWidth, 22, 30.0, 60.0, 1);
    CreateTooltip(editPrimaryDiameter.GetHandle(),
        L"O QUE E: Diametro INTERNO do tubo primario.\n"
        L"COMO MEDIR: Paquimetro no diametro interno do tubo.\n"
        L"Regra: 1.2-1.5× diametro da valvula escape");
    groupY += rowHeight;

    HWND lblSecondaryLength = CreateWindowW(L"STATIC", L"Comprimento Secundario (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblSecondaryLength, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editSecondaryLength.Create(hwndPage, IDC_EDIT_SECONDARY_LENGTH,
        col1X + labelWidth, groupY - 3, editWidth, 22, 0.0, 1500.0, 1);
    CreateTooltip(editSecondaryLength.GetHandle(),
        L"O QUE E: Comprimento do tubo APOS o primeiro merge (4-2-1 apenas).\n"
        L"COMO ESCOLHER: Tri-Y (4-2-1) = melhor torque medio.\n"
        L"4-1 = melhor potencia alta.\n"
        L"Deixe 0 para configuracao 4-1");
    groupY += rowHeight;

    HWND lblSecondaryDiameter = CreateWindowW(L"STATIC", L"Diametro Secundario (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblSecondaryDiameter, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editSecondaryDiameter.Create(hwndPage, IDC_EDIT_SECONDARY_DIAMETER,
        col1X + labelWidth, groupY - 3, editWidth, 22, 40.0, 80.0, 1);
    CreateTooltip(editSecondaryDiameter.GetHandle(),
        L"O QUE E: Diametro do tubo secundario (4-2-1).\n"
        L"COMO CALCULAR: Diametro primario × 1.7.\n"
        L"Exemplo: Primario 42mm -> Secundario ~70mm");

    groupY = currentY + 25;
    
    HWND lblCollectorDiameter = CreateWindowW(L"STATIC", L"Diametro do Coletor (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblCollectorDiameter, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editCollectorDiameter.Create(hwndPage, IDC_EDIT_COLLECTOR_DIAMETER,
        col2X + labelWidth, groupY - 3, editWidth, 22, 50.0, 100.0, 1);
    CreateTooltip(editCollectorDiameter.GetHandle(),
        L"O QUE E: Diametro onde TODOS os tubos se juntam.\n"
        L"COMO CALCULAR: Diametro primario × raiz(no cilindros).\n"
        L"Exemplo 4 cil: 42mm × 2 = 84mm coletor");
    groupY += rowHeight;

    HWND lblTargetRPM = CreateWindowW(L"STATIC", L"RPM Alvo:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblTargetRPM, WM_SETFONT, (WPARAM)hFont, TRUE);
    CreateTooltip(lblTargetRPM,
        L"O QUE E: RPM onde voce quer pico de scavenging.\n"
        L"Software calcula comprimento ideal para esta RPM");
    
    editTargetRPM.Create(hwndPage, IDC_EDIT_TARGET_RPM_EXHAUST,
        col2X + labelWidth, groupY - 3, editWidth, 22, 2000.0, 12000.0, 0);
    editTargetRPM.SetValue(6000.0);
    CreateTooltip(editTargetRPM.GetHandle(),
        L"O QUE E: RPM onde voce quer pico de scavenging.\n"
        L"COMO ESCOLHER: Geralmente mesmo RPM da admissao.\n"
        L"Software calcula comprimento ideal para esta RPM");
    groupY += rowHeight;

    HWND lblExhaustDuration = CreateWindowW(L"STATIC", L"Duracao Escape @ 0.050\" (graus):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblExhaustDuration, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editExhaustDuration.Create(hwndPage, IDC_EDIT_EXHAUST_DUR_CALC,
        col2X + labelWidth, groupY - 3, editWidth, 22, 180.0, 320.0, 0);
    editExhaustDuration.SetValue(220.0);
    CreateTooltip(editExhaustDuration.GetHandle(),
        L"O QUE E: Duracao do comando de escape.\n"
        L"COMO OBTER: Mesmo valor da aba Comando.\n"
        L"Usado para calcular QUANDO a onda negativa retorna.");
    groupY += rowHeight;

    HWND lblExhaustType = CreateWindowW(L"STATIC", L"Tipo de Configuracao:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblExhaustType, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    comboExhaustType = CreateWindowW(
        L"COMBOBOX",
        L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        col2X + labelWidth, groupY - 3, editWidth, 100,
        hwndPage,
        (HMENU)IDC_COMBO_EXHAUST_TYPE,
        hInst,
        nullptr
    );
    SendMessage(comboExhaustType, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(comboExhaustType, CB_ADDSTRING, 0, (LPARAM)L"4-1 (Four-Into-One)");
    SendMessageW(comboExhaustType, CB_ADDSTRING, 0, (LPARAM)L"4-2-1 (Tri-Y)");
    SendMessageW(comboExhaustType, CB_SETCURSEL, 1, 0);
    CreateTooltip(comboExhaustType,
        L"O QUE E: Arquitetura do coletor.\n"
        L"4-1 = Todos tubos juntam de uma vez\n"
        L"4-2-1 (Tri-Y) = Junta em PARES primeiro");

    currentY += 250;

    CreateLabel(L"Analise do Sistema de Escape:", leftMargin, currentY);
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
    wc.lpszClassName = L"EngineCalcGraphExh";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    hwndGraph = CreateWindowExW(WS_EX_CLIENTEDGE, L"EngineCalcGraphExh", L"",
        WS_CHILD | WS_VISIBLE, leftMargin, currentY, GetResultsWidth(0.97f), 220,
        hwndPage, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);
}

void ExhaustTab::Create() {
    RegisterTabPageClass(hInst);

    hwndPage = CreateWindowExW(
        0, L"EngineCalcTabPage", L"",
        WS_CHILD | WS_VSCROLL,
        0, 30, 880, 520, hwndParent, nullptr, hInst, nullptr);

    SetWindowLongPtr(hwndPage, GWLP_USERDATA, (LONG_PTR)this);
    CreateControls();
    SetContentHeight(800);
}

void ExhaustTab::OnCalculate() {
    EngineDataManager* dataManager = EngineDataManager::GetInstance();
    if (dataManager) {
        const EngineProject& project = dataManager->GetProject();
        
        if (project.camshaftData.exhaustDuration > 0 && editExhaustDuration.GetValue() == 220.0) {
            editExhaustDuration.SetValue(project.camshaftData.exhaustDuration);
        }
        
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

    ExhaustManifoldConfig config;
    config.primaryLength = editPrimaryLength.GetValue();
    config.primaryDiameter = editPrimaryDiameter.GetValue();
    config.secondaryLength = editSecondaryLength.GetValue();
    config.secondaryDiameter = editSecondaryDiameter.GetValue();
    config.collectorDiameter = editCollectorDiameter.GetValue();
    config.isFourIntoOne = (SendMessage(comboExhaustType, CB_GETCURSEL, 0, 0) == 0);
    config.mergeLengthRatio = 1.0;

    if (!calc.IsValidExhaustConfig(config)) {
        SetWindowTextW(textResults, L"Configuracao invalida. Verifique os valores inseridos.");
        return;
    }

    double targetRPM = editTargetRPM.GetValue();
    double exhaustDur = editExhaustDuration.GetValue();

    PulseAnalysis analysis = calc.CalculatePulseAnalysis(config, exhaustDur, targetRPM);
    double backpressure = calc.CalculateBackpressure(config, targetRPM, engine.CalculateDisplacement());

    std::wostringstream results;
    results << std::fixed << std::setprecision(1);
    results << L"---------------------------------------------------------------------------\r\n";
    results << L"  ANALISE DO SISTEMA DE ESCAPE\r\n";
    results << L"---------------------------------------------------------------------------\r\n\r\n";

    results << L"ANALISE DE PULSOS:\r\n";
    results << L"  Frequencia Primaria: " << analysis.primaryPulseFrequency << L" Hz\r\n";
    results << L"  RPM de Sintonia: " << analysis.tuningRPM << L" rpm ";
    double rpmDiff = std::abs(targetRPM - analysis.tuningRPM);
    if (rpmDiff < 500) results << L"-> Perfeito\r\n";
    else if (rpmDiff < 1000) results << L"-> Bom\r\n";
    else if (rpmDiff < 2000) results << L"-> Aceitavel\r\n";
    else results << L"-> Muito diferente\r\n\r\n";

    results << L"BACKPRESSURE:\r\n";
    results << L"  Contrapressao: " << backpressure << L" kPa ";
    if (backpressure < 3.0) results << L"-> Excelente\r\n";
    else if (backpressure < 7.0) results << L"-> Bom\r\n";
    else if (backpressure < 14.0) results << L"-> Alto\r\n";
    else results << L"-> Muito alto - reduz potencia\r\n\r\n";

    results << L"RECOMENDACOES:\r\n";
    results << calc.GetExhaustAdvice(config, targetRPM);
    results << L"\r\n";
    results << L"---------------------------------------------------------------------------\r\n";
    results << L"Referencias: Blair - Design and Simulation of Four-Stroke Engines, Ch. 9\r\n";
    results << L"            Heywood - IC Engine Fundamentals, Ch. 6";

    SetWindowTextW(textResults, results.str().c_str());

    UpdateGraph(targetRPM, exhaustDur);

    if (dataManager) {
        EngineProject::ExhaustData exhaustData;
        exhaustData.primaryLength = config.primaryLength;
        exhaustData.primaryDiameter = config.primaryDiameter;
        exhaustData.secondaryLength = config.secondaryLength;
        exhaustData.secondaryDiameter = config.secondaryDiameter;
        exhaustData.collectorDiameter = config.collectorDiameter;
        exhaustData.targetRPM = targetRPM;
        exhaustData.exhaustDuration = exhaustDur;
        exhaustData.isFourIntoOne = config.isFourIntoOne;
        
        dataManager->UpdateExhaustData(exhaustData);
    }
}

void ExhaustTab::OnClear() {
    editPrimaryLength.SetValue(0.0);
    editPrimaryDiameter.SetValue(0.0);
    editSecondaryLength.SetValue(0.0);
    editSecondaryDiameter.SetValue(0.0);
    editCollectorDiameter.SetValue(0.0);
    editTargetRPM.SetValue(6000.0);
    editExhaustDuration.SetValue(220.0);
    SendMessage(comboExhaustType, CB_SETCURSEL, 1, 0);
    SetWindowTextW(textResults, L"");
}

void ExhaustTab::RecalculateLayout() {
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

LRESULT CALLBACK ExhaustTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        ExhaustTab* tab = (ExhaustTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

void ExhaustTab::UpdateGraph(double targetRPM, double exhaustDuration) {
    graphRenderer.ClearSeries();
    if (targetRPM <= 0 || exhaustDuration <= 0) { if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE); return; }

    EngineCore eng;
    EngineDataManager* dm = EngineDataManager::GetInstance();
    if (dm && dm->GetProject().basicData.bore > 0) {
        const auto& bd = dm->GetProject().basicData;
        eng.SetBore(bd.bore); eng.SetStroke(bd.stroke); eng.SetCylinders(bd.cylinders);
    } else {
        eng.SetBore(86.0); eng.SetStroke(86.0); eng.SetCylinders(4);
    }
    IntakeExhaustCalculator calc(&eng);

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Comprimento Primario Ideal vs RPM";
    cfg.xAxisLabel = L"RPM";
    cfg.yAxisLabel = L"Comprimento (mm)";
    float maxRPM = (float)(targetRPM * 1.5);
    cfg.xMin = 2000; cfg.xMax = maxRPM;
    cfg.yMin = 0; cfg.yMax = 1500;
    cfg.xGridStep = maxRPM > 10000 ? 2000.0f : 1000.0f;
    cfg.yGridStep = 250;

    std::vector<float> xRPM, yLen;
    for (float rpm = 2000; rpm <= maxRPM; rpm += 100) {
        float len = (float)calc.CalculateIdealPrimaryLength(exhaustDuration, rpm);
        xRPM.push_back(rpm); yLen.push_back(len);
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xRPM, yLen,
        Gdiplus::Color(255, 255, 140, 0), L"Primario Ideal", 2.5f));

    std::vector<float> xM, yM;
    float idealLen = (float)calc.CalculateIdealPrimaryLength(exhaustDuration, targetRPM);
    xM.push_back((float)targetRPM); yM.push_back(0);
    xM.push_back((float)targetRPM); yM.push_back(idealLen);
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xM, yM,
        Gdiplus::Color(255, 255, 80, 80), L"RPM Alvo", 1.5f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
