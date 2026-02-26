#define _USE_MATH_DEFINES
#include "CompressionCalculator.h"
#include <cmath>
#include <sstream>

CompressionCalculator::CompressionCalculator(const EngineCore* eng) 
    : engine(eng) {
}

CompressionCalculator::~CompressionCalculator() {
}

// Calcula o volume da junta do cabeçote
// V = pi * (bore/2)² * thickness
double CompressionCalculator::CalculateGasketVolume(double boreDiameter, double gasketThickness) const {
    if (boreDiameter <= 0.0 || gasketThickness <= 0.0) return 0.0;
    
    double radius = boreDiameter / 2.0;
    double area = M_PI * radius * radius;
    return area * gasketThickness / 1000.0; // mm³ para cc
}

// Calcula o volume do deck height
// V = pi * (bore/2)² * deck_height
double CompressionCalculator::CalculateDeckVolume(double boreDiameter, double deckHeight) const {
    if (boreDiameter <= 0.0 || deckHeight < 0.0) return 0.0;
    
    double radius = boreDiameter / 2.0;
    double area = M_PI * radius * radius;
    return area * deckHeight / 1000.0; // mm³ para cc
}

// Calcula o volume de folga (clearance volume)
// Vc = Vchamber + Vpiston + Vgasket + Vdeck + Vvalve_relief
double CompressionCalculator::CalculateClearanceVolume(const CompressionData& data) const {
    return data.chamberVolume + 
           data.pistonDomeVolume + 
           data.gasketVolume + 
           data.deckVolume + 
           data.valveRelief;
}

// Calcula a taxa de compressão estática
// CR = (Vd + Vc) / Vc
// Referência: Pulkrabek, "Engineering Fundamentals of ICE", Chapter 2
double CompressionCalculator::CalculateStaticCompressionRatio(const CompressionData& data) const {
    if (!engine || !engine->IsValid()) return 0.0;
    
    // Volume deslocado por cilindro
    double displacementPerCylinder = engine->CalculateDisplacement() / engine->GetCylinders();
    
    // Volume de folga total
    double clearanceVolume = CalculateClearanceVolume(data);
    
    if (clearanceVolume <= 0.0) return 0.0;
    
    // Taxa de compressão
    return (displacementPerCylinder + clearanceVolume) / clearanceVolume;
}

// Calcula a posição do pistão em relação ao ângulo do virabrequim
// Usando a equação cinemática da biela-manivela
// x = r*cos(theta) + sqrt(L² - r^2*sin^2(theta))
// Referência: "Design and Simulation of Four-Stroke Engines" - Gordon Blair
double CompressionCalculator::CalculatePistonPosition(double angle, double rodLength, double stroke) const {
    double angleRad = angle * M_PI / 180.0;
    double crankRadius = stroke / 2.0;
    double sinAngle = sin(angleRad);
    
    double term = rodLength * rodLength - crankRadius * crankRadius * sinAngle * sinAngle;
    if (term < 0.0) term = 0.0;
    
    return crankRadius * cos(angleRad) + sqrt(term);
}

// Calcula a taxa de compressão dinâmica (DCR)
// Considera que a compressão efetiva só começa quando a válvula de admissão fecha
// Referência: SAE Paper 2007-01-0148
double CompressionCalculator::CalculateDynamicCompressionRatio(const DynamicCompressionData& data) const {
    if (data.strokeLength <= 0.0 || data.rodLength <= 0.0) return 0.0;
    
    // Posição do pistão no PMS (0 graus)
    double positionTDC = CalculatePistonPosition(0.0, data.rodLength, data.strokeLength);
    
    // Posição do pistão no fechamento da admissão (ABDC - After Bottom Dead Center)
    double positionIVC = CalculatePistonPosition(180.0 + data.intakeClosingAngle, 
                                                  data.rodLength, data.strokeLength);
    
    // Volume efetivo capturado
    double effectiveStroke = positionTDC - positionIVC;
    
    // Razão de volume efetivo
    double volumeRatio = data.strokeLength / effectiveStroke;
    
    // DCR aproximado
    // Formula simplificada: DCR = 1 + (SCR - 1) * (Ve/Vd)
    double dcr = 1.0 + (data.staticCR - 1.0) / volumeRatio;
    
    return dcr;
}

// Calcula a pressão máxima do cilindro usando processo politrópico
// P2 = P1 * (V1/V2)^n
// n = índice politrópico (1.3 para compressão sem combustão)
// Referência: Heywood, "Internal Combustion Engine Fundamentals"
double CompressionCalculator::CalculateCylinderPressure(double compressionRatio, 
                                                         double atmosphericPressure,
                                                         double polytropicIndex) const {
    if (compressionRatio <= 0.0 || atmosphericPressure <= 0.0) return 0.0;
    
    return atmosphericPressure * pow(compressionRatio, polytropicIndex);
}

bool CompressionCalculator::IsValidCompressionData(const CompressionData& data) const {
    return data.chamberVolume > 0.0 && 
           data.gasketVolume >= 0.0 && 
           data.deckVolume >= 0.0;
}

// Retorna conselhos baseado na taxa de compressão
std::wstring CompressionCalculator::GetCompressionAdvice(double cr) const {
    if (cr <= 0.0) return L"Taxa de compressão inválida";
    
    if (cr < 7.0) return L"Muito baixa - Perda de eficiência";
    else if (cr >= 7.0 && cr < 9.0) return L"Adequada para motores de baixa octanagem";
    else if (cr >= 9.0 && cr < 11.0) return L"Ótima para gasolina comum (octanagem 91-95)";
    else if (cr >= 11.0 && cr < 13.0) return L"Requer gasolina premium (octanagem 98+) - Possível detonação";
    else if (cr >= 13.0 && cr < 15.0) return L"Alto desempenho - Requer combustível de competição (etanol/metanol)";
    else return L"Extremamente alta - Apenas para motores de corrida com combustível especial";
}
