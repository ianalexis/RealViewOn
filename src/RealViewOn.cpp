#include <windows.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "SolidWorks.h"
#include "GPU.h"
#include "Teclado.h"
#include "Registro.h"
#include "Playmidi.h"
#include "Version.h"
#include <thread>

using std::cin;
using std::cout;
using std::string;

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
        system("color 17"); // Fondo azul oscuro y texto blanco
        // Predefinir tamaño de consola
        HWND console = GetConsoleWindow();
        RECT r;
        GetWindowRect(console, &r); // Obtener el tamaño actual de la consola
        MoveWindow(console, r.left, r.top, 825, 600, TRUE); // Cambiar el tamaño de la consola a 800x600
        // Bloquear el redimensionamiento de la ventana
        LONG style = GetWindowLong(console, GWL_STYLE);
        style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX); // Quitar estilos de redimensionamiento
        SetWindowLong(console, GWL_STYLE, style);
        playMidiAsync();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

void encabezado() {
    cout    << " >>=============================================================================================<<\n"
            << " ||                                                                                             ||\n"
            << " || 8888888b.                   888 888     888 d8b                         .d88888b.           ||\n"
            << " || 888   Y88b                  888 888     888 Y8P                        d88P* *Y88b          ||\n"
            << " || 888    888                  888 888     888                            888     888          ||\n"
            << " || 888   d88P .d88b.   8888b.  888 Y88b   d88P 888  .d88b.  888  888  888 888     888 88888b.  ||\n"
            << " || 8888888P* d8P  Y8b     *88b 888  Y88b d88P  888 d8P  Y8b 888  888  888 888     888 888 *88b ||\n"
            << " || 888 1984  88888888 .d888888 888   Y88o88P   888 88888888 888  888  888 888     888 888  888 ||\n"
            << " || 888  T88b Y8b.     888  888 888    Y888P    888 1994     Y88b 888 d88P Y88b. .d88P 888  888 ||\n"
            << " || 888   T88b *Y8888  *Y888888 888     Y8P     888  *Y8888   *Y8888888P*   *Y88888P*  888  888 ||\n"
            << " ||                                                                                             ||\n"
            << " >>=============================================================================================<<\n";
    string data = "v" + string(RVO_VERSION) + " c" + string(RVO_COMPILATION) + " - by [RF47] && [TitanBoreal]";
    int width = 94; // Ancho total de la línea
    int padding = (width - data.length()) / 2;
    cout << " ||" << std::setw(padding) << "" << data << std::setw(padding) << "" << "||\n";
    cout << " >>=============================================================================================<<" << std::endl;
}

// Función principal
int main() {
    configurarConsola();
    encabezado();
    SolidWorks sw;
    try {
        sw.obtenerVersionesInstaladas();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cout << "Press any key to exit..." << std::endl;
        _getch();
        return 1;
    }

    // While para que el programa reinicie si se desea probar otra versión.
    while (true) {
        // Bucle principal para solicitar la versión de SolidWorks
        while (true) {
            cout << "\nEnter the SolidWorks version year (e.g., 2020, 2024) or press ESC to exit: ";
            string entrada = entradaTeclado(4, true);
            // Validar entrada
            try {
                int swVersion = std::stoi(entrada);
                if (sw.esCompatible(swVersion) == 0) {
                    throw std::invalid_argument("Incompatible version");
                }
                cout << "Processing version: " << swVersion << std::endl;
                sw.setVersion(swVersion);
                GPU gpu(sw.obtenerCurrent());
                guardarArchivoReg(swVersion, gpu.completarContenidoReg(sw.obtenerRegBase()));
                cout << "Finished..." << std::endl;
                break; // Salir del bucle principal si todo es correcto
            }
            catch (const std::exception& e) {
                cout << "Error: " << e.what() << ". Try again." << std::endl;
            }
        }
    }
    return 0;
}