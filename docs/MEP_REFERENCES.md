# MEP References

Mean Effective Pressure reference values used for engine classification and validation.

## Definitions

| MEP Type | Full Name | Description | Source |
|----------|-----------|-------------|--------|
| **BMEP** | Brake Mean Effective Pressure | Pressure from measured torque at flywheel | Heywood eq. 2.18 |
| **IMEP** | Indicated Mean Effective Pressure | Pressure from cylinder P-V diagram | Heywood eq. 2.16 |
| **FMEP** | Friction Mean Effective Pressure | Mechanical friction losses | Chen-Flynn (SAE 1980) |
| **PMEP** | Pumping Mean Effective Pressure | Gas exchange (intake/exhaust) losses | Heywood Ch. 13 |

## Formulas

```
BMEP [kPa] = (nR × 2π × T [Nm]) / Vd [L]
    where nR = 2 for 4-stroke, 1 for 2-stroke

IMEP = BMEP / η_mechanical
FMEP = IMEP - BMEP
PMEP = f(intake restriction, exhaust backpressure)
```

## BMEP Reference Values by Engine Type

| Engine Type | BMEP (kPa) | BMEP (bar) | Source |
|-------------|-----------|-----------|--------|
| Small N/A gasoline | 800-1000 | 8-10 | Heywood Table 2.2 |
| Performance N/A | 1100-1400 | 11-14 | Heywood |
| Turbocharged street | 1500-2000 | 15-20 | Heywood |
| Turbocharged race | 2000-2800 | 20-28 | SAE 2007-01-1464 |
| F1 N/A (V10 era) | 1350-1500 | 13.5-15 | SAE data |
| Diesel (DI turbo) | 1800-2400 | 18-24 | Heywood |
| Top Fuel | 5000-8000 | 50-80 | NHRA estimates |

## BMEP Classification Used in Engine Calculator

| Range (bar) | Classification |
|-------------|---------------|
| < 8 | Low — economy/utility engine |
| 8-11 | Moderate — typical street engine |
| 11-14 | High — performance N/A |
| 14-20 | Very High — forced induction street |
| > 20 | Extreme — race/competition |

## FMEP Model (Chen-Flynn Correlation)

```
FMEP [kPa] = C0 + C1·Pmax + C2·Vp + C3·Vp²
```

| Coefficient | Description | Typical Value | Source |
|-------------|-------------|--------------|--------|
| C0 | Base friction | 0.4-0.5 bar | Chen-Flynn (SAE 1980) |
| C1 | Peak pressure factor | 0.004-0.006 | Chen-Flynn |
| C2 | Linear speed factor | 0.09-0.12 bar·s/m | Chen-Flynn |
| C3 | Quadratic speed factor | 0.0008-0.0012 bar·s²/m² | Chen-Flynn |

Where Vp = mean piston speed (m/s), Pmax = peak cylinder pressure (bar).

## Mechanical Efficiency

| Engine Type | η_mechanical | Source |
|-------------|-------------|--------|
| Street (low RPM) | 85-90% | Heywood |
| Street (high RPM) | 75-85% | Heywood |
| Race engine | 88-92% | Baechtel |

## Validation Example

**Honda K20A Type R (2.0L N/A, 220hp @ 8000 RPM, 215 Nm):**
```
BMEP = (2 × 2π × 215) / 1.998 = 1351 kPa = 13.5 bar
Classification: High (performance N/A) ✓
```

## SAE Papers Referenced

- **SAE 1980** — Chen, Flynn — "Friction Correlation for IC Engines"
- **SAE 2007-01-1464** — "BMEP and Thermal Efficiency Analysis in SI Engines"
- **SAE J1349** — "Engine Power Test Code - Spark Ignition and Compression Ignition"
- **SAE 2004-01-1604** — "Friction Reduction in SI Engines"

## Textbook References

- Heywood, J.B. — *Internal Combustion Engine Fundamentals*, Ch. 2, 13 (McGraw-Hill)
- Pulkrabek, W.W. — *Engineering Fundamentals of the ICE*, Ch. 2 (Pearson)
- Blair, G.P. — *Design and Simulation of Four-Stroke Engines*, Ch. 1 (SAE)
