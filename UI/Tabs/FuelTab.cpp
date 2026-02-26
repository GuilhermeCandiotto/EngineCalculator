// ============================================================================
// FuelTab.cpp - Implementação da Aba de Sistema de Combustível
// Engine Calculator v1.0.0 - Março 2026
// ============================================================================
#include "FuelTab.h"
#include "../TabPages.h"
#include "../../Core/FuelSystemCalculator.h"
#include "../../Core/EngineCore.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

FuelTab::FuelTab(HWND parent, HINSTANCE instance) 
    : TabPage(parent, instance), comboSystemType(nullptr), textResults(nullptr), hwndGraph(nullptr) {
}

FuelTab::~FuelTab() {
}

void FuelTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
    HWND label = CreateWindowW(
        L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, width, 20,
        hwndPage, nullptr, hInst, nullptr
    );
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(label, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void FuelTab::CreateControls() {
    int leftMargin = GetMargin();
    int topMargin = GetMargin();
    int labelWidth = GetLabelWidth(0.30f);
    int editWidth = GetEditWidth(0.27f);
    int rowHeight = 35;
    int currentY = topMargin;
    
    // Tipo de Sistema de Combustível
    CreateLabel(L"Tipo de Sistema:", leftMargin, currentY, labelWidth);
    comboSystemType = CreateWindowW(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        leftMargin + labelWidth, currentY - 3, editWidth, 120,
        hwndPage, (HMENU)IDC_COMBO_FUEL_SYSTEM_TYPE, hInst, nullptr
    );
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(comboSystemType, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(comboSystemType, CB_ADDSTRING, 0, (LPARAM)L"Carburador");
    SendMessageW(comboSystemType, CB_ADDSTRING, 0, (LPARAM)L"EFI com Plenum (MPFI)");
    SendMessageW(comboSystemType, CB_ADDSTRING, 0, (LPARAM)L"EFI com ITB");
    SendMessageW(comboSystemType, CB_ADDSTRING, 0, (LPARAM)L"Injeção Direta (GDI)");
    SendMessageW(comboSystemType, CB_SETCURSEL, 0, 0);
    
    CreateTooltip(comboSystemType,
        L"TIPOS DE SISTEMA:\n"
        L"• Carburador - Sistema mecânico tradicional\n"
        L"• EFI Plenum - Injeção multiponto com plenum (intake comum)\n"
        L"• EFI ITB - Individual Throttle Bodies (borboletas individuais)\n"
        L"• GDI - Injeção direta na câmara (alta pressão)");
    currentY += rowHeight;
    
    // Potência Alvo
    CreateLabel(L"Potência Alvo (HP):", leftMargin, currentY, labelWidth);
    editTargetHP.Create(hwndPage, IDC_EDIT_TARGET_HP_FUEL, 
                        leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                        50.0, 3000.0, 1);
    CreateTooltip(editTargetHP.GetHandle(),
        L"O QUE É: Potência ALVO do motor em HP (horsepower).\n"
        L"USADO PARA: Calcular CFM necessário (carburador) ou vazão de injetores.\n"
        L"DICA: Use 10-15% acima da potência real para margem de segurança.");
    currentY += rowHeight;
    
    // Pressão de Boost
    CreateLabel(L"Pressão de Boost (PSI):", leftMargin, currentY, labelWidth);
    editBoostPressure.Create(hwndPage, IDC_EDIT_BOOST_PRESSURE_FUEL, 
                             leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                             0.0, 50.0, 1);
    CreateTooltip(editBoostPressure.GetHandle(),
        L"O QUE É: Pressão positiva de boost (turbo/supercharger) em PSI.\n"
        L"VALORES TÍPICOS:\n"
        L"• Aspirado: 0 PSI\n"
        L"• Street Turbo: 5-15 PSI\n"
        L"• Race Turbo: 20-40+ PSI\n"
        L"ATENÇÃO: Afeta dramaticamente CFM e vazão de combustível!");
    currentY += rowHeight;
    
    // Pressão de Combustível
    CreateLabel(L"Pressão de Combustível (PSI):", leftMargin, currentY, labelWidth);
    editFuelPressure.Create(hwndPage, IDC_EDIT_FUEL_PRESSURE, 
                            leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                            3.0, 100.0, 1);
    CreateTooltip(editFuelPressure.GetHandle(),
        L"O QUE É: Pressão de combustível na linha/rail.\n"
        L"VALORES TÍPICOS:\n"
        L"• Carburador: 6-8 PSI\n"
        L"• EFI Port: 43.5 PSI (3 bar)\n"
        L"• GDI: 500-2900 PSI (alta pressão)\n"
        L"• Turbo: Base + 1:1 com boost");
    currentY += rowHeight;
    
    // AFR Alvo
    CreateLabel(L"AFR Alvo:", leftMargin, currentY, labelWidth);
    editTargetAFR.Create(hwndPage, IDC_EDIT_TARGET_AFR, 
                         leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                         10.0, 18.0, 1);
    CreateTooltip(editTargetAFR.GetHandle(),
        L"O QUE É: Air/Fuel Ratio (relação ar/combustível) alvo.\n"
        L"VALORES TÍPICOS (GASOLINA):\n"
        L"• Estequiométrico: 14.7:1 (economia)\n"
        L"• Máxima Potência: 12.5-13.0:1\n"
        L"• Turbo/Race: 11.5-12.0:1\n"
        L"• Cruise/Economia: 15.0-16.0:1");
    currentY += rowHeight;
    
    // Número de Injetores
    CreateLabel(L"Número de Injetores:", leftMargin, currentY, labelWidth);
    editNumInjectors.Create(hwndPage, IDC_EDIT_NUM_INJECTORS, 
                            leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                            1.0, 16.0, 0);
    CreateTooltip(editNumInjectors.GetHandle(),
        L"O QUE É: Quantidade total de injetores do motor.\n"
        L"CONFIGURAÇÕES COMUNS:\n"
        L"• 4 cilindros: 4 injetores (1 por cilindro)\n"
        L"• V8: 8 injetores\n"
        L"• Direct Injection: Pode ter 2 por cilindro\n"
        L"NOTA: Válido apenas para EFI, ignorado em carburação.");
    currentY += rowHeight + 20;
    
    // Área de resultados
    CreateLabel(L"Resultados:", leftMargin, currentY);
    currentY += 25;
    
    textResults = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        leftMargin, currentY, GetResultsWidth(), 400,
        hwndPage, (HMENU)IDC_RESULT_TEXT, hInst, nullptr
    );
    SendMessage(textResults, WM_SETFONT, (WPARAM)GetAppResultsFont(), TRUE);

    currentY += 405;
    WNDCLASSW wc = {};
    wc.lpfnWndProc = GraphWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"EngineCalcGraphFuel";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    hwndGraph = CreateWindowExW(WS_EX_CLIENTEDGE, L"EngineCalcGraphFuel", L"",
        WS_CHILD | WS_VISIBLE, leftMargin, currentY, GetResultsWidth(), 220,
        hwndPage, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);
}

void FuelTab::Create() {
    RegisterTabPageClass(hInst);
    hwndPage = CreateWindowExW(0, L"EngineCalcTabPage", L"",
        WS_CHILD | WS_VSCROLL, 0, 30, 800, 450,
        hwndParent, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndPage, GWLP_USERDATA, (LONG_PTR)this);
    CreateControls();
    SetContentHeight(900);
}

void FuelTab::OnCalculate() {
    // Cria EngineCore básico para cálculos
    EngineDataManager* dataMgr = EngineDataManager::GetInstance();
    const EngineProject& project = dataMgr->GetProject();
    
    EngineCore engine;
    engine.SetBore(project.basicData.bore);
    engine.SetStroke(project.basicData.stroke);
    engine.SetCylinders(project.basicData.cylinders);
    engine.SetEngineType(project.basicData.engineType);
    
    double displacement = engine.CalculateDisplacement();
    
    // Pega valores dos controles
    int systemType = (int)SendMessage(comboSystemType, CB_GETCURSEL, 0, 0);
    double targetHP = editTargetHP.GetValue();
    double boost = editBoostPressure.GetValue();
    double fuelPressure = editFuelPressure.GetValue();
    double targetAFR = editTargetAFR.GetValue();
    int numInjectors = (int)editNumInjectors.GetValue();
    
    // Cria calculadora
    FuelSystemCalculator fuelCalc(&engine);
    
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << L"---------------------------------------------------------------------------\r\n";
    oss << L"         ANALISE DE SISTEMA DE COMBUSTIVEL\r\n";
    oss << L"---------------------------------------------------------------------------\r\n\r\n";
    
    // CFM necessário (para todos os tipos)
    double cfm = fuelCalc.CalculateRequiredCFM(displacement, project.basicData.maxRPM, 85.0);
    if (boost > 0) {
        cfm = fuelCalc.CalculateCFMWithBoost(cfm, boost);
    }
    
    oss << L"CFM NECESSARIO: " << cfm << L" CFM\r\n";
    
    if (systemType == 0) { // Carburador
        oss << L"\r\nCARBURACAO:\r\n";
        oss << L"  Sugestao de Carburador: ";
        if (cfm < 600) oss << L"~" << (int)(cfm * 1.1) << L" CFM (Holley 600)\r\n";
        else if (cfm < 750) oss << L"~750 CFM (Holley 750 DP)\r\n";
        else if (cfm < 850) oss << L"~850 CFM (Holley 850)\r\n";
        else oss << L"~" << (int)(cfm * 1.05) << L" CFM (Dominator)\r\n";
        
        oss << L"  DICA: Para turbo, use CFM para boost + margem 20%\r\n";
    } else { // EFI
        oss << L"\r\nINJECAO ELETRONICA:\r\n";
        
        // Calcula vazao de injetor necessaria
        // BSFC muda com boost (motores turbo consomem mais)
        double bsfc = (boost > 0) ? 0.55 : 0.50; // lb/HP/hr
        
        // HP efetivo considerando boost (aproximacao)
        double effectiveHP = targetHP;
        if (boost > 0) {
            // Regra aproximada: +7 HP por PSI de boost por litro
            double boostHP = (displacement / 1000.0) * boost * 7.0;
            effectiveHP = targetHP + boostHP;
        }
        
        double fuelFlowLbHr = effectiveHP * bsfc;
        double fuelFlowCcMin = fuelFlowLbHr * 453.6 / 0.75 / 60.0; // Conversao para cc/min
        
        // Vazao por injetor (base)
        double injectorFlowCcMin = fuelFlowCcMin / numInjectors;
        
        // CORRECAO POR PRESSAO DE COMBUSTIVEL
        // Formula: Flow2 = Flow1 * sqrt(Pressure2 / Pressure1)
        // Pressao base: 43.5 PSI (3 bar - padrao EFI)
        double basePressure = 43.5;
        double pressureRatio = sqrt(fuelPressure / basePressure);
        double injectorFlowCorrected = injectorFlowCcMin / pressureRatio;
        
        double injectorFlowLbHr = injectorFlowCorrected * 0.75 * 60.0 / 453.6;
        
        oss << L"  Vazao Total Necessaria: " << fuelFlowCcMin << L" cc/min\r\n";
        oss << L"  Vazao por Injetor (" << numInjectors << L" injetores):\r\n";
        oss << L"    -> " << injectorFlowCorrected << L" cc/min @ " << fuelPressure << L" PSI\r\n";
        oss << L"    -> " << injectorFlowLbHr << L" lb/hr @ " << fuelPressure << L" PSI\r\n";
        oss << L"    -> " << injectorFlowCcMin << L" cc/min @ 43.5 PSI (base)\r\n\r\n";
        
        oss << L"  SUGESTOES DE INJETORES (@ 43.5 PSI base):\r\n";
        if (injectorFlowCcMin < 300) oss << L"    -> ~300 cc/min (28 lb/hr) - Street\r\n";
        else if (injectorFlowCcMin < 550) oss << L"    -> ~550 cc/min (52 lb/hr) - Street/Turbo\r\n";
        else if (injectorFlowCcMin < 850) oss << L"    -> ~850 cc/min (80 lb/hr) - High Performance\r\n";
        else oss << L"    -> ~" << (int)(injectorFlowCcMin + 100) << L" cc/min - Race\r\n";
        
        // Duty cycle CORRETO
        // Duty Cycle = (Flow Necessario / Flow Disponivel) × 100%
        // Flow disponivel = injectorFlowCorrected (corrigido pela pressao)
        // Referencia: Fuel Injector Clinic - "Understanding Duty Cycle"
        double dutyCycle = (injectorFlowCcMin / injectorFlowCorrected) * 100.0;
        if (dutyCycle > 100) dutyCycle = 100.0;
        
        oss << L"\r\n  Duty Cycle Estimado: " << dutyCycle << L"%\r\n";
        oss << L"    (Maximo recomendado: 80-85%)\r\n";
        
        if (dutyCycle > 85) {
            oss << L"    ! ATENCAO: Duty cycle muito alto! Injetores insuficientes!\r\n";
        } else if (dutyCycle < 50) {
            oss << L"    OK: Injetores tem boa margem de seguranca.\r\n";
        }
        
        if (systemType == 2) { // ITB
            oss << L"\r\n  SISTEMA ITB (Individual Throttle Bodies):\r\n";
            oss << L"    + Ganho estimado de VE: +5-8%\r\n";
            oss << L"    + Resposta de acelerador instantanea\r\n";
            oss << L"    ! Requer MAP sensor e tuning avancado\r\n";
        }
        
        // Informacao sobre pressao de combustivel
        if (boost > 0 && fuelPressure < 43.5) {
            oss << L"\r\n  ! AVISO: Pressao de combustivel baixa para turbo!\r\n";
            oss << L"    Recomendado: Usar regulador 1:1 (rising rate)\r\n";
            oss << L"    Pressao ideal: " << (43.5 + boost) << L" PSI\r\n";
        }
    }
    
    // Consumo estimado
    oss << L"\r\nCONSUMO DE COMBUSTIVEL:\r\n";
    double bsfc = (boost > 0) ? 0.55 : 0.50;
    double fuelFlowLH = (targetHP * bsfc * 0.4536) / 0.75; // L/h
    oss << L"  Vazao em potencia maxima: " << fuelFlowLH << L" L/h\r\n";
    oss << L"  BSFC: " << (bsfc * 453.6) << L" g/kWh\r\n";
    
    double kmL = 100.0 / 10.0; // Exemplo simplificado
    oss << L"  Consumo medio estimado: ~" << kmL << L" km/L\r\n";
    
    oss << L"\r\nAVISOS E RECOMENDACOES:\r\n";
    oss << L"  ! Pressao de combustivel deve ser constante!\r\n";
    oss << L"  ! Com turbo, use regulador 1:1 (rising rate)\r\n";
    oss << L"  ! Duty cycle >85% = injetores insuficientes\r\n";
    oss << L"  ! AFR muito rico (<11:1) desperdica combustivel\r\n";
    oss << L"  ! AFR muito pobre (>15:1) causa detonacao\r\n";
    
    SetWindowTextW(textResults, oss.str().c_str());

    UpdateGraph(targetHP);

    // Salva dados no projeto
    EngineProject::FuelData fuelData;
    fuelData.systemType = systemType;
    fuelData.targetHP = targetHP;
    fuelData.boostPressure = boost;
    fuelData.fuelPressure = fuelPressure;
    fuelData.targetAFR = targetAFR;
    fuelData.numberOfInjectors = numInjectors;
    dataMgr->UpdateFuelData(fuelData);
}

void FuelTab::OnClear() {
    editTargetHP.SetValue(0.0);
    editBoostPressure.SetValue(0.0);
    editFuelPressure.SetValue(6.0);
    editTargetAFR.SetValue(14.7);
    editNumInjectors.SetValue(4.0);
    SendMessage(comboSystemType, CB_SETCURSEL, 0, 0);
    SetWindowTextW(textResults, L"");
}

void FuelTab::RecalculateLayout() {
    if (!hwndPage) return;
    int newWidth = GetResultsWidth();
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

LRESULT CALLBACK FuelTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        FuelTab* tab = (FuelTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

void FuelTab::UpdateGraph(double targetHP) {
    graphRenderer.ClearSeries();
    if (targetHP <= 0) { if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE); return; }

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Vazao de Combustivel vs Potencia";
    cfg.xAxisLabel = L"HP";
    cfg.yAxisLabel = L"Vazao (cc/min)";
    float maxHP = (float)(targetHP * 1.5);
    cfg.xMin = 0; cfg.xMax = maxHP;
    cfg.yMin = 0;
    float maxFlow = (float)(targetHP * 1.5 * 0.50 * 10.5); // BSFC * conversion
    cfg.yMax = ceilf(maxFlow / 500.0f) * 500.0f;
    cfg.xGridStep = maxHP > 500 ? 200.0f : 100.0f;
    cfg.yGridStep = cfg.yMax > 3000 ? 1000.0f : 500.0f;

    // BSFC 0.50 lb/hp/hr (tipico)
    std::vector<float> xHP, yFlow;
    for (float hp = 0; hp <= maxHP; hp += 10) {
        float flowLbHr = hp * 0.50f;     // lb/hr
        float flowCCMin = flowLbHr * 10.5f; // cc/min (1 lb/hr ~ 10.5 cc/min gasolina)
        xHP.push_back(hp); yFlow.push_back(flowCCMin);
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xHP, yFlow,
        Gdiplus::Color(255, 0, 150, 255), L"BSFC 0.50", 2.5f));

    // Marcador HP alvo
    float targetFlow = (float)(targetHP * 0.50 * 10.5);
    std::vector<float> xM, yM;
    xM.push_back((float)targetHP); yM.push_back(0);
    xM.push_back((float)targetHP); yM.push_back(targetFlow);
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xM, yM,
        Gdiplus::Color(255, 255, 80, 80), L"HP Alvo", 1.5f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
