#include "TabPages.h"
#include "../Core/EngineCore.h"
#include "../Core/CompressionCalculator.h"
#include "../Core/ValveTrainCalculator.h"
#include "../Core/CamshaftCalculator.h"
#include "../Core/IntakeExhaustCalculator.h"
#include "../Core/EngineDataManager.h"
#include <commctrl.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")

// Estado global do tema
static bool g_darkTheme = false;
static HBRUSH g_darkBrush = nullptr;
static HBRUSH g_darkEditBrush = nullptr;
static bool g_imperial = false;

bool TabPage::IsImperial() { return g_imperial; }
void TabPage::SetImperial(bool imperial) { g_imperial = imperial; }

bool TabPage::IsDarkTheme() { return g_darkTheme; }
void TabPage::SetDarkTheme(bool dark) { 
    g_darkTheme = dark;
    if (g_darkBrush) DeleteObject(g_darkBrush);
    if (g_darkEditBrush) DeleteObject(g_darkEditBrush);
    g_darkBrush = CreateSolidBrush(GetThemeBgColor());
    g_darkEditBrush = CreateSolidBrush(dark ? RGB(30, 30, 30) : RGB(255, 255, 255));
}
COLORREF TabPage::GetThemeBgColor() { return g_darkTheme ? RGB(45, 45, 48) : RGB(240, 240, 240); }
COLORREF TabPage::GetThemeTextColor() { return g_darkTheme ? RGB(220, 220, 220) : RGB(30, 30, 30); }
HBRUSH TabPage::GetThemeBgBrush() { 
    if (!g_darkBrush) g_darkBrush = CreateSolidBrush(GetThemeBgColor());
    return g_darkBrush; 
}

static LRESULT CALLBACK TabPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TabPage* page = (TabPage*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_VSCROLL:
            if (page) {
                page->HandleScroll(wParam);
            }
            return 0;

        case WM_MOUSEWHEEL:
            {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                WPARAM scrollMsg = (delta > 0) ? SB_LINEUP : SB_LINEDOWN;
                if (page) {
                    for (int i = 0; i < abs(delta) / WHEEL_DELTA; i++) {
                        page->HandleScroll(scrollMsg);
                    }
                }
            }
            return 0;

        case WM_SIZE:
            if (page) {
                page->UpdateScrollBar();
                page->RecalculateLayout();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
            if (g_darkTheme) {
                HDC hdcCtrl = (HDC)wParam;
                SetTextColor(hdcCtrl, RGB(220, 220, 220));
                if (msg == WM_CTLCOLOREDIT) {
                    SetBkColor(hdcCtrl, RGB(30, 30, 30));
                    return (LRESULT)g_darkEditBrush;
                }
                SetBkColor(hdcCtrl, RGB(45, 45, 48));
                return (LRESULT)g_darkBrush;
            }
            break;

        case WM_ERASEBKGND:
            if (g_darkTheme) {
                HDC hdc = (HDC)wParam;
                RECT rc;
                GetClientRect(hwnd, &rc);
                FillRect(hdc, &rc, g_darkBrush);
                return 1;
            }
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                if (g_darkTheme) {
                    FillRect(hdc, &ps.rcPaint, g_darkBrush);
                }
                EndPaint(hwnd, &ps);
            }
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool RegisterTabPageClass(HINSTANCE hInstance) {
    static bool registered = false;
    if (registered) return true;
    
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = TabPageProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcex.lpszClassName = L"EngineCalcTabPage";
    
    if (RegisterClassExW(&wcex)) {
        registered = true;
        return true;
    }
    return false;
}

// ============================================================================
// NumericEdit
// ============================================================================

// Fontes compartilhadas (lazy-init)
static HFONT GetEditFont() {
    static HFONT hFont = CreateFontW(
        15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    return hFont;
}

static HFONT GetResultsFont() {
    static HFONT hFont = CreateFontW(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
    return hFont;
}

static HFONT GetLabelFont() {
    static HFONT hFont = CreateFontW(
        15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    return hFont;
}

// Subclass proc para permitir digitos, ponto, virgula e sinal negativo
static LRESULT CALLBACK NumericEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                         UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (msg == WM_CHAR) {
        wchar_t ch = (wchar_t)wParam;
        // Permite: digitos, ponto, virgula, backspace, sinal negativo
        if (ch >= '0' && ch <= '9') goto allow;
        if (ch == '.' || ch == ',') goto allow;
        if (ch == '-') goto allow;
        if (ch == '\b') goto allow; // backspace
        if (ch < ' ') goto allow;  // control chars (Ctrl+C, Ctrl+V, etc.)
        // Bloqueia tudo o mais
        MessageBeep(MB_OK);
        return 0;
    }
    allow:
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

NumericEdit::NumericEdit() 
    : hwnd(nullptr), minValue(0.0), maxValue(10000.0), decimalPlaces(2) {
}

void NumericEdit::Create(HWND parent, int id, int x, int y, int width, int height,
                         double min, double max, int decimals) {
    minValue = min;
    maxValue = max;
    decimalPlaces = decimals;

    hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        x, y, width, height,
        parent,
        (HMENU)(INT_PTR)id,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
        nullptr
    );

    // Subclass para filtrar caracteres
    SetWindowSubclass(hwnd, NumericEditProc, 0, 0);

    SendMessage(hwnd, WM_SETFONT, (WPARAM)GetEditFont(), TRUE);
}

void NumericEdit::SetValue(double value) {
    if (!hwnd) return;
    
    std::wostringstream ss;
    ss << std::fixed << std::setprecision(decimalPlaces) << value;
    SetWindowTextW(hwnd, ss.str().c_str());
}

double NumericEdit::GetValue() const {
    if (!hwnd) return 0.0;

    wchar_t buffer[256];
    GetWindowTextW(hwnd, buffer, 256);

    // Converte virgula para ponto (locale brasileiro)
    for (int i = 0; buffer[i]; i++) {
        if (buffer[i] == L',') buffer[i] = L'.';
    }

    try {
        return std::wcstod(buffer, nullptr);
    }
    catch (...) {
        return 0.0;
    }
}

bool NumericEdit::IsValid() const {
    double value = GetValue();
    return value >= minValue && value <= maxValue;
}

// ============================================================================
// TabPage
// ============================================================================

TabPage::TabPage(HWND parent, HINSTANCE instance) 
    : hwndParent(parent), hwndPage(nullptr), hInst(instance), visible(false),
      scrollPos(0), contentHeight(600), hwndTooltip(nullptr) {
}

TabPage::~TabPage() {
    if (hwndTooltip) {
        DestroyWindow(hwndTooltip);
    }
    if (hwndPage) {
        DestroyWindow(hwndPage);
    }
}

int TabPage::GetPageWidth() const {
if (!hwndPage) return 800;
    RECT rc;
    GetClientRect(hwndPage, &rc);
    return rc.right - rc.left;
}

int TabPage::GetPageHeight() const {
if (!hwndPage) return 450;
    RECT rc;
    GetClientRect(hwndPage, &rc);
    return rc.bottom - rc.top;
}

int TabPage::GetLabelWidth(float percentage) const {
    return static_cast<int>(GetPageWidth() * percentage);
}

int TabPage::GetEditWidth(float percentage) const {
    return static_cast<int>(GetPageWidth() * percentage);
}

int TabPage::GetResultsWidth(float percentage) const {
    int margin = GetMargin();
    return static_cast<int>((GetPageWidth() - 2 * margin) * percentage);
}

void TabPage::RecalculateLayout() {
}

HFONT TabPage::GetAppLabelFont() {
    return GetLabelFont();
}

HFONT TabPage::GetAppResultsFont() {
    return GetResultsFont();
}

void TabPage::CreateTooltip(HWND hwndControl, const wchar_t* text) {
if (!hwndTooltip) {
    hwndTooltip = CreateWindowExW(
            0, TOOLTIPS_CLASS, NULL,
            WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            hwndPage, NULL, hInst, NULL
        );
        
        // Define largura maxima para permitir quebra de linha
        SendMessage(hwndTooltip, TTM_SETMAXTIPWIDTH, 0, 400);
        SendMessage(hwndTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000);
        SendMessage(hwndTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 500);
    }
    
    if (hwndTooltip && hwndControl && text) {
        TOOLINFOW ti = { 0 };
        ti.cbSize = sizeof(TOOLINFOW);
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd = hwndPage;
        ti.uId = (UINT_PTR)hwndControl;
        ti.lpszText = (LPWSTR)text;
        
        SendMessage(hwndTooltip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
    }
}

void TabPage::Show(bool show) {
    if (hwndPage) {
        ShowWindow(hwndPage, show ? SW_SHOW : SW_HIDE);
        visible = show;
        if (show) {
            UpdateScrollBar();
        }
    }
}

void TabPage::UpdateScrollBar() {
    if (!hwndPage) return;
    
    RECT rcClient;
    GetClientRect(hwndPage, &rcClient);
    int pageHeight = rcClient.bottom - rcClient.top;
    
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = contentHeight;
    si.nPage = pageHeight;
    si.nPos = scrollPos;
    
    SetScrollInfo(hwndPage, SB_VERT, &si, TRUE);
}

void TabPage::HandleScroll(WPARAM wParam) {
    if (!hwndPage) return;
    
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(hwndPage, SB_VERT, &si);
    
    int oldPos = scrollPos;
    int newPos = si.nPos;
    
    switch (LOWORD(wParam)) {
        case SB_LINEUP:
            newPos -= 20;
            break;
        case SB_LINEDOWN:
            newPos += 20;
            break;
        case SB_PAGEUP:
            newPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            newPos += si.nPage;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            newPos = HIWORD(wParam);
            break;
    }
    
    newPos = max(0, min(newPos, si.nMax - (int)si.nPage));
    
    if (newPos != oldPos) {
        scrollPos = newPos;
        
        si.nPos = scrollPos;
        SetScrollInfo(hwndPage, SB_VERT, &si, TRUE);
        
        int deltaY = oldPos - newPos;
        HDWP hdwp = BeginDeferWindowPos(50);
        
        HWND hwndChild = GetWindow(hwndPage, GW_CHILD);
        while (hwndChild) {
            RECT rc;
            GetWindowRect(hwndChild, &rc);
            MapWindowPoints(HWND_DESKTOP, hwndPage, (LPPOINT)&rc, 2);
            
            if (hdwp) {
                hdwp = DeferWindowPos(hdwp, hwndChild, NULL,
                                     rc.left, rc.top + deltaY,
                                     0, 0,
                                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
            
            hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
        }
        
        if (hdwp) {
            EndDeferWindowPos(hdwp);
        }
        
        InvalidateRect(hwndPage, NULL, TRUE);
        UpdateWindow(hwndPage);
    }
}
