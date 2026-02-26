# Changelog

## v1.1.0 — Fevereiro 2026 (Atual)

### Correções Críticas
- **Dados hardcoded removidos:** Todas as tabs (Camshaft, Exhaust, Intake, ValveTrain, Compression) agora usam bore/stroke/cilindros reais do projeto em vez de valores fixos (80mm/80mm/4cil). Isso causava cálculos incorretos de RPM, runners, primários e molas para qualquer motor diferente do default.
- **Fórmula de RPM do comando reescrita:** Antiga fórmula linear (`duration × 25`) substituída por modelo baseado em MPS (Mean Piston Speed) conforme Heywood eq. 2.11 e Bell Ch.5. Agora escala corretamente de motores de rua (~5000 RPM) até F1 (~19000 RPM).
- **Carregamento de presets JSON:** Resolvido problema de path — executável em `x64\Debug\` não encontrava `Data\Engines\`. Busca agora percorre até 3 níveis acima com `GetFullPathNameW` e prioriza diretório com mais JSONs.

### Novos Recursos
- **Dark Theme funcional:** `WM_CTLCOLORSTATIC/EDIT` tratados em `TabPageProc` e `MainWindow`. Labels, edits, fundo das tabs e gráficos GDI+ respondem ao tema.
- **Exportar Relatório (Ctrl+E):** Gera `.txt` com dados de todas as seções + validação completa. Abre automaticamente no editor.
- **Validação na troca de aba:** Status bar mostra alertas ao navegar entre abas (ex: "⚠ 2 alerta(s) critico(s)").
- **Formatação numérica:** `FmtVal()` elimina zeros à direita (`272.000000` → `272`, `3.50` → `3.5`).
- **Unidades Métrico/Imperial:** Flag global `TabPage::IsImperial()` com troca via menu Exibir.

### Melhorias de UI
- Botões estilizados com cores (Calcular=azul, Limpar=cinza, Validar=verde)
- Fonte Consolas 14px nos resultados, Segoe UI 15px nos inputs
- Gráficos GDI+ em todas as 8 abas com auto-sync de tema

---

## v1.0.0 — Fevereiro 2026

### Refatoração FASE 1 — Tabs Modulares
- Separação de `TabPages.cpp` (2351 linhas) em 8 módulos independentes
- Criação de `UI/Tabs/` com estrutura modular
- `TabPages.cpp` reduzido para ~315 linhas (classes base)
- Compilação incremental 3-4x mais rápida

### Refatoração FASE 2 — Managers
- Separação de `MainWindow.cpp` em 5 managers especializados
- `MenuManager`, `FileManager`, `TabManager`, `StatusBarManager`, `SettingsManager`
- Todos os handlers implementados (zero TODOs)

### Novos Recursos
- Sistema de Presets de Motores (81 motores)
- Cálculos de MEP (BMEP, IMEP, FMEP, PMEP)
- Combustíveis Alternativos (NOS, Nitrometano, Metanol, Water/Meth)
- Combustíveis de Corrida (VP Racing, Sunoco, AvGas, E85)
- Sistema de Unidades Métrico/Imperial
- Atalhos de Teclado (Ctrl+N/O/S/Q, F5)
- Data Sharing entre abas automático
- Auto-save a cada 5 minutos
- Histórico de projetos recentes (5 últimos)

---

## v0.9.0 — Janeiro 2026
- Sistema de Validação Cruzada (12 tipos)
- Interface responsiva e temas Light/Dark
- Salvamento/Carregamento de projetos (JSON)
- Persistência de configurações no Registry
- 6 abas funcionais

---

## v0.8.0 — Dezembro 2025
- Sistema de Combustível completo (Carburação + Injeção)
- Tooltips educacionais em todos os campos

---

## v0.7.0 — Novembro 2025
- Calculadoras Core implementadas
- Interface Win32 básica
- Primeira versão funcional
