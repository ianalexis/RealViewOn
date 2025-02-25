#include "SolidWorks.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <functional>
#include <sstream>
#include "Teclado.h"


using std::cout;
using std::string;

// Constructor
SolidWorks::SolidWorks() {
    anoActual = obtenerAnoActual() < vMax ? vMax : obtenerAnoActual();
}

// Obtiene el año actual del sistema operativo + 1.
int SolidWorks::obtenerAnoActual() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    return (st.wYear + 1);
}

// Configura la versión de SolidWorks.
void SolidWorks::setVersion(int v) {
    setGenerico(false);
    bool instalado = versionInstalada(v);
    int compatibilidad = esCompatible(v);
    if (compatibilidad == 0) {
        throw std::runtime_error("The version of SolidWorks is not compatible.");
    }
    swVersion = v;
    if (!instalado || compatibilidad == 2) {
        if (!instalado) {
            cout << "SolidWorks " << v << " is not installed.\n";
        }
        if (compatibilidad == 2) {
            if (v > anoActual) {
                calc(99999);
                cout << "WHAT?!?!?! SolidWorks " << v << " really?!\nHow did you get this version? It's from the future!\n";
            } else{
                cout << "SolidWorks " << v << " is a future compatibility version.\n";
            }
        }
        cout << "Do you want to continue in generic mode? ";
        if (yesOrNo()) {
            setGenerico(true);
        } else {
            throw std::runtime_error("Installation canceled by the user.");
        }
    }
}

// Comprueba si una versión es compatible. 0 = No compatible, 1 = Compatible , 2 = Versión futura.
int SolidWorks::esCompatible(int v) {
    return (v >= vMin) ? (v <= vMax) ? 1 : 2 : 0;
}

// Setea como generico el modo de instalación
void SolidWorks::setGenerico(bool g) {
    generico = g;
}

// Recorre las versiones de SolidWorks instaladas y devuelve un listado con el año de la versión y si es compatible.
void SolidWorks::obtenerVersionesInstaladas() {
    versiones.clear();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;

    // Guarda los atributos actuales de la consola
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;

    for (int i = vMin; i <= anoActual; i++) {
        string versionKey = swRegRuta + std::to_string(i);
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, versionKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (versiones.empty()) {
                cout << "\n SW Installed v  | Compatibility\n";
                cout << " ----------------|---------------\n";
            }
            int compatibilidad = esCompatible(i);
            cout << " SolidWorks " << i << " | ";    
            // Cambia el color y muestra "[OK]", "[X]" o "[?]" utilizando un switch case
            switch (compatibilidad) {
                case 1:  // Compatible
                    SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                    cout << "[OK]";
                    break;
                case 0:  // No compatible
                    SetConsoleTextAttribute(hConsole, BACKGROUND_RED | BACKGROUND_INTENSITY);
                    cout << "[X]";
                    break;
                case 2:  // Versión futura
                    SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | BACKGROUND_INTENSITY);
                    cout << "[>" << vMax << "]";
                    break;
                default:
                    SetConsoleTextAttribute(hConsole, saved_attributes);
                    cout << "[?]";
                    break;
            }
            // Restaura los atributos originales
            SetConsoleTextAttribute(hConsole, saved_attributes);
            cout << "\n";
            versiones.push_back(std::make_pair(i, compatibilidad));
            RegCloseKey(hKey);
        }
    }
    if (versiones.empty()) {
        cout << "No SolidWorks versions found installed.\n";
    } else {
        cout << " ----------------|---------------\n";
    } 
}

GPU::Current SolidWorks::obtenerCurrent() {
    current.renderer = "";
    current.vendor = "";
    current.workarounds = "";
    current.origin = "SolidWorks " + std::to_string(swVersion);
    GPU::Current tempCurrent;
    if (swVersion < vCambioRaiz || generico) {
        current = obtenerCurrentAno();
    }
    if (current.renderer.empty() || swVersion >= vCambioRaiz || generico) {
        tempCurrent = obtenerCurrentRaiz();
        if (!tempCurrent.renderer.empty()) {
            current = tempCurrent;
            current.origin = "SolidWorks Root";
        }
    }
    if (!current.renderer.empty()) {
        cout << "Renderer detected: " << current.renderer << ". Is this correct? ";
        if (!yesOrNo()) {
            current.renderer.clear();
        }
    }
    if (current.renderer.empty()) {
        current.vendor.clear();
        current.workarounds.clear();
        current.origin.clear();
        current = elegirRenderer();
    }
    if (current.renderer.empty()) {
        throw std::runtime_error("Renderer not found.");
    }
    cout << "Renderer: " << current.renderer << "\n";
    return current;
}

std::vector<std::pair<std::string, std::string>> SolidWorks::windowsDisplayAdapters() {
    std::vector<std::pair<std::string, std::string>> adaptadores;
    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(dd);
    int deviceIndex = 0;

    while (EnumDisplayDevicesA(NULL, deviceIndex, &dd, 0)) {
        std::string deviceString(dd.DeviceString);

        bool found = false;
        for (auto& adaptador : adaptadores) {
            if (adaptador.first == deviceString) {
                adaptador.second += " ," + std::to_string(deviceIndex);
                found = true;
                break;
            }
        }

        if (!found && !deviceString.empty()) {
            adaptadores.push_back(std::make_pair(deviceString, "Windows Display Adapter " + std::to_string(deviceIndex)));
        }

        deviceIndex++;
    }
    return adaptadores;
}

std::string SolidWorks::rendererManual() {
    string manual = "";
    bool correct = false;
    
    while (manual.empty() || !correct) {
        cout << "Enter the renderer name manually: ";
        manual = entradaTeclado(0, false);
        cout << "Renderer: " << manual << ". Is this correct? ";
        correct = yesOrNo();
    }
    return manual;
}

GPU::Current SolidWorks::obtenerCurrent(string path){
    HKEY hKey;
    GPU::Current currentTemp;
    string regPath = path;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char value[256];
        DWORD bufferSize = sizeof(value);
        DWORD valueType;
        
        // Obtener renderer
        if (RegQueryValueExA(hKey, "renderer", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            currentTemp.renderer = string(value, bufferSize);
            currentTemp.renderer.resize(strnlen(value, bufferSize));
        }
        if (currentTemp.renderer.empty()) {
            cout << "Renderer not found in the " << swVersion << " \n";
            return currentTemp;
        }
        // Obtener vendor
        if (RegQueryValueExA(hKey, "vendor", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            currentTemp.vendor = string(value, bufferSize);
            currentTemp.vendor.resize(strnlen(value, bufferSize));
        }
        // Obtener workarounds
        if (RegQueryValueExA(hKey, "workarounds", NULL, &valueType, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            if (valueType == REG_DWORD) {
                DWORD workaroundsValue = *(DWORD*)value;
                std::stringstream ss;
                ss << std::hex << workaroundsValue;
                currentTemp.workarounds = ss.str();
            } else {
                currentTemp.workarounds = string(value, bufferSize);
                currentTemp.workarounds.resize(strnlen(value, bufferSize));
            }
        }
        RegCloseKey(hKey);
    }
    return currentTemp;
}

// Trae Current en carpeta raiz.
GPU::Current SolidWorks::obtenerCurrentRaiz() {
    string regPath = "SOFTWARE\\SolidWorks\\AllowList\\Current";
    return obtenerCurrent(regPath);
}

// Trae Current en carpeta de version.
GPU::Current SolidWorks::obtenerCurrentAno() {
    string regPath = swRegRuta + std::to_string(swVersion) + "\\Performance\\Graphics\\Hardware\\Current";
    return obtenerCurrent(regPath);
}

// Busca render en todo el registro (modo generico)
std::vector<std::pair<std::string, std::string>> SolidWorks::obtenerRendererGenerico() {
    HKEY hKey;
    std::vector<std::pair<std::string, std::string>> renderers;
    const string basePath = "SOFTWARE\\SolidWorks";

    // Función recursiva para buscar claves Renderer
    std::function<void(HKEY, const string&)> buscarRendererEnSubclaves = [&](HKEY hKey, const string& subKeyPath) {
        char subKeyName[256];
        DWORD subKeyNameSize;
        DWORD index = 0;

        while (true) {
            subKeyNameSize = sizeof(subKeyName);
            if (RegEnumKeyExA(hKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
                break;
            }

            HKEY hSubKey;
            string fullSubKeyPath = subKeyPath + "\\" + subKeyName;
            if (RegOpenKeyExA(HKEY_CURRENT_USER, fullSubKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                DWORD valueType;
                char valueData[256];
                DWORD valueDataSize = sizeof(valueData);

                if (RegQueryValueExA(hSubKey, "Renderer", NULL, &valueType, (LPBYTE)valueData, &valueDataSize) == ERROR_SUCCESS) {
                    if (valueType == REG_SZ) {
                        string rendererValue(valueData, valueDataSize);
                        rendererValue.resize(strnlen(valueData, valueDataSize));
                        renderers.push_back(std::make_pair(rendererValue, fullSubKeyPath));
                    }
                }

                // Llamada recursiva para buscar en subclaves
                buscarRendererEnSubclaves(hSubKey, fullSubKeyPath);
                RegCloseKey(hSubKey);
            }
            index++;
        }
    };

    if (RegOpenKeyExA(HKEY_CURRENT_USER, basePath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        buscarRendererEnSubclaves(hKey, basePath);
        RegCloseKey(hKey);
    } else {
        std::cerr << "Error opening: " << basePath << std::endl;
    }
    return renderers;
}

GPU::Current SolidWorks::elegirRenderer() {
    auto renderers = obtenerRendererGenerico();
    auto adaptadores = windowsDisplayAdapters();
    GPU::Current tempCurrent;
    for (const auto& adaptador : adaptadores) {
        renderers.push_back(std::make_pair(adaptador.first, adaptador.second));
    }
    cout << "Available renderers:\n";
    cout << "0. Enter manually\n";
    for (int i = 0; i < renderers.size(); i++) {
        cout << i + 1 << ". " << renderers[i].first << " (in " << renderers[i].second << ")\n";
    }
    while (true) {
        cout << "Select the renderer (or press Esc to cancel): ";
        string input = entradaTeclado(std::to_string(renderers.size()).length(), true);
        int opcion = std::stoi(input);
        if (opcion >= 1 && opcion <= renderers.size()) {
            tempCurrent.renderer = renderers[opcion - 1].first;
            tempCurrent.origin = renderers[opcion - 1].second;
        }
        if (opcion == 0){
            tempCurrent.renderer = rendererManual();
            tempCurrent.origin = "Manual";
        }
        if (!tempCurrent.renderer.empty()) {
            return tempCurrent;
        }
        cout << "Invalid option. Please try again.\n";
    }
}

// Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
std::vector<std::string> SolidWorks::obtenerRegBase() {
    regBase.clear();
    if (swVersion >= vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseAllowList());
    }
    if (swVersion < vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseAno());
    }
    return regBase;
}

string SolidWorks::obtenerRegBaseAllowList() {
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList";
}

string SolidWorks::obtenerRegBaseAno() {
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + std::to_string(swVersion) + "\\Performance\\Graphics\\Hardware";
}

// Valida que la version esté en el listado de versiones instaladas
bool SolidWorks::versionInstalada(int v) {
    for (const auto& version : versiones) {
        if (version.first == v) {
            return true;
        }
    }
    return false;
}

bool SolidWorks::getGenerico() {
    return generico;
}