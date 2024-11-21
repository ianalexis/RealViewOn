#include "SolidWorks.h"
#include "GPU.h"
#include <iostream>
#include <Windows.h>
#include <vector>
#include <string>

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
    swVersion = v;
    compatible = esCompatible(v);
    if (!compatible) {
        std::cout << "La versión de SolidWorks no es compatible.";
        std::cout << "¿Desea continuar en modo genérico o cancelar la instalación? (Y/N): ";
        char opcion;
        std::cin >> opcion;
        if (opcion == 'y' || opcion == 'Y') {
            std::cout << "Continuando en modo genérico..." << std::endl;
        } else {
            std::cout << "Instalación cancelada." << std::endl;
            Sleep(1000); // Esperar 1 segundo
            exit(1);
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
std::vector<std::pair<int, bool>> SolidWorks::obtenerVersionesInstaladas() {
    versiones.clear();
    std::cout << "Versiones disponibles - Compatibilidad" << std::endl;
    for (int i = vMin; i <= anoActual; i++) {
        std::string versionKey = std::string(swRegRuta.begin(), swRegRuta.end()) + std::to_string(i);
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, versionKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            compatible = esCompatible(i);
            std::cout << "SOLIDWORKS " << i << " - " << (compatible ? "[OK]" : "[X]") << std::endl;
            versiones.push_back(std::make_pair(i, compatible));
            RegCloseKey(hKey);
        }
    }
    if (versiones.empty()) {
        std::cout << "No se detectó ninguna versión instalada." << std::endl;
    }
    return versiones;
}

std::string SolidWorks::obtenerRenderer() {
    if (swVersion >= vCambioRaiz || generico) {
        std::cout << "Probando con SW >= " << vCambioRaiz << " (renderer en carpeta raiz)" << std::endl; //TODO: Borrar
        renderer = obtenerRenderRaiz();
    } 
    if (swVersion < vCambioRaiz || generico || renderer.empty()) {
        std::cout << "Probando con SW < " << vCambioRaiz << " (renderer en carpeta de version)" << std::endl; //TODO: Borrar
        renderer = obtenerRendererAno();
    }
    if (renderer.empty()) {
        std::cout << "Probando con modo genérico (renderer en todo el registro)" << std::endl; //TODO: Borrar
        renderer = obtenerRendererGenerico();
    }
    if (renderer.empty()) {
        throw std::runtime_error("No se encontró el renderer.");
    }
    std::cout << "Renderer: " << renderer << std::endl;
    return renderer;
}

// Busca render en carpeta raiz.
std::string SolidWorks::obtenerRenderRaiz() {
    HKEY hKey;
    std::wstring regPath = L"SOFTWARE\\SolidWorks\\AllowList\\Current";
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t value[256];
        DWORD bufferSize = sizeof(value);
        if (RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), NULL, 0, NULL, NULL);
            renderer = std::string(sizeNeeded, 0);
            std::cout << "Renderer encontrado en carpeta raiz: " << renderer << std::endl; // TODO: Borrar
            WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), &renderer[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    return renderer;
}

// Busca render en carpeta de version.
std::string SolidWorks::obtenerRendererAno() {
    HKEY hKey;
    std::wstring regPath = swRegRuta + std::to_wstring(swVersion) + L"\\Performance\\Graphics\\Hardware\\Current";
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t value[256];
        DWORD bufferSize = sizeof(value);
        if (RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize) == ERROR_SUCCESS) {
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), NULL, 0, NULL, NULL);
            renderer = std::string(sizeNeeded, 0);
            std::cout << "Renderer encontrado en carpeta de version: " << renderer << std::endl; // TODO: Borrar
            WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), &renderer[0], sizeNeeded, NULL, NULL);
        }
        RegCloseKey(hKey);
    }
    return renderer;
}

// Busca render en todo el registro (modo generico)
std::string SolidWorks::obtenerRendererGenerico() { // TODO: Implementar una busqueda por todo el registro de solidworks.
    HKEY hKey;
    return "";
}

// Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
std::vector<std::string> SolidWorks::obtenerRegBase(){
    regBase.clear();
    if (swVersion >= 2022 || generico) {
        regBase.push_back(obtenerRegBaseRaiz());
    } 
    if (swVersion < 2022 || generico) {
        regBase.push_back(obtenerRegBaseAno());
    } 
    return regBase;
}

std::string SolidWorks::obtenerRegBaseRaiz(){
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\";
}

std::string SolidWorks::obtenerRegBaseAno(){
    return "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + std::to_string(swVersion) + "\\Performance\\Graphics\\Hardware\\";
}

