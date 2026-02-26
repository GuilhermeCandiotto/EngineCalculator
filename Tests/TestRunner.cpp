// ============================================================================
// TestRunner.cpp - Testes basicos dos calculadores Core
// Engine Calculator
//
// Compilar: cl /EHsc /std:c++17 /I. Tests\TestRunner.cpp Core\EngineCore.cpp Core\CompressionCalculator.cpp /Fe:Tests\TestRunner.exe
// Executar: Tests\TestRunner.exe
// ============================================================================
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "../Core/EngineCore.h"
#include "../Core/CompressionCalculator.h"

static int testsRun = 0;
static int testsPassed = 0;
static int testsFailed = 0;

static bool ApproxEqual(double a, double b, double tol) {
    if (b == 0.0) return fabs(a) < tol;
    return fabs((a - b) / b) < tol;
}

static void Check(const char* name, bool ok, double got = 0, double expected = 0) {
    testsRun++;
    if (ok) {
        testsPassed++;
        printf("  [PASS] %s\n", name);
    } else {
        testsFailed++;
        printf("  [FAIL] %s  (got %.6f, expected %.6f)\n", name, got, expected);
    }
}

// ============================================================================
// EngineCore
// ============================================================================

void TestEngineCore() {
    printf("\n=== EngineCore ===\n");
    
    // Honda B18C: bore=81mm, stroke=87.2mm, 4 cil -> 1797cc
    {
        EngineCore e;
        e.SetBore(81.0); e.SetStroke(87.2); e.SetCylinders(4);
        double d = e.CalculateDisplacement();
        Check("B18C displacement ~1797cc", ApproxEqual(d, 1797.0, 0.01), d, 1797.0);
    }
    
    // Toyota 2JZ: bore=86mm, stroke=86mm, 6 cil -> 2997cc
    {
        EngineCore e;
        e.SetBore(86.0); e.SetStroke(86.0); e.SetCylinders(6);
        double d = e.CalculateDisplacement();
        Check("2JZ displacement ~2997cc", ApproxEqual(d, 2997.0, 0.01), d, 2997.0);
    }
    
    // BMW S54: bore=87mm, stroke=91mm, 6 cil -> 3246cc
    {
        EngineCore e;
        e.SetBore(87.0); e.SetStroke(91.0); e.SetCylinders(6);
        double d = e.CalculateDisplacement();
        Check("S54 displacement ~3246cc", ApproxEqual(d, 3246.0, 0.01), d, 3246.0);
    }
    
    // Bore area: pi * (86/2)^2
    {
        EngineCore e;
        e.SetBore(86.0);
        double area = e.CalculateBoreArea();
        double exp = M_PI * 43.0 * 43.0;
        Check("Bore area 86mm", ApproxEqual(area, exp, 0.001), area, exp);
    }
    
    // Bore/stroke ratio square
    {
        EngineCore e;
        e.SetBore(86.0); e.SetStroke(86.0);
        Check("B/S ratio square = 1.0", ApproxEqual(e.CalculateBoreStrokeRatio(), 1.0, 0.001),
              e.CalculateBoreStrokeRatio(), 1.0);
    }
    
    // Bore/stroke ratio oversquare
    {
        EngineCore e;
        e.SetBore(100.0); e.SetStroke(75.0);
        double r = e.CalculateBoreStrokeRatio();
        Check("B/S ratio oversquare 100/75", ApproxEqual(r, 1.333, 0.01), r, 1.333);
    }
    
    // Piston speed: Vp = 2 * stroke * rpm / 60000
    {
        EngineCore e;
        e.SetBore(86.0); e.SetStroke(86.0);
        double v = e.CalculatePistonSpeed(7000.0);
        double exp = 2.0 * 86.0 * 7000.0 / 60000.0;
        Check("Piston speed 86mm@7000rpm", ApproxEqual(v, exp, 0.001), v, exp);
    }
    
    // Rod/stroke ratio
    {
        EngineCore e;
        e.SetStroke(86.0);
        double r = e.CalculateRodStrokeRatio(150.0);
        Check("Rod/stroke 150/86", ApproxEqual(r, 1.7442, 0.01), r, 1.7442);
    }
    
    // Validation
    {
        EngineCore e;
        e.SetBore(86.0); e.SetStroke(86.0); e.SetCylinders(4);
        Check("Valid engine", e.IsValid());
    }
    {
        EngineCore e;
        e.SetBore(0.0); e.SetStroke(86.0); e.SetCylinders(4);
        Check("Invalid engine (zero bore)", !e.IsValid());
    }
    
    // Edge: displacement with zero bore
    {
        EngineCore e;
        e.SetBore(0.0); e.SetStroke(86.0); e.SetCylinders(4);
        Check("Displacement zero bore = 0", e.CalculateDisplacement() == 0.0);
    }
    
    // BMEP: (Torque * 4 * pi) / Vd_liters
    {
        EngineCore e;
        double bmep = e.CalculateBMEP(200.0, 2.0);
        double exp = (200.0 * 4.0 * M_PI) / 2.0;
        Check("BMEP 200Nm 2.0L", ApproxEqual(bmep, exp, 0.01), bmep, exp);
    }
    
    // Piston speed edge: zero rpm
    {
        EngineCore e;
        e.SetStroke(86.0);
        Check("Piston speed zero rpm = 0", e.CalculatePistonSpeed(0.0) == 0.0);
    }
}

// ============================================================================
// CompressionCalculator
// ============================================================================

void TestCompressionCalculator() {
    printf("\n=== CompressionCalculator ===\n");
    
    // Gasket volume: pi * (bore/2)^2 * thick / 1000
    {
        EngineCore e;
        e.SetBore(86.0); e.SetStroke(86.0); e.SetCylinders(4);
        CompressionCalculator calc(&e);
        double v = calc.CalculateGasketVolume(86.0, 1.2);
        double exp = M_PI * 43.0 * 43.0 * 1.2 / 1000.0;
        Check("Gasket volume 86mm 1.2mm", ApproxEqual(v, exp, 0.001), v, exp);
    }
    
    // Deck volume
    {
        EngineCore e;
        CompressionCalculator calc(&e);
        double v = calc.CalculateDeckVolume(86.0, 0.5);
        double exp = M_PI * 43.0 * 43.0 * 0.5 / 1000.0;
        Check("Deck volume 86mm 0.5mm", ApproxEqual(v, exp, 0.001), v, exp);
    }
    
    // Static CR = (Vd + Vc) / Vc -> set Vc so CR = 11.0
    {
        EngineCore e;
        e.SetBore(86.0); e.SetStroke(86.0); e.SetCylinders(4);
        CompressionCalculator calc(&e);
        
        double dispPerCyl = e.CalculateDisplacement() / 4.0;
        double targetCR = 11.0;
        double clearance = dispPerCyl / (targetCR - 1.0);
        
        CompressionData data = {};
        data.chamberVolume = clearance;
        
        double cr = calc.CalculateStaticCompressionRatio(data);
        Check("Static CR = 11.0", ApproxEqual(cr, targetCR, 0.01), cr, targetCR);
    }
    
    // Clearance volume = sum
    {
        EngineCore e;
        CompressionCalculator calc(&e);
        CompressionData data = {};
        data.chamberVolume = 40.0;
        data.pistonDomeVolume = -3.0;
        data.gasketVolume = 5.0;
        data.deckVolume = 2.0;
        data.valveRelief = 1.0;
        
        double vc = calc.CalculateClearanceVolume(data);
        Check("Clearance volume sum = 45", ApproxEqual(vc, 45.0, 0.001), vc, 45.0);
    }
    
    // Cylinder pressure: P2 = P1 * CR^n
    {
        EngineCore e;
        CompressionCalculator calc(&e);
        double p = calc.CalculateCylinderPressure(10.0, 101.325, 1.3);
        double exp = 101.325 * pow(10.0, 1.3);
        Check("Cyl pressure CR=10", ApproxEqual(p, exp, 0.01), p, exp);
    }
    
    // Edge: zero bore gasket
    {
        EngineCore e;
        CompressionCalculator calc(&e);
        Check("Gasket vol zero bore = 0", calc.CalculateGasketVolume(0.0, 1.2) == 0.0);
    }
    
    // Edge: zero CR pressure
    {
        EngineCore e;
        CompressionCalculator calc(&e);
        Check("Cyl pressure zero CR = 0", calc.CalculateCylinderPressure(0.0) == 0.0);
    }
}

// ============================================================================
// Main
// ============================================================================

int main() {
    printf("========================================\n");
    printf("  Engine Calculator - Test Runner\n");
    printf("========================================\n");
    
    TestEngineCore();
    TestCompressionCalculator();
    
    printf("\n========================================\n");
    printf("  Resultado: %d/%d testes passaram\n", testsPassed, testsRun);
    
    if (testsFailed == 0) {
        printf("  TODOS OS TESTES PASSARAM!\n");
    } else {
        printf("  %d TESTE(S) FALHARAM!\n", testsFailed);
    }
    printf("========================================\n");
    
    return testsFailed > 0 ? 1 : 0;
}
