#include "SolidWorks.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include "teclado.h"

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
        throw std::runtime_error("La versión de SolidWorks no es compatible.");
    }
    swVersion = v;
    if (!instalado || compatibilidad == 2) {
        if (!instalado) {
            cout << "La versión de SolidWorks no está instalada.\n";
        }
        if (compatibilidad == 2) {
            cout << "La versión de SolidWorks es mayor a " << vMax << ".\n";
        }
        cout << "¿Desea continuar en modo genérico? ";
        if (yesOrNo()) {
            cout << "Continuando en modo genérico...\n";
            setGenerico(true);
        } else {
            throw std::runtime_error("Instalación cancelada por el usuario.");
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

    cout << "\nVersiones disponibles - Compatibilidad\n";
    for (int i = vMin; i <= anoActual; i++) {
        wstring versionKey = swRegRuta + std::to_wstring(i);
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, versionKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            int compatibilidad = esCompatible(i);
            cout << "SolidWorks " << i << " - ";

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
        throw std::runtime_error("No se encontraron versiones de SolidWorks instaladas.");
    }
}

string SolidWorks::obtenerRenderer() {
    renderer.clear();
    string tempRenderer;
    if (swVersion < vCambioRaiz || generico) {
        cout << "Buscando con metodo < " << vCambioRaiz << " (renderer en carpeta de version)\n";
        renderer = obtenerRendererAno();
    }
    if (renderer.empty() || swVersion >= vCambioRaiz || generico) {
        cout << "Buscando con metodo >= " << vCambioRaiz << " (renderer en carpeta raiz)\n";
        tempRenderer = obtenerRenderRaiz();
        renderer = tempRenderer.empty() ? renderer : tempRenderer;
    }
    if (renderer.empty()) {
        cout << "Buscando con metodo genérico (renderer en todo el registro)\n";
        renderer = obtenerRendererGenerico();
    }
    if (renderer.empty()) {
        cout << "No se encontró el renderer. Ingrese el nombre manualmente.\n";
        renderer = rendererManual();
        if (renderer.empty()) {
            throw std::runtime_error("No se ingresó el renderer.");
        }
    }
    cout << "Renderer: " << renderer << "\n";
    return renderer;
}

   string SolidWorks::rendererManual() {
    cout << "Ingrese el nombre del renderer manualmente: ";
    return entradaTeclado(0);
}

// Busca render en carpeta raiz.
string SolidWorks::obtenerRenderRaiz() {
    HKEY hKey;
    std::wstring regPath = L"SOFTWARE\\SolidWorks\\AllowList\\Current";
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t value[256];
        DWORD bufferSize = sizeof(value);
        if (RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), NULL, 0, NULL, NULL);
            renderer = string(sizeNeeded, 0);
            WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), &renderer[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    cout << "Renderer encontrado en carpeta raiz: " << renderer << "\n";
    return renderer;
}

// Busca render en carpeta de version.
string SolidWorks::obtenerRendererAno() {
    HKEY hKey;
    std::wstring regPath = swRegRuta + std::to_wstring(swVersion) + L"\\Performance\\Graphics\\Hardware\\Current";
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t value[256];
        DWORD bufferSize = sizeof(value);
        if (RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), NULL, 0, NULL, NULL);
            renderer = string(sizeNeeded, 0);
            WideCharToMultiByte(CP_UTF8, 0, value, (int)wcslen(value), &renderer[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    cout << "Renderer encontrado en carpeta de version: " << renderer << "\n";
    return renderer;
}

// Busca render en todo el registro (modo generico)
string SolidWorks::obtenerRendererGenerico() { // TODO: Implementar una búsqueda por todo el registro de SolidWorks.
    return "";
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
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\";
}

string SolidWorks::obtenerRegBaseAno() {
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + std::to_string(swVersion) + "\\Performance\\Graphics\\Hardware\\";
}

// Valida que la version esté en el listado de versiones instaladas
bool SolidWorks::versionInstalada(int v) {
    if (versiones.empty()) {
        obtenerVersionesInstaladas();
    }
    for (const auto& version : versiones) {
        if (version.first == v) {
            return true;
        }
    }
    return false;
}