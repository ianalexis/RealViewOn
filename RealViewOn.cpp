#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Registro.h"
#include "SolidWorks.h"

// Variables globales
std::string swVersion;

// Prototipos de funciones
void configurarConsola();


// Definición de funciones
void configurarConsola() {
    system("color 17"); // Azul
    SetConsoleOutputCP(CP_UTF8); // Soporte para UTF-8
}

// Función principal
int main() {
    configurarConsola();

    std::cout << "RealView Cracker V0.3 by RF47\n";
    SolidWorks sw = SolidWorks();
    sw.obtenerVersionesInstaladas();
    

    std::cout << "Ingrese la versión de SolidWorks instalada (e.g., 2024 o 0 para salir): ";
    std::cin >> swVersion;
    if (swVersion == "0") {
        std::cout << "Saliendo..." << std::endl;
        return 0;
    }

    // Primero buscar en la ruta principal
    std::wstring rutaRegistro = L"SOFTWARE\\SolidWorks\\AllowList\\Current";
    std::string renderer = obtenerRenderer(rutaRegistro);

    // Si no se encuentra el renderer en la ruta alternativa, buscar en la ruta original
    if (renderer.empty()) {
        std::cout << "Buscando en ruta alternativa" << std::endl;
        rutaRegistro = construirRutaRegistro(swVersion);
        renderer = obtenerRenderer(rutaRegistro);
    }

    if (!renderer.empty()) {
        std::cout << "GPU detectado: " << renderer << std::endl;

        // Crear el contenido del archivo .reg
        std::string contenidoReg = crearContenidoReg(swVersion, renderer);
        if (contenidoReg.empty()) {
            std::cout << "Error: No se pudo generar el contenido del archivo .reg." << std::endl;
            return 1;
        }

        // Guardar el archivo .reg
        guardarArchivoReg(contenidoReg);
    }
    else {
        std::cout << "Error: No se pudo encontrar la clave 'renderer' en ninguna de las rutas." << std::endl;
    }

    // Pausar antes de salir
    std::cin.ignore();
    std::cout << "Presione enter para salir..." << std::endl;
    std::cin.get();
    return 0;
}

