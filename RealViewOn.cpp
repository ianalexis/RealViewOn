#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Registro.h"




// Variables globales
std::string version;

// Prototipos de funciones
void configurarConsola();


// Definición de funciones

void configurarConsola() {
    system("color 17"); // Azul
    SetConsoleOutputCP(CP_UTF8);//soporte para UTF-8
}

// Función principal
int main() {
    configurarConsola();

    std::cout << u8"RealView Cracker V0.3 by RF47\n";
    std::cout << u8"Ingrese la versión de SolidWorks instalada (e.g., 2022): ";
    std::cin >> version;

    // Primero buscar en la ruta principal
    std::wstring rutaRegistro = L"SOFTWARE\\SolidWorks\\AllowList\\Current";
    std::string renderer = obtenerRenderer(rutaRegistro);

    // Si no se encuentra el renderer en la ruta alternativa, buscar en la ruta original
    if (renderer.empty()) {
        std::cout << u8"Buscando en ruta alternativa" << std::endl;
        rutaRegistro = construirRutaRegistro(version);
        renderer = obtenerRenderer(rutaRegistro);
    }

    if (!renderer.empty()) {
        std::cout << u8"GPU detectado: " << renderer << std::endl;

        // Crear el contenido del archivo .reg
        std::string contenidoReg = crearContenidoReg(version, renderer);
        if (contenidoReg.empty()) {
            std::cout << u8"Error: No se pudo generar el contenido del archivo .reg." << std::endl;
            return 1;
        }

        // Guardar el archivo .reg
        guardarArchivoReg(contenidoReg);
    }
    else {
        std::cout << u8"Error: No se pudo encontrar la clave 'renderer' en ninguna de las rutas." << std::endl;
    }

    // Pausar antes de salir
    std::cin.ignore();
    std::cout << u8"Presione enter para salir..." << std::endl;
    std::cin.get();
    return 0;
}

