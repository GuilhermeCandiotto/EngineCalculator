#pragma once
// ============================================================================
// TabPages.h - Definição da Classe Base de Abas e NumericEdit
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// REFATORADO: Implementações das tabs agora estão em UI/Tabs/*.cpp
// Este arquivo contém APENAS a classe base TabPage e NumericEdit
// ============================================================================
#include <windows.h>
#include <string>
#include <vector>

// ============================================================================
// FORWARD DECLARATIONS DAS TABS (implementacoes em arquivos separados)
// ============================================================================
class BasicTab;
class CompressionTab;
class ValveTrainTab;
class CamshaftTab;
class IntakeTab;
class ExhaustTab;

// Função para registrar a classe de janela customizada das tabs
bool RegisterTabPageClass(HINSTANCE hInstance);

// IDs de controles
#define ID_TAB_CONTROL          1001
#define ID_BTN_CALCULATE        1002
#define ID_BTN_CLEAR            1003
#define ID_BTN_EXPORT           1004

// IDs das abas
#define TAB_BASIC               0
#define TAB_COMPRESSION         1
#define TAB_VALVETRAIN          2
#define TAB_CAMSHAFT            3
#define TAB_INTAKE              4
#define TAB_EXHAUST             5
#define TAB_FUEL                6
#define TAB_ALTERNATIVE_FUELS   7

// IDs de controles - Aba Básica
#define IDC_EDIT_BORE           2001
#define IDC_EDIT_STROKE         2002
#define IDC_EDIT_CYLINDERS      2003
#define IDC_COMBO_ENGINE_TYPE   2004
#define IDC_EDIT_ROD_LENGTH     2005
#define IDC_EDIT_RPM            2006
#define IDC_EDIT_TORQUE         2007

// IDs de controles - Aba Compressão
#define IDC_EDIT_BORE_COMP      3000
#define IDC_EDIT_STROKE_COMP    3001
#define IDC_EDIT_CYLINDERS_COMP 3002
#define IDC_EDIT_CHAMBER_VOL    3003
#define IDC_EDIT_PISTON_DOME    3004
#define IDC_EDIT_GASKET_THICK   3005
#define IDC_EDIT_DECK_HEIGHT    3006
#define IDC_EDIT_VALVE_RELIEF   3007
#define IDC_EDIT_IVC_ANGLE      3008

// IDs de controles - Aba Trem de Válvulas
#define IDC_EDIT_INTAKE_VALVE_DIA       3101
#define IDC_EDIT_EXHAUST_VALVE_DIA      3102
#define IDC_EDIT_NUM_INTAKE_VALVES      3103
#define IDC_EDIT_NUM_EXHAUST_VALVES     3104
#define IDC_EDIT_SEAT_ANGLE             3105
#define IDC_EDIT_MAX_LIFT               3106
#define IDC_EDIT_SPRING_RATE            3107
#define IDC_EDIT_SPRING_PRELOAD         3108
#define IDC_EDIT_SPRING_INSTALLED_H     3109
#define IDC_EDIT_SPRING_COILBIND_H      3110
#define IDC_EDIT_RETAINER_WEIGHT        3111
#define IDC_EDIT_VALVE_WEIGHT           3112
#define IDC_EDIT_ROCKER_RATIO           3113
#define IDC_EDIT_TEST_RPM               3114

// IDs de controles - Aba Comando de Válvulas
#define IDC_EDIT_INTAKE_DURATION        3201
#define IDC_EDIT_EXHAUST_DURATION       3202
#define IDC_EDIT_INTAKE_LIFT_CAM        3203
#define IDC_EDIT_EXHAUST_LIFT_CAM       3204
#define IDC_EDIT_LSA                    3205
#define IDC_EDIT_ADVANCE_RETARD         3206
#define IDC_EDIT_ROCKER_RATIO_CAM       3207
#define IDC_EDIT_CAM_RPM                3208

// IDs de controles - Aba Admissão
#define IDC_EDIT_RUNNER_LENGTH          3301
#define IDC_EDIT_RUNNER_DIAMETER        3302
#define IDC_EDIT_PLENUM_VOLUME          3303
#define IDC_EDIT_TRUMPET_LENGTH         3304
#define IDC_EDIT_TRUMPET_DIAMETER       3305
#define IDC_EDIT_TARGET_RPM_INTAKE      3306
#define IDC_EDIT_BOOST_PRESSURE         3307

// IDs de controles - Aba Escape
#define IDC_EDIT_PRIMARY_LENGTH         3401
#define IDC_EDIT_PRIMARY_DIAMETER       3402
#define IDC_EDIT_SECONDARY_LENGTH       3403
#define IDC_EDIT_SECONDARY_DIAMETER     3404
#define IDC_EDIT_COLLECTOR_DIAMETER     3405
#define IDC_EDIT_TARGET_RPM_EXHAUST     3406
#define IDC_EDIT_EXHAUST_DUR_CALC       3407
#define IDC_COMBO_EXHAUST_TYPE          3408

// IDs de controles - Aba Combustível
#define IDC_COMBO_FUEL_SYSTEM_TYPE      3501
#define IDC_EDIT_TARGET_HP_FUEL         3502
#define IDC_EDIT_BOOST_PRESSURE_FUEL    3503
#define IDC_EDIT_FUEL_PRESSURE          3504
#define IDC_EDIT_TARGET_AFR             3505
#define IDC_EDIT_NUM_INJECTORS          3506

// IDs de controles - Aba Combustíveis Alternativos
#define IDC_COMBO_NITROUS_SYSTEM        3601
#define IDC_EDIT_NITROUS_HP             3602
#define IDC_EDIT_BOTTLE_PRESSURE        3603
#define IDC_EDIT_NITROMETHANE_PERCENT   3604
#define IDC_EDIT_METHANOL_PERCENT       3605
#define IDC_EDIT_WATER_METH_RATIO       3606
#define IDC_EDIT_WATER_METH_BOOST       3607

// IDs de resultados
#define IDC_RESULT_TEXT         4001

// Classe para gerenciar controles de entrada numérica
class NumericEdit {
private:
    HWND hwnd;
    double minValue;
    double maxValue;
    int decimalPlaces;
    
public:
    NumericEdit();
    void Create(HWND parent, int id, int x, int y, int width, int height,
                double min = 0.0, double max = 10000.0, int decimals = 2);
    void SetValue(double value);
    double GetValue() const;
    bool IsValid() const;
    HWND GetHandle() const { return hwnd; }
};

// Classe base para abas
class TabPage {
protected:
    HWND hwndParent;
    HWND hwndPage;
    HINSTANCE hInst;
    bool visible;
    int scrollPos;          // Posição atual do scroll
    int contentHeight;      // Altura total do conteúdo
    HWND hwndTooltip;       // Controle de tooltip
    
    // Helper para criar tooltips
    void CreateTooltip(HWND hwndControl, const wchar_t* text);
    
    // ========== LAYOUT DINÂMICO - NOVOS MÉTODOS ==========
    // Calcula dimensões baseadas no tamanho da janela
    int GetPageWidth() const;
    int GetPageHeight() const;
    int GetLabelWidth(float percentage = 0.30f) const;    // 30% da largura
    int GetEditWidth(float percentage = 0.25f) const;     // 25% da largura
    int GetResultsWidth(float percentage = 0.95f) const;  // 95% da largura
    int GetMargin() const { return 20; }                  // Margem fixa
    
public:
    TabPage(HWND parent, HINSTANCE instance);
    virtual ~TabPage();
    
    virtual void Create() = 0;
    virtual void Show(bool show);
    virtual void OnCalculate() = 0;
    virtual void OnClear() = 0;
    virtual void OnCommand(WPARAM wParam, LPARAM lParam) {}  // Override for combo/button events
    
    HWND GetHandle() const { return hwndPage; }
    bool IsVisible() const { return visible; }
    
    // Scroll support
    void UpdateScrollBar();
    void HandleScroll(WPARAM wParam);
    void SetContentHeight(int height) { contentHeight = height; UpdateScrollBar(); }
    
    // ========== REDIMENSIONAMENTO EM TEMPO REAL ==========
    virtual void RecalculateLayout();  // Recalcula e reposiciona controles

    // Fontes padrao da aplicacao
    static HFONT GetAppLabelFont();
    static HFONT GetAppResultsFont();

    // Tema global (acessivel por TabPageProc)
    static bool IsDarkTheme();
    static void SetDarkTheme(bool dark);
    static COLORREF GetThemeBgColor();
    static COLORREF GetThemeTextColor();
    static HBRUSH GetThemeBgBrush();

    // Sistema de unidades
    static bool IsImperial();
    static void SetImperial(bool imperial);
};
