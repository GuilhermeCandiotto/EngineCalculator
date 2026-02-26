#pragma once
// ============================================================================
// EngineValidator.h - Sistema de Validação Cruzada de Projetos
// Engine Calculator v1.0.0 - Fevereiro 2026
// 
// Valida compatibilidade entre componentes e previne erros críticos
// (ex: folga pistão-válvula, CR vs comando, molas vs RPM, etc.)
// ============================================================================
#include "EngineDataManager.h"
#include <vector>
#include <string>

// Severidade do problema
enum class ValidationSeverity {
    INFO,       // Informação
    WARNING,    // Aviso (não crítico)
    VALIDATION_ERROR,      // Erro (problema sério)
    CRITICAL    // Crítico (pode causar dano ao motor)
};

// Resultado de validação
struct ValidationIssue {
    ValidationSeverity severity;
    std::wstring category;      // Ex: "Compressão", "Válvulas", etc.
    std::wstring message;
    std::wstring recommendation;
    
    ValidationIssue(ValidationSeverity sev, const std::wstring& cat, 
                   const std::wstring& msg, const std::wstring& rec = L"")
        : severity(sev), category(cat), message(msg), recommendation(rec) {}
};

// Resultado completo da validação
struct ValidationResult {
    std::vector<ValidationIssue> issues;
    bool hasErrors;
    bool hasWarnings;
    int criticalCount;
    int errorCount;
    int warningCount;
    int infoCount;
    
    ValidationResult() 
        : hasErrors(false), hasWarnings(false), 
          criticalCount(0), errorCount(0), warningCount(0), infoCount(0) {}
    
    void AddIssue(const ValidationIssue& issue) {
        issues.push_back(issue);
        
        switch (issue.severity) {
            case ValidationSeverity::CRITICAL:
                criticalCount++;
                hasErrors = true;
                break;
            case ValidationSeverity::VALIDATION_ERROR:
                errorCount++;
                hasErrors = true;
                break;
            case ValidationSeverity::WARNING:
                warningCount++;
                hasWarnings = true;
                break;
            case ValidationSeverity::INFO:
                infoCount++;
                break;
        }
    }
    
    bool IsOK() const {
        return !hasErrors;
    }
};

// Validador de projeto completo
class EngineValidator {
private:
    const EngineProject* project;
    
    // Validações específicas
    void ValidateBasicData(ValidationResult& result) const;
    void ValidateCompression(ValidationResult& result) const;
    void ValidateValveTrain(ValidationResult& result) const;
    void ValidateCamshaft(ValidationResult& result) const;
    void ValidateIntake(ValidationResult& result) const;
    void ValidateExhaust(ValidationResult& result) const;
    
    // Validações cruzadas
    void ValidatePistonToValveClearance(ValidationResult& result) const;
    void ValidateCompressionVsCamshaft(ValidationResult& result) const;
    void ValidateSpringVsRPM(ValidationResult& result) const;
    void ValidateIntakeVsExhaust(ValidationResult& result) const;
    void ValidateBoostVsCompression(ValidationResult& result) const;
    void ValidateValveSizes(ValidationResult& result) const;
    
public:
    EngineValidator(const EngineProject* proj);
    ~EngineValidator();
    
    // Validação completa
    ValidationResult ValidateComplete() const;
    
    // Validações parciais
    ValidationResult ValidateBasicDataOnly() const;
    ValidationResult ValidateCompressionOnly() const;
    ValidationResult ValidateValveTrainOnly() const;
    
    // Gerar relatório formatado
    std::wstring GenerateReport(const ValidationResult& result) const;
};
