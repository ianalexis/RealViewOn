#include "SolidWorks.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <cstring>
#include <functional>
#include <sstream>
#include <stdexcept>
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
    current = GPU::Current{};
    GPU::Current tempCurrent;
    if (swVersion < vCambioRaiz || generico) {
        current = obtenerCurrentAno();
        // El origin se asigna sobre el resultado: obtenerCurrent(path) devuelve un
        // Current nuevo, así que fijarlo antes de esta línea se perdía y el .reg
        // salía sin la línea "Origin" para las versiones previas a vCambioRaiz.
        if (!current.renderer.empty()) {
            current.origin = "SolidWorks " + std::to_string(swVersion);
        }
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

// Lee un valor del registro en el buffer indicado.
// bytesLeidos SIEMPRE se reinicia a la capacidad del buffer antes de consultar:
// RegQueryValueExA escribe ahi la cantidad copiada (y, ante ERROR_MORE_DATA, el
// tamaño requerido, que puede superar la capacidad real). Reutilizar el valor
// entre lecturas hacía que la segunda consulta recibiera la capacidad sobrante de
// la primera, salteando valores en silencio o declarando un buffer mayor que el
// real.
static bool leerValorDeRegistro(HKEY hKey, const char* valueName, char* buffer, DWORD capacidad,
                                DWORD& bytesLeidos, DWORD& tipo) {
    bytesLeidos = capacidad;
    tipo = 0;
    if (RegQueryValueExA(hKey, valueName, NULL, &tipo, reinterpret_cast<LPBYTE>(buffer),
                         &bytesLeidos) != ERROR_SUCCESS) {
        return false;
    }
    // El valor no entraba en el buffer: se descarta en lugar de leer de más.
    return bytesLeidos <= capacidad;
}

// Los valores REG_SZ del registro no tienen garantizado el terminador nulo, así
// que se acota por strnlen en lugar de asumir bytesLeidos - 1.
static string comoTexto(const char* buffer, DWORD bytes) {
    return string(buffer, strnlen(buffer, bytes));
}

GPU::Current SolidWorks::obtenerCurrent(string path){
    HKEY hKey;
    GPU::Current currentTemp;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return currentTemp;
    }

    char value[256];
    DWORD bytesLeidos = 0;
    DWORD valueType = 0;

    // Obtener renderer
    if (leerValorDeRegistro(hKey, "renderer", value, sizeof(value), bytesLeidos, valueType)) {
        currentTemp.renderer = comoTexto(value, bytesLeidos);
    }
    if (currentTemp.renderer.empty()) {
        cout << "Renderer not found in " << path << "\n";
        RegCloseKey(hKey);
        return currentTemp;
    }
    // Obtener vendor
    if (leerValorDeRegistro(hKey, "vendor", value, sizeof(value), bytesLeidos, valueType)) {
        currentTemp.vendor = comoTexto(value, bytesLeidos);
    }
    // Obtener workarounds
    if (leerValorDeRegistro(hKey, "workarounds", value, sizeof(value), bytesLeidos, valueType)) {
        if (valueType == REG_DWORD) {
            if (bytesLeidos >= sizeof(DWORD)) {
                DWORD workaroundsValue = 0;
                // memcpy en lugar de *(DWORD*)value: el buffer es char[] y no
                // tiene garantizada la alineación de un DWORD.
                std::memcpy(&workaroundsValue, value, sizeof(workaroundsValue));
                std::stringstream ss;
                ss << std::hex << workaroundsValue;
                currentTemp.workarounds = ss.str();
            }
        } else {
            currentTemp.workarounds = comoTexto(value, bytesLeidos);
        }
    }
    RegCloseKey(hKey);
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