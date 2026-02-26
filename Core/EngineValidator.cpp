#define _USE_MATH_DEFINES
#include "EngineValidator.h"
#include "CompressionCalculator.h"
#include "CamshaftCalculator.h"
#include <sstream>
#include <iomanip>
#include <cmath>

EngineValidator::EngineValidator(const EngineProject* proj) 
    : project(proj) {
}

EngineValidator::~EngineValidator() {
}

ValidationResult EngineValidator::ValidateComplete() const {
    ValidationResult result;
    
    if (!project) return result;
    
    // Validacoes individuais
    ValidateBasicData(result);
    ValidateCompression(result);
    ValidateValveTrain(result);
    ValidateCamshaft(result);
    ValidateIntake(result);
    ValidateExhaust(result);
    
    // Validacoes cruzadas (so se dados basicos estao OK)
    if (project->IsBasicDataComplete()) {
        ValidatePistonToValveClearance(result);
        ValidateCompressionVsCamshaft(result);
        ValidateSpringVsRPM(result);
        ValidateIntakeVsExhaust(result);
        ValidateBoostVsCompression(result);
        ValidateValveSizes(result);
    }
    
    return result;
}

void EngineValidator::ValidateBasicData(ValidationResult& result) const {
if (project->basicData.bore <= 0.0) {
    result.AddIssue(ValidationIssue(
        ValidationSeverity::VALIDATION_ERROR,
        L"Dados Basicos",
        L"Diametro do cilindro nao definido",
        L"Insira o diametro do cilindro (bore) na aba Basica"
    ));
}
    
if (project->basicData.stroke <= 0.0) {
    result.AddIssue(ValidationIssue(
        ValidationSeverity::VALIDATION_ERROR,
        L"Dados Basicos",
        L"Curso do pistao nao definido",
        L"Insira o curso (stroke) na aba Basica"
    ));
}
    
    // Verifica relacao bore/stroke
    if (project->basicData.bore > 0 && project->basicData.stroke > 0) {
        double ratio = project->basicData.bore / project->basicData.stroke;
        
        if (ratio > 1.5) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::WARNING,
                L"Dados Basicos",
                L"Relacao bore/stroke muito alta (oversquare extremo)",
                L"Motor de alta rotacao - verifique se componentes suportam RPM elevado"
            ));
        } else if (ratio < 0.7) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::WARNING,
                L"Dados Basicos",
                L"Relacao bore/stroke muito baixa (undersquare extremo)",
                L"Motor de torque - velocidade do pistao pode ser limitante"
            ));
        }
    }
}

void EngineValidator::ValidateCompression(ValidationResult& result) const {
    // Implementacao simplificada
    if (project->compressionData.chamberVolume > 0 && project->IsBasicDataComplete()) {
        EngineCore engine;
        engine.SetBore(project->basicData.bore);
        engine.SetStroke(project->basicData.stroke);
        engine.SetCylinders(project->basicData.cylinders);
        
        CompressionCalculator calc(&engine);
        
        ::CompressionData data;
        data.chamberVolume = project->compressionData.chamberVolume;
        data.pistonDomeVolume = project->compressionData.pistonDomeVolume;
        data.gasketVolume = calc.CalculateGasketVolume(project->basicData.bore, 
                                                        project->compressionData.gasketThickness);
        data.deckVolume = calc.CalculateDeckVolume(project->basicData.bore, 
                                                     project->compressionData.deckHeight);
        data.valveRelief = project->compressionData.valveRelief;
        
        double cr = calc.CalculateStaticCompressionRatio(data);
        
        if (cr > 13.0 && project->intakeData.boostPressure == 0.0) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::WARNING,
                L"Compressao",
                L"Taxa de compressao muito alta para motor aspirado",
                L"CR > 13:1 pode causar detonacao. Use combustivel de alta octanagem ou reduza CR."
            ));
        }
    }
}

void EngineValidator::ValidatePistonToValveClearance(ValidationResult& result) const {
    if (project->camshaftData.intakeLift > 0 && project->compressionData.deckHeight >= 0) {
        // Estimativa simplificada de folga
        double maxLift = std::max(project->camshaftData.intakeLift, project->camshaftData.exhaustLift);
        double minClearance = 1.0 + (maxLift * 0.1); // 1mm + 10% do lift
        
        if (project->compressionData.deckHeight < minClearance) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::CRITICAL,
                L"Pistao-Valvula",
                L"PERIGO: Folga pistao-valvula insuficiente!",
                L"Deck height muito baixo para este lift. Pistao pode bater nas valvulas! Aumente deck height ou reduza lift."
            ));
        }
    }
}

void EngineValidator::ValidateCompressionVsCamshaft(ValidationResult& result) const {
    // Comando longo requer CR menor
    if (project->camshaftData.intakeDuration > 0 && project->compressionData.chamberVolume > 0) {
        double avgDuration = (project->camshaftData.intakeDuration + project->camshaftData.exhaustDuration) / 2.0;
        
        // Estima CR (simplificado)
        if (avgDuration > 260.0) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::WARNING,
                L"Comando vs Compressao",
                L"Comando muito longo pode exigir CR mais baixa",
                L"Comandos >260 graus tem overlap alto. Considere CR maxima de 9.5:1 para evitar detonacao."
            ));
        }
    }
}

void EngineValidator::ValidateSpringVsRPM(ValidationResult& result) const {
    if (project->valveTrainData.springRate > 0 && project->basicData.maxRPM > 0) {
        // Validacao simplificada de float
        if (project->basicData.maxRPM > 8000 && project->valveTrainData.springRate < 50.0) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::VALIDATION_ERROR,
                L"Molas",
                L"Molas muito fracas para RPM elevado",
                L"RPM >8000 requer molas mais fortes (>50 N/mm) para evitar float."
            ));
        }
    }
}

void EngineValidator::ValidateIntakeVsExhaust(ValidationResult& result) const {
    // Verifica coerencia entre RPM alvo de admissao e escape
    if (std::abs(project->intakeData.targetRPM - project->exhaustData.targetRPM) > 2000) {
        result.AddIssue(ValidationIssue(
            ValidationSeverity::WARNING,
            L"Admissao vs Escape",
            L"RPM alvo de admissao e escape muito diferentes",
            L"Para melhor performance, admissao e escape devem estar sintonizados para RPM similar."
        ));
    }
}

void EngineValidator::ValidateBoostVsCompression(ValidationResult& result) const {
    if (project->intakeData.boostPressure > 0 && project->compressionData.chamberVolume > 0) {
        // CR efetiva com boost
        if (project->intakeData.boostPressure > 10.0) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::WARNING,
                L"Boost vs Compressao",
                L"Boost alto (>10 PSI) exige CR baixa",
                L"Com boost >10 PSI, CR estatica deve ser <=9:1 para evitar detonacao."
            ));
        }
    }
}

void EngineValidator::ValidateValveSizes(ValidationResult& result) const {
    if (project->valveTrainData.intakeValveDiameter > 0 && project->basicData.bore > 0) {
        double ratio = project->valveTrainData.intakeValveDiameter / project->basicData.bore;
        
        if (ratio > 0.50) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::WARNING,
                L"Valvulas",
                L"Valvulas muito grandes para o bore",
                L"Valvulas >50% do bore podem causar problemas. Verifique compatibilidade."
            ));
        } else if (ratio < 0.35 && project->valveTrainData.numberOfIntakeValves <= 2) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::INFO,
                L"Valvulas",
                L"Valvulas pequenas - fluxo pode ser limitante",
                L"Para melhor performance, considere valvulas maiores (40-45% do bore)."
            ));
        }
    }
}

void EngineValidator::ValidateValveTrain(ValidationResult& result) const {
    // Valida dados do trem de valvulas
    if (project->valveTrainData.springRate > 0 && project->valveTrainData.maxLift > 0) {
        // Verifica margem de coil bind
        if (project->valveTrainData.springInstalledHeight > 0 && 
            project->valveTrainData.springCoilBindHeight > 0) {
            double compressed = project->valveTrainData.springInstalledHeight - project->valveTrainData.maxLift;
            double margin = compressed - project->valveTrainData.springCoilBindHeight;
            if (margin < 1.0) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::CRITICAL,
                    L"Trem de Valvulas",
                    L"PERIGO: Margem de coil bind insuficiente! Mola pode travar.",
                    L"Aumente altura instalada ou use mola com menor coil bind height."
                ));
            } else if (margin < 2.0) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::WARNING,
                    L"Trem de Valvulas",
                    L"Margem de coil bind baixa (<2mm)",
                    L"Considere molas com maior travel disponivel."
                ));
            }
        }

        // Verifica relacao lift/diametro (L/D)
        if (project->valveTrainData.intakeValveDiameter > 0) {
            double ld = project->valveTrainData.maxLift / project->valveTrainData.intakeValveDiameter;
            if (ld > 0.35) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::WARNING,
                    L"Trem de Valvulas",
                    L"Relacao L/D muito alta (>0.35) - Retornos decrescentes de fluxo",
                    L"Acima de L/D 0.30 o ganho de fluxo diminui. Considere valvulas maiores."
                ));
            }
        }
    }
}

void EngineValidator::ValidateCamshaft(ValidationResult& result) const {
    // Valida dados do comando
    if (project->camshaftData.intakeDuration > 0 && project->camshaftData.exhaustDuration > 0) {
        // Verifica se duracao de escape e razoavel vs admissao
        double durDiff = project->camshaftData.exhaustDuration - project->camshaftData.intakeDuration;
        if (std::abs(durDiff) > 20.0) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::INFO,
                L"Comando",
                L"Diferenca de duracao admissao/escape > 20 graus",
                L"Tipico: escape 5-15 graus menor que admissao. Verifique especificacoes."
            ));
        }

        // Verifica LSA fora de faixa razoavel
        if (project->camshaftData.lsa < 100.0 || project->camshaftData.lsa > 120.0) {
            result.AddIssue(ValidationIssue(
                ValidationSeverity::WARNING,
                L"Comando",
                L"LSA fora da faixa tipica (100-120 graus)",
                L"LSA < 100: overlap extremo. LSA > 120: perda de potencia. Faixa normal: 106-114."
            ));
        }

        // Verifica lift vs duracao (agressividade)
        if (project->camshaftData.intakeLift > 0) {
            double avgDuration = (project->camshaftData.intakeDuration + project->camshaftData.exhaustDuration) / 2.0;
            double avgLift = (project->camshaftData.intakeLift + project->camshaftData.exhaustLift) / 2.0;
            double ratio = avgDuration / avgLift;
            if (ratio < 18.0) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::WARNING,
                    L"Comando",
                    L"Perfil de came muito agressivo (duracao/lift < 18)",
                    L"Perfil agressivo requer molas mais fortes e componentes premium."
                ));
            }
        }
    }
}

void EngineValidator::ValidateIntake(ValidationResult& result) const {
    // Valida sistema de admissao
    if (project->intakeData.runnerDiameter > 0 && project->intakeData.targetRPM > 0) {
        // Verifica velocidade de fluxo estimada
        if (project->basicData.bore > 0 && project->basicData.stroke > 0) {
            double dispPerCyl = M_PI * (project->basicData.bore / 2.0) * (project->basicData.bore / 2.0) 
                                * project->basicData.stroke / 1000.0; // cc
            double area = M_PI * (project->intakeData.runnerDiameter / 2.0) * (project->intakeData.runnerDiameter / 2.0);
            double flowRate = (dispPerCyl * project->intakeData.targetRPM) / (120.0 * 1e6); // m^3/s
            double velocity = flowRate / (area / 1e6); // m/s

            if (velocity > 100.0) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::WARNING,
                    L"Admissao",
                    L"Velocidade de fluxo no runner muito alta (>100 m/s)",
                    L"Aumente o diametro do runner para reduzir restricao."
                ));
            } else if (velocity < 40.0) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::INFO,
                    L"Admissao",
                    L"Velocidade de fluxo no runner baixa (<40 m/s)",
                    L"Runner pode ser grande demais - perda de atomizacao e resposta."
                ));
            }
        }
    }
}

void EngineValidator::ValidateExhaust(ValidationResult& result) const {
    // Valida sistema de escape
    if (project->exhaustData.primaryDiameter > 0) {
        // Verifica diametro do primario vs cilindrada
        if (project->basicData.bore > 0 && project->basicData.stroke > 0) {
            double dispPerCyl = M_PI * (project->basicData.bore / 2.0) * (project->basicData.bore / 2.0) 
                                * project->basicData.stroke / 1000.0; // cc

            // Regra empirica: diametro primario ~ 1.5-2.2 * sqrt(disp_per_cyl / 25)
            double idealMin = 1.5 * sqrt(dispPerCyl / 25.0);
            double idealMax = 2.2 * sqrt(dispPerCyl / 25.0);

            if (project->exhaustData.primaryDiameter < idealMin) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::WARNING,
                    L"Escape",
                    L"Diametro do primario pode ser pequeno para esta cilindrada",
                    L"Primario restritivo limita potencia em alta rotacao."
                ));
            } else if (project->exhaustData.primaryDiameter > idealMax) {
                result.AddIssue(ValidationIssue(
                    ValidationSeverity::INFO,
                    L"Escape",
                    L"Diametro do primario grande - pode perder velocidade de gas",
                    L"Primario grande demais reduz scavenging e torque medio."
                ));
            }
        }
    }
}

ValidationResult EngineValidator::ValidateBasicDataOnly() const {
    ValidationResult result;
    ValidateBasicData(result);
    return result;
}

ValidationResult EngineValidator::ValidateCompressionOnly() const {
    ValidationResult result;
    ValidateCompression(result);
    return result;
}

ValidationResult EngineValidator::ValidateValveTrainOnly() const {
    ValidationResult result;
    ValidateValveTrain(result);
    return result;
}

std::wstring EngineValidator::GenerateReport(const ValidationResult& result) const {
    std::wostringstream report;
    
    report << L"=======================================\n";
    report << L"      RELATORIO DE VALIDACAO DO PROJETO\n";
    report << L"=======================================\n\n";
    
    report << L"Status: ";
    if (result.criticalCount > 0) {
        report << L"! CRITICO - NAO MONTAR!\n";
    } else if (result.errorCount > 0) {
        report << L"! ERRO - Corrigir antes de montar\n";
    } else if (result.warningCount > 0) {
        report << L"! AVISOS - Revisar recomendacoes\n";
    } else {
        report << L"OK - Sem problemas detectados\n";
    }
    
    report << L"\nResumo:\n";
    report << L"  Criticos: " << result.criticalCount << L"\n";
    report << L"  Erros: " << result.errorCount << L"\n";
    report << L"  Avisos: " << result.warningCount << L"\n";
    report << L"  Informacoes: " << result.infoCount << L"\n\n";
    
    if (!result.issues.empty()) {
        report << L"Detalhes:\n\n";
        
        for (const auto& issue : result.issues) {
            const wchar_t* icon = L"";
            switch (issue.severity) {
                case ValidationSeverity::CRITICAL: icon = L"! CRITICO"; break;
                case ValidationSeverity::VALIDATION_ERROR: icon = L"! ERRO"; break;
                case ValidationSeverity::WARNING: icon = L"! AVISO"; break;
                case ValidationSeverity::INFO: icon = L"i INFO"; break;
            }
            
            report << icon << L" [" << issue.category << L"]\n";
            report << L"  " << issue.message << L"\n";
            if (!issue.recommendation.empty()) {
                report << L"  -> " << issue.recommendation << L"\n";
            }
            report << L"\n";
        }
    }

    report << L"=======================================\n";

    return report.str();
}
