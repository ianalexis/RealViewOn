#include "Registro.h"
#include <string>
#include <windows.h>
#include <iostream>
#include <fstream>



// Claves constantes para modificar en el registro
const std::string clave1 = "40000";
const std::string clave2 = "70408";

//Construye la ruta del registro basada en la versión de SolidWorks proporcionada.

std::wstring construirRutaRegistro(const std::string& version) {
    std::wstring basePath = L"SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    return basePath + std::wstring(version.begin(), version.end()) + L"\\Performance\\Graphics\\Hardware\\Current";
}


//Intenta leer el valor 'renderer' desde el registro.Si falla, muestra un mensaje de error.

std::string obtenerRenderer(const std::wstring& rutaRegistro) {
    HKEY hKey = nullptr;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, rutaRegistro.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        std::cout << u8"Error: No se pudo encontrar la clave." << std::endl;
        return "";
    }

    wchar_t value[256];
    DWORD bufferSize = sizeof(value);
    if (RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize) != ERROR_SUCCESS) {
        std::cout << u8"Error: No se pudo obtener el valor 'renderer' del registro." << std::endl;
        RegCloseKey(hKey);
        return "";
    }

    RegCloseKey(hKey);

    // Convertir wchar_t a std::string
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), NULL, 0, NULL, NULL);
    std::string rendererValue(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), &rendererValue[0], sizeNeeded, NULL, NULL);

    return rendererValue;
}



//Genera el contenido del archivo .reg basado en la versión y el renderer.

std::string crearContenidoReg(const std::string& version, const std::string& renderer) {
    // Verificar si el renderer contiene palabras clave NVIDIA o AMD
    std::string contenido;
    if (renderer.find("NVIDIA") != std::string::npos || renderer.find("QUADRO")) {
        contenido =
            "Windows Registry Editor Version 5.00\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\NVIDIA Corporation\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave1 + "\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\Gl2Shaders\\NV40\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave2 + "\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + version + "\\Performance\\Graphics\\Hardware\\NVIDIA Corporation\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave1 + "\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + version + "\\Performance\\Graphics\\Hardware\\Gl2Shaders\\NV40\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave2 + "\n";
    }
    else if (renderer.find("AMD") != std::string::npos || renderer.find("ATI") != std::string::npos) {
        contenido =
            "Windows Registry Editor Version 5.00\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\ATI Technologies Inc.\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave1 + "\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\Gl2Shaders\\RV900\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave2 + "\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + version + "\\Performance\\Graphics\\Hardware\\ATI Technologies Inc.\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave1 + "\n\n"
            "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + version + "\\Performance\\Graphics\\Hardware\\Gl2Shaders\\RV900\\" + renderer + "]\n"
            "\"Workarounds\"=dword:" + clave2 + "\n";
    }
    else {
        // Si no es NVIDIA ni AMD, devolver un mensaje de error vacío
        std::cout << "Error: GPU no compatible." << std::endl;
        return "";
    }

    return contenido;
}



//Guarda el contenido generado en un archivo .reg y maneja posibles errores.

void guardarArchivoReg(const std::string& contenido) {
    std::ofstream regFile("RealViewEnabler.reg");
    if (regFile.is_open()) {
        regFile << contenido;
        regFile.close();
        std::cout << u8"Archivo .reg creado con éxito!" << std::endl;
    }
    else {
        std::cout << u8"Error: No se pudo crear el archivo .reg." << std::endl;
    }
}