#include "AdvanceMode.h"
#include "Teclado.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <Windows.h>

using std::string;

namespace {
// Prefijo que produce setSwVersion(): "\n" + rutaBase hasta la raíz HKCU.
// getOriginalValue() lo recorta para recuperar la subclave real. Si rutaBase
// cambiara, getOriginalValue deja de encontrar valores (falla en seguro) en lugar
// de leer subclaves equivocadas.
const string kPrefijoPath = "\n[HKEY_CURRENT_USER\\";
} // namespace

AdvanceMode::AdvanceMode(){
}

void AdvanceMode::setSwVersion(int swVersion, bool generico){
    rutaVersionada = "\n" + std::string(rutaBase.begin(), rutaBase.end()) + std::to_string(swVersion) + "\\";
    this->generico = generico;
    optionsQty = static_cast<int>(regOptions.size()) + (generico ? 0 : 1);
    optionNumber = 1;
    // Se reinicia junto con el resto del estado por versión: main() reutiliza la
    // misma instancia de AdvanceMode en cada vuelta del bucle.
    anySelected = false;
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
    std::cout << "(" + std::to_string(optionNumber++) + "/" + std::to_string(optionsQty) + ") Enable " + optionName + "? ";
}

string AdvanceMode::qolCommands(){
    string tempOptions = "";
    askToEnable ("QoL Commands");
    if (yesOrNo()){
        tempOptions += "\n;### QoL Commands ###";
        // enableTabs/enableBtn marcan anySelected solo si agregaron algo. Antes se
        // forzaba a true acá, así que aceptar QoL sin que hubiera nada que
        // habilitar igual ocultaba el aviso "No Advance options selected".
        tempOptions += enableTabs();
        tempOptions += enableBtn();
    }
    return tempOptions;
}

string AdvanceMode::enableBtn(){
    string tempBtn = "";
    for (std::size_t i = 0 ; i < btnsToEnable.size() ; i++){
        string path = completeBase(btnsToEnable[i].path) + "]";
        // Estado por ruta: antes estaba declarado fuera del bucle, así que los
        // botones ya presentes en una ruta se consideraban presentes en las
        // siguientes (suprimiendo altas legítimas), y un `error` en la primera
        // ruta saltaba en silencio todas las demás.
        std::vector<string> btnsAlreadyEnabled;
        string tempBtnData = "";
        bool error = false;
        int btnNumber = 0;
        string originalValue;
        while (!(originalValue = getOriginalValue(path, "Btn" + std::to_string(btnNumber))).empty() && !error){
            btnsAlreadyEnabled.push_back(originalValue);
            btnNumber++;
            error = (btnNumber > 25);
        }
        if (!error){
            for (std::size_t j = 0 ; j < btnsToEnable[i].value.size() ; j++){
                if (std::find(btnsAlreadyEnabled.begin(), btnsAlreadyEnabled.end(), btnsToEnable[i].value[j]) == btnsAlreadyEnabled.end()){
                    tempBtnData += "\n\"Btn" + std::to_string(btnNumber) + "\"=\"" + btnsToEnable[i].value[j] + "\"";
                    btnNumber++;
                }
            }
            if (!tempBtnData.empty()){
                tempBtn += path + tempBtnData + "\n";
            }
        }
    }
    anySelected = anySelected || !tempBtn.empty();
    return !tempBtn.empty() ? "\n; - Enable Buttons\n;```" + tempBtn + ";```\n" : ";No buttons added";
}

string AdvanceMode::selectRegOptions(){
    string tempOptions = "";
    for (std::size_t i = 0 ; i < regOptions.size() ; i++){
        askToEnable(regOptions[i].name);
        if (yesOrNo()){
            tempOptions += "\n; - " + regOptions[i].name + "\n;```";
            for (std::size_t j = 0 ; j < regOptions[i].content.size() ; j++){
                tempOptions += completeBase(regOptions[i].content[j].path) + "]";
                for (std::size_t k = 0 ; k < regOptions[i].content[j].value.size() ; k++){
                    tempOptions += "\n" + regOptions[i].content[j].value[k];
                }
            }
            tempOptions += "\n;```\n";
        }
    }
    // Acumula en lugar de asignar: enableTabs() corre después y sobrescribía este
    // resultado con el suyo.
    anySelected = anySelected || !tempOptions.empty();
    return tempOptions;
}

string AdvanceMode::enableTabs(){
    string tempTabs = "";
    for (std::size_t i = 0; i < tabsToEnable.size(); i++) {
        string path = completeBase(rutaTab + std::to_string(tabsToEnable[i])) + "]";
        string value = enableTab(getOriginalValue(path, "Tab Props"));
        if (!value.empty()) {
            tempTabs += "\n; - Enable Tab " + std::to_string(tabsToEnable[i]) + "\n;```";
            tempTabs += path + "\n\"Tab Props\"=\"" + value + "\"\n;```\n";
        }
    }
    anySelected = anySelected || !tempTabs.empty();
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
    // path viene de completeBase() + "]", o sea kPrefijoPath + subclave + "]".
    // Antes se recortaba con substr(20, size - 21): los números coincidían con el
    // largo de kPrefijoPath, pero cualquier cambio en rutaBase habría producido
    // subclaves incorrectas en silencio, y un path más corto que el prefijo
    // lanzaba std::out_of_range.
    if (path.rfind(kPrefijoPath, 0) != 0 || path.size() <= kPrefijoPath.size() ||
        path.back() != ']') {
        return "";
    }
    const string cleanPath =
        path.substr(kPrefijoPath.size(), path.size() - kPrefijoPath.size() - 1);
    if (cleanPath.empty()) {
        return "";
    }

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, cleanPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return ""; // No se pudo abrir la clave del registro
    }
    char buffer[1024];
    DWORD bufferSize = sizeof(buffer);
    DWORD type = 0;
    const LSTATUS resultado = RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type,
                                               reinterpret_cast<LPBYTE>(buffer), &bufferSize);
    RegCloseKey(hKey);

    // bufferSize == 0 se da con un REG_SZ de datos vacíos: el "bufferSize - 1"
    // anterior producía un largo de 0xFFFFFFFF y hacía fallar la construcción del
    // string. bufferSize > sizeof(buffer) es el caso ERROR_MORE_DATA.
    if (resultado != ERROR_SUCCESS || type != REG_SZ || bufferSize == 0 ||
        bufferSize > sizeof(buffer)) {
        return ""; // No se encontró el valor, o no se puede usar
    }
    // Los REG_SZ no tienen garantizado el terminador nulo.
    return string(buffer, strnlen(buffer, bufferSize));
}