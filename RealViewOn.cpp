#include <windows.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "SolidWorks.h"
#include "GPU.h"
#include "teclado.h"
#include "Registro.h"
#include "Playmidi.h"
#include <thread>

using std::cin;
using std::cout;
using std::string;

std::string getVersionFromDateTime() {
    std::tm t = {};
    std::istringstream date_ss(__DATE__);
    date_ss >> std::get_time(&t, "%b %d %Y");

    std::istringstream time_ss(__TIME__);
    time_ss >> std::get_time(&t, "%H:%M:%S");

    std::ostringstream version_ss;
    version_ss << std::setfill('0') << std::setw(2) << (t.tm_year % 100)
               << std::setw(2) << (t.tm_mon + 1)
               << std::setw(2) << t.tm_mday
               << std::setw(2) << t.tm_hour
               << std::setw(2) << t.tm_min;

    return version_ss.str();
}

const std::string RVO_VERSION = getVersionFromDateTime();

// Prototipos de funciones
void configurarConsola();

void playMidiAsync() {
    // Esta función se ejecutará en un hilo separado
    try{
        std::thread midiThread(playMidi, loadEmbeddedMidi());
        midiThread.detach();
    } catch (const std::exception& ex) {
        std::cerr << "Music error: " << ex.what() << std::endl;
    }
}

// Definición de funciones
void configurarConsola() {
    try {
        SetConsoleOutputCP(CP_UTF8); // Soporte para UTF-8
        system("color 17"); // Fondo azul oscuro y texto verde
        playMidiAsync();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

// Función principal
int main() {
    configurarConsola();
    cout << "\n";
    cout << "██████                          ███  ███                     ████████        \n";
    cout << " ██  ██ ███████   ████  ███      ██  ██ ████ ███████ ███ ████ ██  ██  ███ ███\n";
    cout << " ██  ██  ██  ██  ██  ██  ██      ██  ██  ██  ██  ██  ██   ██  ██  ██  ███ ██ \n";
    cout << " ██  ██  ██  ██  ██  ██  ██      ██  ██  ██  ██  ██  ██   ██  ██  ██  ██████ \n";
    cout << " ██  ██  ██      ██  ██  ██      ██  ██  ██  ██      ██   ██  ██  ██  ██ ███ \n";
    cout << " █████   █████   ██████  ██      ██  ██  ██  █████   ██   ██  ██  ██  ██  ██ \n";
    cout << " ██  ██  ██      ██  ██  ██      ██  ██  ██  ██      ██ █ ██  ██  ██  ██  ██ \n";
    cout << " ██  ██  ██  ██  ██  ██  ██  ██  ██  ██  ██  ██  ██  ███████  ██  ██  ██  ██ \n";
    cout << " ██  ██  ██  ██  ██  ██  ██  ██  ██████  ██  ██  ██  ███████  ██  ██  ██  ██ \n";
    cout << " ██  ██ ███████ ███ ████████████  ████       ███████ ███  ███ ██  ██  ███ ███\n";
    cout << "███ ███                            ██                        ████████        \n";
    cout << "------------------------------------------\n";
    cout << "|v" << RVO_VERSION << " - by [RF47] && [TitanBoreal]|\n";
    cout << "------------------------------------------\n";
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
            cout << "\nIngrese el año de versión de SolidWorks (e.g., 2020, 2024) o presione ESC para salir: ";
            string entrada = entradaTeclado(4);
            // Validar entrada
            try {
                int swVersion = std::stoi(entrada);
                if (sw.esCompatible(swVersion) == 0) {
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