#pragma once
#include "../TabPages.h"
#include "../GraphRenderer.h"

// ============================================================================
// CamshaftTab - Aba de Comando de Válvulas e LSA
// ============================================================================
class CamshaftTab : public TabPage {
private:
    NumericEdit editIntakeDuration, editExhaustDuration;
    NumericEdit editIntakeLift, editExhaustLift;
    NumericEdit editLSA, editAdvanceRetard, editRockerRatio, editCamRPM;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 220);
    void CreateGroupBox(const wchar_t* text, int x, int y, int width, int height);
    void UpdateGraph();

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    CamshaftTab(HWND parent, HINSTANCE instance);
    ~CamshaftTab();

    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;
};
