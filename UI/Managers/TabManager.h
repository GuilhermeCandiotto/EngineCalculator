#pragma once
// ============================================================================
// TabManager.h - Gerenciador de Tabs (Abas)
// Engine Calculator v1.0.0 - FASE 2 Refatoração - Março 2026
// 
// Responsável por criar e gerenciar o controle de tabs e as abas
// ============================================================================
#include <windows.h>
#include <vector>

// Forward declarations
class MainWindow;
class TabPage;
class BasicTab;
class CompressionTab;
class ValveTrainTab;
class CamshaftTab;
class IntakeTab;
class ExhaustTab;
class FuelTab;
class AlternativeFuelsTab;
class TurboTab;

class TabManager {
private:
    MainWindow* mainWindow;
    HWND hwndMain;
    HWND hwndTab;
    HINSTANCE hInstance;
    
    // Ponteiros para as tabs
    BasicTab* tabBasic;
    CompressionTab* tabCompression;
    ValveTrainTab* tabValveTrain;
    CamshaftTab* tabCamshaft;
    IntakeTab* tabIntake;
    ExhaustTab* tabExhaust;
    FuelTab* tabFuel;
    AlternativeFuelsTab* tabAlternativeFuels;
    TurboTab* tabTurbo;
    
    std::vector<TabPage*> tabs;
    int currentTab;
    
public:
    TabManager(MainWindow* window, HWND hwnd, HINSTANCE hInst);
    ~TabManager();
    
    // Criação das tabs
    void CreateTabControl();
    void InitializeTabs();
    
    // Eventos
    void OnTabChange();
    void OnCalculate();
    void OnClear();
    
    // Atualização
    void RefreshAllTabsFromProject();
    
    // Getters
    HWND GetTabControlHandle() const { return hwndTab; }
    int GetCurrentTab() const { return currentTab; }
    const std::vector<TabPage*>& GetTabs() const { return tabs; }
    
    BasicTab* GetBasicTab() const { return tabBasic; }
    CompressionTab* GetCompressionTab() const { return tabCompression; }
    ValveTrainTab* GetValveTrainTab() const { return tabValveTrain; }
    CamshaftTab* GetCamshaftTab() const { return tabCamshaft; }
    IntakeTab* GetIntakeTab() const { return tabIntake; }
    ExhaustTab* GetExhaustTab() const { return tabExhaust; }
    FuelTab* GetFuelTab() const { return tabFuel; }
    AlternativeFuelsTab* GetAlternativeFuelsTab() const { return tabAlternativeFuels; }
};
