// ============================================================================
// CompressionTab.cpp - Implementação da Aba de Análise de Compressão
// Engine Calculator v1.0.0 - Fevereiro 2026
// ============================================================================
#include "CompressionTab.h"
#include "../../Core/EngineCore.h"
#include "../../Core/CompressionCalculator.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

extern bool RegisterTabPageClass(HINSTANCE hInstance);

CompressionTab::CompressionTab(HWND parent, HINSTANCE instance) 
    : TabPage(parent, instance), textResults(nullptr), hwndGraph(nullptr) {
}

CompressionTab::~CompressionTab() {
}

void CompressionTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
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

void CompressionTab::CreateControls() {
    int leftMargin = GetMargin();
    int topMargin = GetMargin();
    int labelWidth = GetLabelWidth(0.35f);
    int editWidth = GetEditWidth(0.22f);
    int rowHeight = 35;
    int currentY = topMargin;
    
    CreateLabel(L"DADOS DO MOTOR:", leftMargin, currentY, 200);
    currentY += 20;
    
    HWND lblBore = CreateWindowW(L"STATIC", L"Diametro (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblBore, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    editBore.Create(hwndPage, IDC_EDIT_BORE_COMP, leftMargin + labelWidth, currentY - 3, 
                    editWidth, 24, 30.0, 200.0, 2);
    currentY += 28;
    
    HWND lblStroke = CreateWindowW(L"STATIC", L"Curso (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblStroke, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    editStroke.Create(hwndPage, IDC_EDIT_STROKE_COMP, leftMargin + labelWidth, currentY - 3, 
                      editWidth, 24, 30.0, 200.0, 2);
    currentY += 28;
    
    HWND lblCyl = CreateWindowW(L"STATIC", L"Cilindros:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblCyl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    editCylinders.Create(hwndPage, IDC_EDIT_CYLINDERS_COMP, leftMargin + labelWidth, currentY - 3, 
                         editWidth, 24, 1.0, 16.0, 0);
    editCylinders.SetValue(4.0);
    currentY += 35;
    
    CreateLabel(L"DADOS DE COMPRESSAO:", leftMargin, currentY, 300);
    currentY += 20;
    
    HWND lblChamber = CreateWindowW(L"STATIC", L"Volume da Camara (cc):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblChamber, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editChamberVol.Create(hwndPage, IDC_EDIT_CHAMBER_VOL, leftMargin + labelWidth, currentY - 3, 
                          editWidth, 24, 10.0, 500.0, 2);
    CreateTooltip(editChamberVol.GetHandle(),
        L"O QUE E: Volume TOTAL da camara de combustao no cabecote.\n"
        L"COMO MEDIR: Burreta ou CCing (cc-ing).\n"
        L"1. Vede valvulas com graxa\n"
        L"2. Vire cabecote de cabeca pra baixo\n"
        L"3. Encha com oleo/fluido ate deck surface\n"
        L"4. Medir volume = CC chamber");
    currentY += rowHeight;
    
    HWND lblDome = CreateWindowW(L"STATIC", L"Volume da Cupula do Pistao (cc):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblDome, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    editPistonDome.Create(hwndPage, IDC_EDIT_PISTON_DOME, leftMargin + labelWidth, currentY - 3, 
                          editWidth, 24, -100.0, 100.0, 2);
    CreateTooltip(editPistonDome.GetHandle(),
        L"O QUE E: Volume da cupula (dome/dish) do topo do pistao.\n"
        L"COMO MEDIR: Burreta invertida sobre o pistao.\n"
        L"POSITIVO = Dome (sobe) = AUMENTA compressao\n"
        L"NEGATIVO = Dish (afunda) = REDUZ compressao");
    currentY += rowHeight;
    
    HWND lblGasket = CreateWindowW(L"STATIC", L"Espessura da Junta (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblGasket, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editGasketThick.Create(hwndPage, IDC_EDIT_GASKET_THICK, leftMargin + labelWidth, currentY - 3, 
                           editWidth, 24, 0.5, 3.0, 2);
    CreateTooltip(editGasketThick.GetHandle(),
        L"O QUE E: Espessura da junta do cabecote COMPRIMIDA (instalada).\n"
        L"COMO MEDIR: Paquimetro na junta INSTALADA (comprimida).\n"
        L"Espessura comprimida = 80-90% da original.");
    currentY += rowHeight;
    
    HWND lblDeck = CreateWindowW(L"STATIC", L"Deck Height (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblDeck, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    CreateTooltip(lblDeck,
        L"Zero deck (0.0mm) = pistao flush com deck\n"
        L"Negativo = pistao ACIMA do deck (pop-up)\n"
        L"Positivo = pistao ABAIXO do deck (in-the-hole)");
    
    editDeckHeight.Create(hwndPage, IDC_EDIT_DECK_HEIGHT, leftMargin + labelWidth, currentY - 3, 
                          editWidth, 24, 0.0, 5.0, 2);
    CreateTooltip(editDeckHeight.GetHandle(),
        L"O QUE E: Distancia do TOPO do pistao (no PMS) ate o DECK do bloco.\n"
        L"COMO MEDIR: Com pistao no PMS (topo):...\n"
        L"1. Relogio comparador ou profundimetro\n"
        L"2. Medir do deck ate topo do pistao");
    currentY += rowHeight;
    
    HWND lblRelief = CreateWindowW(L"STATIC", L"Volume Rebaixos Valvulas (cc):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRelief, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editValveRelief.Create(hwndPage, IDC_EDIT_VALVE_RELIEF, leftMargin + labelWidth, currentY - 3, 
                           editWidth, 24, 0.0, 50.0, 2);
    CreateTooltip(editValveRelief.GetHandle(),
        L"O QUE E: Volume dos rebaixos (valve reliefs/pockets) no pistao.\n"
        L"COMO MEDIR: Burreta com pistao invertido.\n"
        L"Encher rebaixos com fluido e medir volume.\n"
        L"IMPORTANTE para CR REAL!");
    currentY += rowHeight;
    
    HWND lblIVC = CreateWindowW(L"STATIC", L"Fechamento Admissao (graus ABDC):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, leftMargin, currentY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblIVC, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    editIVCAngle.Create(hwndPage, IDC_EDIT_IVC_ANGLE, leftMargin + labelWidth, currentY - 3, 
                        editWidth, 24, 0.0, 90.0, 1);
    CreateTooltip(editIVCAngle.GetHandle(), 
        L"O QUE E: Angulo onde a valvula de admissao FECHA.\n"
        L"ABDC = After Bottom Dead Center (graus APOS o PMI).\n"
        L"COMO OBTER: Datasheet do comando (IVC @ 0.050\").\n"
        L"Usado para calcular COMPRESSAO DINAMICA (DCR).\n"
        L"Quanto MAIS TARDE fecha = MENOR DCR (menos pressao efetiva)");
    currentY += rowHeight + 20;
    
    CreateLabel(L"Resultados:", leftMargin, currentY);
    currentY += 25;
    
    textResults = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        leftMargin, currentY, GetResultsWidth(), 180,
        hwndPage,
        (HMENU)IDC_RESULT_TEXT,
        hInst,
        nullptr
    );
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(textResults, WM_SETFONT, (WPARAM)GetAppResultsFont(), TRUE);

    currentY += 185;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = GraphWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"EngineCalcGraphComp";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    hwndGraph = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EngineCalcGraphComp",
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

void CompressionTab::Create() {
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
    SetContentHeight(950);
}

void CompressionTab::OnCalculate() {
    EngineDataManager* dataManager = EngineDataManager::GetInstance();
    if (dataManager) {
        const EngineProject& project = dataManager->GetProject();
        
        if (project.basicData.bore > 0 && editBore.GetValue() == 0) {
            editBore.SetValue(project.basicData.bore);
        }
        if (project.basicData.stroke > 0 && editStroke.GetValue() == 0) {
            editStroke.SetValue(project.basicData.stroke);
        }
        if (project.basicData.cylinders > 0 && editCylinders.GetValue() == 4.0) {
            editCylinders.SetValue(static_cast<double>(project.basicData.cylinders));
        }
    }
    
    EngineCore engine;
    double bore = editBore.GetValue();
    double stroke = editStroke.GetValue();
    int cyls = static_cast<int>(editCylinders.GetValue());

    // Se campos estão vazios, tenta buscar do projeto
    if (bore <= 0 || stroke <= 0 || cyls <= 0) {
        EngineDataManager* dm = EngineDataManager::GetInstance();
        if (dm && dm->GetProject().basicData.bore > 0) {
            const auto& bd = dm->GetProject().basicData;
            if (bore <= 0) bore = bd.bore;
            if (stroke <= 0) stroke = bd.stroke;
            if (cyls <= 0) cyls = bd.cylinders;
        }
    }
    engine.SetBore(bore > 0 ? bore : 86.0);
    engine.SetStroke(stroke > 0 ? stroke : 86.0);
    engine.SetCylinders(cyls > 0 ? cyls : 4);
    engine.SetEngineType(EngineType::FOUR_STROKE);
    
    if (editChamberVol.GetValue() <= 0) {
        SetWindowTextW(textResults, 
            L"ATENCAO: Preencha o Volume da Camara para calcular a compressao.\n\n"
            L"Se ainda nao tiver este valor:\n"
            L"1. Va para aba Basica e preencha bore/stroke\n"
            L"2. Meca o volume da camara (CC chamber)\n"
            L"3. Volte aqui e calcule novamente");
        return;
    }
    
    CompressionCalculator calc(&engine);
    
    CompressionData data;
    data.chamberVolume = editChamberVol.GetValue();
    data.pistonDomeVolume = editPistonDome.GetValue();
    data.gasketVolume = calc.CalculateGasketVolume(engine.GetBore(), editGasketThick.GetValue());
    data.deckVolume = calc.CalculateDeckVolume(engine.GetBore(), editDeckHeight.GetValue());
    data.valveRelief = editValveRelief.GetValue();
    
    double staticCR = calc.CalculateStaticCompressionRatio(data);
    double sweptVolume = engine.CalculateDisplacement() / engine.GetCylinders();
    double clearanceVolume = calc.CalculateClearanceVolume(data);
    
    DynamicCompressionData dynData;
    dynData.staticCR = staticCR;
    dynData.intakeClosingAngle = editIVCAngle.GetValue();
    dynData.rodLength = 150.0;
    dynData.strokeLength = engine.GetStroke();
    
    double dynamicCR = calc.CalculateDynamicCompressionRatio(dynData);
    double cylinderPressure = calc.CalculateCylinderPressure(staticCR);
    
    std::wostringstream results;
    results << std::fixed << std::setprecision(2);
    results << L"===============================================================\r\n";
    results << L"  ANALISE DE COMPRESSAO\r\n";
    results << L"===============================================================\r\n\r\n";
    
    results << L"VOLUMES (por cilindro):\r\n";
    results << L"  Volume Varrido (Swept): " << sweptVolume << L" cc\r\n";
    results << L"  Volume de Clearance Total: " << clearanceVolume << L" cc\r\n";
    results << L"    - Camara: " << data.chamberVolume << L" cc\r\n";
    results << L"    - Junta: " << data.gasketVolume << L" cc\r\n";
    results << L"    - Deck: " << data.deckVolume << L" cc\r\n";
    results << L"    - Dome/Dish: " << data.pistonDomeVolume << L" cc";
    if (data.pistonDomeVolume > 0) results << L" (Dome - aumenta CR)\r\n";
    else if (data.pistonDomeVolume < 0) results << L" (Dish - reduz CR)\r\n";
    else results << L" (Flat top)\r\n";
    results << L"    - Rebaixos Valvulas: " << data.valveRelief << L" cc\r\n\r\n";
    
    results << L"COMPRESSAO:\r\n";
    results << std::setprecision(1);
    results << L"  Taxa de Compressao ESTATICA (SCR): " << staticCR << L":1";
    
    if (staticCR < 8.0) results << L" (Baixa)\r\n";
    else if (staticCR < 10.0) results << L" (Moderada)\r\n";
    else if (staticCR < 12.0) results << L" (Alta)\r\n";
    else results << L" (Muito Alta - Race)\r\n";
    
    results << L"  Taxa de Compressao DINAMICA (DCR): " << dynamicCR << L":1";
    if (dynamicCR < 7.0) results << L" (Baixa)\r\n";
    else if (dynamicCR < 8.5) results << L" (Ideal)\r\n";
    else if (dynamicCR > 9.0) results << L" ! ALTA - Risco de detonacao\r\n";
    else results << L" (Boa)\r\n";
    
    results << L"  IVC usado: " << editIVCAngle.GetValue() << L" graus ABDC\r\n\r\n";
    
    results << std::setprecision(0);
    results << L"  Pressao Estimada no Cilindro: " << cylinderPressure << L" kPa\r\n\r\n";
    
    results << L"RECOMENDACOES:\r\n";
    
    if (staticCR > 13.0) {
        results << L"! CRITICO: CR muito alta para motor aspirado!\r\n";
        results << L"  Acoes: Usar combustivel alta octanagem (100+)\r\n";
        results << L"         Ou aumentar volume de clearance\r\n\r\n";
    } else if (staticCR > 11.5) {
        results << L"! ATENCAO: CR alta - Requer combustivel premium\r\n";
        results << L"  Minimo: 95 octanas (gasolina comum pode detonar)\r\n\r\n";
    } else if (staticCR < 8.0) {
        results << L"! Compressao baixa - Performance comprometida\r\n";
        results << L"  Considere pistoes com dome ou cabecote menor\r\n\r\n";
    }
    
    if (dynamicCR > 9.0) {
        results << L"! DCR ALTA: Risco de detonacao!\r\n";
        results << L"  Solucoes:\r\n";
        results << L"  1. Usar comando com IVC mais tarde (>60 ABDC)\r\n";
        results << L"  2. Reduzir CR estatica\r\n";
        results << L"  3. Combustivel alta octanagem\r\n\r\n";
    }
    
    if (data.deckVolume < 0) {
        results << L"! ATENCAO: Pistao ACIMA do deck (pop-up)\r\n";
        results << L"  Verificar folga pistao-valvula OBRIGATORIA!\r\n\r\n";
    }
    
    results << L"===============================================================\r\n";
    results << L"Formulas Blair - Design and Simulation of Four-Stroke Engines\r\n";
    results << L"CR = (Vswept + Vclearance) / Vclearance\r\n";
    results << L"DCR considera volume perdido ate IVC fechar\r\n";
    results << L"SAE Paper 2007-01-0148 - Dynamic Compression Ratio Analysis\r\n";
    
    SetWindowTextW(textResults, results.str().c_str());

    UpdateGraph(staticCR);

    if (dataManager) {
        EngineProject::CompressionData compressionData;
        compressionData.chamberVolume = editChamberVol.GetValue();
        compressionData.pistonDomeVolume = editPistonDome.GetValue();
        compressionData.gasketThickness = editGasketThick.GetValue();
        compressionData.deckHeight = editDeckHeight.GetValue();
        compressionData.valveRelief = editValveRelief.GetValue();
        compressionData.ivcAngle = editIVCAngle.GetValue();
        
        dataManager->UpdateCompressionData(compressionData);
    }
}

void CompressionTab::SetBoreStroke(double bore, double stroke, int cylinders, double rodLength) {
    editBore.SetValue(bore);
    editStroke.SetValue(stroke);
    editCylinders.SetValue(static_cast<double>(cylinders));
}

void CompressionTab::OnClear() {
    editBore.SetValue(0.0);
    editStroke.SetValue(0.0);
    editCylinders.SetValue(4.0);
    editChamberVol.SetValue(0.0);
    editPistonDome.SetValue(0.0);
    editGasketThick.SetValue(1.0);
    editDeckHeight.SetValue(0.0);
    editValveRelief.SetValue(0.0);
    editIVCAngle.SetValue(45.0);
    SetWindowTextW(textResults, L"");
}

void CompressionTab::RecalculateLayout() {
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

LRESULT CALLBACK CompressionTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        CompressionTab* tab = (CompressionTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

void CompressionTab::UpdateGraph(double currentCR) {
    graphRenderer.ClearSeries();

    if (currentCR <= 0) {
        if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
        return;
    }

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Pressao no Cilindro vs Taxa de Compressao";
    cfg.xAxisLabel = L"Taxa de Compressao";
    cfg.yAxisLabel = L"Pressao (kPa)";
    cfg.xMin = 6; cfg.xMax = 16;
    cfg.yMin = 0; cfg.yMax = 4000;
    cfg.xGridStep = 2;
    cfg.yGridStep = 1000;

    // Curva P = P_atm * CR^n (n=1.3)
    std::vector<float> xCR, yPressure;
    for (float cr = 6.0f; cr <= 16.0f; cr += 0.2f) {
        float p = 101.325f * powf(cr, 1.3f);
        xCR.push_back(cr);
        yPressure.push_back(p);
    }
    graphRenderer.AddSeries(
        GraphRenderer::CreateSeries(xCR, yPressure,
            Gdiplus::Color(255, 0, 150, 255), L"P = Patm * CR^1.3", 2.5f));

    // Marcador do CR atual
    float pCurrent = 101.325f * powf((float)currentCR, 1.3f);
    std::vector<float> xMark, yMark;
    xMark.push_back((float)currentCR); yMark.push_back(0);
    xMark.push_back((float)currentCR); yMark.push_back(pCurrent);
    graphRenderer.AddSeries(
        GraphRenderer::CreateSeries(xMark, yMark,
            Gdiplus::Color(255, 255, 80, 80), L"CR Atual", 2.0f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
