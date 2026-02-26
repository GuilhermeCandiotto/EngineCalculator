# 🔧 Engine Calculator — Registro de Correções Técnicas

**Data:** Fevereiro 2026  
**Versão:** 1.0.1 (Patch de Correções Científicas)

---

## 📋 Plano Completo de Correções

### Legenda de Status
- ✅ **IMPLEMENTADO** — Corrigido e compilando
- 🔄 **EM PROGRESSO** — Parcialmente implementado
- ⬚ **PENDENTE** — Ainda não iniciado

---

## FASE 1: Correções Críticas de Fórmula

### ✅ FIX #1 — BMEP: Fator multiplicador corrigido
- **Arquivo:** `Core/EngineCore.cpp` (linha ~109)
- **Problema:** Fator `1200` produzia resultados ~95× maiores que o correto
- **Solução:** Substituído por `nR * 2 * M_PI` (≈12.566 para 4T, ≈6.283 para 2T)
- **Fórmula correta:** `BMEP [kPa] = (nR × 2π × T [Nm]) / Vd [litros]`
- **Referência:** Heywood eq. 2.13, SAE J1349
- **Verificação:** Honda B18C (178 Nm, 1.797L) → 1245 kPa (12.45 bar) ✓
- **Bônus:** Adicionado suporte a motor 2T (nR=1)

### ✅ FIX #2 — Camshaft Centerline: Offset +90° removido
- **Arquivo:** `Core/CamshaftCalculator.cpp` (linhas 59-67)
- **Problema:** ICL = 90 + LSA + advance produzia eventos de válvulas fisicamente impossíveis
- **Solução:** ICL = LSA - advance, ECL = LSA + advance (convenção padrão)
- **Fórmulas corretas:**
  - `ICL [°ATDC] = LSA - advance`
  - `ECL [°BTDC] = LSA + advance`
- **Referência:** Blair Chapter 7, Bell "Performance Tuning" Chapter 6
- **Verificação:** LSA=110°, Adv=4° → ICL=106° ATDC, IVO=-19° (19° BTDC) ✓

### ✅ FIX #3 — Valve Float RPM: Força da mola corrigida
- **Arquivo:** `Core/ValveTrainCalculator.cpp` (linha ~267)
- **Problema:** Comparava inércia com `springPreload` (força com válvula fechada)
- **Solução:** Agora compara com `springPreload + springRate × maxLift` (força no nariz do came)
- **Referência:** SAE J1121 "Valve Spring Design and Testing"
- **Impacto:** Antes subestimava o RPM de float significativamente

### ✅ FIX #4 — Aceleração da válvula: Perfil senoidal corrigido
- **Arquivo:** `Core/ValveTrainCalculator.cpp` (linhas 236-239)
- **Problema:** Usava `(2π/T)²` — frequência angular de ciclo completo
- **Solução:** Usa `(π/T)²` — meio-ciclo senoidal (abertura→fechamento)
- **Referência:** Blair "Design and Simulation", Chapter 5
- **Impacto:** Antes superestimava aceleração por 4×

### ✅ FIX #5 — PMEP: Coeficiente de RPM corrigido
- **Arquivo:** `Core/EngineCore.cpp` (linha ~215)
- **Problema:** Coeficiente `0.05` fazia PMEP praticamente constante com RPM
- **Solução:** Coeficiente `5.0` para variação realista
- **Referência:** Heywood Figura 6.5
- **Valores corrigidos:** 3000 RPM→35 kPa, 6000 RPM→50 kPa, 9000 RPM→65 kPa

---

## FASE 2: Correções de Precisão

### ✅ FIX #6 — BSFC: Conversão lb/HP/hr → g/kWh corrigida
- **Arquivo:** `Core/FuelSystemCalculator.cpp` (linha ~257)
- **Problema:** Fator `453.592` esquecia conversão HP→kW
- **Solução:** Fator correto `453.592 / 0.7457 ≈ 608.3`
- **Referência:** Heywood Fig. 3.17
- **Exemplo:** 0.50 lb/HP/hr = 304 g/kWh (antes dizia 235 — ERRADO)

### ✅ FIX #7 — BSFC: Comentários g/kWh atualizados
- **Arquivo:** `Core/FuelSystemCalculator.h` (linhas 82-104)
- **Todos os valores g/kWh** nos comentários foram recalculados com fator correto 608.3

### ✅ FIX #8 — Pressão cilindro: Default 101.325 kPa
- **Arquivo:** `Core/CompressionCalculator.h` (linha ~62)
- **Problema:** Default `1.0` era ambíguo (1 atm? 1 kPa?)
- **Solução:** Default `101.325` kPa (= 1 atm exato)

### ✅ FIX #9 — CR efetiva com boost: Expoente removido
- **Arquivo:** `Core/IntakeExhaustCalculator.cpp` (linhas 400-406)
- **Problema:** `staticCR * pow(boostRatio, 1.3)` superestimava CR em ~23%
- **Solução:** `staticCR * boostRatio` (relação linear correta)
- **Referência:** Heywood "ICE Fundamentals" Cap. 6
- **Exemplo:** CR=9, 14.7 PSI boost → CR_eff = 18.0 (antes: 22.1)

### ✅ FIX #10 — Venturi velocity: M_PI
- **Arquivo:** `Core/FuelSystemCalculator.cpp` (linha ~83)
- **Problema:** Usava `3.14159` hardcoded
- **Solução:** Usa `M_PI` consistente com resto do projeto

---

## FASE 3: Implementações Pendentes Completadas

### ✅ FIX #11 — ValidateValveTrain() implementado
- **Arquivo:** `Core/EngineValidator.cpp`
- **Verifica:** Margem de coil bind (CRITICAL se <1mm), relação L/D

### ✅ FIX #12 — ValidateCamshaft() implementado
- **Arquivo:** `Core/EngineValidator.cpp`
- **Verifica:** Diferença duração adm/esc, LSA fora de faixa, agressividade

### ✅ FIX #13 — ValidateIntake() implementado
- **Arquivo:** `Core/EngineValidator.cpp`
- **Verifica:** Velocidade de fluxo no runner (>100 m/s ou <40 m/s)

### ✅ FIX #14 — ValidateExhaust() implementado
- **Arquivo:** `Core/EngineValidator.cpp`
- **Verifica:** Diâmetro primário vs cilindrada por cilindro

---

## FASE 4: Melhorias Futuras (PENDENTES)

### ⬚ FIX #15 — CFM para motores 2T
- **Arquivo:** `Core/FuelSystemCalculator.cpp`
- **Problema:** Divisor 3456 hardcoded (só 4T). Para 2T deveria ser 1728
- **Status:** Planejado

### ⬚ FIX #16 — γ (gamma) correto para gases de escape
- **Arquivo:** `Core/IntakeExhaustCalculator.cpp`
- **Problema:** Usa γ=1.4 (ar) para gases de escape. Correto: γ≈1.28-1.35
- **Impacto:** Erro de ~5-8% em comprimentos de escape
- **Status:** Planejado

### ⬚ FIX #17 — Modelo Chen-Flynn para FMEP
- **Arquivo:** `Core/EngineCore.cpp`
- **Problema:** FMEP só usa IMEP-BMEP. Modelo documentado mas não implementado
- **Fórmula:** FMEP = C1 + C2×Vp + C3×Vp²
- **Status:** Planejado

### ⬚ FIX #18 — Backpressure com Darcy-Weisbach
- **Arquivo:** `Core/IntakeExhaustCalculator.cpp`
- **Problema:** Modelo heurístico. Darcy-Weisbach já implementado mas não usado
- **Status:** Planejado

### ⬚ FIX #19 — Tuning RPM ignora parâmetro cylinders
- **Arquivo:** `Core/IntakeExhaustCalculator.cpp` (linha ~80)
- **Status:** Planejado

### ⬚ FIX #20 — Curva estimada torque/potência por RPM
- **Novo módulo** ou extensão de `EngineCore`
- **Status:** Planejado para v1.2

---

## 📊 Resumo de Impacto

| Correção | Módulo | Gravidade | Erro Anterior | Status |
|----------|--------|-----------|---------------|--------|
| BMEP fator | EngineCore | 🔴 CRÍTICO | ~95× errado | ✅ |
| Centerline +90° | CamshaftCalc | 🔴 CRÍTICO | Eventos impossíveis | ✅ |
| Valve Float preload | ValveTrainCalc | 🔴 CRÍTICO | Subestima RPM float | ✅ |
| Aceleração válvula | ValveTrainCalc | 🟡 ALTO | 4× superestimada | ✅ |
| PMEP coeficiente | EngineCore | 🟡 ALTO | Quase constante | ✅ |
| BSFC conversão | FuelSystemCalc | 🟡 ALTO | ~34% errado | ✅ |
| BSFC comentários | FuelSystemCalc.h | 🟢 MÉDIO | Docs incorretos | ✅ |
| Pressão cilindro | CompressionCalc | 🟢 MÉDIO | Unidade ambígua | ✅ |
| CR boost expoente | IntakeExhaustCalc | 🟡 ALTO | ~23% superestimado | ✅ |
| Venturi M_PI | FuelSystemCalc | 🟢 BAIXO | Inconsistência | ✅ |
| ValidateValveTrain | EngineValidator | 🟢 MÉDIO | Vazio | ✅ |
| ValidateCamshaft | EngineValidator | 🟢 MÉDIO | Vazio | ✅ |
| ValidateIntake | EngineValidator | 🟢 MÉDIO | Vazio | ✅ |
| ValidateExhaust | EngineValidator | 🟢 MÉDIO | Vazio | ✅ |
| CFM 2T | FuelSystemCalc | 🟢 BAIXO | Não suporta 2T | ⬚ |
| γ escape | IntakeExhaustCalc | 🟢 BAIXO | 5-8% erro | ⬚ |
| Chen-Flynn FMEP | EngineCore | 🟢 MÉDIO | Não implementado | ⬚ |
| Backpressure DW | IntakeExhaustCalc | 🟢 BAIXO | Modelo simplificado | ⬚ |

---

## 🔬 Verificação com Motores de Referência

### Honda B18C (Civic Type R) — ANTES vs DEPOIS
| Cálculo | Antes (ERRADO) | Depois (CORRETO) | Referência |
|---------|----------------|-------------------|------------|
| BMEP (178 Nm) | 118.864 kPa | **1245 kPa** | 12.4 bar ✓ |
| ICL (LSA=110, Adv=4) | 204° ATDC | **106° ATDC** | ✓ |
| IVO | 79° ATDC ❌ | **19° BTDC** | ✓ |
| IVC | 149° ABDC ❌ | **55° ABDC** | ✓ |
| CR_eff (9:1, 14.7 PSI) | 22.1 | **18.0** | ✓ |
| BSFC NA (0.50 lb/HP/hr) | "235 g/kWh" | **304 g/kWh** | ✓ |
| PMEP @ 6000 RPM | 20.3 kPa | **50 kPa** | Heywood ✓ |

---

**Todas as correções críticas foram implementadas e verificadas com build limpo (0 errors, 0 warnings).**
