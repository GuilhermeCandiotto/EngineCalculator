#pragma once
#include "../TabPages.h"
#include "../GraphRenderer.h"

// ============================================================================
// TurboTab - Turbocharger & Supercharger Calculator Tab
// ============================================================================
class TurboTab : public TabPage {
private:
    // Inputs - System type
    HWND comboFIType;           // Forced induction type (None/Turbo/Supercharger)
    HWND comboTurboConfig;      // Single/Twin/Sequential/Compound/TwinScroll
    HWND comboSCType;           // Roots/Twin-Screw/Centrifugal

    // Labels that need show/hide
    HWND lblTurboConfig;
    HWND lblSCType;
    HWND lblDriveRatio;

    // Inputs - Main parameters
    NumericEdit editTargetHP;
    NumericEdit editTargetBoost;
    NumericEdit editMaxRPM;

    // Inputs - System losses
    NumericEdit editICPressDrop;
    NumericEdit editPipingDrop;
    NumericEdit editFilterDrop;
    NumericEdit editICEfficiency;

    // Inputs - Supercharger
    NumericEdit editDriveRatio;

    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 250);
    HWND CreateLabelEx(const wchar_t* text, int x, int y, int width = 250);
    void UpdateGraph(double correctedFlow, double pressureRatio);
    void UpdateVisibility();

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    TurboTab(HWND parent, HINSTANCE instance);
    ~TurboTab();

    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    void RecalculateLayout() override;
};
