#pragma once
// ============================================================================
// FuelTab.h - Aba de Sistema de Combustível
// Engine Calculator v1.0.0 - Março 2026
// ============================================================================
#include "../TabPages.h"
#include "../GraphRenderer.h"

class FuelTab : public TabPage {
private:
    HWND comboSystemType;
    NumericEdit editTargetHP, editBoostPressure, editFuelPressure;
    NumericEdit editTargetAFR, editNumInjectors;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 200);
    void UpdateGraph(double targetHP);

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
public:
    FuelTab(HWND parent, HINSTANCE instance);
    ~FuelTab();
    
    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;
};
