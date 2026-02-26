# Fuel References

Technical reference data used in the fuel system calculations.

## Gasoline (Baseline)

| Property | Value | Source |
|----------|-------|--------|
| Stoichiometric AFR | 14.7:1 | Heywood Table 3.4 |
| Density | 0.72 g/cc (SG) | ASTM D4814 |
| Octane (RON) | 87-93 | Pump grades |
| Energy Content | 44.0 MJ/kg | Heywood |
| Latent Heat | 305 kJ/kg | Heywood Table 3.3 |

## Alternative Fuels

| Fuel | AFR | Octane (RON) | Density (g/cc) | Energy (MJ/kg) | Source |
|------|-----|-------------|----------------|-----------------|--------|
| E85 | 9.8:1 | 105 | 0.78 | 29.2 | ASTM D5798 |
| E100 (Ethanol) | 9.0:1 | 109 | 0.79 | 26.8 | SAE J1297 |
| Methanol | 6.4:1 | 109 | 0.79 | 19.9 | Heywood |
| Nitromethane | 1.7:1 | — | 1.13 | 11.3 | NHRA |
| AvGas 100LL | 14.7:1 | 100 | 0.72 | 44.0 | ASTM D910, FAA |
| LPG (Propane) | 15.7:1 | 105 | 0.51 | 46.4 | ASTM D1835 |

## Race Fuels

| Fuel | Octane (MON/RON) | Density (g/cc) | Application | Source |
|------|-----------------|----------------|-------------|--------|
| VP Racing C16 | 117 MON | 0.73 | Drag, sprint | VP Racing |
| VP Racing Q16 | 116.5 MON | 0.72 | Drag, leaded | VP Racing |
| Sunoco 260 GT Plus | 104 RON | 0.73 | Road race | Sunoco |
| Sunoco Maximal | 116 MON | 0.72 | Drag, N/A | Sunoco |

## Nitrous Oxide (NOS)

| Parameter | Value | Source |
|-----------|-------|--------|
| O2 content by mass | 36% | Chemistry |
| Jet ratio (fuel:nitrous) | 1:6 to 1:8 | NOS/Holley |
| Max safe CR (50hp shot) | 10.5:1 | NOS Tech Manual |
| Max safe CR (150hp shot) | 9.5:1 | NOS Tech Manual |
| Activation RPM | > 3000 | Industry practice |

## Water/Methanol Injection

| Parameter | Value | Source |
|-----------|-------|--------|
| Typical mix ratio | 50/50 water/meth | AEM, Snow Performance |
| IAT reduction | 50-100°F | SAE 2008-01-0398 |
| Effective octane boost | 20-25 points | Industry data |
| Flow rate (per 100hp) | 100-150 cc/min | Snow Performance |

## BSFC Reference Values

| Engine Type | BSFC (lb/hp/hr) | BSFC (g/kWh) | Source |
|-------------|-----------------|--------------|--------|
| N/A gasoline (street) | 0.45-0.50 | 274-304 | Heywood Fig. 3.17 |
| N/A gasoline (race) | 0.40-0.45 | 243-274 | Baechtel |
| Turbocharged | 0.50-0.60 | 304-365 | Heywood |
| Diesel (DI) | 0.35-0.40 | 213-243 | Heywood |

## Injector Sizing

| Formula | Source |
|---------|--------|
| Flow (cc/min) = (HP × BSFC) / (AFR × duty% × injectors) | SAE J1832 |
| Max duty cycle (street) | 80% | RC Engineering |
| Max duty cycle (race) | 85% | RC Engineering |
| Pressure correction: Flow_new = Flow_base × √(P_new/P_base) | Bosch |

## Carburetor Sizing

| Formula | Source |
|---------|--------|
| CFM = (CID × RPM × VE%) / 3456 | Holley |
| Typical VE (street) | 80-85% | Pulkrabek |
| Typical VE (race) | 90-100% | Baechtel |

## Standards Referenced

- **ASTM D4814** — Standard Specification for Automotive Spark-Ignition Engine Fuel
- **ASTM D5798** — Standard Specification for Ethanol Fuel Blends
- **ASTM D910** — Standard Specification for Aviation Gasolines
- **ASTM D1835** — Standard Specification for Liquefied Petroleum Gases
- **EN 228** — European Automotive Fuels (Unleaded Petrol)
- **ANP (Brazil)** — Resolução ANP Nº 40/2013 (Gasolina Comum/Aditivada)
- **SAE J1832** — Low Pressure Gasoline Fuel Injector
- **SAE J1297** — Alternative Automotive Fuels
