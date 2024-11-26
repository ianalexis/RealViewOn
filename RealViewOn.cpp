#include <windows.h>
#include <iostream>
#include <string>
#include <conio.h>
#include "SolidWorks.h"
#include "GPU.h"
#include "teclado.h"
#include "Registro.h"

using std::cin;
using std::cout;
using std::string;

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
    SolidWorks sw;

    try {
        sw.obtenerVersionesInstaladas();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cout << "Presione cualquier tecla para salir..." << std::endl;
        _getch();
        return 1;
    }

    // While para que el programa reinicie si se desea probar otra versión.
    while (true) {
        // Bucle principal para solicitar la versión de SolidWorks
        while (true) {
            cout << "\nIngrese el año de versión de SolidWorks instalada (e.g., 2023, 2024) o presione ESC para salir: ";
            string entrada = entradaTeclado(4);
            // Validar entrada
            try {
                int swVersion = std::stoi(entrada);
                if (!sw.esCompatible(swVersion)) {
                    throw std::invalid_argument("Versión inválida");
                }
                cout << "Procesando la versión: " << swVersion << std::endl;
                sw.setVersion(swVersion);
                GPU gpu(sw.obtenerRenderer());
                guardarArchivoReg(swVersion, gpu.completarContenidoReg(sw.obtenerRegBase()));
                cout << "Finalizado..." << std::endl;
                break; // Salir del bucle principal si todo es correcto
            }
            catch (const std::exception& e) {
                cout << "Error: " << e.what() << ". Intente nuevamente." << std::endl;
            }
        }
    }
    return 0;
}