#pragma once
#include "../TabPages.h"
#include "../GraphRenderer.h"

// ============================================================================
// ValveTrainTab - Aba de Trem de Válvulas e Molas
// ============================================================================
class ValveTrainTab : public TabPage {
private:
    NumericEdit editIntakeValveDia, editExhaustValveDia;
    NumericEdit editNumIntakeValves, editNumExhaustValves, editSeatAngle;
    NumericEdit editMaxLift, editSpringRate, editSpringPreload;
    NumericEdit editSpringInstalledHeight, editSpringCoilBindHeight;
    NumericEdit editRetainerWeight, editValveWeight, editRockerRatio, editTestRPM;
    HWND textResults;
    HWND hwndGraph;

    GraphRenderer graphRenderer;

    void CreateControls();
    void CreateLabel(const wchar_t* text, int x, int y, int width = 220);
    void CreateGroupBox(const wchar_t* text, int x, int y, int width, int height);
    void UpdateGraph(double floatRPM, double testRPM);

    static LRESULT CALLBACK GraphWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
public:
    ValveTrainTab(HWND parent, HINSTANCE instance);
    ~ValveTrainTab();
    
    void Create() override;
    void OnCalculate() override;
    void OnClear() override;
    void RecalculateLayout() override;
    
    void SetEngineData(double bore, double stroke, int cylinders);
};
