#include "ProjectManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>

ProjectManager::ProjectManager() {
}

ProjectManager::~ProjectManager() {
}

std::wstring ProjectManager::EscapeJSON(const std::wstring& str) const {
    std::wostringstream escaped;
    for (wchar_t c : str) {
        switch (c) {
            case L'\"': escaped << L"\\\""; break;
            case L'\\': escaped << L"\\\\"; break;
            case L'\n': escaped << L"\\n"; break;
            case L'\r': escaped << L"\\r"; break;
            case L'\t': escaped << L"\\t"; break;
            default: escaped << c; break;
        }
    }
    return escaped.str();
}

std::wstring ProjectManager::UnescapeJSON(const std::wstring& str) const {
    std::wostringstream unescaped;
    bool escaping = false;
    
    for (wchar_t c : str) {
        if (escaping) {
            switch (c) {
                case L'\"': unescaped << L'\"'; break;
                case L'\\': unescaped << L'\\'; break;
                case L'n': unescaped << L'\n'; break;
                case L'r': unescaped << L'\r'; break;
                case L't': unescaped << L'\t'; break;
                default: unescaped << c; break;
            }
            escaping = false;
        } else {
            if (c == L'\\') {
                escaping = true;
            } else {
                unescaped << c;
            }
        }
    }
    
    return unescaped.str();
}

std::wstring ProjectManager::ProjectToJSON(const EngineProject& project) const {
std::wostringstream json;
json << std::fixed << std::setprecision(2);
    
json << L"{\n";
json << L"  \"fileFormat\": \"Engine Calculator Project\",\n";
json << L"  \"version\": \"1.0.0\",\n";
json << L"  \"date\": \"2026\",\n";
json << L"  \"projectName\": \"" << EscapeJSON(project.projectName) << L"\",\n";
    json << L"  \"description\": \"" << EscapeJSON(project.description) << L"\",\n";
    json << L"  \"author\": \"" << EscapeJSON(project.author) << L"\",\n";
    
    // Dados básicos
    json << L"  \"basicData\": {\n";
    json << L"    \"bore\": " << project.basicData.bore << L",\n";
    json << L"    \"stroke\": " << project.basicData.stroke << L",\n";
    json << L"    \"cylinders\": " << project.basicData.cylinders << L",\n";
    json << L"    \"engineType\": " << (int)project.basicData.engineType << L",\n";
    json << L"    \"rodLength\": " << project.basicData.rodLength << L",\n";
    json << L"    \"maxRPM\": " << project.basicData.maxRPM << L"\n";
    json << L"  },\n";
    
    // Compressão
    json << L"  \"compressionData\": {\n";
    json << L"    \"chamberVolume\": " << project.compressionData.chamberVolume << L",\n";
    json << L"    \"pistonDomeVolume\": " << project.compressionData.pistonDomeVolume << L",\n";
    json << L"    \"gasketThickness\": " << project.compressionData.gasketThickness << L",\n";
    json << L"    \"deckHeight\": " << project.compressionData.deckHeight << L",\n";
    json << L"    \"valveRelief\": " << project.compressionData.valveRelief << L",\n";
    json << L"    \"ivcAngle\": " << project.compressionData.ivcAngle << L"\n";
    json << L"  },\n";
    
    // Trem de válvulas
    json << L"  \"valveTrainData\": {\n";
    json << L"    \"intakeValveDiameter\": " << project.valveTrainData.intakeValveDiameter << L",\n";
    json << L"    \"exhaustValveDiameter\": " << project.valveTrainData.exhaustValveDiameter << L",\n";
    json << L"    \"numberOfIntakeValves\": " << project.valveTrainData.numberOfIntakeValves << L",\n";
    json << L"    \"numberOfExhaustValves\": " << project.valveTrainData.numberOfExhaustValves << L",\n";
    json << L"    \"seatAngle\": " << project.valveTrainData.seatAngle << L",\n";
    json << L"    \"maxLift\": " << project.valveTrainData.maxLift << L",\n";
    json << L"    \"springRate\": " << project.valveTrainData.springRate << L",\n";
    json << L"    \"springPreload\": " << project.valveTrainData.springPreload << L",\n";
    json << L"    \"springInstalledHeight\": " << project.valveTrainData.springInstalledHeight << L",\n";
    json << L"    \"springCoilBindHeight\": " << project.valveTrainData.springCoilBindHeight << L",\n";
    json << L"    \"retainerWeight\": " << project.valveTrainData.retainerWeight << L",\n";
    json << L"    \"valveWeight\": " << project.valveTrainData.valveWeight << L",\n";
    json << L"    \"rockerRatio\": " << project.valveTrainData.rockerRatio << L"\n";
    json << L"  },\n";
    
    // Comando
    json << L"  \"camshaftData\": {\n";
    json << L"    \"intakeDuration\": " << project.camshaftData.intakeDuration << L",\n";
    json << L"    \"exhaustDuration\": " << project.camshaftData.exhaustDuration << L",\n";
    json << L"    \"intakeLift\": " << project.camshaftData.intakeLift << L",\n";
    json << L"    \"exhaustLift\": " << project.camshaftData.exhaustLift << L",\n";
    json << L"    \"lsa\": " << project.camshaftData.lsa << L",\n";
    json << L"    \"advanceRetard\": " << project.camshaftData.advanceRetard << L",\n";
    json << L"    \"rockerRatio\": " << project.camshaftData.rockerRatio << L"\n";
    json << L"  },\n";
    
    // Admissão
    json << L"  \"intakeData\": {\n";
    json << L"    \"runnerLength\": " << project.intakeData.runnerLength << L",\n";
    json << L"    \"runnerDiameter\": " << project.intakeData.runnerDiameter << L",\n";
    json << L"    \"plenumVolume\": " << project.intakeData.plenumVolume << L",\n";
    json << L"    \"trumpetLength\": " << project.intakeData.trumpetLength << L",\n";
    json << L"    \"trumpetDiameter\": " << project.intakeData.trumpetDiameter << L",\n";
    json << L"    \"targetRPM\": " << project.intakeData.targetRPM << L",\n";
    json << L"    \"boostPressure\": " << project.intakeData.boostPressure << L"\n";
    json << L"  },\n";
    
    // Escape
    json << L"  \"exhaustData\": {\n";
    json << L"    \"primaryLength\": " << project.exhaustData.primaryLength << L",\n";
    json << L"    \"primaryDiameter\": " << project.exhaustData.primaryDiameter << L",\n";
    json << L"    \"secondaryLength\": " << project.exhaustData.secondaryLength << L",\n";
    json << L"    \"secondaryDiameter\": " << project.exhaustData.secondaryDiameter << L",\n";
    json << L"    \"collectorDiameter\": " << project.exhaustData.collectorDiameter << L",\n";
    json << L"    \"targetRPM\": " << project.exhaustData.targetRPM << L",\n";
    json << L"    \"exhaustDuration\": " << project.exhaustData.exhaustDuration << L",\n";
    json << L"    \"isFourIntoOne\": " << (project.exhaustData.isFourIntoOne ? L"true" : L"false") << L"\n";
    json << L"  }\n";
    
    json << L"}\n";
    
    return json.str();
}

bool ProjectManager::ParseJSONDouble(const std::wstring& json, const std::wstring& key, double& value) const {
    std::wstring searchKey = L"\"" + key + L"\":";
    size_t pos = json.find(searchKey);
    
    if (pos == std::wstring::npos) return false;
    
    pos += searchKey.length();
    
    // Pula espaços
    while (pos < json.length() && (json[pos] == L' ' || json[pos] == L'\t' || json[pos] == L'\n')) {
        pos++;
    }
    
    // Lê o número
    size_t endPos = pos;
    while (endPos < json.length() && (isdigit(json[endPos]) || json[endPos] == L'.' || 
                                       json[endPos] == L'-' || json[endPos] == L'e' || json[endPos] == L'E')) {
        endPos++;
    }
    
    if (endPos > pos) {
        std::wstring numberStr = json.substr(pos, endPos - pos);
        value = std::wcstod(numberStr.c_str(), nullptr);
        return true;
    }
    
    return false;
}

bool ProjectManager::ParseJSONInt(const std::wstring& json, const std::wstring& key, int& value) const {
    double dValue;
    if (ParseJSONDouble(json, key, dValue)) {
        value = static_cast<int>(dValue);
        return true;
    }
    return false;
}

bool ProjectManager::ParseJSONBool(const std::wstring& json, const std::wstring& key, bool& value) const {
    std::wstring searchKey = L"\"" + key + L"\":";
    size_t pos = json.find(searchKey);
    
    if (pos == std::wstring::npos) return false;
    
    pos += searchKey.length();
    
    // Pula espaços
    while (pos < json.length() && (json[pos] == L' ' || json[pos] == L'\t' || json[pos] == L'\n')) {
        pos++;
    }
    
    if (json.substr(pos, 4) == L"true") {
        value = true;
        return true;
    } else if (json.substr(pos, 5) == L"false") {
        value = false;
        return true;
    }
    
    return false;
}

bool ProjectManager::ParseJSONString(const std::wstring& json, const std::wstring& key, std::wstring& value) const {
    std::wstring searchKey = L"\"" + key + L"\":";
    size_t pos = json.find(searchKey);
    
    if (pos == std::wstring::npos) return false;
    
    pos += searchKey.length();
    
    // Pula espaços até encontrar "
    while (pos < json.length() && json[pos] != L'\"') {
        pos++;
    }
    
    if (pos >= json.length()) return false;
    
    pos++; // Pula o "
    
    size_t endPos = pos;
    bool escaping = false;
    
    while (endPos < json.length()) {
        if (escaping) {
            escaping = false;
        } else if (json[endPos] == L'\\') {
            escaping = true;
        } else if (json[endPos] == L'\"') {
            break;
        }
        endPos++;
    }
    
    if (endPos > pos) {
        value = UnescapeJSON(json.substr(pos, endPos - pos));
        return true;
    }
    
    return false;
}

bool ProjectManager::JSONToProject(const std::wstring& json, EngineProject& project) const {
    // Parse campos básicos
    ParseJSONString(json, L"projectName", project.projectName);
    ParseJSONString(json, L"description", project.description);
    ParseJSONString(json, L"author", project.author);
    
    // Parse basicData
    ParseJSONDouble(json, L"bore", project.basicData.bore);
    ParseJSONDouble(json, L"stroke", project.basicData.stroke);
    ParseJSONInt(json, L"cylinders", project.basicData.cylinders);
    int engineType;
    if (ParseJSONInt(json, L"engineType", engineType)) {
        project.basicData.engineType = static_cast<EngineType>(engineType);
    }
    ParseJSONDouble(json, L"rodLength", project.basicData.rodLength);
    ParseJSONDouble(json, L"maxRPM", project.basicData.maxRPM);
    
    // Parse compressionData
    ParseJSONDouble(json, L"chamberVolume", project.compressionData.chamberVolume);
    ParseJSONDouble(json, L"pistonDomeVolume", project.compressionData.pistonDomeVolume);
    ParseJSONDouble(json, L"gasketThickness", project.compressionData.gasketThickness);
    ParseJSONDouble(json, L"deckHeight", project.compressionData.deckHeight);
    ParseJSONDouble(json, L"valveRelief", project.compressionData.valveRelief);
    ParseJSONDouble(json, L"ivcAngle", project.compressionData.ivcAngle);
    
    // Parse valveTrainData
    ParseJSONDouble(json, L"intakeValveDiameter", project.valveTrainData.intakeValveDiameter);
    ParseJSONDouble(json, L"exhaustValveDiameter", project.valveTrainData.exhaustValveDiameter);
    ParseJSONInt(json, L"numberOfIntakeValves", project.valveTrainData.numberOfIntakeValves);
    ParseJSONInt(json, L"numberOfExhaustValves", project.valveTrainData.numberOfExhaustValves);
    ParseJSONDouble(json, L"seatAngle", project.valveTrainData.seatAngle);
    ParseJSONDouble(json, L"maxLift", project.valveTrainData.maxLift);
    ParseJSONDouble(json, L"springRate", project.valveTrainData.springRate);
    ParseJSONDouble(json, L"springPreload", project.valveTrainData.springPreload);
    ParseJSONDouble(json, L"springInstalledHeight", project.valveTrainData.springInstalledHeight);
    ParseJSONDouble(json, L"springCoilBindHeight", project.valveTrainData.springCoilBindHeight);
    ParseJSONDouble(json, L"retainerWeight", project.valveTrainData.retainerWeight);
    ParseJSONDouble(json, L"valveWeight", project.valveTrainData.valveWeight);
    
    // Parse camshaftData
    ParseJSONDouble(json, L"intakeDuration", project.camshaftData.intakeDuration);
    ParseJSONDouble(json, L"exhaustDuration", project.camshaftData.exhaustDuration);
    ParseJSONDouble(json, L"intakeLift", project.camshaftData.intakeLift);
    ParseJSONDouble(json, L"exhaustLift", project.camshaftData.exhaustLift);
    ParseJSONDouble(json, L"lsa", project.camshaftData.lsa);
    ParseJSONDouble(json, L"advanceRetard", project.camshaftData.advanceRetard);
    
    // Parse intakeData
    ParseJSONDouble(json, L"runnerLength", project.intakeData.runnerLength);
    ParseJSONDouble(json, L"runnerDiameter", project.intakeData.runnerDiameter);
    ParseJSONDouble(json, L"plenumVolume", project.intakeData.plenumVolume);
    ParseJSONDouble(json, L"trumpetLength", project.intakeData.trumpetLength);
    ParseJSONDouble(json, L"trumpetDiameter", project.intakeData.trumpetDiameter);
    ParseJSONDouble(json, L"targetRPM", project.intakeData.targetRPM);
    ParseJSONDouble(json, L"boostPressure", project.intakeData.boostPressure);
    
    // Parse exhaustData
    ParseJSONDouble(json, L"primaryLength", project.exhaustData.primaryLength);
    ParseJSONDouble(json, L"primaryDiameter", project.exhaustData.primaryDiameter);
    ParseJSONDouble(json, L"secondaryLength", project.exhaustData.secondaryLength);
    ParseJSONDouble(json, L"secondaryDiameter", project.exhaustData.secondaryDiameter);
    ParseJSONDouble(json, L"collectorDiameter", project.exhaustData.collectorDiameter);
    ParseJSONDouble(json, L"targetRPM", project.exhaustData.targetRPM);
    ParseJSONDouble(json, L"exhaustDuration", project.exhaustData.exhaustDuration);
    ParseJSONBool(json, L"isFourIntoOne", project.exhaustData.isFourIntoOne);
    
    return true;
}

bool ProjectManager::SaveProject(const std::wstring& filename, const EngineProject& project) {
    try {
        std::wstring json = ProjectToJSON(project);
        
        std::wofstream file(filename);
        if (!file.is_open()) {
            lastError = L"Não foi possível abrir o arquivo para escrita.";
            return false;
        }
        
        file << json;
        file.close();
        
        lastError.clear();
        return true;
    }
    catch (const std::exception& e) {
        lastError = L"Erro ao salvar o projeto.";
        return false;
    }
}

bool ProjectManager::LoadProject(const std::wstring& filename, EngineProject& project) {
    try {
        std::wifstream file(filename);
        if (!file.is_open()) {
            lastError = L"Não foi possível abrir o arquivo para leitura.";
            return false;
        }
        
        std::wostringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        std::wstring json = buffer.str();
        
        if (!JSONToProject(json, project)) {
            lastError = L"Erro ao interpretar o arquivo do projeto.";
            return false;
        }
        
        lastError.clear();
        return true;
    }
    catch (const std::exception& e) {
        lastError = L"Erro ao carregar o projeto.";
        return false;
    }
}

bool ProjectManager::IsValidProjectFile(const std::wstring& filename) const {
    std::wifstream file(filename);
    if (!file.is_open()) return false;
    
    std::wostringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    std::wstring json = buffer.str();
    
    // Verifica se contém campos essenciais
    return json.find(L"\"version\"") != std::wstring::npos &&
           json.find(L"\"basicData\"") != std::wstring::npos;
}
