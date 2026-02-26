# Funcionalidades

## Cálculos Disponíveis

### Básico
Cilindrada total/unitária, relação bore/stroke, velocidade média do pistão, área do pistão.
MEP: BMEP, IMEP, FMEP, PMEP com classificação de eficiência.

### Compressão
CR estática e dinâmica (com IVC), volumes (câmara, junta, deck, dome/dish),
pressão estimada no cilindro, recomendações automáticas de combustível.

### Trem de Válvulas
Dimensionamento ideal, análise de fluxo, cálculo de molas (força, frequência natural, coil bind),
dinâmica de válvulas (aceleração, inércia), estimativa de RPM de float.

### Comando de Válvulas
Eventos IVO/IVC/EVO/EVC, overlap, LSA, faixa de RPM ideal, índice de agressividade.

### Admissão
Ressonância de Helmholtz, comprimento de runners, volume de plenum,
velocity stacks, análise de boost, comparação ITB vs Plenum.

### Escape
Comprimento de primários (reflexão de ondas), análise de pulsos,
comparação 4-1 vs 4-2-1, eficiência de scavenging, backpressure.

### Combustível
Carburação (CFM, jatos, venturi), injeção (vazão, duty cycle, pressão),
BSFC, consumo (L/h, km/L, MPG).

### Combustíveis Alternativos
- **NOS:** jet size, ganho HP, CR máxima, sistemas Dry/Wet/Direct Port
- **Nitrometano:** AFR, CR suportada, especificações Top Fuel
- **Metanol:** AFR, octanagem, latent heat
- **Water/Meth Injection:** vazão, redução IAT, boost threshold
- **Corrida:** VP Racing, Sunoco, AvGas, E85, Etanol E100, LPG

### Validação Cruzada (12 tipos)
Folga pistão-válvula, CR vs comando, molas vs RPM, boost vs CR,
sintonia admissão/escape, entre outros.
Alertas automáticos na status bar ao trocar de aba.

### Exportação
- **Ctrl+E** — Exporta relatório `.txt` completo (dados + validação)

---

## Interface

- **Layout responsivo:** 800x600 até 4K
- **Temas:** Light/Dark funcional (labels, edits, gráficos, fundo)
- **Atalhos:** Ctrl+N/O/S/E/Q, F5
- **Tooltips educacionais** em todos os campos
- **Projetos recentes** (últimos 5)
- **Auto-save** a cada 5 minutos
- **Unidades:** Métrico ↔ Imperial com checkmarks visuais
- **Gráficos GDI+** em todas as 8 abas com anti-aliasing
- **Botões estilizados** com cores temáticas

---

## Referências Bibliográficas

### Livros
1. Blair, G.P. — "Design and Simulation of Four-Stroke Engines" (SAE)
2. Blair, G.P. — "Design and Simulation of Two-Stroke Engines" (SAE)
3. Heywood, J.B. — "Internal Combustion Engine Fundamentals" (McGraw-Hill)
4. Pulkrabek, W.W. — "Engineering Fundamentals of the ICE" (Pearson)
5. Bell, A.G. — "Four-Stroke Performance Tuning" (Haynes)
6. Baechtel, J. — "Competition Engine Building" (CarTech)

### SAE Papers (18)
SAE 2001-01-0662, 2003-01-0001, 2004-01-1604, 2005-01-1688, 2007-01-0148,
J1121, J1832, 890243, 2008-01-0398, 2003-01-3242, 2005-01-0547,
2004-01-3514, 2002-01-2743, 2006-01-3334, J1349, 2007-01-1464,
2002-01-0483, 1980 (Chen-Flynn)

### Outras Fontes
Holley, Bosch, NHRA, VP Racing, Sunoco, ASTM (D4814, D5798, D910, D1835),
ANP Brasil, EN 228, FAA.

Detalhes completos em `FUEL_REFERENCES.md` e `MEP_REFERENCES.md`.
