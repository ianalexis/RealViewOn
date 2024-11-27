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
    if (!esCompatible(v)) {
        throw std::runtime_error("La versión de SolidWorks no es compatible.");
    }
    swVersion = v;
    if (!versionInstalada(v)) {
        cout << "Error: La versión de SolidWorks no está instalada.\n";
        cout << "¿Desea continuar en modo genérico o cancelar la instalación? (Y/N): ";
        if (yesOrNo()) {
            cout << "Continuando en modo genérico...\n";
            setGenerico(true);
        } else {
            throw std::runtime_error("Instalación cancelada por el usuario.");
        }
    }
}

// Comprueba si una versión es compatible.
bool SolidWorks::esCompatible(int v) {
    return (v >= vMin && v <= vMax);
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
            bool compatible = esCompatible(i);
            cout << "SolidWorks " << i << " - ";

            // Cambia el color para "[OK]" y "[X]"
            if (compatible) {
                SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                cout << "[OK]";
            } else {
                SetConsoleTextAttribute(hConsole, BACKGROUND_RED | BACKGROUND_INTENSITY);
                cout << "[X]";
            }
            // Restaura los atributos originales
            SetConsoleTextAttribute(hConsole, saved_attributes);
            cout << "\n";
            versiones.push_back(std::make_pair(i, compatible));
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
        tempRenderer = obtenerRendererGenerico();
        renderer = tempRenderer.empty() ? throw std::runtime_error("No se encontró el renderer.") : tempRenderer;
    }
    cout << "Renderer: " << renderer << "\n";
    return renderer;
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
    return "INSERT GPU NAME HERE";
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