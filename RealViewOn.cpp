#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "SolidWorks.h"
#include "GPU.h"
#include <conio.h> //TODO: Revisar para salir con escape en cualquier momento.

using std::cin;
using std::cout;
using std::string;

// Prototipos de funciones
bool versionInstalada(int v);
void guardarArchivoReg(const string& contenido);
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
    cout << "RealView Cracker V0.3 by RF47\n";
    SolidWorks sw = SolidWorks();

    try {
        versionesInstaladas = sw.obtenerVersionesInstaladas();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cerrar();
    }
    

    cout << "Ingrese el año de versión de SolidWorks instalada (e.g.,2023, 2024 o 0 para salir): ";
    cin >> swVersion;
    if (swVersion <= 0) {
        cout << "Cerrando..." << std::endl;
        Sleep(250); // Esperar 0.25 segundos
        return 0;
    }

    try{
       // Verificar si la versión de SolidWorks está instalada o si se desea continuar con una instalación forzada genérica.
       if (!versionInstalada(swVersion)) {
           cout << "Error: La versión de SolidWorks no está instalada." << std::endl;
           cout << "Desea continuar con una instalacion forzada genérica? (Y/N): ";
           char opcion;
           cin >> opcion;
           if (opcion == 'y' || opcion == 'Y') {
               cout << "Continuando en modo genérico..." << std::endl;
               sw.setGenerico(true);
           } else {
               cout << "Instalación cancelada." << std::endl;
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
        string renderer = sw.obtenerRenderer();
        GPU gpu = GPU(renderer);
        guardarArchivoReg(gpu.completarContenidoReg(sw.obtenerRegBase()));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cerrar();
    }


    // Pausar antes de salir
    cin.ignore();
    cout << "Presione enter para salir..." << std::endl;
    cin.get();
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
void guardarArchivoReg(const string& contenido) {
    std::ofstream regFile("RealViewEnabler.reg");
    if (regFile.is_open()) {
        regFile << contenido;
        regFile.close();
        cout << "Archivo .reg creado con éxito!" << std::endl;
    }
    else {
        cout << "Error: No se pudo crear el archivo .reg." << std::endl;
    }
}

//Cuenta regresiva de cerrado
void cerrar() {
    for (int i = 10; i > 0; --i) {
        cout << "Cerrando en " << i << "s..." << std::endl;
        Sleep(1000); // Esperar 1 segundo
    }
    exit(1);
}