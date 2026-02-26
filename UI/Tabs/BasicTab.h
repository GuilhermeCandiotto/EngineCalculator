#pragma once
#include "../TabPages.h"
#include "../GraphRenderer.h"

// ============================================================================
// BasicTab - Aba de Cálculos Básicos
// ============================================================================
class BasicTab : public TabPage {
private:
    NumericEdit editBore, editStroke, editCylinders;
    NumericEdit editRodLength, editRPM, editTorque;
    HWND comboEngineType;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 150);
    void UpdateGraph();

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    BasicTab(HWND parent, HINSTANCE instance);
    ~BasicTab();

    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;
};
