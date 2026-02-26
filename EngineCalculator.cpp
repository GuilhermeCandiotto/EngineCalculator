// EngineCalculator.cpp : Define o ponto de entrada para o aplicativo.
//

#include "framework.h"
#include "EngineCalculator.h"
#include "UI/MainWindow.h"
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Inicializa GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Cria a janela principal
    MainWindow mainWindow(hInstance);
    
    if (!mainWindow.Create(nCmdShow)) {
        MessageBoxW(nullptr, L"Falha ao criar a janela principal.", L"Erro", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    MSG msg = { 0 };
    
    // Obtem handle da janela e tabela de aceleradores
    HWND hwndMain = mainWindow.GetHandle();
    HACCEL hAccel = mainWindow.GetAcceleratorTable();

    // Loop de mensagem principal
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        // Tenta traduzir acelerador primeiro (Ctrl+N, Ctrl+S, F5, etc.)
        if (!TranslateAccelerator(hwndMain, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Finaliza GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return (int)msg.wParam;
}
