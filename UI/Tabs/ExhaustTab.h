#pragma once
#include "../TabPages.h"
#include "../GraphRenderer.h"

// ============================================================================
// ExhaustTab - Aba de Sistema de Escape
// ============================================================================
class ExhaustTab : public TabPage {
private:
    NumericEdit editPrimaryLength, editPrimaryDiameter;
    NumericEdit editSecondaryLength, editSecondaryDiameter;
    NumericEdit editCollectorDiameter, editTargetRPM, editExhaustDuration;
    HWND comboExhaustType;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 220);
    void CreateGroupBox(const wchar_t* text, int x, int y, int width, int height);
    void UpdateGraph(double targetRPM, double exhaustDuration);

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
public:
    ExhaustTab(HWND parent, HINSTANCE instance);
    ~ExhaustTab();
    
    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;
};
