# Copilot Instructions

## Project Guidelines
- Engine Calculator is a standalone C++17 / Win32 API project.
- All engine calculations must reference published scientific literature (SAE Papers, Heywood, Blair, etc.).
- No hardcoded bore/stroke/cylinder values in calculation tabs — always read from `EngineDataManager::GetProject().basicData`.
- UI controls are dynamically sized based on window dimensions, not hardcoded DLU values.
- Follow existing code style: `EngineCore` pattern for calculators, `TabPage` pattern for UI tabs.