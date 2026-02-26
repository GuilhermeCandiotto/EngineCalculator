# Arquitetura do Projeto

## Estrutura de Diretórios

```
EngineCalculator/
├── Core/                              # Núcleo de cálculos
│   ├── EngineCore.h/.cpp              # Cálculos básicos + MEP
│   ├── CompressionCalculator.h/.cpp   # Sistema de compressão
│   ├── ValveTrainCalculator.h/.cpp    # Trem de válvulas
│   ├── CamshaftCalculator.h/.cpp      # Comando de válvulas
│   ├── IntakeExhaustCalculator.h/.cpp # Admissão e escape
│   ├── FuelSystemCalculator.h/.cpp    # Sistema de combustível
│   ├── AlternativeFuelsCalculator.h/.cpp # Combustíveis alternativos
│   ├── EnginePresets.h/.cpp           # Sistema de presets
│   ├── JSONDataLoader.h/.cpp          # Carregamento de dados JSON
│   ├── UnitConverter.h/.cpp           # Conversão de unidades
│   ├── EngineDataManager.h/.cpp       # Gerenciador central (Singleton)
│   ├── ProjectManager.h/.cpp          # Salvar/Carregar projetos (JSON)
│   └── EngineValidator.h/.cpp         # Validação cruzada (12 tipos)
│
├── UI/                                # Interface Win32
│   ├── MainWindow.h/.cpp              # Janela principal (orquestrador)
│   ├── EnginePresetsDialog.h/.cpp     # Dialog de presets
│   ├── GraphRenderer.h/.cpp           # Componente de gráficos GDI+
│   ├── TabPages.h/.cpp                # Classes base: TabPage + NumericEdit
│   ├── Managers/                      # Managers especializados (FASE 2)
│   │   ├── MenuManager.h/.cpp         # Menu principal e handlers
│   │   ├── FileManager.h/.cpp         # Dialogs Open/Save e I/O
│   │   ├── TabManager.h/.cpp          # Controle das abas
│   │   ├── StatusBarManager.h/.cpp    # Barra de status e título
│   │   └── SettingsManager.h/.cpp     # Temas, Registry, auto-save
│   └── Tabs/                          # Abas individuais (FASE 1)
│       ├── BasicTab.h/.cpp
│       ├── CompressionTab.h/.cpp
│       ├── ValveTrainTab.h/.cpp
│       ├── CamshaftTab.h/.cpp
│       ├── IntakeTab.h/.cpp
│       ├── ExhaustTab.h/.cpp
│       ├── FuelTab.h/.cpp
│       └── AlternativeFuelsTab.h/.cpp
│
├── Data/                              # Dados JSON
│   ├── Engines/                       # 9 arquivos JSON com 81 presets
│   │   ├── american_v8.json
│   │   ├── brazilian_performance.json
│   │   ├── classic_muscle.json
│   │   ├── diesel_performance.json
│   │   ├── european.json
│   │   ├── japanese_na.json
│   │   ├── japanese_turbo.json
│   │   ├── modern_turbo.json
│   │   └── racing_engines.json
│   └── Components/                    # Base de componentes
│       ├── camshafts.json
│       ├── connecting_rods.json
│       ├── crankshafts.json
│       ├── pistons.json
│       ├── valve_springs.json
│       └── valves.json
│
├── docs/                              # Documentação
├── EngineCalculator.cpp               # Entry point (WinMain)
├── EngineCalculator.slnx              # Visual Studio solution
└── EngineCalculator.vcxproj           # Projeto Visual Studio
```

## Design Patterns

### 1. Singleton — `EngineDataManager`
Gerenciador central de dados do projeto. Única instância acessível globalmente.

### 2. Observer — Data Sharing entre Abas
```
BasicTab → EngineDataManager → {Compression, Camshaft, Intake, Exhaust}
CamshaftTab → EngineDataManager → ExhaustTab
```

### 3. Template Method — `TabPage`
Classe base com métodos virtuais (`Create()`, `OnCalculate()`, `OnClear()`, `RecalculateLayout()`).
Cada aba herda e implementa seu comportamento específico.

### 4. Delegation — MainWindow + Managers
`MainWindow` orquestra 5 managers especializados:
- `MenuManager` — criação do menu e handlers de comando
- `FileManager` — dialogs de arquivo e I/O de projetos
- `TabManager` — criação e controle das abas
- `StatusBarManager` — barra de status e título da janela
- `SettingsManager` — temas, persistência no Registry, auto-save

## Fluxo de Dados

```
[Usuário preenche BasicTab]
        ↓
[BasicTab::OnCalculate()]
        ↓
[EngineDataManager::UpdateBasicData()]
        ↓
[NotifyDataChanged()]
        ↓
[Todas as tabs usam dados reais do projeto via EngineDataManager::GetProject()]
        ↓
[CamshaftTab, IntakeTab, ExhaustTab, ValveTrainTab criam EngineCore com bore/stroke reais]
```

**Importante:** Nenhuma tab usa valores hardcoded de bore/stroke. Todas consultam
`EngineDataManager::GetInstance()->GetProject().basicData` para criar o `EngineCore`
usado nos cálculos.

## Persistência

- **Projetos:** Formato `.ecproj` (JSON) via `ProjectManager`
- **Configurações:** Windows Registry (`HKCU\Software\EngineCalculator`)
  - Posição/tamanho da janela, tema, sistema de unidades
- **Auto-save:** Arquivo temporário a cada 5 minutos

## Tecnologias

- **C++17**, Win32 API, Common Controls v6, GDI+
- **Libs:** `comctl32.lib`, `comdlg32.lib`, `gdiplus.lib`, `shlwapi.lib`
- **Build:** Visual Studio 2019+, Windows SDK 10.0+

## Referências Científicas dos Cálculos

### Faixa de RPM do Comando (`CamshaftCalculator::CalculatePeakRPMFromDuration`)
- Modelo baseado em Mean Piston Speed (MPS) — Heywood eq. 2.11
- MPS de projeto interpolado pela duração do comando — Bell Ch.5, Vizard
- Validado contra 6 motores reais: Stock Civic → F1 V10 (erro ≤9%)

### Compressão Dinâmica (`CompressionCalculator::CalculateDynamicCompressionRatio`)
- Modelo de ciclo real com IVC — Blair Ch.4
- SAE Paper 2007-01-0148

### Admissão/Escape (`IntakeExhaustCalculator`)
- Helmholtz resonance — Blair Ch.6
- Tuned length — SAE Paper 2003-01-0376
