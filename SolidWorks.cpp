#include "SolidWorks.h"
#include "GPU.h"
#include <iostream>
#include <Windows.h>
#include <vector>
#include <string>
#include "teclado.h"


using std::cin;
using std::cout;
using std::string;

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
    // Verificar si la versión de SolidWorks está instalada o si se desea continuar con una instalación forzada genérica.
    if (!versionInstalada(v)) {
        cout << "Error: La versión de SolidWorks no está instalada." << std::endl;
        cout << "¿Desea continuar en modo genérico o cancelar la instalación? (Y/N): ";
        if (yesOrNo()) {
            cout << "Continuando en modo genérico..." << std::endl;
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
    cout << "Versiones disponibles - Compatibilidad" << std::endl;
    bool compatible;
    for (int i = vMin; i <= anoActual; i++) {
        string versionKey = string(swRegRuta.begin(), swRegRuta.end()) + std::to_string(i);
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, versionKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            compatible = esCompatible(i);
            cout << "SOLIDWORKS " << i << " - " << (compatible ? "[OK]" : "[X]") << std::endl;
            versiones.push_back(std::make_pair(i, compatible));
            RegCloseKey(hKey);
        }
    }
    if (versiones.empty()) {
        versiones.clear();
        throw std::runtime_error("No se encontraron versiones de SolidWorks instaladas.");
    }
}

string SolidWorks::obtenerRenderer() {
    renderer.clear();
    if (swVersion < vCambioRaiz || generico) {
        cout << "Probando con SW < " << vCambioRaiz << " (renderer en carpeta de version)" << std::endl;
        renderer = obtenerRendererAno();
    }
    if (renderer.empty() || swVersion >= vCambioRaiz || generico) {
        cout << "Probando con SW >= " << vCambioRaiz << " (renderer en carpeta raiz)" << std::endl;
        renderer = obtenerRenderRaiz();
    } 
    if (renderer.empty()) {
        cout << "Probando con modo genérico (renderer en todo el registro)" << std::endl;
        renderer = obtenerRendererGenerico();
    }
    if (renderer.empty()) {
        throw std::runtime_error("No se encontró el renderer.");
    }
    cout << "Renderer: " << renderer << std::endl;
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
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), NULL, 0, NULL, NULL);
            renderer = string(sizeNeeded, 0);
            WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), &renderer[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    cout << "Renderer encontrado en carpeta raiz: " << renderer << std::endl;
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
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), NULL, 0, NULL, NULL);
            renderer = string(sizeNeeded, 0);
            WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), &renderer[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    cout << "Renderer encontrado en carpeta de version: " << renderer << std::endl;
    return renderer;
}

// Busca render en todo el registro (modo generico)
string SolidWorks::obtenerRendererGenerico() { // TODO: Implementar una busqueda por todo el registro de solidworks.
    //HKEY hKey;
    return "INSERT GPU NAME HERE";
}

// Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
std::vector<std::string> SolidWorks::obtenerRegBase(){
    regBase.clear();
    if (swVersion >= vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseRaiz());
    } 
    if (swVersion < vCambioRaiz || generico) {
        regBase.push_back(obtenerRegBaseAno());
    } 
    return regBase;
}

string SolidWorks::obtenerRegBaseRaiz(){
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\";
}

string SolidWorks::obtenerRegBaseAno(){
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + std::to_string(swVersion) + "\\Performance\\Graphics\\Hardware\\";
}

// Valida que la version esté en el listado de versiones instaladas
bool SolidWorks::versionInstalada(int v) {
    if (versiones.empty()) {
        obtenerVersionesInstaladas();
    }
    for (int i = 0; i < versiones.size(); i++) {
        if (versiones[i].first == v) {
            return true;
        }
    }
    return false;
}

