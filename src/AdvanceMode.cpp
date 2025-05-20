#include "AdvanceMode.h"
#include "teclado.h"
#include <iostream>
#include <sstream>
#include <Windows.h>

using std::string;

AdvanceMode::AdvanceMode(){
}

void AdvanceMode::setSwVersion(int swVersion, bool generico){
    rutaVersionada = "\n" + std::string(rutaBase.begin(), rutaBase.end()) + std::to_string(swVersion) + "\\";
    this->generico = generico;
    optionsQty = regOptions.size() + (generico ? 0 : 1);
    optionNumber = 1;
}

string AdvanceMode::completeBase (string complement){
    return rutaVersionada + complement;
}

string AdvanceMode::askAdvanceOptions(){
    string options = "\n\n;## Advance Mode: ##";
    options += selectRegOptions();
    if (!generico){
        options += qolCommands();
    }
    if (!anySelected){
        options += "\n; - No Advance options selected.";
    }
    return options;
}

void AdvanceMode::askToEnable(string optionName){
    std::cout << "(" + std::to_string(optionNumber++) + "/" + std::to_string(optionsQty) + ") Enable " + optionName + "? (Y/N): ";
}

string AdvanceMode::qolCommands(){
    string tempOptions = "";
    askToEnable ("QoL Commands");
    if (yesOrNo()){
        tempOptions += "\n;### QoL Commands ###";
        tempOptions += enableTabs();
        tempOptions += enableBtn();
        anySelected = true;
    }
    return tempOptions;
}

string AdvanceMode::enableBtn(){
    string tempBtn = "";
    string tempBtnData = "";
    std::vector<string> btnsAlreadyEnabled;
    bool error = false;
    for (int i = 0 ; i < btnsToEnable.size() ; i++){
        string path = completeBase(btnsToEnable[i].path) + "]";
        int btnNumber = 0;
        string originalValue;
        while (!(originalValue = getOriginalValue(path, "Btn" + std::to_string(btnNumber))).empty() && !error){
            btnsAlreadyEnabled.push_back(originalValue);
            btnNumber++;
            error = (btnNumber > 25);
        }
        if (!error){
            for (int j = 0 ; j < btnsToEnable[i].value.size() ; j++){
                if (!(std::find(btnsAlreadyEnabled.begin(), btnsAlreadyEnabled.end(), btnsToEnable[i].value[j]) != btnsAlreadyEnabled.end())){
                    tempBtnData += "\n\"Btn" + std::to_string(btnNumber) + "\"=\"" + btnsToEnable[i].value[j] + "\"";
                    btnNumber++;
                }
            }
            if (!tempBtnData.empty()){
                tempBtn += path + tempBtnData+="\n";
                tempBtnData = "";
            }
        }
    }
    return !tempBtn.empty() ? "\n; - Enable Buttons\n;```" + tempBtn + ";```\n" : ";No buttons added";
}

string AdvanceMode::selectRegOptions(){
    string tempOptions = "";
    for (int i = 0 ; i < regOptions.size() ; i++){
        askToEnable(regOptions[i].name);
        if (yesOrNo()){
            tempOptions += "\n; - " + regOptions[i].name + "\n;```";
            for (int j = 0 ; j < regOptions[i].content.size() ; j++){
                tempOptions += completeBase(regOptions[i].content[j].path) + "]";
                for (int k = 0 ; k < regOptions[i].content[j].value.size() ; k++){
                    tempOptions += "\n" + regOptions[i].content[j].value[k];
                }
            }
            tempOptions += "\n;```\n";
        }
    }
    anySelected = !tempOptions.empty();
    return tempOptions;
}

string AdvanceMode::enableTabs(){
    string tempTabs = "";
    for (int i = 0; i < tabsToEnable.size(); i++) {
        string path = completeBase(rutaTab + std::to_string(tabsToEnable[i])) + "]";
        string value = enableTab(getOriginalValue(path, "Tab Props"));
        if (!value.empty()) {
            tempTabs += "\n; - Enable Tab " + std::to_string(tabsToEnable[i]) + "\n;```";
            tempTabs += path + "\n\"Tab Props\"=\"" + value + "\"\n;```\n";
        }
    }
    anySelected = !tempTabs.empty();
    return !tempTabs.empty() ? tempTabs : ";No tabs added";
}

//Pasa el tercer valor a 1 para habilitar la pestaña, si ya estaba habilitada devuelve un string vacío.
string AdvanceMode::enableTab(string value){
    std::stringstream ss(value);
    std::string segment;
    std::vector<std::string> segments;

    while (std::getline(ss, segment, ',')) {
        segments.push_back(segment);
    }
    if (!value.empty() && segments.size() >= 3 && segments[2] == "0") {
        segments[2] = "1";
        std::string result;
        for (size_t i = 0; i < segments.size(); ++i) {
            if (i != 0) {
                result += ",";
            }
            result += segments[i];
        }
        return result;
    } else {
        return "";
    }
}

string AdvanceMode::getOriginalValue(string path, string valueName) {
    string cleanPath = path.substr(20, path.size() - 21);
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, cleanPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return ""; // No se pudo abrir la clave del registro
    }
    char buffer[1024];
    DWORD bufferSize = sizeof(buffer);
    DWORD type;
    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS && type == REG_SZ) {
        RegCloseKey(hKey);
        return std::string(buffer, bufferSize - 1); // Excluir el carácter nulo final
    }
    RegCloseKey(hKey);
    return ""; // No se encontró el valor
}