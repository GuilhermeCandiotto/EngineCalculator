#pragma once
// ============================================================================
// GraphRenderer.h - Componente de Graficos GDI+
// Engine Calculator - Marco 2026
//
// Renderiza graficos 2D reutilizaveis (curvas, eixos, grid, legendas)
// ============================================================================
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <string>

#pragma comment(lib, "gdiplus.lib")

struct GraphSeries {
    std::vector<Gdiplus::PointF> points;
    Gdiplus::Color color;
    std::wstring label;
    float lineWidth;
    
    GraphSeries() : color(255, 0, 120, 215), lineWidth(2.0f) {}
};

struct GraphConfig {
    std::wstring title;
    std::wstring xAxisLabel;
    std::wstring yAxisLabel;
    
    float xMin, xMax;
    float yMin, yMax;
    float xGridStep;
    float yGridStep;
    
    bool showGrid;
    bool showLegend;
    bool darkTheme;
    
    GraphConfig()
        : xMin(0), xMax(360), yMin(0), yMax(15),
          xGridStep(90), yGridStep(5),
          showGrid(true), showLegend(true), darkTheme(false) {}
};

class GraphRenderer {
private:
    RECT area;
    GraphConfig config;
    std::vector<GraphSeries> series;
    
    // Margens internas (px)
    static constexpr int MARGIN_LEFT   = 55;
    static constexpr int MARGIN_RIGHT  = 15;
    static constexpr int MARGIN_TOP    = 30;
    static constexpr int MARGIN_BOTTOM = 35;
    
    // Converte coordenadas de dados para pixels
    float DataToPixelX(float dataX) const;
    float DataToPixelY(float dataY) const;
    
    void DrawBackground(Gdiplus::Graphics& g) const;
    void DrawGrid(Gdiplus::Graphics& g) const;
    void DrawAxes(Gdiplus::Graphics& g) const;
    void DrawSeries(Gdiplus::Graphics& g) const;
    void DrawTitle(Gdiplus::Graphics& g) const;
    void DrawLegend(Gdiplus::Graphics& g) const;
    
public:
    GraphRenderer();
    ~GraphRenderer();
    
    void SetArea(const RECT& rect) { area = rect; }
    void SetConfig(const GraphConfig& cfg) { config = cfg; }
    GraphConfig& GetConfig() { return config; }
    
    void ClearSeries() { series.clear(); }
    void AddSeries(const GraphSeries& s) { series.push_back(s); }
    
    void Render(HDC hdc) const;
    
    // Helper: cria serie a partir de pontos X,Y separados
    static GraphSeries CreateSeries(
        const std::vector<float>& xData,
        const std::vector<float>& yData,
        Gdiplus::Color color,
        const std::wstring& label,
        float lineWidth = 2.0f);
};
