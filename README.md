# Engine Calculator

Professional internal combustion engine design calculator. C++17 / Win32 API.

<p align="center">
  <img src="https://img.shields.io/badge/C++-17-blue?logo=cplusplus" alt="C++17"/>
  <img src="https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows" alt="Windows"/>
  <img src="https://img.shields.io/badge/Build-Passing-brightgreen" alt="Build"/>
  <img src="https://img.shields.io/badge/Presets-81_Engines-orange" alt="81 Engines"/>
  <img src="https://img.shields.io/badge/License-Proprietary-red" alt="License"/>
</p>

Scientific-grade calculations for internal combustion engine design, based on 18 SAE Papers and 6 reference textbooks (Heywood, Blair, Pulkrabek, Bell, Baechtel, Vizard).

## Features

| Module | Description |
|--------|-------------|
| **Basic** | Displacement, bore/stroke ratio, mean piston speed, MEP (BMEP/IMEP/FMEP/PMEP) |
| **Compression** | Static & dynamic CR, chamber/gasket/deck/dome volumes, fuel recommendations |
| **Valve Train** | Valve sizing, spring analysis (force, natural frequency, coil bind), float RPM |
| **Camshaft** | Valve events (IVO/IVC/EVO/EVC), overlap, LSA analysis, RPM range via MPS model |
| **Intake** | Helmholtz resonance, runner/plenum sizing, velocity stacks, ITB vs Plenum comparison |
| **Exhaust** | Primary tuned length (wave reflection), pulse analysis, 4-1 vs 4-2-1, scavenging |
| **Fuel** | Carburetor CFM, injector sizing, duty cycle, BSFC, fuel consumption |
| **Alternative Fuels** | NOS, Nitromethane, Methanol, Water/Meth injection, race fuels (VP, Sunoco, E85) |
| **Validation** | 12 cross-checks (piston-to-valve clearance, CR vs cam, springs vs RPM, boost vs CR) |

**81 engine presets** included — F1, NASCAR, Le Mans, JDM legends, muscle cars, Brazilian engines, European performance.

## Quick Start

### Requirements

- Visual Studio 2019+ (MSVC v142+)
- Windows SDK 10.0+
- C++17 standard

### Build

```bash
git clone https://github.com/GuilhermeCandiotto/EngineCalculator.git
cd EngineCalculator
```

1. Open `EngineCalculator.slnx` in Visual Studio
2. Build → Build Solution (`Ctrl+Shift+B`)
3. `F5` to run

Zero external dependencies — only Windows SDK libs (`comctl32`, `comdlg32`, `gdiplus`, `shlwapi`), all pre-configured in the `.vcxproj`.

## Usage

1. **Ctrl+N** — New project
2. **Basic** tab — fill bore, stroke, cylinders, RPM → Calculate
3. Navigate tabs — basic data auto-propagates to all modules
4. **F5** — Validate entire project (watch for CRITICAL alerts)
5. **Ctrl+S** — Save project (`.ecproj`, JSON format)
6. **Ctrl+E** — Export full report (`.txt`)

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Ctrl+N | New Project |
| Ctrl+O | Open Project |
| Ctrl+S | Save Project |
| Ctrl+E | Export Report (.txt) |
| Ctrl+Q | Quit |
| F5 | Validate Project |

## Interface

- **Responsive layout** — 800×600 to 4K
- **Light/Dark theme** — full support (labels, edits, graphs, backgrounds)
- **GDI+ graphs** in all 8 tabs with anti-aliasing
- **Metric ↔ Imperial** unit switching
- **Educational tooltips** on every input field
- **Recent projects** (last 5)
- **Auto-save** every 5 minutes
- **Validation alerts** on tab switch via status bar

## Engine Presets Database

81 engines organized in 9 JSON category files:

| Category | Examples |
|----------|----------|
| American V8 | GM LS3, Ford Coyote 5.0, Chevy 454 |
| Classic Muscle | Hemi 426, Boss 429, Pontiac 455 |
| Japanese Turbo | 2JZ-GTE, RB26DETT, 4G63T, SR20DET |
| Japanese N/A | Honda K20A, F20C, Toyota 4A-GE 20V |
| European | BMW S54, Porsche Mezger, Ferrari F136 |
| Modern Turbo | Mercedes M139, BMW B58, VW EA888 |
| Brazilian | VW AP 1.8T, Fiat Firefly, Chevrolet 4.1 |
| Racing | F1 V10 (Ferrari 056), NASCAR SB2, Cosworth DFV |
| Diesel | Cummins 6BT, Duramax LML |

## Technical Foundation

All calculations reference published engineering literature:

### Books

1. Heywood, J.B. — *Internal Combustion Engine Fundamentals* (McGraw-Hill)
2. Blair, G.P. — *Design and Simulation of Four-Stroke Engines* (SAE)
3. Pulkrabek, W.W. — *Engineering Fundamentals of the ICE* (Pearson)
4. Bell, A.G. — *Four-Stroke Performance Tuning* (Haynes, 3rd ed.)
5. Baechtel, J. — *Competition Engine Building* (CarTech)
6. Vizard, D. — *How to Build Horsepower* (CarTech)

### Key SAE Papers

- SAE 2005-01-1688 — Camshaft Design and LSA
- SAE 2007-01-0148 — Dynamic Compression Ratio Analysis
- SAE 2003-01-0376 — Intake Runner Tuning
- SAE J1349 — Engine Power Test Code
- SAE J1832 — Fuel Injector Sizing

### RPM Estimation Model

The camshaft RPM range uses a **Mean Piston Speed (MPS)** model instead of simple linear correlation:

- MPS design speed interpolated from cam duration (Bell, Vizard empirical data)
- Peak RPM = `(MPS × 30000) / stroke_mm` (Heywood eq. 2.11)
- Validated against 6 real engines from stock Civic to F1 V10 (error ≤ 9%)

## Project Structure

```
EngineCalculator/
├── Core/                           # Calculation engines
│   ├── EngineCore                  # Basic calculations + MEP
│   ├── CompressionCalculator       # Compression analysis
│   ├── ValveTrainCalculator        # Valve train dynamics
│   ├── CamshaftCalculator          # Camshaft analysis + RPM model
│   ├── IntakeExhaustCalculator     # Intake & exhaust tuning
│   ├── FuelSystemCalculator        # Fuel system sizing
│   ├── AlternativeFuelsCalculator  # Alternative fuels
│   ├── EnginePresets + JSONDataLoader  # 81 presets from JSON
│   ├── EngineDataManager           # Central data (Singleton)
│   ├── EngineValidator             # 12 cross-validations
│   └── ProjectManager              # Save/Load (.ecproj JSON)
│
├── UI/
│   ├── MainWindow                  # Main window orchestrator
│   ├── GraphRenderer               # GDI+ graph component
│   ├── TabPages                    # Base classes (TabPage + NumericEdit)
│   ├── Managers/                   # Specialized managers
│   │   ├── MenuManager             # Menu + command handlers
│   │   ├── FileManager             # File I/O + report export
│   │   ├── TabManager              # Tab control + data propagation
│   │   ├── StatusBarManager        # Status bar + window title
│   │   └── SettingsManager         # Themes, Registry, auto-save
│   └── Tabs/                       # 8 independent tab modules
│
├── Data/
│   ├── Engines/                    # 9 JSON files, 81 engine presets
│   └── Components/                 # Component database (JSON)
│
├── docs/                           # Documentation
└── EngineCalculator.slnx            # Visual Studio solution
```

### Design Patterns

- **Singleton** — `EngineDataManager` (central project data)
- **Observer** — Automatic data sharing between tabs
- **Template Method** — `TabPage` base class (`Create`, `OnCalculate`, `OnClear`)
- **Delegation** — `MainWindow` → 5 specialized managers

## Documentation

| Document | Content |
|----------|---------|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Project structure, design patterns, data flow |
| [docs/FEATURES.md](docs/FEATURES.md) | Complete feature list with scientific references |
| [docs/CHANGELOG.md](docs/CHANGELOG.md) | Version history |
| [docs/FUEL_REFERENCES.md](docs/FUEL_REFERENCES.md) | Fuel properties database |
| [docs/MEP_REFERENCES.md](docs/MEP_REFERENCES.md) | Mean effective pressure references |

## Contributing

This is a personal project. Bug reports and feature suggestions are welcome via [Issues](https://github.com/GuilhermeCandiotto/EngineCalculator/issues).

## License

© 2026 Guilherme Candiotto — All rights reserved.

**Allowed:** personal, professional, and educational use.  
**Not allowed:** source code redistribution, resale, or credit removal.
