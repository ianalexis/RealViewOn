#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "SolidWorks.h"
#include "GPU.h"

// Prototipos de funciones
bool versionInstalada(int v);
void guardarArchivoReg(const std::string& contenido);
void cerrar();


// Variables globales
int swVersion;
std::vector<std::pair<int, bool>> versionesInstaladas;

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

    try {
        versionesInstaladas = sw.obtenerVersionesInstaladas();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cerrar();
    }
    

    std::cout << "Ingrese el año de versión de SolidWorks instalada (e.g.,2023, 2024 o 0 para salir): ";
    std::cin >> swVersion;
    if (swVersion <= 0) {
        std::cout << "Cerrando..." << std::endl;
        Sleep(250); // Esperar 0.25 segundos
        return 0;
    }

    try{
       // Verificar si la versión de SolidWorks está instalada o si se desea continuar con una instalación forzada genérica.
       if (!versionInstalada(swVersion)) {
           std::cout << "Error: La versión de SolidWorks no está instalada." << std::endl;
           std::cout << "Desea continuar con una instalacion forzada genérica? (Y/N): ";
           char opcion;
           std::cin >> opcion;
           if (opcion == 'y' || opcion == 'Y') {
               std::cout << "Continuando en modo genérico..." << std::endl;
               sw.setGenerico(true);
           } else {
               std::cout << "Instalación cancelada." << std::endl;
               Sleep(1000); // Esperar 1 segundo
               return 1;
           }
       }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cerrar();
    }


    try{
        sw.setVersion(swVersion);
        std::string renderer = sw.obtenerRenderer();
        GPU gpu = GPU(renderer);
        guardarArchivoReg(gpu.completarContenidoReg(sw.obtenerRegBase()));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cerrar();
    }


    // Pausar antes de salir
    std::cin.ignore();
    std::cout << "Presione enter para salir..." << std::endl;
    std::cin.get();
    return 0;
}

// Valida que la version esté en el listado de versiones instaladas
bool versionInstalada (int v){
    for (int i = 0; i < versionesInstaladas.size(); i++) {
        if (versionesInstaladas[i].first == v) {
            return true;
        }
    }
    return false;
}

//Guarda el contenido generado en un archivo .reg y maneja posibles errores.
void guardarArchivoReg(const std::string& contenido) {
    std::ofstream regFile("RealViewEnabler.reg");
    if (regFile.is_open()) {
        regFile << contenido;
        regFile.close();
        std::cout << "Archivo .reg creado con éxito!" << std::endl;
    }
    else {
        std::cout << "Error: No se pudo crear el archivo .reg." << std::endl;
    }
}

//Cuenta regresiva de cerrado
void cerrar() {
    for (int i = 10; i > 0; --i) {
        std::cout << "Cerrando en " << i << "s..." << std::endl;
        Sleep(1000); // Esperar 1 segundo
    }
    exit(1);
}