#pragma once
// ============================================================================
// EngineCore.h - Núcleo de Cálculos do Motor
// Engine Calculator v1.0.0 - Fevereiro 2026
// ============================================================================
#include <cmath>
#include <string>

// Tipo de motor
enum class EngineType {
    FOUR_STROKE,
    TWO_STROKE
};

// Unidades de medida
enum class UnitSystem {
    METRIC,     // mm, cc, kg
    IMPERIAL    // in, ci, lb
};

// Classe principal para cálculos fundamentais do motor
class EngineCore {
private:
    double bore;            // Diâmetro do cilindro (mm)
    double stroke;          // Curso do pistão (mm)
    int cylinders;          // Número de cilindros
    EngineType type;        // 4 tempos ou 2 tempos
    double deckHeight;      // Altura do pistão ao topo do bloco (mm)
    
public:
    EngineCore();
    ~EngineCore();
    
    // Setters
    void SetBore(double value) { bore = value; }
    void SetStroke(double value) { stroke = value; }
    void SetCylinders(int value) { cylinders = value; }
    void SetEngineType(EngineType value) { type = value; }
    void SetDeckHeight(double value) { deckHeight = value; }
    
    // Getters
    double GetBore() const { return bore; }
    double GetStroke() const { return stroke; }
    int GetCylinders() const { return cylinders; }
    EngineType GetEngineType() const { return type; }
    double GetDeckHeight() const { return deckHeight; }
    
    // Cálculos fundamentais
    double CalculateBoreArea() const;
    double CalculateDisplacement() const;
    double CalculateBoreStrokeRatio() const;
    double CalculateRodStrokeRatio(double rodLength) const;
    double CalculatePistonSpeed(double rpm) const;
    
    // Cálculos de MEP (Mean Effective Pressure)
    // Baseado em: Heywood "ICE Fundamentals" Cap. 2.6-2.7
    double CalculateBMEP(double torqueNm, double displacementLiters) const;
    double CalculateIMEP(double bmep, double mechanicalEfficiency = 0.85) const;
    double CalculateFMEP(double imep, double bmep) const;
    double CalculatePMEP(double rpm, double throttlePosition = 1.0) const;
    
    // Análise e classificação
    std::wstring ClassifyBMEP(double bmep) const;
    double GetTypicalBMEP(const std::wstring& engineType) const;
    
    // Validação
    bool IsValid() const;
    std::wstring GetValidationError() const;
};
