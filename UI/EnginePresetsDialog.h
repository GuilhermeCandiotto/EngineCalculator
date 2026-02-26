#pragma once
// ============================================================================
// EnginePresetsDialog.h - Dialog para seleção de engine presets
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// CRIADO PROGRAMATICAMENTE (SEM ARQUIVOS .RC)
// ============================================================================
#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <vector>
#include "../Core/EnginePresets.h"

class EnginePresetsDialog {
private:
    HWND hDlg;
    HWND hListView;
    HWND hCategoryCombo;
    HWND hSearchEdit;
    HWND hDescriptionEdit;
    HWND hOkButton;
    HWND hCancelButton;
    HINSTANCE hInstance;
    
    EnginePresetsManager* presetsManager;
    std::vector<EnginePreset> filteredPresets;
    EnginePreset selectedPreset;
    bool presetSelected;
    
    // Criação programática
    void CreateControls();
    void InitializeListView();
    void PopulateCategoryCombo();
    void LoadPresets(const std::wstring& category = L"");
    
    // Eventos
    void OnCategoryChanged();
    void OnSearchChanged();
    void OnPresetSelected(int index);
    void OnPresetDoubleClick(int index);
    void OnOK();
    void OnCancel();
    
    // Helper
    void UpdateDescription(const EnginePreset& preset);
    void FilterPresets(const std::wstring& searchText);
    int AddListViewItem(const EnginePreset& preset, int index);
    
    // Dialog Proc
    static LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

public:
    EnginePresetsDialog(EnginePresetsManager* manager, HINSTANCE hInst);
    ~EnginePresetsDialog();
    
    // Exibe o dialog e retorna se usuário selecionou
    bool Show(HWND parent);
    
    // Retorna o preset selecionado
    EnginePreset GetSelectedPreset() const { return selectedPreset; }
};
