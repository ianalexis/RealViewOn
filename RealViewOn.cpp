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

bool checkEscape() {
    if (_kbhit()) {
        int ch = _getch(); // Captura tecla presionada
        if (ch == 27) {    // Código ASCII para Escape
            return true;   // Indica que Escape fue presionado
        }
    }
    return false; // No se presionó Escape
}

// Valida que la version esté en el listado de versiones instaladas
bool versionInstalada(int v) {
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
        versionesInstaladas = sw.obtenerVersionesInstaladas();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cerrar();
    }

    while (true) {
        
        while (true) {
            cout << "\nIngrese el año de versión de SolidWorks instalada (e.g., 2023, 2024) o presione ESC para salir: ";

            string entrada;
            while (true) {
                // Detectar Escape
                if (_kbhit()) {
                    char ch = _getch();

                    if (ch == 27) { // Escape presionado
                        cout << "\nSaliendo del programa...\n";
                        return 0;
                    }
                    else if (ch == '\r') { // Enter presionado
                        cout << "\n";
                        break; // Terminar la entrada
                    }
                    else if (isdigit(ch)) { // Agregar dígitos válidos
                        entrada += ch;
                        cout << ch; // Mostrarlo en pantalla
                    }
                    else if (ch == '\b' && !entrada.empty()) { // Retroceso
                        entrada.pop_back();
                        cout << "\b \b"; // Borrar en pantalla
                    }
                }

                Sleep(50); // Pausa breve para evitar saturar el CPU
            }

            // Validar entrada
            if (!entrada.empty()) {
                try {
                    swVersion = std::stoi(entrada);
                    cout << "Procesando la versión: " << swVersion << std::endl;
                    break; // Salir del bucle principal si todo es correcto
                }
                catch (const std::exception&) {
                    cout << "Entrada inválida. Intente nuevamente.\n";
                }
            }
            else {
                cout << "No se ingresó ninguna versión. Intente nuevamente.\n";
            }
        }


        try {
            // Verificar si la versión de SolidWorks está instalada o si se desea continuar con una instalación forzada genérica.
            if (!versionInstalada(swVersion)) {
                cout << "Error: La versión de SolidWorks no está instalada." << std::endl;
                cout << "Desea continuar con una instalacion forzada genérica? (Y/N): ";
                char opcion;
                while (true) {
                    if (checkEscape()) return 0;
                    if (_kbhit()) {
                        cin >> opcion;
                        break;
                    }
                }
                if (opcion == 'y' || opcion == 'Y') {
                    cout << "Continuando en modo genérico..." << std::endl;
                    sw.setGenerico(true);
                }
                else {
                    cout << "Instalación cancelada." << std::endl;
                    Sleep(1000);
                    continue;  // Vuelve al inicio del bucle principal
                }
            }

            sw.setVersion(swVersion);
            string renderer = sw.obtenerRenderer();
            GPU gpu = GPU(renderer);
            guardarArchivoReg(gpu.completarContenidoReg(sw.obtenerRegBase()));

            cout << "Presione enter para continuar o Escape para salir..." << std::endl;
            while (true) {
                if (checkEscape()) return 0;
                if (_kbhit() && _getch() == '\r') break;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            cerrar();
        }
    }

    return 0;
}

