// ============================================================================
// GraphRenderer.cpp - Implementacao do Componente de Graficos GDI+
// Engine Calculator - Marco 2026
// ============================================================================
#include "GraphRenderer.h"
#include "TabPages.h"
#include <sstream>
#include <iomanip>
#include <cmath>

GraphRenderer::GraphRenderer() {
    area = { 0, 0, 400, 250 };
}

GraphRenderer::~GraphRenderer() {
}

float GraphRenderer::DataToPixelX(float dataX) const {
    float plotLeft = (float)(area.left + MARGIN_LEFT);
    float plotWidth = (float)(area.right - area.left - MARGIN_LEFT - MARGIN_RIGHT);
    float range = config.xMax - config.xMin;
    if (range <= 0) return plotLeft;
    return plotLeft + ((dataX - config.xMin) / range) * plotWidth;
}

float GraphRenderer::DataToPixelY(float dataY) const {
    float plotTop = (float)(area.top + MARGIN_TOP);
    float plotHeight = (float)(area.bottom - area.top - MARGIN_TOP - MARGIN_BOTTOM);
    float range = config.yMax - config.yMin;
    if (range <= 0) return plotTop;
    return plotTop + plotHeight - ((dataY - config.yMin) / range) * plotHeight;
}

void GraphRenderer::DrawBackground(Gdiplus::Graphics& g) const {
    Gdiplus::Color bgColor = config.darkTheme 
        ? Gdiplus::Color(255, 30, 30, 33) 
        : Gdiplus::Color(255, 255, 255, 255);
    Gdiplus::SolidBrush bgBrush(bgColor);
    g.FillRectangle(&bgBrush, (INT)area.left, (INT)area.top, 
                    (INT)(area.right - area.left), (INT)(area.bottom - area.top));

    // Borda
    Gdiplus::Color borderColor = config.darkTheme
        ? Gdiplus::Color(255, 80, 80, 80)
        : Gdiplus::Color(255, 180, 180, 180);
    Gdiplus::Pen borderPen(borderColor, 1.0f);
    g.DrawRectangle(&borderPen, (INT)area.left, (INT)area.top,
                    (INT)(area.right - area.left - 1), (INT)(area.bottom - area.top - 1));
}

void GraphRenderer::DrawGrid(Gdiplus::Graphics& g) const {
    if (!config.showGrid) return;
    
    Gdiplus::Color gridColor = config.darkTheme
        ? Gdiplus::Color(40, 255, 255, 255)
        : Gdiplus::Color(40, 0, 0, 0);
    Gdiplus::Pen gridPen(gridColor, 1.0f);
    gridPen.SetDashStyle(Gdiplus::DashStyleDot);
    
    float plotLeft = DataToPixelX(config.xMin);
    float plotRight = DataToPixelX(config.xMax);
    float plotTop = DataToPixelY(config.yMax);
    float plotBottom = DataToPixelY(config.yMin);
    
    // Grid vertical
    if (config.xGridStep > 0) {
        for (float x = config.xMin + config.xGridStep; x < config.xMax; x += config.xGridStep) {
            float px = DataToPixelX(x);
            g.DrawLine(&gridPen, px, plotTop, px, plotBottom);
        }
    }
    
    // Grid horizontal
    if (config.yGridStep > 0) {
        for (float y = config.yMin + config.yGridStep; y < config.yMax; y += config.yGridStep) {
            float py = DataToPixelY(y);
            g.DrawLine(&gridPen, plotLeft, py, plotRight, py);
        }
    }
}

void GraphRenderer::DrawAxes(Gdiplus::Graphics& g) const {
    Gdiplus::Color axisColor = config.darkTheme
        ? Gdiplus::Color(255, 200, 200, 200)
        : Gdiplus::Color(255, 60, 60, 60);
    Gdiplus::Pen axisPen(axisColor, 1.5f);
    Gdiplus::SolidBrush textBrush(axisColor);
    
    Gdiplus::Font labelFont(L"Segoe UI", 8.0f);
    Gdiplus::Font axisFont(L"Segoe UI", 8.5f, Gdiplus::FontStyleBold);
    Gdiplus::StringFormat sfCenter;
    sfCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
    Gdiplus::StringFormat sfRight;
    sfRight.SetAlignment(Gdiplus::StringAlignmentFar);
    
    float plotLeft = DataToPixelX(config.xMin);
    float plotRight = DataToPixelX(config.xMax);
    float plotTop = DataToPixelY(config.yMax);
    float plotBottom = DataToPixelY(config.yMin);
    
    // Eixo X
    g.DrawLine(&axisPen, plotLeft, plotBottom, plotRight, plotBottom);
    // Eixo Y
    g.DrawLine(&axisPen, plotLeft, plotTop, plotLeft, plotBottom);
    
    // Tick labels X
    if (config.xGridStep > 0) {
        for (float x = config.xMin; x <= config.xMax + 0.001f; x += config.xGridStep) {
            float px = DataToPixelX(x);
            g.DrawLine(&axisPen, px, plotBottom, px, plotBottom + 4);
            
            wchar_t buf[16];
            swprintf(buf, 16, L"%.0f", x);
            Gdiplus::RectF labelRect(px - 25, plotBottom + 5, 50, 15);
            g.DrawString(buf, -1, &labelFont, labelRect, &sfCenter, &textBrush);
        }
    }
    
    // Tick labels Y
    if (config.yGridStep > 0) {
        for (float y = config.yMin; y <= config.yMax + 0.001f; y += config.yGridStep) {
            float py = DataToPixelY(y);
            g.DrawLine(&axisPen, plotLeft - 4, py, plotLeft, py);
            
            wchar_t buf[16];
            if (config.yGridStep >= 1.0f)
                swprintf(buf, 16, L"%.0f", y);
            else
                swprintf(buf, 16, L"%.1f", y);
            Gdiplus::RectF labelRect((float)area.left, py - 8, (float)MARGIN_LEFT - 8, 16);
            g.DrawString(buf, -1, &labelFont, labelRect, &sfRight, &textBrush);
        }
    }
    
    // Label eixo X
    if (!config.xAxisLabel.empty()) {
        float cx = (plotLeft + plotRight) / 2.0f;
        Gdiplus::RectF xLabelRect(cx - 80, plotBottom + 18, 160, 16);
        g.DrawString(config.xAxisLabel.c_str(), -1, &axisFont, xLabelRect, &sfCenter, &textBrush);
    }
    
    // Label eixo Y (vertical)
    if (!config.yAxisLabel.empty()) {
        Gdiplus::Matrix oldTransform;
        g.GetTransform(&oldTransform);
        
        float cy = (plotTop + plotBottom) / 2.0f;
        g.TranslateTransform((float)area.left + 12, cy);
        g.RotateTransform(-90.0f);
        
        Gdiplus::RectF yLabelRect(-60, -8, 120, 16);
        g.DrawString(config.yAxisLabel.c_str(), -1, &axisFont, yLabelRect, &sfCenter, &textBrush);
        
        g.SetTransform(&oldTransform);
    }
}

void GraphRenderer::DrawSeries(Gdiplus::Graphics& g) const {
    for (const auto& s : series) {
        if (s.points.size() < 2) continue;
        
        // Clip ao area do plot
        float plotLeft = DataToPixelX(config.xMin);
        float plotTop = DataToPixelY(config.yMax);
        float plotWidth = DataToPixelX(config.xMax) - plotLeft;
        float plotHeight = DataToPixelY(config.yMin) - plotTop;
        g.SetClip(Gdiplus::RectF(plotLeft, plotTop, plotWidth, plotHeight));
        
        // Converte pontos de dados para pixels
        std::vector<Gdiplus::PointF> pixelPoints;
        pixelPoints.reserve(s.points.size());
        for (const auto& pt : s.points) {
            pixelPoints.push_back(Gdiplus::PointF(DataToPixelX(pt.X), DataToPixelY(pt.Y)));
        }
        
        Gdiplus::Pen pen(s.color, s.lineWidth);
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawLines(&pen, pixelPoints.data(), (int)pixelPoints.size());
        
        g.ResetClip();
    }
}

void GraphRenderer::DrawTitle(Gdiplus::Graphics& g) const {
    if (config.title.empty()) return;
    
    Gdiplus::Color titleColor = config.darkTheme
        ? Gdiplus::Color(255, 230, 230, 230)
        : Gdiplus::Color(255, 30, 30, 30);
    Gdiplus::SolidBrush titleBrush(titleColor);
    Gdiplus::Font titleFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleBold);
    Gdiplus::StringFormat sfCenter;
    sfCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
    
    float cx = (float)(area.left + area.right) / 2.0f;
    Gdiplus::RectF titleRect(cx - 200, (float)area.top + 5, 400, 20);
    g.DrawString(config.title.c_str(), -1, &titleFont, titleRect, &sfCenter, &titleBrush);
}

void GraphRenderer::DrawLegend(Gdiplus::Graphics& g) const {
    if (!config.showLegend || series.empty()) return;
    
    // Conta series com labels
    int count = 0;
    for (const auto& s : series) {
        if (!s.label.empty()) count++;
    }
    if (count == 0) return;
    
    Gdiplus::Font legendFont(L"Segoe UI", 8.0f);
    Gdiplus::Color textColor = config.darkTheme
        ? Gdiplus::Color(255, 200, 200, 200)
        : Gdiplus::Color(255, 60, 60, 60);
    Gdiplus::SolidBrush textBrush(textColor);
    
    float plotRight = DataToPixelX(config.xMax);
    float plotTop = DataToPixelY(config.yMax);
    
    float legendX = plotRight - 130;
    float legendY = plotTop + 5;
    float lineHeight = 18.0f;
    
    // Fundo da legenda
    Gdiplus::Color bgColor = config.darkTheme
        ? Gdiplus::Color(200, 40, 40, 43)
        : Gdiplus::Color(200, 250, 250, 250);
    Gdiplus::SolidBrush bgBrush(bgColor);
    Gdiplus::Pen bgPen(Gdiplus::Color(100, 128, 128, 128), 1.0f);
    g.FillRectangle(&bgBrush, legendX - 5, legendY - 3, 
                    130.0f, count * lineHeight + 6);
    g.DrawRectangle(&bgPen, legendX - 5, legendY - 3,
                    130.0f, count * lineHeight + 6);
    
    for (const auto& s : series) {
        if (s.label.empty()) continue;
        
        Gdiplus::Pen linePen(s.color, s.lineWidth);
        g.DrawLine(&linePen, legendX, legendY + 8, legendX + 20, legendY + 8);
        
        Gdiplus::RectF labelRect(legendX + 25, legendY, 100, lineHeight);
        g.DrawString(s.label.c_str(), -1, &legendFont, labelRect, nullptr, &textBrush);
        
        legendY += lineHeight;
    }
}

void GraphRenderer::Render(HDC hdc) const {
    // Auto-sync tema com estado global
    const_cast<GraphConfig&>(config).darkTheme = TabPage::IsDarkTheme();

    Gdiplus::Graphics g(hdc);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    
    DrawBackground(g);
    DrawGrid(g);
    DrawAxes(g);
    DrawSeries(g);
    DrawTitle(g);
    DrawLegend(g);
}

GraphSeries GraphRenderer::CreateSeries(
    const std::vector<float>& xData,
    const std::vector<float>& yData,
    Gdiplus::Color color,
    const std::wstring& label,
    float lineWidth)
{
    GraphSeries s;
    s.color = color;
    s.label = label;
    s.lineWidth = lineWidth;
    
    size_t count = (xData.size() < yData.size()) ? xData.size() : yData.size();
    s.points.reserve(count);
    for (size_t i = 0; i < count; i++) {
        s.points.push_back(Gdiplus::PointF(xData[i], yData[i]));
    }
    
    return s;
}
