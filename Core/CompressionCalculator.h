#pragma once
// ============================================================================
// CompressionCalculator.h - Cálculos de Taxa de Compressão
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Referências:
// - SAE Paper 2007-01-0148 - Dynamic Compression Ratio
// - SAE Paper 2004-01-1604 - Piston Speed Effects
// ============================================================================
#include "EngineCore.h"
#include <cmath>

// Estrutura para dados de compressão
struct CompressionData {
    double chamberVolume;       // Volume da câmara de combustão (cc)
    double pistonDomeVolume;    // Volume da cúpula do pistão (cc) - negativo para côncavo
    double gasketVolume;        // Volume da junta (cc)
    double deckVolume;          // Volume do deck height (cc)
    double valveRelief;         // Volume dos rebaixos das válvulas (cc)
};

// Estrutura para taxa de compressão dinâmica
struct DynamicCompressionData {
    double staticCR;            // Taxa de compressão estática
    double intakeClosingAngle;  // Ângulo de fechamento da admissão (graus ABDC)
    double rodLength;           // Comprimento da biela (mm)
    double strokeLength;        // Curso (mm)
};

// Classe para cálculos de taxa de compressão
// Baseado em: Pulkrabek - "Engineering Fundamentals of the Internal Combustion Engine"
// e Technical Paper SAE 2007-01-0148
class CompressionCalculator {
private:
    const EngineCore* engine;
    
    // Calcula a posição do pistão em função do ângulo
    double CalculatePistonPosition(double angle, double rodLength, double stroke) const;
    
public:
    CompressionCalculator(const EngineCore* eng);
    ~CompressionCalculator();
    
    // Cálculos de volume
    double CalculateGasketVolume(double boreDiameter, double gasketThickness) const;
    double CalculateDeckVolume(double boreDiameter, double deckHeight) const;
    
    // Taxa de compressão estática
    // Fórmula: CR = (Vd + Vc) / Vc
    // Onde: Vd = volume deslocado, Vc = volume de compressão
    double CalculateStaticCompressionRatio(const CompressionData& data) const;
    
    // Taxa de compressão dinâmica (DCR)
    // Considera o fechamento real da válvula de admissão
    // Fórmula: DCR baseada no volume efetivo no momento do fechamento da admissão
    // Referência: "Design and Simulation of Four-Stroke Engines" - Gordon Blair
    double CalculateDynamicCompressionRatio(const DynamicCompressionData& data) const;

    // Calcula a pressão de cilindro (aproximação politrópica)
    // P2 = P1 * (V1/V2)^n onde n ~ 1.3 para compressão
    // P1 em kPa (101.325 = 1 atm), resultado em kPa
    double CalculateCylinderPressure(double compressionRatio, 
                                     double atmosphericPressure = 101.325,
                                     double polytropicIndex = 1.3) const;

    // Volume mínimo (clearance volume)
    double CalculateClearanceVolume(const CompressionData& data) const;
    
    // Validação
    bool IsValidCompressionData(const CompressionData& data) const;
    std::wstring GetCompressionAdvice(double cr) const;
};
