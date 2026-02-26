#include "EnginePresetsDialog.h"
#include <windowsx.h>
#include <sstream>
#include <algorithm>
#include <set>

// IDs dos controles
#define IDC_PRESETS_LISTVIEW    1001
#define IDC_CATEGORY_COMBO      1002
#define IDC_SEARCH_EDIT         1003
#define IDC_DESCRIPTION_EDIT    1004
#define IDC_OK_BUTTON           IDOK
#define IDC_CANCEL_BUTTON       IDCANCEL

EnginePresetsDialog::EnginePresetsDialog(EnginePresetsManager* manager, HINSTANCE hInst)
    : presetsManager(manager), hInstance(hInst), hDlg(nullptr), 
      hListView(nullptr), hCategoryCombo(nullptr), hSearchEdit(nullptr),
      hDescriptionEdit(nullptr), hOkButton(nullptr), hCancelButton(nullptr),
      presetSelected(false) {
}

EnginePresetsDialog::~EnginePresetsDialog() {
}

bool EnginePresetsDialog::Show(HWND parent) {
    presetSelected = false;
    
    // Registra classe de janela
    const wchar_t CLASS_NAME[] = L"EnginePresetsDialog";
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = DialogProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClassW(&wc);
    
    // Cria janela modal
    hDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE,
        CLASS_NAME,
        L"Selecionar Motor - Engine Presets Database",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 550,
        parent,
        nullptr,
        hInstance,
        this
    );
    
    if (!hDlg) {
        return false;
    }
    
    // Centraliza
    RECT rcParent, rcDlg;
    GetWindowRect(parent, &rcParent);
    GetWindowRect(hDlg, &rcDlg);
    int x = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;
    SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    // Desabilita janela pai
    EnableWindow(parent, FALSE);
    
    // Message loop do diálogo modal
    MSG msg;
    while (IsWindow(hDlg) && GetMessage(&msg, nullptr, 0, 0)) {
        // Se recebeu WM_QUIT, re-envia para a janela principal e sai
        if (msg.message == WM_QUIT) {
            PostQuitMessage((int)msg.wParam);
            break;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Re-habilita janela pai
    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);
    
    return presetSelected;
}

LRESULT CALLBACK EnginePresetsDialog::DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    EnginePresetsDialog* dialog = nullptr;
    
    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        dialog = reinterpret_cast<EnginePresetsDialog*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);
        dialog->hDlg = hwnd;
        dialog->CreateControls();
        return 0;
    }
    
    dialog = reinterpret_cast<EnginePresetsDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    
    if (dialog) {
        return dialog->HandleMessage(msg, wParam, lParam);
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT EnginePresetsDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_CATEGORY_COMBO) {
                OnCategoryChanged();
                return 0;
            }
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_SEARCH_EDIT) {
                OnSearchChanged();
                return 0;
            }
            if (LOWORD(wParam) == IDC_OK_BUTTON) {
                OnOK();
                return 0;
            }
            if (LOWORD(wParam) == IDC_CANCEL_BUTTON) {
                OnCancel();
                return 0;
            }
            break;
            
        case WM_NOTIFY: {
            LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);
            if (nmhdr->idFrom == IDC_PRESETS_LISTVIEW) {
                if (nmhdr->code == LVN_ITEMCHANGED) {
                    LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(lParam);
                    if (pnmv->uNewState & LVIS_SELECTED) {
                        OnPresetSelected(pnmv->iItem);
                    }
                }
                if (nmhdr->code == NM_DBLCLK) {
                    LPNMITEMACTIVATE pnmia = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
                    if (pnmia->iItem != -1) {
                        OnPresetDoubleClick(pnmia->iItem);
                    }
                }
            }
            break;
        }
        
        case WM_CLOSE:
            OnCancel();
            return 0;
            
        case WM_DESTROY:
            // NÃO chama PostQuitMessage aqui - isso fecharia o app todo!
            // O message loop do Show() detectará que a janela foi destruída
            return 0;
    }
    
    return DefWindowProcW(hDlg, msg, wParam, lParam);
}

void EnginePresetsDialog::CreateControls() {
    HFONT hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    // Label Categoria
    HWND hLabel1 = CreateWindowW(L"STATIC", L"Categoria:",
        WS_CHILD | WS_VISIBLE,
        10, 10, 70, 20,
        hDlg, nullptr, hInstance, nullptr);
    SendMessage(hLabel1, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Combo Categoria
    hCategoryCombo = CreateWindowW(L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        85, 8, 140, 200,
        hDlg, (HMENU)IDC_CATEGORY_COMBO, hInstance, nullptr);
    SendMessage(hCategoryCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Label Buscar
    HWND hLabel2 = CreateWindowW(L"STATIC", L"Buscar:",
        WS_CHILD | WS_VISIBLE,
        240, 10, 50, 20,
        hDlg, nullptr, hInstance, nullptr);
    SendMessage(hLabel2, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Edit Buscar
    hSearchEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
        295, 8, 180, 22,
        hDlg, (HMENU)IDC_SEARCH_EDIT, hInstance, nullptr);
    SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // ListView
    hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, nullptr,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP,
        10, 40, 605, 210,
        hDlg, (HMENU)IDC_PRESETS_LISTVIEW, hInstance, nullptr);
    
    // Label Descrição
    HWND hLabel3 = CreateWindowW(L"STATIC", L"Descrição:",
        WS_CHILD | WS_VISIBLE,
        10, 258, 80, 20,
        hDlg, nullptr, hInstance, nullptr);
    SendMessage(hLabel3, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Edit Descrição (multiline, readonly)
    hDescriptionEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
        10, 280, 605, 145,
        hDlg, (HMENU)IDC_DESCRIPTION_EDIT, hInstance, nullptr);
    SendMessage(hDescriptionEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Botão OK
    hOkButton = CreateWindowW(L"BUTTON", L"OK - Carregar Motor",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
        390, 435, 140, 30,
        hDlg, (HMENU)IDC_OK_BUTTON, hInstance, nullptr);
    SendMessage(hOkButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Botão Cancelar
    hCancelButton = CreateWindowW(L"BUTTON", L"Cancelar",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        540, 435, 75, 30,
        hDlg, (HMENU)IDC_CANCEL_BUTTON, hInstance, nullptr);
    SendMessage(hCancelButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Inicializa controles
    InitializeListView();
    PopulateCategoryCombo();
    LoadPresets();
}

void EnginePresetsDialog::InitializeListView() {
    ListView_SetExtendedListViewStyle(hListView, 
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    col.fmt = LVCFMT_LEFT;
    
    col.pszText = const_cast<LPWSTR>(L"Motor");
    col.cx = 180;
    ListView_InsertColumn(hListView, 0, &col);
    
    col.pszText = const_cast<LPWSTR>(L"Aplicação");
    col.cx = 210;
    ListView_InsertColumn(hListView, 1, &col);
    
    col.pszText = const_cast<LPWSTR>(L"HP");
    col.cx = 60;
    col.fmt = LVCFMT_CENTER;
    ListView_InsertColumn(hListView, 2, &col);
    
    col.pszText = const_cast<LPWSTR>(L"Torque");
    col.cx = 60;
    ListView_InsertColumn(hListView, 3, &col);
    
    col.pszText = const_cast<LPWSTR>(L"Cil");
    col.cx = 40;
    ListView_InsertColumn(hListView, 4, &col);
    
    col.pszText = const_cast<LPWSTR>(L"Turbo");
    col.cx = 50;
    ListView_InsertColumn(hListView, 5, &col);
}

void EnginePresetsDialog::PopulateCategoryCombo() {
    ComboBox_AddString(hCategoryCombo, L"Todas as Categorias");
    ComboBox_AddString(hCategoryCombo, L"American V8");
    ComboBox_AddString(hCategoryCombo, L"Brazilian Performance");
    ComboBox_AddString(hCategoryCombo, L"Classic Muscle Cars");
    ComboBox_AddString(hCategoryCombo, L"European Performance");
    ComboBox_AddString(hCategoryCombo, L"High Performance Diesel");
    ComboBox_AddString(hCategoryCombo, L"Japanese N/A");
    ComboBox_AddString(hCategoryCombo, L"Japanese Turbo");
    ComboBox_AddString(hCategoryCombo, L"Modern Turbo");
    ComboBox_AddString(hCategoryCombo, L"Race Engines");
    
    ComboBox_SetCurSel(hCategoryCombo, 0);
}

void EnginePresetsDialog::LoadPresets(const std::wstring& category) {
    ListView_DeleteAllItems(hListView);
    
    // DEBUG: Mostra informações de carregamento
    std::wstring debugMsg = L"=======================================\r\n";
    debugMsg += L"  DEBUG - CARREGAMENTO DE PRESETS\r\n";
    debugMsg += L"=======================================\r\n\r\n";
    debugMsg += L"Diretorio: " + presetsManager->GetEnginesDirectory() + L"\r\n\r\n";

    if (category.empty() || category == L"Todas as Categorias") {
        filteredPresets = presetsManager->GetAllPresets();
        debugMsg += L"Buscando: TODAS AS CATEGORIAS\r\n";
    } else {
        filteredPresets = presetsManager->GetPresetsByCategory(category);
        debugMsg += L"Buscando categoria: \"" + category + L"\"\r\n";
    }
    
    debugMsg += L"Total encontrado: " + std::to_wstring(filteredPresets.size()) + L"\r\n";
    debugMsg += L"Total no banco: " + std::to_wstring(presetsManager->GetEngineCount()) + L"\r\n\r\n";
    
    // Lista todas as categorias únicas encontradas
    if (presetsManager->GetEngineCount() > 0) {
        debugMsg += L"Categorias disponíveis no banco:\r\n";
        std::vector<EnginePreset> allPresets = presetsManager->GetAllPresets();
        std::set<std::wstring> uniqueCategories;
        
        for (const auto& p : allPresets) {
            if (!p.category.empty()) {
                uniqueCategories.insert(p.category);
            }
        }
        
        for (const auto& cat : uniqueCategories) {
            debugMsg += L"  - \"" + cat + L"\"\r\n";
        }
    }
    
    SetWindowTextW(hDescriptionEdit, debugMsg.c_str());
    
    // Verifica se há presets carregados
    if (filteredPresets.empty()) {
        return;
    }
    
    for (size_t i = 0; i < filteredPresets.size(); i++) {
        AddListViewItem(filteredPresets[i], (int)i);
    }
}

int EnginePresetsDialog::AddListViewItem(const EnginePreset& preset, int index) {
    LVITEMW item = {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = index;
    item.lParam = index;
    
    item.pszText = const_cast<LPWSTR>(preset.name.c_str());
    int itemIndex = ListView_InsertItem(hListView, &item);
    
    ListView_SetItemText(hListView, itemIndex, 1, const_cast<LPWSTR>(preset.application.c_str()));
    
    wchar_t hp[32];
    swprintf_s(hp, L"%.0f", preset.factoryHP);
    ListView_SetItemText(hListView, itemIndex, 2, hp);
    
    wchar_t torque[32];
    swprintf_s(torque, L"%.0f", preset.factoryTorque);
    ListView_SetItemText(hListView, itemIndex, 3, torque);
    
    wchar_t cyl[32];
    swprintf_s(cyl, L"%d", preset.projectData.basicData.cylinders);
    ListView_SetItemText(hListView, itemIndex, 4, cyl);
    
    ListView_SetItemText(hListView, itemIndex, 5, 
        preset.hasForcedInduction ? const_cast<LPWSTR>(L"Sim") : const_cast<LPWSTR>(L"Não"));
    
    return itemIndex;
}

void EnginePresetsDialog::OnCategoryChanged() {
    int sel = ComboBox_GetCurSel(hCategoryCombo);
    if (sel == CB_ERR) return;
    
    wchar_t category[256];
    ComboBox_GetLBText(hCategoryCombo, sel, category);
    
    LoadPresets(category);
}

void EnginePresetsDialog::OnSearchChanged() {
    wchar_t searchText[256];
    GetWindowTextW(hSearchEdit, searchText, 256);
    
    FilterPresets(searchText);
}

void EnginePresetsDialog::FilterPresets(const std::wstring& searchText) {
    if (searchText.empty()) {
        OnCategoryChanged();
        return;
    }
    
    ListView_DeleteAllItems(hListView);
    
    std::wstring searchLower = searchText;
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::towlower);
    
    std::vector<EnginePreset> allPresets = presetsManager->GetAllPresets();
    filteredPresets.clear();
    
    for (const auto& preset : allPresets) {
        std::wstring nameLower = preset.name;
        std::wstring appLower = preset.application;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
        std::transform(appLower.begin(), appLower.end(), appLower.begin(), ::towlower);
        
        if (nameLower.find(searchLower) != std::wstring::npos ||
            appLower.find(searchLower) != std::wstring::npos) {
            filteredPresets.push_back(preset);
        }
    }
    
    for (size_t i = 0; i < filteredPresets.size(); i++) {
        AddListViewItem(filteredPresets[i], (int)i);
    }
}

void EnginePresetsDialog::OnPresetSelected(int index) {
    if (index < 0 || index >= (int)filteredPresets.size()) return;
    
    UpdateDescription(filteredPresets[index]);
}

void EnginePresetsDialog::OnPresetDoubleClick(int index) {
    if (index < 0 || index >= (int)filteredPresets.size()) return;
    
    selectedPreset = filteredPresets[index];
    presetSelected = true;
    DestroyWindow(hDlg);
}

void EnginePresetsDialog::UpdateDescription(const EnginePreset& preset) {
    std::wstringstream desc;
    desc << L"MOTOR: " << preset.name << L"\r\n\r\n";
    desc << L"DESCRIÇÃO:\r\n" << preset.description << L"\r\n\r\n";
    desc << L"APLICAÇÃO:\r\n" << preset.application << L"\r\n\r\n";
    desc << L"ESPECIFICAÇÕES:\r\n";
    desc << L"  Potência: " << preset.factoryHP << L" HP @ " << preset.factoryRPM << L" RPM\r\n";
    desc << L"  Torque: " << preset.factoryTorque << L" lb-ft\r\n";
    desc << L"  Combustível: " << preset.fuelType << L"\r\n";
    desc << L"  Indução Forçada: " << (preset.hasForcedInduction ? L"Sim" : L"Não") << L"\r\n\r\n";
    desc << L"DADOS MOTOR:\r\n";
    desc << L"  Diâmetro: " << preset.projectData.basicData.bore << L" mm\r\n";
    desc << L"  Curso: " << preset.projectData.basicData.stroke << L" mm\r\n";
    desc << L"  Cilindros: " << preset.projectData.basicData.cylinders << L"\r\n";
    
    SetWindowTextW(hDescriptionEdit, desc.str().c_str());
}

void EnginePresetsDialog::OnOK() {
    int sel = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (sel == -1) {
        MessageBoxW(hDlg, L"Por favor, selecione um motor.", L"Atenção", MB_OK | MB_ICONWARNING);
        return;
    }
    
    LVITEMW item = {};
    item.mask = LVIF_PARAM;
    item.iItem = sel;
    ListView_GetItem(hListView, &item);
    
    int index = (int)item.lParam;
    if (index >= 0 && index < (int)filteredPresets.size()) {
        selectedPreset = filteredPresets[index];
        presetSelected = true;
        DestroyWindow(hDlg);
    }
}

void EnginePresetsDialog::OnCancel() {
    presetSelected = false;
    DestroyWindow(hDlg);
}
