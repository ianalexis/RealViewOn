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
#include "AdvanceMode.h"

using std::cin;
using std::cout;
using std::string;

void playMidiAsync() {
    // Esta función se ejecutará en un hilo separado
    try{
        std::thread midiThread(playMidi, loadEmbeddedMidi());
        midiThread.detach();
    } catch (const std::exception& ex) {
        std::cerr << "Music error: " << ex.what() << std::endl;
    }
}

void terminalColorAuto(bool advMode){
    if (FILE_VERSION_STABLE == 1) { // Si es estable
        if (!advMode) {
            system("color 17"); // Fondo azul oscuro y texto blanco
        } else {
            system("color 0B"); // Fondo negro y texto aguamarina claro
        }
    } else {
        if (!advMode) {
            system("color 4E"); // Fondo rojo y texto amarillo
        } else {
            system("color 06"); // Fondo negro y texto amarillo oscuro
        }
    }
}

// Definición de funciones
void configurarConsola() {
    try {
        SetConsoleOutputCP(CP_UTF8); // Soporte para UTF-8
        terminalColorAuto(false);
        // Predefinir tamaño de consola
        HWND console = GetConsoleWindow();
        RECT r;
        GetWindowRect(console, &r); // Obtener el tamaño actual de la consola
        MoveWindow(console, r.left, r.top, 825, 700, TRUE); // Cambiar el tamaño de la consola a 825x700 (Tamaño perfecto para todos los easteregg)
        // Bloquear el redimensionamiento de la ventana
        LONG style = GetWindowLong(console, GWL_STYLE);
        style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX); // Quitar estilos de redimensionamiento
        SetWindowLong(console, GWL_STYLE, style);
        playMidiAsync();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

void lineaEncabezado(string texto) {
    int width = 93; // Ancho total de la línea
    int padding = (width - texto.length()) / 2;
    cout << " ||" << std::setw(padding) << "" << texto << std::setw(padding) << "" << " ||\n";
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
    lineaEncabezado("v" + string(RVO_VERSION) + " c" + string(RVO_COMPILATION) + " - " + releaseType() + " - by [RF47] && [IanAlexis]");
    lineaEncabezado("Oficial Repository: https://github.com/ianalexis");
    cout << " >>=============================================================================================<<" << std::endl;
}

vector<string> regContent;
SolidWorks sw;
AdvanceMode advMode;

void modoAvanzado(int swVersion, bool generico) {
    cout << "Advanced Mode? (Recommended) (Y/N): ";
    if (yesOrNo()) {
        terminalColorAuto(true);
        try {
            guardarBackUp();
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
        advMode.setSwVersion(swVersion, generico);
        regContent.push_back(advMode.askAdvanceOptions());
        terminalColorAuto(false);
    }
}

// Función principal
int main() {
    configurarConsola();
    encabezado();
    try {
        sw.obtenerVersionesInstaladas();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // While para que el programa reinicie si se desea probar otra versión.
    while (true) {
        // Bucle principal para solicitar la versión de SolidWorks
        while (true) {
            cout << "\nEnter the SolidWorks version year (e.g. " + sw.versions + ") or press ESC to exit: ";
            string entrada = entradaTeclado(4, true);
            // Validar entrada
            try {
                int swVersion = std::stoi(entrada);
                if (sw.esCompatible(swVersion) == 0) {
                    throw std::invalid_argument("Incompatible version");
                }
                sw.setVersion(swVersion);
                GPU gpu(sw.obtenerCurrent());
                regContent = gpu.completarContenidoReg(sw.obtenerRegBase());
                modoAvanzado(swVersion, sw.getGenerico());
                guardarArchivoReg(std::to_string(swVersion), regContent, RVO_VERSION);
                break; // Salir del bucle principal si todo es correcto
            }
            catch (const std::exception& e) {
                cout << "Error: " << e.what() << ". Try again." << std::endl;
            }
        }
    }
    return 0;
}