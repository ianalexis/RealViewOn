#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "SolidWorks.h"
#include "GPU.h"
#include <conio.h>
#include "teclado.h"

using std::cin;
using std::cout;
using std::string;

// Variables globales
int swVersion;

// Prototipos de funciones
void configurarConsola();


// Definición de funciones
void configurarConsola() {
    system("color 17"); // Azul
    SetConsoleOutputCP(CP_UTF8); // Soporte para UTF-8
}

//Guarda el contenido generado en un archivo .reg y maneja posibles errores.
void guardarArchivoReg(int& version, const string& contenido) {
    std::ofstream regFile("RealViewEnable" + std::to_string(version) + ".reg");
    if (regFile.is_open()) {
        regFile << contenido;
        regFile.close();
        cout << "Archivo " << "RealViewEnable" + std::to_string(version) + ".reg creado exitosamente." << std::endl;
    }
    else {
        throw std::runtime_error("No se pudo crear el archivo .reg.");
    }
}

//Cuenta regresiva de CERRADO TODO: Deprecar
void cerrar() {
    for (int i = 3; i > 0; --i) {
        cout << "Cerrando en " << i << "s..." << std::endl;
        Sleep(500); // Esperar 0,5 segundo
    }
    exit(1);
}

// Función principal
int main() {
    configurarConsola();
    cout << "RealView Cracker V0.3 by RF47\n";
    SolidWorks sw = SolidWorks();

    try {
        sw.obtenerVersionesInstaladas();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cerrar(); // TODO: modificar para que reinicie la ejecución del programa.
    }

    //While para que el programa reinicie si no se presiona escape.
    while (true) {
        
        // 
        while (true) {
            cout << "\nIngrese el año de versión de SolidWorks instalada (e.g., 2023, 2024) o presione ESC para salir: ";
            string entrada = entradaTeclado(4);
            // Validar entrada
            try {
                swVersion = std::stoi(entrada);
                if (!sw.esCompatible(swVersion)) {
                    throw std::invalid_argument("Versión inválida");
                }
                cout << "Procesando la versión: " << swVersion << std::endl;
                break; // Salir del bucle principal si todo es correcto
            }
            catch (const std::exception& e) {
                cout << "Error: " << e.what() << ". Intente nuevamente." << std::endl;
            }
        }

        try {
            sw.setVersion(swVersion);
            string renderer = sw.obtenerRenderer();
            GPU gpu = GPU(renderer);
            guardarArchivoReg(swVersion, gpu.completarContenidoReg(sw.obtenerRegBase()));
            cout << "Finalizado..." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}

