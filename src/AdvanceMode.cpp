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

string AdvanceMode::qolCommands(){
    string tempOptions = "";
    std::cout << "Enable QoL Commands? (Y/N): "; 
    if (yesOrNo()){
        tempOptions += "\n; - QoL Commands";
        tempOptions += enableTabs();
        tempOptions += enableBtn();
        anySelected = true;
    }
    return tempOptions;
}

string AdvanceMode::enableBtn(){//TODO: Revisar que el boton no exista ya en ese path.
    string tempBtn = ";  - Enable Buttons;\n;```";
    bool error = false;
    for (int i = 0 ; i < btnsToEnable.size() ; i++){
        string path = completeBase(btnsToEnable[i].path) + "]";
        int btnNumber = 0;
        while (!getOriginalValue(path, "Btn" + std::to_string(btnNumber)).empty() && !error){
            btnNumber++;
            error = (btnNumber > 25);
        }
        if (!error){
            tempBtn += path;
            for (int j = 0 ; j < btnsToEnable[i].value.size() ; j++){
                tempBtn += "\n\"Btn" + std::to_string(btnNumber) + "\"=\"" + btnsToEnable[i].value[j] + "\"";
                btnNumber++;
            }
        }
        tempBtn += "\n";
    }
    tempBtn += ";```\n";
    return tempBtn;
}

string AdvanceMode::selectRegOptions(){
    string tempOptions = "";
    for (int i = 0 ; i < regOptions.size() ; i++){
        std::cout << "Enable " + regOptions[i].name + "? (Y/N): ";
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
            tempTabs += "\n;  - Enable Tab " + std::to_string(tabsToEnable[i]) + "\n;```";
            tempTabs += path + "\n\"Tab Props\"=\"" + value + "\"\n;```\n";
        }
    }
    anySelected = !tempTabs.empty();
    return tempTabs;
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