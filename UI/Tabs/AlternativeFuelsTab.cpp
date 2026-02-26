// ============================================================================
// AlternativeFuelsTab.cpp - Implementação da Aba de Combustíveis Alternativos
// Engine Calculator v1.0.0 - Março 2026
// ============================================================================
#include "AlternativeFuelsTab.h"
#include "../TabPages.h"
#include "../../Core/AlternativeFuelsCalculator.h"
#include "../../Core/EngineDataManager.h"
#include <sstream>
#include <iomanip>
#include <cmath>

AlternativeFuelsTab::AlternativeFuelsTab(HWND parent, HINSTANCE instance) 
    : TabPage(parent, instance), comboNitrousSystem(nullptr), textResults(nullptr), hwndGraph(nullptr) {
}

AlternativeFuelsTab::~AlternativeFuelsTab() {
}

void AlternativeFuelsTab::CreateLabel(const wchar_t* text, int x, int y, int width) {
    HWND label = CreateWindowW(
        L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, width, 20,
        hwndPage, nullptr, hInst, nullptr
    );
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(label, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void AlternativeFuelsTab::CreateControls() {
    int leftMargin = GetMargin();
    int topMargin = GetMargin();
    int labelWidth = GetLabelWidth(0.35f);
    int editWidth = GetEditWidth(0.27f);
    int rowHeight = 35;
    int currentY = topMargin;
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    
    // Seção Óxido Nitroso
    HWND lblNitrous = CreateWindowW(L"STATIC", L"=== OXIDO NITROSO (N2O / NOS) ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        leftMargin, currentY, GetResultsWidth(), 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblNitrous, WM_SETFONT, (WPARAM)hFont, TRUE);
    currentY += 30;
    
    // Tipo de Sistema de Nitro
    CreateLabel(L"Tipo de Sistema Nitro:", leftMargin, currentY, labelWidth);
    comboNitrousSystem = CreateWindowW(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        leftMargin + labelWidth, currentY - 3, editWidth, 100,
        hwndPage, (HMENU)IDC_COMBO_NITROUS_SYSTEM, hInst, nullptr
    );
    SendMessage(comboNitrousSystem, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(comboNitrousSystem, CB_ADDSTRING, 0, (LPARAM)L"Dry Shot");
    SendMessageW(comboNitrousSystem, CB_ADDSTRING, 0, (LPARAM)L"Wet Shot");
    SendMessageW(comboNitrousSystem, CB_ADDSTRING, 0, (LPARAM)L"Direct Port");
    SendMessageW(comboNitrousSystem, CB_SETCURSEL, 1, 0);
    
    CreateTooltip(comboNitrousSystem,
        L"TIPOS DE SISTEMA:\n"
        L"• Dry Shot - Só N2O, precisa enriquecer separado (PERIGOSO!)\n"
        L"• Wet Shot - N2O + combustível (mais seguro)\n"
        L"• Direct Port - Injeção por cilindro (melhor)");
    currentY += rowHeight;
    
    // Ganho de HP com Nitro
    CreateLabel(L"Ganho de HP com Nitro:", leftMargin, currentY, labelWidth);
    editNitrousHP.Create(hwndPage, IDC_EDIT_NITROUS_HP, 
                         leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                         25.0, 500.0, 1);
    CreateTooltip(editNitrousHP.GetHandle(),
        L"O QUE É: Ganho ADICIONAL de HP com óxido nitroso.\n"
        L"SHOTS COMUNS:\n"
        L"• 50 HP - Street safe\n"
        L"• 75 HP - Street/Mild\n"
        L"• 100-150 HP - Performance\n"
        L"• 200+ HP - Race (requer pistões forjados!)");
    currentY += rowHeight;
    
    // Pressão do Cilindro de N2O
    CreateLabel(L"Pressão do Cilindro (PSI):", leftMargin, currentY, labelWidth);
    editBottlePressure.Create(hwndPage, IDC_EDIT_BOTTLE_PRESSURE, 
                               leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                               800.0, 1100.0, 0);
    CreateTooltip(editBottlePressure.GetHandle(),
        L"O QUE É: Pressão do cilindro de N2O.\n"
        L"VALORES TÍPICOS:\n"
        L"• 900-1000 PSI - Operação normal\n"
        L"• <800 PSI - Cilindro vazio\n"
        L"NOTA: Pressão varia com temperatura ambiente!");
    currentY += rowHeight + 20;
    
    // Seção Nitrometano
    HWND lblNitromethane = CreateWindowW(L"STATIC", L"=== NITROMETANO (CH3NO2) ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        leftMargin, currentY, GetResultsWidth(), 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblNitromethane, WM_SETFONT, (WPARAM)hFont, TRUE);
    currentY += 30;
    
    // Percentual de Nitrometano
    CreateLabel(L"Percentual de Nitrometano (%):", leftMargin, currentY, labelWidth);
    editNitromethanePercent.Create(hwndPage, IDC_EDIT_NITROMETHANE_PERCENT, 
                                    leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                                    0.0, 100.0, 1);
    CreateTooltip(editNitromethanePercent.GetHandle(),
        L"O QUE É: % de nitrometano no combustível.\n"
        L"APLICAÇÕES:\n"
        L"• 10-20% - Drag racing mild\n"
        L"• 50-60% - Drag racing serious\n"
        L"• 90-100% - Top Fuel (EXTREMO!)\n"
        L"ATENÇÃO: Requer CR alta e ignição especial!");
    currentY += rowHeight + 20;
    
    // Seção Metanol
    HWND lblMethanol = CreateWindowW(L"STATIC", L"=== METANOL (CH3OH) ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        leftMargin, currentY, GetResultsWidth(), 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblMethanol, WM_SETFONT, (WPARAM)hFont, TRUE);
    currentY += 30;
    
    // Percentual de Metanol
    CreateLabel(L"Percentual de Metanol (%):", leftMargin, currentY, labelWidth);
    editMethanolPercent.Create(hwndPage, IDC_EDIT_METHANOL_PERCENT, 
                                leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                                0.0, 100.0, 1);
    CreateTooltip(editMethanolPercent.GetHandle(),
        L"O QUE É: % de metanol no combustível.\n"
        L"APLICAÇÕES:\n"
        L"• 85% (M85) - Similar E85, mais octanagem\n"
        L"• 100% (M100) - Racing puro (IndyCar usa!)\n"
        L"VANTAGENS: RON ~110, efeito refrigerante\n"
        L"DESVANTAGENS: Corrosivo, consome 2x mais!");
    currentY += rowHeight + 20;
    
    // Seção Water/Methanol Injection
    HWND lblWaterMeth = CreateWindowW(L"STATIC", L"=== WATER/METHANOL INJECTION ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        leftMargin, currentY, GetResultsWidth(), 20,
        hwndPage, nullptr, hInst, nullptr);
    SendMessage(lblWaterMeth, WM_SETFONT, (WPARAM)hFont, TRUE);
    currentY += 30;
    
    // Proporção Água/Metanol
    CreateLabel(L"Proporção Água/Metanol (%):", leftMargin, currentY, labelWidth);
    editWaterMethRatio.Create(hwndPage, IDC_EDIT_WATER_METH_RATIO, 
                               leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                               0.0, 100.0, 1);
    CreateTooltip(editWaterMethRatio.GetHandle(),
        L"O QUE É: % de metanol na mistura (resto é água).\n"
        L"MISTURAS COMUNS:\n"
        L"• 50/50 - Balanceada (padrão)\n"
        L"• 30/70 - Mais refrigeração\n"
        L"• 100/0 - Metanol puro (máxima octanagem)\n"
        L"NOTA: Água resfria, metanol aumenta octanagem!");
    currentY += rowHeight;
    
    // Boost de Ativação
    CreateLabel(L"Ativação em Boost (PSI):", leftMargin, currentY, labelWidth);
    editWaterMethBoost.Create(hwndPage, IDC_EDIT_WATER_METH_BOOST, 
                               leftMargin + labelWidth, currentY - 3, editWidth, 24, 
                               0.0, 30.0, 1);
    CreateTooltip(editWaterMethBoost.GetHandle(),
        L"O QUE É: PSI de boost para ativar injeção.\n"
        L"VALORES TÍPICOS:\n"
        L"• 5-7 PSI - Ativação precoce (mais seguro)\n"
        L"• 10-15 PSI - Ativação tardia\n"
        L"BENEFÍCIO: Permite mais timing e boost sem detonação!");
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
    wc.lpszClassName = L"EngineCalcGraphAlt";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    hwndGraph = CreateWindowExW(WS_EX_CLIENTEDGE, L"EngineCalcGraphAlt", L"",
        WS_CHILD | WS_VISIBLE, leftMargin, currentY, GetResultsWidth(), 220,
        hwndPage, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndGraph, GWLP_USERDATA, (LONG_PTR)this);
}

void AlternativeFuelsTab::Create() {
    RegisterTabPageClass(hInst);
    hwndPage = CreateWindowExW(0, L"EngineCalcTabPage", L"",
        WS_CHILD | WS_VSCROLL, 0, 30, 800, 450,
        hwndParent, nullptr, hInst, nullptr);
    SetWindowLongPtr(hwndPage, GWLP_USERDATA, (LONG_PTR)this);
    CreateControls();
    SetContentHeight(1150);
}

void AlternativeFuelsTab::OnCalculate() {
    EngineDataManager* dataMgr = EngineDataManager::GetInstance();
    const EngineProject& project = dataMgr->GetProject();
    
    int nitrousType = (int)SendMessage(comboNitrousSystem, CB_GETCURSEL, 0, 0);
    double nitrousHP = editNitrousHP.GetValue();
    double bottlePressure = editBottlePressure.GetValue();
    double nitromethanePercent = editNitromethanePercent.GetValue();
    double methanolPercent = editMethanolPercent.GetValue();
    double waterMethRatio = editWaterMethRatio.GetValue();
    double waterMethBoost = editWaterMethBoost.GetValue();
    
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << L"---------------------------------------------------------------------------\r\n";
    oss << L"      ANALISE DE COMBUSTIVEIS ALTERNATIVOS\r\n";
    oss << L"---------------------------------------------------------------------------\r\n\r\n";
    
    // Analise de Oxido Nitroso
    if (nitrousHP > 0) {
        oss << L"OXIDO NITROSO (N2O):\r\n";
        oss << L"  Sistema: ";
        if (nitrousType == 0) oss << L"Dry Shot\r\n";
        else if (nitrousType == 1) oss << L"Wet Shot\r\n";
        else oss << L"Direct Port\r\n";
        
        oss << L"  Ganho de HP: +" << nitrousHP << L" HP\r\n";
        oss << L"  Pressao do cilindro: " << bottlePressure << L" PSI\r\n\r\n";
        
        // Estimativa de jet sizes (formula simplificada)
        double nitrousJet = nitrousHP * 0.0008; // inches
        double fuelJet = nitrousJet * 0.7; // 70% do nitro
        
        oss << L"  JET SIZES ESTIMADOS:\r\n";
        oss << L"    -> Jet de N2O: ~" << (nitrousJet * 1000) << L" mil (" << nitrousJet << L"\")\r\n";
        if (nitrousType > 0) {
            oss << L"    -> Jet de Combustivel: ~" << (fuelJet * 1000) << L" mil (" << fuelJet << L"\")\r\n";
        }
        
        // CR maxima segura
        double maxCR = 11.0 - (nitrousHP / 100.0);
        if (maxCR < 8.5) maxCR = 8.5;
        oss << L"\r\n  CR MAXIMA SEGURA: ~" << maxCR << L":1\r\n";
        oss << L"    (Com shot de " << nitrousHP << L" HP)\r\n";
        
        // Reducao de temperatura
        double iatDrop = nitrousHP * 0.3; // F estimado
        oss << L"\r\n  Reducao de IAT: ~" << iatDrop << L"F (~" << (iatDrop * 0.556) << L"C)\r\n";
        
        oss << L"\r\n  AVISOS CRITICOS:\r\n";
        oss << L"    ! NUNCA use sem enriquecimento (Dry Shot PERIGOSO!)\r\n";
        oss << L"    ! Shots >100 HP requerem pistoes FORJADOS\r\n";
        oss << L"    ! Shots >200 HP requerem motor completo RACE\r\n";
        oss << L"    ! Verifique AFR com wideband O2 SEMPRE\r\n";
        oss << L"    ! Use velas FRIAS (1-2 graus)\r\n\r\n";
    }
    
    // Analise de Nitrometano
    if (nitromethanePercent > 0) {
        oss << L"NITROMETANO (CH3NO2):\r\n";
        oss << L"  Percentual: " << nitromethanePercent << L"%\r\n";
        
        // AFR estequiometrico muda com % nitro
        double stoichAFR = 14.7 - (nitromethanePercent / 100.0 * 13.0);
        oss << L"  AFR Estequiometrico: " << stoichAFR << L":1\r\n";
        
        // CR maxima suportada
        double maxCR = 10.0 + (nitromethanePercent / 100.0 * 7.0);
        oss << L"  CR Maxima Suportada: ate " << maxCR << L":1\r\n";
        
        // Ganho estimado de potencia
        double hpGain = nitromethanePercent * 0.3; // % aproximado
        oss << L"  Ganho de Potencia: ~+" << hpGain << L"%\r\n\r\n";
        
        oss << L"  ESPECIFICACOES TECNICAS:\r\n";
        oss << L"    -> RON Equivalente: ~130 (100% nitro)\r\n";
        oss << L"    -> Velocidade de chama: 2x gasolina\r\n";
        oss << L"    -> Consumo: 2-3x maior que gasolina!\r\n";
        oss << L"    -> Aplicacao: Top Fuel, Funny Car, Drag Racing\r\n\r\n";
        
        if (nitromethanePercent > 50) {
            oss << L"    ! >50% REQUER: Ignicao especial (magneto)\r\n";
            oss << L"    ! >50% REQUER: Sistema de combustivel dedicado\r\n";
            oss << L"    ! >50% REQUER: Motor especialmente preparado\r\n\r\n";
        }
    }
    
    // Analise de Metanol
    if (methanolPercent > 0) {
        oss << L"METANOL (CH3OH):\r\n";
        oss << L"  Percentual: " << methanolPercent << L"%\r\n";
        
        double stoichAFR = 14.7 - (methanolPercent / 100.0 * 8.3);
        oss << L"  AFR Estequiometrico: " << stoichAFR << L":1\r\n";
        oss << L"  AFR para Maxima Potencia: ~" << (stoichAFR * 0.85) << L":1\r\n\r\n";
        
        oss << L"  VANTAGENS:\r\n";
        oss << L"    + RON ~110 (vs 91-98 gasolina)\r\n";
        oss << L"    + Forte efeito refrigerante (latent heat)\r\n";
        oss << L"    + Permite CR e timing agressivos\r\n";
        oss << L"    + Usado na IndyCar e Formula E\r\n\r\n";
        
        oss << L"  DESVANTAGENS:\r\n";
        oss << L"    - Consome 2x mais que gasolina\r\n";
        oss << L"    - Corrosivo (ataca aluminio/magnesio)\r\n";
        oss << L"    - Dificil partida a frio\r\n";
        oss << L"    - Incolor (chama invisivel - PERIGO!)\r\n\r\n";
    }
    
    // Analise de Water/Meth Injection
    if (waterMethBoost > 0) {
        oss << L"WATER/METHANOL INJECTION:\r\n";
        oss << L"  Mistura: " << waterMethRatio << L"% Metanol / " 
            << (100 - waterMethRatio) << L"% Agua\r\n";
        oss << L"  Ativacao em: " << waterMethBoost << L" PSI boost\r\n\r\n";
        
        // Reducao de IAT
        double iatDrop = 40.0 + (waterMethRatio * 0.3); // F
        oss << L"  Reducao de IAT: ~" << iatDrop << L"F (~" << (iatDrop * 0.556) << L"C)\r\n";
        
        // Ganho de octanagem
        double octaneGain = 3.0 + (waterMethRatio * 0.05);
        oss << L"  Ganho de Octanagem: ~+" << octaneGain << L" RON\r\n";
        
        // Permite mais boost/timing
        oss << L"\r\n  BENEFICIOS:\r\n";
        oss << L"    + Permite +3-5 PSI de boost extra\r\n";
        oss << L"    + Permite +5-8 graus de timing\r\n";
        oss << L"    + Reduz knock/detonacao\r\n";
        oss << L"    + Ganho tipico: +20-50 HP\r\n\r\n";
        
        oss << L"  ATENCAO:\r\n";
        oss << L"    ! NUNCA confie so em W/M para evitar knock!\r\n";
        oss << L"    ! Sempre tenha AFR/timing seguro SEM injecao\r\n";
        oss << L"    ! Use kit quality (Snow, AEM, Aquamist)\r\n";
        oss << L"    ! Monitor level! (tanque vazio = BOOM)\r\n\r\n";
    }
    
    if (nitrousHP == 0 && nitromethanePercent == 0 && methanolPercent == 0 && waterMethBoost == 0) {
        oss << L"Configure os parametros acima e clique em CALCULAR.\r\n";
    }
    
    SetWindowTextW(textResults, oss.str().c_str());

    UpdateGraph(nitrousHP);

    // Salva dados
    EngineProject::AlternativeFuelsData altFuelsData;
    altFuelsData.nitrousSystemType = nitrousType;
    altFuelsData.nitrousHP = nitrousHP;
    altFuelsData.nitrousBottlePressure = bottlePressure;
    altFuelsData.nitrometanePercent = nitromethanePercent;
    altFuelsData.methanolPercent = methanolPercent;
    altFuelsData.waterMethRatio = waterMethRatio;
    altFuelsData.waterMethStartBoost = waterMethBoost;
    dataMgr->UpdateAlternativeFuelsData(altFuelsData);
}

void AlternativeFuelsTab::OnClear() {
    editNitrousHP.SetValue(50.0);
    editBottlePressure.SetValue(900.0);
    editNitromethanePercent.SetValue(0.0);
    editMethanolPercent.SetValue(0.0);
    editWaterMethRatio.SetValue(50.0);
    editWaterMethBoost.SetValue(5.0);
    SendMessage(comboNitrousSystem, CB_SETCURSEL, 1, 0);
    SetWindowTextW(textResults, L"");
}

void AlternativeFuelsTab::RecalculateLayout() {
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

LRESULT CALLBACK AlternativeFuelsTab::GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        AlternativeFuelsTab* tab = (AlternativeFuelsTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

void AlternativeFuelsTab::UpdateGraph(double nitrousHP) {
    graphRenderer.ClearSeries();
    if (nitrousHP <= 0) { if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE); return; }

    GraphConfig& cfg = graphRenderer.GetConfig();
    cfg.title = L"Ganho NOS vs Jet Size";
    cfg.xAxisLabel = L"Jet Size";
    cfg.yAxisLabel = L"HP Ganho";
    cfg.xMin = 20; cfg.xMax = 100;
    float maxHP = (float)(nitrousHP * 2.5);
    cfg.yMin = 0; cfg.yMax = maxHP;
    cfg.xGridStep = 10;
    cfg.yGridStep = maxHP > 200 ? 50.0f : 25.0f;

    // Curva HP vs Jet (aproximacao quadratica)
    std::vector<float> xJet, yHP;
    for (float jet = 20; jet <= 100; jet += 2) {
        float hp = (jet * jet) / 100.0f * (float)(nitrousHP / 50.0); // escala relativa
        if (hp > maxHP) hp = maxHP;
        xJet.push_back(jet); yHP.push_back(hp);
    }
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xJet, yHP,
        Gdiplus::Color(255, 0, 200, 255), L"HP vs Jet", 2.5f));

    // Marcador HP alvo
    std::vector<float> xM, yM;
    xM.push_back(20); yM.push_back((float)nitrousHP);
    xM.push_back(100); yM.push_back((float)nitrousHP);
    graphRenderer.AddSeries(GraphRenderer::CreateSeries(xM, yM,
        Gdiplus::Color(255, 255, 80, 80), L"HP Alvo", 1.5f));

    if (hwndGraph) InvalidateRect(hwndGraph, NULL, TRUE);
}
