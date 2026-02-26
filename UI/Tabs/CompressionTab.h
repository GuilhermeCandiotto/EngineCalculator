#pragma once
#include "../TabPages.h"
#include "../GraphRenderer.h"

// ============================================================================
// CompressionTab - Aba de Análise de Compressão
// ============================================================================
class CompressionTab : public TabPage {
private:
    NumericEdit editBore, editStroke, editCylinders;
    NumericEdit editChamberVol, editPistonDome, editGasketThick;
    NumericEdit editDeckHeight, editValveRelief, editIVCAngle;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 200);
    void UpdateGraph(double currentCR);

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    CompressionTab(HWND parent, HINSTANCE instance);
    ~CompressionTab();

    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;

    void SetBoreStroke(double bore, double stroke, int cylinders, double rodLength);
};
