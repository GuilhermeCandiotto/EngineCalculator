// ============================================================================
// BasicTab.cpp - Implementação da Aba de Cálculos Básicos
// Engine Calculator v1.0.0 - Fevereiro 2026
// ============================================================================
#include "BasicTab.h"
#include "../../Core/EngineCore.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

// Funções auxiliares externas (definidas em TabPages.cpp)
extern bool RegisterTabPageClass(HINSTANCE hInstance);

BasicTab::BasicTab(HWND parent, HINSTANCE instance) 
    : TabPage(parent, instance), comboEngineType(nullptr), textResults(nullptr), hwndGraph(nullptr) {
}

BasicTab::~BasicTab() {
}

void BasicTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
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

void BasicTab::CreateControls() {
    int leftMargin = GetMargin();
    int topMargin = GetMargin();
    int labelWidth = GetLabelWidth(0.30f);
    int editWidth = GetEditWidth(0.27f);
    int rowHeight = 35;
    int currentY = topMargin;
    
    // Diâmetro do cilindro (Bore)
    HWND lblBore = CreateWindowW(L"STATIC", L"Diâmetro do Cilindro (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblBore, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editBore.Create(hwndPage, IDC_EDIT_BORE, leftMargin + labelWidth, currentY - 3, 
                    editWidth, 24, 30.0, 200.0, 2);
    CreateTooltip(editBore.GetHandle(),
        L"O QUE É: Diâmetro INTERNO do cilindro (bore).\n"
        L"COMO MEDIR: Paquímetro/micrômetro interno no topo do cilindro.\n"
        L"ATENÇÃO: Para motores retificados, usar medida FINAL!");
    currentY += rowHeight;
    
    // Curso (Stroke)
    HWND lblStroke = CreateWindowW(L"STATIC", L"Curso do Pistão (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblStroke, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editStroke.Create(hwndPage, IDC_EDIT_STROKE, leftMargin + labelWidth, currentY - 3, 
                      editWidth, 24, 30.0, 200.0, 2);
    CreateTooltip(editStroke.GetHandle(),
        L"O QUE É: Distância que o pistão percorre do PMI (fundo) ao PMS (topo).\n"
        L"COMO CALCULAR: Curso = 2 × raio do virabrequim.\n"
        L"COMO MEDIR: Paquímetro do PMI ao PMS ou use specs fabricante.");
    currentY += rowHeight;
    
    // Número de cilindros
    HWND lblCylinders = CreateWindowW(L"STATIC", L"Número de Cilindros:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblCylinders, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editCylinders.Create(hwndPage, IDC_EDIT_CYLINDERS, leftMargin + labelWidth, currentY - 3, 
                         editWidth, 24, 1.0, 16.0, 0);
    CreateTooltip(editCylinders.GetHandle(),
        L"O QUE É: Quantidade total de cilindros do motor.");
    currentY += rowHeight;
    
    // Tipo de motor
    HWND lblEngineType = CreateWindowW(L"STATIC", L"Tipo de Motor:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblEngineType, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    comboEngineType = CreateWindowW(
        L"COMBOBOX",
        L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        leftMargin + labelWidth, currentY - 3, editWidth, 100,
        hwndPage,
        (HMENU)IDC_COMBO_ENGINE_TYPE,
        hInst,
        nullptr
    );
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(comboEngineType, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(comboEngineType, CB_ADDSTRING, 0, (LPARAM)L"4 Tempos");
    SendMessageW(comboEngineType, CB_ADDSTRING, 0, (LPARAM)L"2 Tempos");
    SendMessageW(comboEngineType, CB_SETCURSEL, 0, 0);

    currentY += rowHeight;
    
    // Comprimento da biela
    HWND lblRod = CreateWindowW(L"STATIC", L"Comprimento da Biela (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRod, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editRodLength.Create(hwndPage, IDC_EDIT_ROD_LENGTH, leftMargin + labelWidth, currentY - 3, 
                         editWidth, 24, 80.0, 300.0, 2);
    CreateTooltip(editRodLength.GetHandle(),
        L"O QUE É: Distância centro-a-centro da biela (connecting rod).\n"
        L"COMO MEDIR: Paquímetro do centro do mancal ao centro do pino.");
    currentY += rowHeight;
    
    // RPM
    HWND lblRPM = CreateWindowW(L"STATIC", L"RPM Máximo:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRPM, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editRPM.Create(hwndPage, IDC_EDIT_RPM, leftMargin + labelWidth, currentY - 3, 
                   editWidth, 24, 1000.0, 20000.0, 0);
    CreateTooltip(editRPM.GetHandle(),
        L"O QUE É: RPM (rotações por minuto) MÁXIMO de operação.\n"
        L"Usado para calcular velocidade média do pistão (MPS).\n"
        L"ATENÇÃO: MPS >25 m/s exige componentes RACE!");
    currentY += rowHeight;
    
    // Torque Máximo
    HWND lblTorque = CreateWindowW(L"STATIC", L"Torque Máximo (Nm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblTorque, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editTorque.Create(hwndPage, IDC_EDIT_TORQUE, leftMargin + labelWidth, currentY - 3, 
                      editWidth, 24, 50.0, 2000.0, 1);
    CreateTooltip(editTorque.GetHandle(),
        L"O QUE É: Torque MÁXIMO do motor (em Nm - Newton-metro).\n"
        L"USADO PARA: Calcular BMEP (Pressão Média Efetiva).\n"
        L"COMO OBTER: Datasheet do fabricante ou dyno.\n"
        L"CONVERSÃO: lb-ft × 1.356 = Nm");
    currentY += rowHeight + 20;
    
    // Área de resultados
    CreateLabel(L"Resultados:", leftMargin, currentY);
    currentY += 25;
    
    textResults = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        leftMargin, currentY, GetResultsWidth(), 400,
        hwndPage,
        (HMENU)IDC_RESULT_TEXT,
        hInst,
        nullptr
    );
    
    SendMessage(textResults, WM_SETFONT, (WPARAM)GetAppResultsFont(), TRUE);

    currentY += 405;
    WNDCLASSW wc = {};
    wc.lpfnWndProc = GraphWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"EngineCalcGraphBasic";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    hwndGraph = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EngineCalcGraphBasic",
        L"",
        WS_CHILD | WS_VISIBLE,
        leftMargin, currentY, GetResultsWidth(), 220,
        hwndPage,
        nullptr,
        hInst,
        nullptr
    );
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);
}

void BasicTab::Create() {
    RegisterTabPageClass(hInst);
    
    hwndPage = CreateWindowExW(
        0,
        L"EngineCalcTabPage",
        L"",
        WS_CHILD | WS_VSCROLL,
        0, 30, 800, 450,
        hwndParent,
        nullptr,
        hInst,
        nullptr
    );
    
    SetWindowLongPtr(hwndPage, GWLP_USERDATA, (LONG_PTR)this);
    CreateControls();
    SetContentHeight(980);
}

void BasicTab::OnCalculate() {
    EngineCore engine;
    engine.SetBore(editBore.GetValue());
    engine.SetStroke(editStroke.GetValue());
    engine.SetCylinders(static_cast<int>(editCylinders.GetValue()));
    engine.SetEngineType(SendMessage(comboEngineType, CB_GETCURSEL, 0, 0) == 0 ? 
                         EngineType::FOUR_STROKE : EngineType::TWO_STROKE);
    
    if (!engine.IsValid()) {
        SetWindowTextW(textResults, engine.GetValidationError().c_str());
        return;
    }
    
    double displacement = engine.CalculateDisplacement();
    double boreStrokeRatio = engine.CalculateBoreStrokeRatio();
    double rodStrokeRatio = engine.CalculateRodStrokeRatio(editRodLength.GetValue());
    double pistonSpeed = engine.CalculatePistonSpeed(editRPM.GetValue());
    double boreArea = engine.CalculateBoreArea();
    
    std::wostringstream results;
    results << std::fixed << std::setprecision(2);
    results << L"-------------------------------------------\r\n";
    results << L"  CALCULOS FUNDAMENTAIS DO MOTOR\r\n";
    results << L"-------------------------------------------\r\n\r\n";
    
    results << L"Cilindrada Total: " << displacement << L" cc\r\n";
    results << L"Cilindrada por Cilindro: " << (displacement / engine.GetCylinders()) << L" cc\r\n";
    results << L"Area do Cilindro: " << boreArea << L" mm2\r\n\r\n";
    
    results << L"Relacao Diametro/Curso: " << boreStrokeRatio << L"\r\n";
    if (boreStrokeRatio > 1.1) results << L"  -> Oversquare (alta rotacao)\r\n";
    else if (boreStrokeRatio < 0.9) results << L"  -> Undersquare (torque)\r\n";
    else results << L"  -> Square (equilibrado)\r\n\r\n";
    
    results << L"Relacao Biela/Curso: " << rodStrokeRatio << L"\r\n";
    if (rodStrokeRatio < 1.5) results << L"  -> Baixa - Maior estresse lateral\r\n";
    else if (rodStrokeRatio > 2.2) results << L"  -> Excelente - Baixo estresse\r\n";
    else results << L"  -> Boa\r\n\r\n";
    
    results << L"Velocidade Media do Pistao: " << pistonSpeed << L" m/s\r\n";
    if (pistonSpeed > 25.0) results << L"  -> Alta - Apenas para competicao\r\n";
    else if (pistonSpeed > 20.0) results << L"  -> Moderada-Alta\r\n";
    else results << L"  -> Adequada para uso normal\r\n\r\n";
    
    double torqueNm = editTorque.GetValue();
    
    if (torqueNm > 0.0 && displacement > 0.0) {
        double displacementLiters = displacement / 1000.0;
        
        double bmep = engine.CalculateBMEP(torqueNm, displacementLiters);
        double imep = engine.CalculateIMEP(bmep, 0.85);
        double fmep = engine.CalculateFMEP(imep, bmep);
        double pmep = engine.CalculatePMEP(editRPM.GetValue(), 1.0);
        
        results << L"-------------------------------------------\r\n";
        results << L"  PRESSOES MEDIAS EFETIVAS (MEP)\r\n";
        results << L"-------------------------------------------\r\n\r\n";
        
        results << std::fixed << std::setprecision(1);
        results << L"BMEP (Brake MEP): " << bmep << L" kPa (" << (bmep/100.0) << L" bar)\r\n";
        results << L"  -> " << engine.ClassifyBMEP(bmep) << L"\r\n\r\n";
        
        results << L"IMEP (Indicated MEP): " << imep << L" kPa (" << (imep/100.0) << L" bar)\r\n";
        results << L"  -> Pressao antes das perdas mecanicas\r\n\r\n";
        
        results << L"FMEP (Friction MEP): " << fmep << L" kPa (" << (fmep/100.0) << L" bar)\r\n";
        results << L"  -> Perdas por atrito\r\n";
        results << L"  -> Eficiencia Mecanica: " << std::setprecision(1) << (bmep/imep*100.0) << L"%\r\n\r\n";
        
        results << L"PMEP (Pumping MEP @ WOT): " << pmep << L" kPa\r\n";
        results << L"  -> Perdas de bombeamento (admissao/escape)\r\n\r\n";
        
        results << L"-----------------------------------------\r\n";
        results << L"VALORES TIPICOS DE BMEP:\r\n";
        results << L"-----------------------------------------\r\n";
        results << L"Aspirado Rua:        850-1050 kPa (8.5-10.5 bar)\r\n";
        results << L"Aspirado Race:      1100-1400 kPa (11-14 bar)\r\n";
        results << L"Turbo Rua:          1200-1700 kPa (12-17 bar)\r\n";
        results << L"Turbo Race:         1700-2500 kPa (17-25 bar)\r\n";
        results << L"F1 (V10 Aspirado):  ~1450 kPa (14.5 bar)\r\n";
        results << L"Top Fuel Dragster:  ~1900 kPa (19 bar)\r\n\r\n";
        
        results << L"ANALISE:\r\n";
        if (bmep < 700.0) {
            results << L"[!] BMEP muito baixo - Verifique motor!\r\n";
        }
        else if (bmep < 900.0) {
            results << L"[!] BMEP baixo - Normal para diesel antigo\r\n";
        }
        else if (bmep < 1100.0) {
            results << L"[OK] BMEP adequado para motor de rua\r\n";
        }
        else if (bmep < 1400.0) {
            results << L"[OK] BMEP bom - Motor bem projetado\r\n";
        }
        else if (bmep < 1700.0) {
            results << L"[OK] BMEP muito bom - Performance elevada\r\n";
        }
        else if (bmep < 2000.0) {
            results << L"[OK] BMEP excelente - Motor turbo potente\r\n";
        }
        else {
            results << L"[!!!] BMEP extremo - Nivel de competicao!\r\n";
        }
        results << L"\r\n";
    }
    else {
        results << L"-------------------------------------------\r\n";
        results << L"[i] Para calculos de MEP (Pressao Media Efetiva),\r\n";
        results << L"  preencha o campo 'Torque Maximo (Nm)'.\r\n\r\n";
        results << L"MEP indica a eficiencia volumetrica e termica\r\n";
        results << L"do motor, sendo um dos principais indicadores\r\n";
        results << L"de performance.\r\n\r\n";
    }
    
    results << L"-------------------------------------------\r\n";
    results << L"Referencias Cientificas:\r\n";
    results << L"- Heywood - ICE Fundamentals (Cap. 2.6-2.7)\r\n";
    results << L"- SAE J1349 - Engine Power Test Code\r\n";
    results << L"- SAE 2007-01-1464 - BMEP Analysis\r\n";
    results << L"- Blair - Design and Simulation (Cap. 4)\r\n";
    
    SetWindowTextW(textResults, results.str().c_str());

    UpdateGraph();

    EngineDataManager* dataManager = EngineDataManager::GetInstance();
    if (dataManager) {
        EngineProject::BasicData basicData;
        basicData.bore = editBore.GetValue();
        basicData.stroke = editStroke.GetValue();
        basicData.cylinders = static_cast<int>(editCylinders.GetValue());
        basicData.engineType = engine.GetEngineType();
        basicData.rodLength = editRodLength.GetValue();
        basicData.maxRPM = editRPM.GetValue();
        
        dataManager->UpdateBasicData(basicData);
    }
}

void BasicTab::OnClear() {
    editBore.SetValue(0.0);
    editStroke.SetValue(0.0);
    editCylinders.SetValue(4.0);
    editRodLength.SetValue(0.0);
    editRPM.SetValue(0.0);
    editTorque.SetValue(0.0);
    SendMessage(comboEngineType, CB_SETCURSEL, 0, 0);
    SetWindowTextW(textResults, L"");
}

void BasicTab::RecalculateLayout() {
    if (!hwndPage) return;
    
    int labelWidth = GetLabelWidth(0.30f);
    int editWidth = GetEditWidth(0.27f);
    int leftMargin = GetMargin();
    
    HWND hwndChild = GetWindow(hwndPage, GW_CHILD);
    
    while (hwndChild) {
        wchar_t className[256];
        GetClassNameW(hwndChild, className, 256);
        
        RECT rc;
        GetWindowRect(hwndChild, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        
        if (wcscmp(className, L"STATIC") == 0) {
            SetWindowPos(hwndChild, NULL,
                         rc.left, rc.top, labelWidth, rc.bottom - rc.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else if (wcscmp(className, L"EDIT") == 0) {
            if (rc.bottom - rc.top > 100) {
                int newWidth = GetResultsWidth();
                SetWindowPos(hwndChild, NULL,
                             rc.left, rc.top, newWidth, rc.bottom - rc.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            }
            else {
                SetWindowPos(hwndChild, NULL,
                             leftMargin + labelWidth, rc.top, editWidth, rc.bottom - rc.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        else if (wcscmp(className, L"COMBOBOX") == 0) {
            SetWindowPos(hwndChild, NULL,
                         leftMargin + labelWidth, rc.top, editWidth, rc.bottom - rc.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
    }
    
    if (hwndGraph) {
        RECT rc;
        GetWindowRect(hwndGraph, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(hwndGraph, NULL,
                     rc.left, rc.top, GetResultsWidth(), rc.bottom - rc.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        InvalidateRect(hwndGraph, NULL, TRUE);
    }

    InvalidateRect(hwndPage, NULL, TRUE);
}

LRESULT CALLBACK BasicTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        BasicTab* tab = (BasicTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

void BasicTab::UpdateGraph() {
    graphRenderer.ClearSeries();

    double stroke = editStroke.GetValue();
    double maxRPM = editRPM.GetValue();

    if (stroke <= 0 || maxRPM <= 0) {
        if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
        return;
    }

    // Configura eixos
    float rpmMax = (float)(maxRPM * 1.1);
    float speedMax = (float)(2.0 * stroke * maxRPM * 1.1 / 60000.0);
    speedMax = ceilf(speedMax / 5.0f) * 5.0f; // Arredonda para multiplo de 5

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Velocidade Media do Pistao vs RPM";
    cfg.xAxisLabel = L"RPM";
    cfg.yAxisLabel = L"MPS (m/s)";
    cfg.xMin = 0; cfg.xMax = rpmMax;
    cfg.yMin = 0; cfg.yMax = speedMax;
    cfg.xGridStep = rpmMax > 12000 ? 3000.0f : (rpmMax > 6000 ? 2000.0f : 1000.0f);
    cfg.yGridStep = speedMax > 20 ? 5.0f : 2.5f;

    // Gera curva de velocidade do pistao
    std::vector<float> xRPM, ySpeed;
    for (int rpm = 0; rpm <= (int)rpmMax; rpm += 100) {
        float speed = (float)(2.0 * stroke * rpm / 60000.0);
        xRPM.push_back((float)rpm);
        ySpeed.push_back(speed);
    }

    graphRenderer.AddSeries(
        GraphRenderer::CreateSeries(xRPM, ySpeed,
            Gdiplus::Color(255, 0, 150, 255), L"MPS", 2.5f));

    // Linha de limite 25 m/s (zona de perigo)
    std::vector<float> xLimit, yLimit;
    xLimit.push_back(0); yLimit.push_back(25.0f);
    xLimit.push_back(rpmMax); yLimit.push_back(25.0f);
    graphRenderer.AddSeries(
        GraphRenderer::CreateSeries(xLimit, yLimit,
            Gdiplus::Color(255, 255, 60, 60), L"Limite 25 m/s", 1.5f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
