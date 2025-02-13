#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

int main() {
    // Configurar la consola para utilizar UTF-8
    SetConsoleOutputCP(CP_UTF8);

    HKEY hKey = nullptr;
    LONG lRes;

    // Lista de versiones posibles
    std::string versions[] = { "2020", "2021", "2022", "2023", "2024", "2025", "2026" };
    std::wstring basePath = L"SOFTWARE\\SolidWorks\\SOLIDWORKS ";

    bool found = false;
    std::wstring fullPath;
    std::string foundVersion;

    // Intentar abrir la clave para cada versión
    for (const std::string& version : versions) {
        fullPath = basePath + std::wstring(version.begin(), version.end()) + L"\\Performance\\Graphics\\Hardware\\Current";
        lRes = RegOpenKeyEx(HKEY_CURRENT_USER, fullPath.c_str(), 0, KEY_READ, &hKey);
        if (lRes == ERROR_SUCCESS) {
            found = true;
            foundVersion = version;
            break;
        }
    }

    if (found) {
        wchar_t value[256]; // Tamaño del buffer para almacenar el valor de la cadena
        DWORD bufferSize = sizeof(value);
        lRes = RegQueryValueEx(hKey, L"renderer", NULL, NULL, (LPBYTE)value, &bufferSize);
        if (lRes == ERROR_SUCCESS) {
            // Convertir wchar_t a std::string
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), NULL, 0, NULL, NULL);
            std::string rendererValue(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, &value[0], (int)wcslen(value), &rendererValue[0], size_needed, NULL, NULL);

            std::cout << "Valor de la cadena renderer: " << rendererValue << std::endl;

            // Crear el contenido del archivo .reg
            std::string regContent =
                "Windows Registry Editor Version 5.00\n\n"
                "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\NVIDIA Corporation\\" + rendererValue + "]\n"
                "\"Workarounds\"=dword:40000\n\n"
                "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\AllowList\\Gl2Shaders\\NV40\\" + rendererValue + "]\n"
                "\"Workarounds\"=dword:70408\n\n"
                "[HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks\\SOLIDWORKS " + foundVersion + "\\Performance\\Graphics\\Hardware\\NVIDIA Corporation\\" + rendererValue + "]\n"
                "\"Workarounds\"=dword:40000\n";

            // Guardar el contenido en un archivo .reg
            std::ofstream regFile("RealViewEnabler.reg");
            if (regFile.is_open()) {
                regFile << regContent;
                regFile.close();
                std::cout << "Archivo .reg creado con éxito!" << std::endl;
            }
            else {
                std::cout << "Error al crear el archivo .reg." << std::endl;
            }
        }
        else {
            std::cout << "Error al leer el valor de la cadena." << std::endl;
        }
        RegCloseKey(hKey);
    }
    else {
        std::cout << "No se encontró una versión de SolidWorks compatible." << std::endl;
    }

    // Evita que la consola se cierre automáticamente
    std::cin.ignore();
    std::cin.get();

    return 0;
}
