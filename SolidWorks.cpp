#include "SolidWorks.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include "teclado.h"
#include <functional> // Añadir este encabezado


using std::cout;
using std::string;
using std::wstring;

// Constructor
SolidWorks::SolidWorks() {
    anoActual = obtenerAnoActual();
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
            cout << "SolidWorks " << v << " is a future compatibility version.\n";
        }
        cout << "Do you want to continue in generic mode? ";
        if (yesOrNo()) {
            //cout << "Continuando en modo genérico...\n";
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
        wstring versionKey = swRegRuta + std::to_wstring(i);
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, versionKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (versiones.empty()) {
                cout << "\nSW Installed v | Compatibility\n";
                cout << "---------------|--------------\n";
            }
            int compatibilidad = esCompatible(i);
            cout << "SolidWorks " << i << " | ";    
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
        cout << "Do you want to proceed manually? ";
        if (!yesOrNo()) {
            throw std::runtime_error("Installation canceled by the user.");
        }
    } else {
        cout << "---------------|--------------\n";
    } 
}

string SolidWorks::obtenerRenderer() {
    renderer.clear();
    string tempRenderer = "";
    if (swVersion < vCambioRaiz || generico) {
        renderer = obtenerRendererAno();
    }
    if (renderer.empty() || swVersion >= vCambioRaiz || generico) {
        tempRenderer = obtenerRenderRaiz();
        if (!tempRenderer.empty()) {
            if (!renderer.empty()) {
                cout << "A renderer was found in both the root folder and the version folder.\n";
                cout << "Do you want to use the renderer from the root folder? ";
                if (yesOrNo()) {
                    renderer = tempRenderer;
                }
            } else {
                renderer = tempRenderer;
            }
        }
        renderer = tempRenderer.empty() ? renderer : tempRenderer;
    }
    if (renderer.empty()) {
        renderer = obtenerRendererGenerico();
    }
    if (renderer.empty()) {
        cout << "Renderer not found.\n";
        cout << "Do you want to enter the renderer manually? ";
        if (yesOrNo()) {
            renderer = rendererManual();
            if (renderer.empty()) {
                throw std::runtime_error("No renderer was entered.");
            }
        } else {
            throw std::runtime_error("Installation canceled by the user.");
        }
    }
    cout << "Renderer: " << renderer << "\n";
    return renderer;
}

string SolidWorks::rendererManual() {
    cout << "Enter the renderer name manually: ";
    return entradaTeclado(0);
}

// Busca render en carpeta raiz.
string SolidWorks::obtenerRenderRaiz() {
    HKEY hKey;
    string rendererRaiz = "";
    std::wstring regPath = L"SOFTWARE\\SolidWorks\\AllowList\\Current";
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t value[256];
        DWORD bufferSize = sizeof(value);
        if (RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), NULL, 0, NULL, NULL);
            rendererRaiz = string(sizeNeeded, 0);
            WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), &rendererRaiz[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    if (rendererRaiz.empty()) {
        cout << "Renderer not found in the root folder.\n";
    } else {
        cout << "Renderer found in root folder: " << rendererRaiz << "\n";
    }
    return rendererRaiz;
}

// Busca render en carpeta de version.
string SolidWorks::obtenerRendererAno() {
    HKEY hKey;
    string rendererAno = "";
    std::wstring regPath = swRegRuta + std::to_wstring(swVersion) + L"\\Performance\\Graphics\\Hardware\\Current";
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t value[256];
        DWORD bufferSize = sizeof(value);
        if (RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), NULL, 0, NULL, NULL);
            rendererAno = string(sizeNeeded, 0);
            WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), &rendererAno[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    if (rendererAno.empty()) {
        cout << "Renderer not found in the " << swVersion << " folder.\n";
    } else {
        cout << "Renderer found in " << swVersion << " folder: " << rendererAno << "\n";
    }
    return rendererAno;
}

// Busca render en todo el registro (modo generico)
string SolidWorks::obtenerRendererGenerico() {
    HKEY hKey;
    std::vector<std::pair<std::string, std::string>> renderers;
    const std::wstring basePath = L"SOFTWARE\\SolidWorks";

    // Función recursiva para buscar claves Renderer
    std::function<void(HKEY, const std::wstring&)> buscarRendererEnSubclaves = [&](HKEY hKey, const std::wstring& subKeyPath) {
        wchar_t subKeyName[256];
        DWORD subKeyNameSize;
        DWORD index = 0;

        while (true) {
            subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
            if (RegEnumKeyEx(hKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
                break;
            }

            HKEY hSubKey;
            std::wstring fullSubKeyPath = subKeyPath + L"\\" + subKeyName;
            if (RegOpenKeyEx(HKEY_CURRENT_USER, fullSubKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                //wchar_t valueName[256]; // TODO: ELIMINMAR
                //DWORD valueNameSize = sizeof(valueName); // TODO: ELIMINAR
                DWORD valueType;
                BYTE valueData[256];
                DWORD valueDataSize = sizeof(valueData);

                if (RegQueryValueEx(hSubKey, L"Renderer", NULL, &valueType, valueData, &valueDataSize) == ERROR_SUCCESS) {
                    if (valueType == REG_SZ) {
                        std::wstring rendererValue(reinterpret_cast<wchar_t*>(valueData), valueDataSize / sizeof(wchar_t));
                        //std::wcout << L"Renderer en : " << fullSubKeyPath << std::endl;
                        //std::wcout << L"Clave: " << rendererValue << std::endl;
                        renderers.push_back(std::make_pair(std::string(rendererValue.begin(), rendererValue.end()), std::string(fullSubKeyPath.begin(), fullSubKeyPath.end())));
                    }
                }

                // Llamada recursiva para buscar en subclaves
                buscarRendererEnSubclaves(hSubKey, fullSubKeyPath);
                RegCloseKey(hSubKey);
            }
            index++;
        }
    };

    if (RegOpenKeyEx(HKEY_CURRENT_USER, basePath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        buscarRendererEnSubclaves(hKey, basePath);
        RegCloseKey(hKey);
    } else {
        std::wcerr << L"Error opening: " << basePath << std::endl;
    }
    if (renderers.empty()) { // TODO: Ver si se puede meter en el ternario. Cuando lo intenté no funcó.
        return "";
    }
    return renderers.size() == 1 ? renderers[0].first : elegirRenderer(renderers);
}

string SolidWorks::elegirRenderer(std::vector<std::pair<std::string, std::string>> renderers) {
    if (renderers.empty()) {
        throw std::runtime_error("No renderers found in the registry.");
    }
    cout << "Available renderers:\n";
    cout << "0. Enter manually\n";
    for (int i = 0; i < renderers.size(); i++) {
        cout << i + 1 << ". " << renderers[i].first << " (in " << renderers[i].second << ")\n";
    }
    while (true) {
        cout << "Select the renderer (or press Esc to cancel): ";
        string input = entradaTeclado(1);
        int opcion = std::stoi(input);
        if (opcion >= 1 && opcion <= renderers.size()) {
            return renderers[opcion - 1].first;
        }
        if (opcion == 0){
            return rendererManual();
        }
        cout << "Invalid option. Please try again.\n";
    }
}

// Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
std::vector<std::string> SolidWorks::obtenerRegBase() {
    regBase.clear();
    if (swVersion >= vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseRaiz());
    }
    if (swVersion < vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseAno());
    }
    return regBase;
}

string SolidWorks::obtenerRegBaseRaiz() {
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