// ============================================================================
// CamshaftTab.cpp - Implementação da Aba de Comando de Válvulas
// Engine Calculator v1.0.0 - Fevereiro 2026
// ============================================================================
#include "CamshaftTab.h"
#include "../../Core/EngineCore.h"
#include "../../Core/CamshaftCalculator.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

extern bool RegisterTabPageClass(HINSTANCE hInstance);

CamshaftTab::CamshaftTab(HWND parent, HINSTANCE instance)
    : TabPage(parent, instance), textResults(nullptr), hwndGraph(nullptr) {
}

CamshaftTab::~CamshaftTab() {
}

void CamshaftTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
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

void CamshaftTab::CreateGroupBox(const wchar_t* text, int x, int y, int width, int height) {
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

void CamshaftTab::CreateControls() {
    int leftMargin = 10;
    int topMargin = 10;
    int labelWidth = GetLabelWidth(0.25f);
    int editWidth = GetEditWidth(0.11f);
    int rowHeight = 32;
    int currentY = topMargin;

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    CreateGroupBox(L"Especificacoes do Comando de Valvulas", leftMargin, currentY, 850, 200);
    int groupY = currentY + 25;
    int col1X = leftMargin + 15;
    int col2X = 450;

    HWND lblIntakeDuration = CreateWindowW(L"STATIC", L"Duracao Admissao @ 0.050\" (graus):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblIntakeDuration, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editIntakeDuration.Create(hwndPage, IDC_EDIT_INTAKE_DURATION,
        col1X + labelWidth, groupY - 3, editWidth, 22, 180.0, 320.0, 1);
    CreateTooltip(editIntakeDuration.GetHandle(),
        L"Graus de rotacao que a valvula fica aberta a 0.050\" (1.27mm).\n"
        L"Encontrado no datasheet do comando");
    groupY += rowHeight;

    HWND lblExhaustDuration = CreateWindowW(L"STATIC", L"Duracao Escape @ 0.050\" (graus):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblExhaustDuration, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editExhaustDuration.Create(hwndPage, IDC_EDIT_EXHAUST_DURATION,
        col1X + labelWidth, groupY - 3, editWidth, 22, 180.0, 320.0, 1);
    CreateTooltip(editExhaustDuration.GetHandle(),
        L"Graus de rotacao que a valvula de escape fica aberta a 0.050\" (1.27mm).\n"
        L"Encontrado no datasheet do comando.");
    groupY += rowHeight;

    HWND lblIntakeLift = CreateWindowW(L"STATIC", L"Lift Admissao na Valvula (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblIntakeLift, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editIntakeLift.Create(hwndPage, IDC_EDIT_INTAKE_LIFT_CAM,
        col1X + labelWidth, groupY - 3, editWidth, 22, 5.0, 18.0, 2);
    CreateTooltip(editIntakeLift.GetHandle(),
        L"Abertura maxima da valvula de ADMISSAO.\n"
        L"Ja multiplicado pelo rocker ratio");
    groupY += rowHeight;

    HWND lblExhaustLift = CreateWindowW(L"STATIC", L"Lift Escape na Valvula (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col1X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblExhaustLift, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editExhaustLift.Create(hwndPage, IDC_EDIT_EXHAUST_LIFT_CAM,
        col1X + labelWidth, groupY - 3, editWidth, 22, 5.0, 18.0, 2);
    CreateTooltip(editExhaustLift.GetHandle(),
        L"Abertura maxima da valvula de ESCAPE.\n"
        L"Ja multiplicado pelo rocker ratio");

    groupY = currentY + 25;
    
    HWND lblLSA = CreateWindowW(L"STATIC", L"LSA - Lobe Separation Angle (graus):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblLSA, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editLSA.Create(hwndPage, IDC_EDIT_LSA,
        col2X + labelWidth, groupY - 3, editWidth, 22, 100.0, 120.0, 1);
    editLSA.SetValue(110.0);
    CreateTooltip(editLSA.GetHandle(),
        L"LSA = Angulo entre os picos dos lobos de admissao e escape.\n"
        L"LSA pequeno = overlap alto, LSA grande = overlap baixo\n\n"
        L"Estreito (104-108 graus): +potencia, -torque, -vacuo\n"
        L"Medio (108-112 graus): Equilibrado\n"
        L"Largo (112-116 graus): +torque, +vacuo, -potencia alta");
    groupY += rowHeight;

    HWND lblAdvanceRetard = CreateWindowW(L"STATIC", L"Avanco/Retardo (graus):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblAdvanceRetard, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    CreateLabel(L"(+ avanca, - atrasa)", col2X + 10, groupY + 18, 200);
    editAdvanceRetard.Create(hwndPage, IDC_EDIT_ADVANCE_RETARD,
        col2X + labelWidth, groupY - 3, editWidth, 22, -10.0, 10.0, 1);
    editAdvanceRetard.SetValue(0.0);
    CreateTooltip(editAdvanceRetard.GetHandle(),
        L"Ajuste do timing do comando.\n"
        L"Tipico: -2 graus a +4 graus.\n"
        L"+4 graus = melhor torque baixo, -2 graus = melhor potencia alta");
    groupY += rowHeight + 20;

    HWND lblRockerRatio = CreateWindowW(L"STATIC", L"Relacao do Balanceiro:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRockerRatio, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editRockerRatio.Create(hwndPage, IDC_EDIT_ROCKER_RATIO_CAM,
        col2X + labelWidth, groupY - 3, editWidth, 22, 1.0, 2.5, 2);
    editRockerRatio.SetValue(1.5);
    CreateTooltip(editRockerRatio.GetHandle(),
        L"Multiplicacao do lift do came.\n"
        L"Mesmo que na aba Trem de Valvulas\n"
        L"Se mudar aqui, mudar tambem em Trem de Valvulas!");
    groupY += rowHeight;

    HWND lblCamRPM = CreateWindowW(L"STATIC", L"RPM de Analise:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, col2X, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblCamRPM, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editCamRPM.Create(hwndPage, IDC_EDIT_CAM_RPM,
        col2X + labelWidth, groupY - 3, editWidth, 22, 1000.0, 12000.0, 0);
    editCamRPM.SetValue(6000.0);
    CreateTooltip(editCamRPM.GetHandle(),
        L"RPM para analise dos eventos e faixas de potencia.\n"
        L"Use o RPM de pico desejado\n"
        L"Software estima faixa de RPM util baseado na duracao");

    currentY += 210;

    CreateLabel(L"Analise Completa do Comando:", leftMargin, currentY);
    currentY += 25;

    textResults = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        leftMargin, currentY, GetResultsWidth(0.97f), 260,
        hwndPage,
        (HMENU)IDC_RESULT_TEXT,
        hInst,
        nullptr
    );

    SendMessage(textResults, WM_SETFONT, (WPARAM)GetAppResultsFont(), TRUE);

    currentY += 265;
    WNDCLASSW wc = {};
    wc.lpfnWndProc = GraphWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"EngineCalcGraph";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    hwndGraph = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EngineCalcGraph",
        L"",
        WS_CHILD | WS_VISIBLE,
        leftMargin, currentY, GetResultsWidth(0.97f), 220,
        hwndPage,
        nullptr,
        hInst,
        nullptr
    );
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);
}

void CamshaftTab::Create() {
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
    SetContentHeight(1000);
}

void CamshaftTab::OnCalculate() {
    EngineDataManager* dataManager = EngineDataManager::GetInstance();
    if (dataManager) {
        const EngineProject& project = dataManager->GetProject();
        
        if (project.basicData.maxRPM > 0 && editCamRPM.GetValue() == 6000.0) {
            editCamRPM.SetValue(project.basicData.maxRPM);
        }
    }
    
    EngineCore engine;

    // Usa dados reais do projeto (bore, stroke, cilindros) para calculos precisos
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

    CamshaftCalculator calc(&engine);

    CamshaftSpec spec;
    spec.intakeDuration = editIntakeDuration.GetValue();
    spec.exhaustDuration = editExhaustDuration.GetValue();
    spec.intakeLift = editIntakeLift.GetValue();
    spec.exhaustLift = editExhaustLift.GetValue();
    spec.lsa = editLSA.GetValue();
    spec.advanceRetard = editAdvanceRetard.GetValue();
    spec.rockerRatio = editRockerRatio.GetValue();

    if (!calc.IsValidCamshaftSpec(spec)) {
        SetWindowTextW(textResults, L"Especificacoes invalidas. Verifique os valores inseridos.");
        return;
    }

    double rpm = editCamRPM.GetValue();

    ValveEvents events = calc.CalculateValveEvents(spec);
    CamAreaAnalysis area = calc.CalculateCamAreaAnalysis(spec);
    CamshaftCalculator::PowerBand band = calc.CalculatePowerBand((spec.intakeDuration + spec.exhaustDuration) / 2.0);
    double aggressiveness = calc.CalculateAggressivenessIndex(spec);
    double recommendedCR = calc.RecommendedCompressionRatio((spec.intakeDuration + spec.exhaustDuration) / 2.0);
    double minClearance = calc.CalculateMinimumPistonToValveClearance(spec);

    std::wostringstream results;
    results << std::fixed << std::setprecision(1);
    results << L"---------------------------------------------------------------------------\r\n";
    results << L"  ANALISE COMPLETA DO COMANDO DE VALVULAS\r\n";
    results << L"---------------------------------------------------------------------------\r\n\r\n";

    results << L"EVENTOS DAS VALVULAS (Valve Timing):\r\n";
    results << L"  Admissao Abre: " << std::abs(events.intakeOpen) << L" graus ";
    results << (events.intakeOpen < 0 ? L"BTDC" : L"ATDC") << L"\r\n";
    results << L"  Admissao Fecha: " << events.intakeClose << L" graus ABDC\r\n";
    results << L"  Escape Abre: " << events.exhaustOpen << L" graus BBDC\r\n";
    results << L"  Escape Fecha: " << std::abs(events.exhaustClose) << L" graus ";
    results << (events.exhaustClose > 0 ? L"ATDC" : L"BTDC") << L"\r\n";
    results << L"  Overlap: " << events.overlap << L" graus " << calc.GetOverlapAdvice(events.overlap) << L"\r\n";
    results << L"  Centerline Admissao: " << events.intakeCenterline << L" graus ATDC\r\n";
    results << L"  Centerline Escape: " << events.exhaustCenterline << L" graus BTDC\r\n\r\n";

    results << L"ANALISE DE LSA:\r\n";
    results << L"  LSA: " << spec.lsa << L" graus " << calc.GetLSAAdvice(spec.lsa) << L"\r\n\r\n";

    results << L"CARACTERISTICAS DE FLUXO:\r\n";
    results << std::setprecision(2);
    results << L"  Area Admissao: " << area.intakeArea << L" mm·graus\r\n";
    results << L"  Area Escape: " << area.exhaustArea << L" mm·graus\r\n";
    results << L"  Velocidade Media Adm: " << area.intakeVelocity << L" mm/grau\r\n";
    results << L"  Velocidade Media Esc: " << area.exhaustVelocity << L" mm/grau\r\n\r\n";

    results << L"FAIXA DE OPERACAO:\r\n";
    results << std::setprecision(0);
    results << L"  RPM Minimo Util: " << band.minRPM << L" rpm\r\n";
    results << L"  RPM Pico Estimado: " << band.peakRPM << L" rpm\r\n";
    results << L"  RPM Maximo Util: " << band.maxRPM << L" rpm\r\n\r\n";

    results << L"RECOMENDACOES:\r\n";
    results << std::setprecision(1);
    results << L"  Indice de Agressividade: " << aggressiveness << L"/100 ";
    if (aggressiveness < 30) results << L"(Suave)\r\n";
    else if (aggressiveness < 60) results << L"(Performance)\r\n";
    else results << L"(Competicao)\r\n";
    results << L"  Taxa Compressao Recomendada: " << recommendedCR << L":1 ou menos\r\n";
    results << L"  Folga Pistao-Valvula Minima: " << minClearance << L" mm\r\n\r\n";

    results << calc.GetCamshaftRecommendation(spec);
    results << L"\r\n---------------------------------------------------------------------------\r\n";
    results << L"Referencias: Blair - Design and Simulation of Four-Stroke Engines\r\n";
    results << L"            SAE Paper 2005-01-1688 - Camshaft Design & LSA";

    SetWindowTextW(textResults, results.str().c_str());

    // Atualiza grafico de lift
    UpdateGraph();

    if (dataManager) {
        EngineProject::CamshaftData camData;
        camData.intakeDuration = spec.intakeDuration;
        camData.exhaustDuration = spec.exhaustDuration;
        camData.intakeLift = spec.intakeLift;
        camData.exhaustLift = spec.exhaustLift;
        camData.lsa = spec.lsa;
        camData.advanceRetard = spec.advanceRetard;
        camData.rockerRatio = spec.rockerRatio;
        
        dataManager->UpdateCamshaftData(camData);
    }
}

void CamshaftTab::OnClear() {
    editIntakeDuration.SetValue(0.0);
    editExhaustDuration.SetValue(0.0);
    editIntakeLift.SetValue(0.0);
    editExhaustLift.SetValue(0.0);
    editLSA.SetValue(110.0);
    editAdvanceRetard.SetValue(0.0);
    editRockerRatio.SetValue(1.5);
    editCamRPM.SetValue(6000.0);
    SetWindowTextW(textResults, L"");
}

void CamshaftTab::RecalculateLayout() {
    if (!hwndPage) return;

    int newWidth = GetResultsWidth(0.97f);

    if (textResults) {
        RECT rc;
        GetWindowRect(textResults, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(textResults, NULL,
                     rc.left, rc.top, newWidth, rc.bottom - rc.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (hwndGraph) {
        RECT rc;
        GetWindowRect(hwndGraph, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(hwndGraph, NULL,
                     rc.left, rc.top, newWidth, rc.bottom - rc.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        InvalidateRect(hwndGraph, NULL, TRUE);
    }
}

// ============================================================================
// GRÁFICO DE LIFT DO COMANDO
// ============================================================================

LRESULT CALLBACK CamshaftTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        CamshaftTab* tab = (CamshaftTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
    if (msg == WM_ERASEBKGND) {
        return 1; // Evita flicker
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CamshaftTab::UpdateGraph() {
    graphRenderer.ClearSeries();

    double intakeDur = editIntakeDuration.GetValue();
    double exhaustDur = editExhaustDuration.GetValue();
    double intakeLift = editIntakeLift.GetValue();
    double exhaustLift = editExhaustLift.GetValue();
    double lsa = editLSA.GetValue();
    double advance = editAdvanceRetard.GetValue();

    if (intakeDur <= 0 || exhaustDur <= 0 || intakeLift <= 0 || exhaustLift <= 0) {
        if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
        return;
    }

    // Configura eixos
    float maxLift = (float)((intakeLift > exhaustLift ? intakeLift : exhaustLift) * 1.15);
    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Curva de Lift do Comando";
    cfg.xAxisLabel = L"Angulo do Virabrequim (graus)";
    cfg.yAxisLabel = L"Lift (mm)";
    cfg.xMin = 0; cfg.xMax = 720;
    cfg.yMin = 0; cfg.yMax = maxLift;
    cfg.xGridStep = 90;
    cfg.yGridStep = (maxLift > 10) ? 5.0f : 2.0f;

    // Calcula pontos usando modelo senoidal
    // Admissao: abre BTDC, fecha ABDC
    double intakeCenterline = lsa - advance; // ATDC
    double intakeOpenAngle = 360.0 - (intakeDur / 2.0 - intakeCenterline);

    // Escape: abre BBDC, fecha ATDC
    double exhaustCenterline = lsa + advance; // BTDC -> 360 - ecl 
    double exhaustOpenAngle = 180.0 - (exhaustDur / 2.0);

    EngineCore graphEngine;
    EngineDataManager* dmGraph = EngineDataManager::GetInstance();
    if (dmGraph && dmGraph->GetProject().basicData.bore > 0) {
        const auto& bd = dmGraph->GetProject().basicData;
        graphEngine.SetBore(bd.bore);
        graphEngine.SetStroke(bd.stroke);
        graphEngine.SetCylinders(bd.cylinders);
    } else {
        graphEngine.SetBore(86.0); graphEngine.SetStroke(86.0); graphEngine.SetCylinders(4);
    }
    CamshaftCalculator calc(&graphEngine);

    std::vector<float> xIntake, yIntake;
    std::vector<float> xExhaust, yExhaust;

    // Gera curva de admissao (0-720 graus)
    for (int deg = 0; deg <= 720; deg += 2) {
        float angle = (float)deg;

        // Admissao: centro em (360 + intakeCenterline) graus no ciclo
        double intakeCenter = 360.0 + intakeCenterline;
        double distFromCenter = fabs((double)deg - intakeCenter);
        if (distFromCenter > 360.0) distFromCenter = 720.0 - distFromCenter;
        double halfDur = intakeDur / 2.0;

        double lift = 0.0;
        if (distFromCenter < halfDur) {
            // Modelo senoidal: lift = maxLift * cos^2(pi/2 * dist/halfDur)
            double ratio = distFromCenter / halfDur;
            double cosVal = cos(ratio * 3.14159265 / 2.0);
            lift = intakeLift * cosVal * cosVal;
        }
        xIntake.push_back(angle);
        yIntake.push_back((float)lift);
    }

    // Gera curva de escape
    for (int deg = 0; deg <= 720; deg += 2) {
        // Escape: centro em (360 - exhaustCenterline) = (360 - lsa - advance)
        double exhaustCenter = 360.0 - exhaustCenterline;
        if (exhaustCenter < 0) exhaustCenter += 720.0;

        double dist = fabs((double)deg - exhaustCenter);
        if (dist > 360.0) dist = 720.0 - dist;
        double halfDur = exhaustDur / 2.0;

        double lift = 0.0;
        if (dist < halfDur) {
            double ratio = dist / halfDur;
            double cosVal = cos(ratio * 3.14159265 / 2.0);
            lift = exhaustLift * cosVal * cosVal;
        }
        xExhaust.push_back((float)deg);
        yExhaust.push_back((float)lift);
    }

    // Adiciona series
    graphRenderer.AddSeries(
        GraphRenderer::CreateSeries(xIntake, yIntake,
            Gdiplus::Color(255, 0, 150, 255), L"Admissao", 2.5f));

    graphRenderer.AddSeries(
        GraphRenderer::CreateSeries(xExhaust, yExhaust,
            Gdiplus::Color(255, 255, 80, 80), L"Escape", 2.5f));

    if (hwndGraph) {
        InvalidateRect(hwndGraph, NULL, TRUE);
    }
}
