#pragma once
// ============================================================================
// AlternativeFuelsTab.h - Aba de Combustíveis Alternativos
// Engine Calculator v1.0.0 - Março 2026
// ============================================================================
#include "../TabPages.h"
#include "../GraphRenderer.h"

class AlternativeFuelsTab : public TabPage {
private:
    HWND comboNitrousSystem;
    NumericEdit editNitrousHP, editBottlePressure;
    NumericEdit editNitromethanePercent, editMethanolPercent;
    NumericEdit editWaterMethRatio, editWaterMethBoost;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 200);
    void UpdateGraph(double nitrousHP);

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
public:
    AlternativeFuelsTab(HWND parent, HINSTANCE instance);
    ~AlternativeFuelsTab();
    
    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;
};
