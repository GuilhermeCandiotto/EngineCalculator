#pragma once
#include "../TabPages.h"
#include "../GraphRenderer.h"

// ============================================================================
// IntakeTab - Aba de Sistema de Admissão
// ============================================================================
class IntakeTab : public TabPage {
private:
    NumericEdit editRunnerLength, editRunnerDiameter;
    NumericEdit editPlenumVolume, editTrumpetLength, editTrumpetDiameter;
    NumericEdit editTargetRPM, editBoostPressure;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 220);
    void CreateGroupBox(const wchar_t* text, int x, int y, int width, int height);
    void UpdateGraph(double tuningRPM, double targetRPM);

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
public:
    IntakeTab(HWND parent, HINSTANCE instance);
    ~IntakeTab();
    
    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;
};
