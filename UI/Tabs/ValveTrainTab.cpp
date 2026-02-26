// ============================================================================
// ValveTrainTab.cpp - Implementação da Aba de Trem de Válvulas
// Engine Calculator v1.0.0 - Fevereiro 2026
// ============================================================================
#include "ValveTrainTab.h"
#include "../../Core/EngineCore.h"
#include "../../Core/ValveTrainCalculator.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>#include <cmath>



ValveTrainTab::ValveTrainTab(HWND parent, HINSTANCE instance)
    : TabPage(parent, instance), textResults(nullptr), hwndGraph(nullptr) {
}

ValveTrainTab::~ValveTrainTab() {
}

void ValveTrainTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
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

void ValveTrainTab::CreateGroupBox(const wchar_t* text, int x, int y, int width, int height) {
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

void ValveTrainTab::CreateControls() {
    int leftMargin = 10;
    int topMargin = 10;
    int labelWidth = GetLabelWidth(0.23f);
    int editWidth = GetEditWidth(0.11f);
    int rowHeight = 28;
    int currentY = topMargin;

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    CreateGroupBox(L"Dimensoes das Valvulas", leftMargin, currentY, 430, 175);
    int groupY = currentY + 20;
    int groupX = leftMargin + 15;

    HWND lblIntakeValve = CreateWindowW(L"STATIC", L"Diametro Valvula Admissao (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblIntakeValve, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editIntakeValveDia.Create(hwndPage, IDC_EDIT_INTAKE_VALVE_DIA,
        groupX + labelWidth, groupY - 3, editWidth, 22, 15.0, 80.0, 2);
    CreateTooltip(editIntakeValveDia.GetHandle(),
        L"O QUE E: Diametro externo da cabeca da valvula de admissao.\n"
        L"COMO MEDIR: Paquimetro no diametro da cabeca da valvula.\n"
        L"Geralmente de 40 a 47% do bore.");
    groupY += rowHeight;

    HWND lblExhaustValve = CreateWindowW(L"STATIC", L"Diametro Valvula Escape (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblExhaustValve, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editExhaustValveDia.Create(hwndPage, IDC_EDIT_EXHAUST_VALVE_DIA,
        groupX + labelWidth, groupY - 3, editWidth, 22, 15.0, 70.0, 2);
    CreateTooltip(editExhaustValveDia.GetHandle(),
        L"O QUE E: Diametro externo da cabeca da valvula de escape.\n"
        L"COMO MEDIR: Paquimetro no diametro da cabeca da valvula.\n"
        L"Geralmente 85-90% do diametro da admissao");
    groupY += rowHeight;

    HWND lblNumIntake = CreateWindowW(L"STATIC", L"No Valvulas Admissao/Cilindro:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblNumIntake, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editNumIntakeValves.Create(hwndPage, IDC_EDIT_NUM_INTAKE_VALVES,
        groupX + labelWidth, groupY - 3, editWidth, 22, 1.0, 3.0, 0);
    editNumIntakeValves.SetValue(2.0);
    CreateTooltip(lblNumIntake,
        L"Quantidade de valvulas de ADMISSAO por cilindro.");
    groupY += rowHeight;

    HWND lblNumExhaust = CreateWindowW(L"STATIC", L"No Valvulas Escape/Cilindro:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblNumExhaust, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editNumExhaustValves.Create(hwndPage, IDC_EDIT_NUM_EXHAUST_VALVES,
        groupX + labelWidth, groupY - 3, editWidth, 22, 1.0, 3.0, 0);
    editNumExhaustValves.SetValue(2.0);
    CreateTooltip(lblNumExhaust,
        L"Quantidade de valvulas de ESCAPE por cilindro.");
    groupY += rowHeight;

    HWND lblSeatAngle = CreateWindowW(L"STATIC", L"Angulo do Assento (graus):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblSeatAngle, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editSeatAngle.Create(hwndPage, IDC_EDIT_SEAT_ANGLE,
        groupX + labelWidth, groupY - 3, editWidth, 22, 30.0, 60.0, 1);
    editSeatAngle.SetValue(45.0);
    CreateTooltip(editSeatAngle.GetHandle(),
        L"Angulo do assento da valvula no cabecote.\n"
        L"Afeta a vedacao e o fluxo\n"
        L"Padrao: 45 graus melhor vedacao.\n"
        L"Ideal: Multi Angulos 30 - 45 - 60 graus");

    currentY += 185;

    CreateGroupBox(L"Trem de Valvulas e Molas", 450, topMargin, 410, 340);
    groupY = topMargin + 20;
    groupX = 465;
    labelWidth = 180;

    HWND lblMaxLift = CreateWindowW(L"STATIC", L"Lift Maximo (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblMaxLift, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editMaxLift.Create(hwndPage, IDC_EDIT_MAX_LIFT,
        groupX + labelWidth, groupY - 3, editWidth, 22, 1.0, 20.0, 2);
    CreateTooltip(editMaxLift.GetHandle(),
        L"Abertura maxima da valvula em relacao ao assento.\n"
        L"Tipico: 9-12mm (street), 12-15mm (performance), 15+ (race).\n"
        L"ATENCAO: Verificar folga pistao-valvula!");
    groupY += rowHeight;

    HWND lblSpringRate = CreateWindowW(L"STATIC", L"Taxa da Mola (N/mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblSpringRate, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editSpringRate.Create(hwndPage, IDC_EDIT_SPRING_RATE,
        groupX + labelWidth, groupY - 3, editWidth, 22, 5.0, 200.0, 2);
    CreateTooltip(editSpringRate.GetHandle(),
        L"O QUE E: Rigidez da mola (spring rate) em Newtons por milimetro.\n"
        L"E a FORCA necessaria para comprimir a mola 1mm.\n"
        L"COMO MEDIR: Dinamometro de Molas (ou specs do fabricante).\n\n"
        L"VALORES TIPICOS:\n"
        L"Street: 30-50 N/mm (springs simples)\n"
        L"Performance: 50-80 N/mm (dual springs)\n"
        L"Race: 80-150+ N/mm (triple springs, titanium)\n"
        L"ATENCAO: Taxa muito baixa = valve float em alta RPM!");
    groupY += rowHeight;

    HWND lblSpringPreload = CreateWindowW(L"STATIC", L"Pre-carga da Mola (N):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblSpringPreload, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editSpringPreload.Create(hwndPage, IDC_EDIT_SPRING_PRELOAD,
        groupX + labelWidth, groupY - 3, editWidth, 22, 50.0, 1000.0, 1);
    CreateTooltip(editSpringPreload.GetHandle(),
        L"O QUE E: Forca da mola quando INSTALADA (valvula fechada).\n"
        L"Tambem chamada 'seat pressure'.\n"
        L"Garante que a valvula sempre retorna e fecha.\n"
        L"COMO CALCULAR: Pre-carga = Taxa × (H_livre - H_instalada)\n\n"
        L"VALORES TIPICOS:\n"
        L"Street: 100-200 N (dual spring)\n"
        L"Performance: 200-350 N\n"
        L"Race: 350-500+ N\n"
        L"Muito baixo = valve float | Muito alto = desgaste excessivo");
    groupY += rowHeight;

    HWND lblInstalledHeight = CreateWindowW(L"STATIC", L"Altura Instalada (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblInstalledHeight, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editSpringInstalledHeight.Create(hwndPage, IDC_EDIT_SPRING_INSTALLED_H,
        groupX + labelWidth, groupY - 3, editWidth, 22, 20.0, 80.0, 2);
    CreateTooltip(editSpringInstalledHeight.GetHandle(),
        L"O QUE E: Altura da mola quando instalada no cabecote.\n"
        L"Tambem chamada 'installed height'.\n"
        L"COMO MEDIR: Paquimetro do assento ao prato da mola.\n"
        L"Medir com mola INSTALADA e valvula FECHADA.\n\n"
        L"AJUSTE: Shimming (arruelas) sob mola:\n"
        L"Adicionar shim = reduz altura = AUMENTA pre-carga\n"
        L"Remover shim = aumenta altura = REDUZ pre-carga");
    groupY += rowHeight;

    HWND lblCoilBind = CreateWindowW(L"STATIC", L"Altura Coil Bind (mm):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblCoilBind, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editSpringCoilBindHeight.Create(hwndPage, IDC_EDIT_SPRING_COILBIND_H,
        groupX + labelWidth, groupY - 3, editWidth, 22, 10.0, 60.0, 2);
    CreateTooltip(editSpringCoilBindHeight.GetHandle(),
        L"O QUE E: Altura onde as espiras da mola SE TOCAM (coil bind).\n"
        L"COMO MEDIR: Spec do fabricante ou comprimir totalmente.\n\n"
        L"MARGEM DE SEGURANCA MINIMA:\n"
        L"Altura Instalada - Lift - Coil Bind = 2-3mm\n\n"
        L"Exemplo: H_inst 35mm, Lift 12mm, Coil 20mm\n"
        L"Margem = 35 - 12 - 20 = 3mm OK\n\n"
        L"ATENCAO: Coil bind = Possivel quebra do motor.");
    groupY += rowHeight;

    HWND lblRetainerWeight = CreateWindowW(L"STATIC", L"Peso Prato/Trava (gramas):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRetainerWeight, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editRetainerWeight.Create(hwndPage, IDC_EDIT_RETAINER_WEIGHT,
        groupX + labelWidth, groupY - 3, editWidth, 22, 5.0, 100.0, 1);
    CreateTooltip(editRetainerWeight.GetHandle(),
        L"Peso do prato da mola + travas + locks.\n"
        L"Tipico: 10-30g (steel), 5-15g (titanium).");
    groupY += rowHeight;

    HWND lblValveWeight = CreateWindowW(L"STATIC", L"Peso da Valvula (gramas):",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblValveWeight, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editValveWeight.Create(hwndPage, IDC_EDIT_VALVE_WEIGHT,
        groupX + labelWidth, groupY - 3, editWidth, 22, 20.0, 200.0, 1);
    CreateTooltip(editValveWeight.GetHandle(),
        L"Peso total da valvula.\n"
        L"Tipico: 50-100g (steel), 30-60g (titanium), 40-70g (inconel).");
    groupY += rowHeight;

    HWND lblRockerRatio = CreateWindowW(L"STATIC", L"Relacao do Balanceiro:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblRockerRatio, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    editRockerRatio.Create(hwndPage, IDC_EDIT_ROCKER_RATIO,
        groupX + labelWidth, groupY - 3, editWidth, 22, 1.0, 2.5, 2);
    editRockerRatio.SetValue(1.5);
    CreateTooltip(editRockerRatio.GetHandle(),
        L"O QUE E: Multiplicacao do movimento do comando na valvula.\n"
        L"Lift na valvula = Lift do came × Rocker Ratio");
    groupY += rowHeight;

    HWND lblTestRPM = CreateWindowW(L"STATIC", L"RPM de Teste:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, groupX, groupY, labelWidth, 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblTestRPM, WM_SETFONT, (WPARAM)hFont, TRUE);
    CreateTooltip(lblTestRPM,
        L"RPM para analise de float e forcas.\n"
        L"Use o RPM MAXIMO que o motor vai atingir");
    
    editTestRPM.Create(hwndPage, IDC_EDIT_TEST_RPM,
        groupX + labelWidth, groupY - 3, editWidth, 22, 1000.0, 15000.0, 0);
    editTestRPM.SetValue(6000.0);
    CreateTooltip(editTestRPM.GetHandle(),
        L"RPM para analise de float e forcas.\n"
        L"Use o RPM MAXIMO que o motor vai atingir.\n"
        L"O software calcula se a mola aguenta esta RPM.\n"
        L"Se RPM Float < RPM Teste = PERIGO!");

    currentY = 370;
    CreateLabel(L"Resultados:", leftMargin, currentY);
    currentY += 25;

    textResults = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        leftMargin, currentY, GetResultsWidth(0.97f), 150,
        hwndPage,
        (HMENU)IDC_RESULT_TEXT,
        hInst,
        nullptr
    );

    SendMessage(textResults, WM_SETFONT, (WPARAM)GetAppResultsFont(), TRUE);

    currentY += 155;
    WNDCLASSW wc = {};
    wc.lpfnWndProc = GraphWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"EngineCalcGraphVT";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    hwndGraph = CreateWindowExW(WS_EX_CLIENTEDGE, L"EngineCalcGraphVT", L"",
        WS_CHILD | WS_VISIBLE, leftMargin, currentY, GetResultsWidth(0.97f), 220,
        hwndPage, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);
}

void ValveTrainTab::Create() {
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
    SetContentHeight(930);
}

void ValveTrainTab::SetEngineData(double bore, double stroke, int cylinders) {
    // Implementado para receber dados da aba basica se necessario
}

void ValveTrainTab::OnCalculate() {
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

    ValveTrainCalculator calc(&engine);

    ValveData valveData;
    valveData.intakeValveDiameter = editIntakeValveDia.GetValue();
    valveData.exhaustValveDiameter = editExhaustValveDia.GetValue();
    valveData.numberOfIntakeValves = static_cast<int>(editNumIntakeValves.GetValue());
    valveData.numberOfExhaustValves = static_cast<int>(editNumExhaustValves.GetValue());
    valveData.seatAngle = editSeatAngle.GetValue();

    ValveTrainData trainData;
    trainData.maxLift = editMaxLift.GetValue();
    trainData.springRate = editSpringRate.GetValue();
    trainData.springPreload = editSpringPreload.GetValue();
    trainData.springInstalledHeight = editSpringInstalledHeight.GetValue();
    trainData.springCoilBindHeight = editSpringCoilBindHeight.GetValue();
    trainData.retainerWeight = editRetainerWeight.GetValue();
    trainData.valveWeight = editValveWeight.GetValue();
    trainData.rockerRatio = editRockerRatio.GetValue();

    if (!calc.IsValidValveData(valveData) || !calc.IsValidValveTrainData(trainData)) {
        SetWindowTextW(textResults, L"Dados invalidos. Verifique os valores inseridos.");
        return;
    }

    double rpm = editTestRPM.GetValue();

    double valveToPistonRatio = calc.CalculateValveToPistonAreaRatio(valveData);
    FlowResults flowResults = calc.CalculateFlowCharacteristics(valveData, trainData.maxLift, rpm);
    double springForce = calc.CalculateSpringForceAtMaxLift(trainData);
    double coilBindMargin = calc.CalculateCoilBindMargin(trainData);
    double maxAccel = calc.CalculateMaxValveAcceleration(trainData.maxLift, 240.0, rpm);
    double inertiaForce = calc.CalculateInertiaForce(trainData, maxAccel);
    double floatRPM = calc.CalculateValveFloatRPM(trainData, 240.0);

    std::wostringstream results;
    results << std::fixed << std::setprecision(2);
    results << L"---------------------------------------------------------------\r\n";
    results << L"  ANALISE DO TREM DE VALVULAS\r\n";
    results << L"---------------------------------------------------------------\r\n\r\n";

    results << L"VALVULAS:\r\n";
    results << L"  Relacao Valvula/Pistao: " << (valveToPistonRatio * 100.0) << L"% ";
    results << calc.GetValveSizingAdvice(valveData) << L"\r\n";
    results << L"  Area Fluxo Admissao: " << flowResults.intakeFlowArea << L" mm2\r\n";
    results << L"  Area Fluxo Escape: " << flowResults.exhaustFlowArea << L" mm2\r\n";
    results << L"  Relacao Adm/Esc: " << flowResults.flowRatio << L"\r\n\r\n";

    results << L"MOLAS:\r\n";
    results << L"  Forca no Lift Maximo: " << springForce << L" N\r\n";
    results << L"  Margem Coil Bind: " << coilBindMargin << L" mm ";
    results << calc.GetSpringAdvice(trainData, rpm) << L"\r\n";
    results << L"  Aceleracao Maxima: " << (maxAccel / 9.81) << L" g's\r\n";
    results << L"  Forca de Inercia: " << inertiaForce << L" N\r\n";
    results << L"  RPM Float Estimado: " << floatRPM << L" rpm";
    if (floatRPM < rpm) results << L" ! CRITICO!";
    results << L"\r\n\r\n";

    results << L"Referencias: Blair - Design and Simulation of Four-Stroke Engines";

    SetWindowTextW(textResults, results.str().c_str());

    UpdateGraph(floatRPM, rpm);
}

void ValveTrainTab::OnClear() {
    editIntakeValveDia.SetValue(0.0);
    editExhaustValveDia.SetValue(0.0);
    editNumIntakeValves.SetValue(2.0);
    editNumExhaustValves.SetValue(2.0);
    editSeatAngle.SetValue(45.0);
    editMaxLift.SetValue(0.0);
    editSpringRate.SetValue(0.0);
    editSpringPreload.SetValue(0.0);
    editSpringInstalledHeight.SetValue(0.0);
    editSpringCoilBindHeight.SetValue(0.0);
    editRetainerWeight.SetValue(0.0);
    editValveWeight.SetValue(0.0);
    editRockerRatio.SetValue(1.5);
    editTestRPM.SetValue(6000.0);
    SetWindowTextW(textResults, L"");
}

void ValveTrainTab::RecalculateLayout() {
    if (!hwndPage) return;
    int newWidth = GetResultsWidth(0.97f);
    if (textResults) {
        RECT rc;
        GetWindowRect(textResults, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(textResults, NULL, rc.left, rc.top, newWidth, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    if (hwndGraph) {
        RECT rc;
        GetWindowRect(hwndGraph, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
        SetWindowPos(hwndGraph, NULL, rc.left, rc.top, newWidth, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
        InvalidateRect(hwndGraph, NULL, TRUE);
    }
}

LRESULT CALLBACK ValveTrainTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        ValveTrainTab* tab = (ValveTrainTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

void ValveTrainTab::UpdateGraph(double floatRPM, double testRPM) {
    graphRenderer.ClearSeries();
    double springRate = editSpringRate.GetValue();
    double preload = editSpringPreload.GetValue();
    double maxLift = editMaxLift.GetValue();
    if (springRate <= 0 || maxLift <= 0) { if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE); return; }

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Forca da Mola vs Lift";
    cfg.xAxisLabel = L"Lift (mm)";
    cfg.yAxisLabel = L"Forca (N)";
    cfg.xMin = 0; cfg.xMax = (float)(maxLift * 1.2);
    float maxForce = (float)(preload + springRate * maxLift * 1.2);
    cfg.yMin = 0; cfg.yMax = maxForce;
    cfg.xGridStep = cfg.xMax > 10 ? 2.0f : 1.0f;
    cfg.yGridStep = maxForce > 500 ? 200.0f : (maxForce > 200 ? 100.0f : 50.0f);

    std::vector<float> xLift, yForce;
    for (float l = 0; l <= cfg.xMax; l += 0.2f) {
        xLift.push_back(l);
        yForce.push_back((float)(preload + springRate * l));
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xLift, yForce,
        Gdiplus::Color(255, 0, 180, 80), L"Forca Mola", 2.5f));

    // Marcador no lift maximo
    std::vector<float> xM, yM;
    xM.push_back((float)maxLift); yM.push_back(0);
    xM.push_back((float)maxLift); yM.push_back((float)(preload + springRate * maxLift));
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xM, yM,
        Gdiplus::Color(255, 255, 80, 80), L"Lift Max", 1.5f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
