#include "SolidWorks.h"
#include "GPU.h"
#include <iostream>
#include <Windows.h>
#include <vector>
#include <string>

int vMin = 2019; // Versión mínima soportada
int vMax = 2024; // Versión máxima soportada
int anoActual = vMax; // Año actual + 1
std::wstring swRegRuta = L"SOFTWARE\\SolidWorks\\SOLIDWORKS ";

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

// Recorre las versiones de SolidWorks instaladas y devuelve un listado con la versión y si es compatible.
std::vector<std::pair<std::string, bool>> SolidWorks::obtenerVersionesInstaladas() {
    std::vector<std::pair<std::string, bool>> versiones;
    std::cout << "Versiones disponibles - Compatibilidad" << std::endl;
    for (int i = vMin; i <= anoActual; i++) {
        std::string versionKey = std::string(swRegRuta.begin(), swRegRuta.end()) + std::to_string(i);
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, versionKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            bool compatible = (i >= vMin && i <= vMax);
            std::cout << "SOLIDWORKS " << i << " - " << (compatible ? "[OK]" : "[X]") << std::endl;
            versiones.push_back(std::make_pair("SOLIDWORKS " + std::to_string(i), compatible));
            RegCloseKey(hKey);
        }
    }
    if (versiones.empty()) {
            std::cout << "No se detectó ninguna versión instalada." << std::endl;
    }
    return versiones;
}