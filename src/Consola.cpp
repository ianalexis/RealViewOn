#include "Consola.h"
#include <Windows.h>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "AdvanceMode.h"
#include "GPU.h"
#include "SolidWorks.h"
#include "Teclado.h"

using std::cout;
using std::string;

namespace {

string rendererManual() {
    string manual = "";
    bool correct = false;

    while (manual.empty() || !correct) {
        cout << "Enter the renderer name manually: ";
        manual = entradaTeclado(0, false);
        cout << "Renderer: " << manual << ". Is this correct? ";
        correct = yesOrNo();
    }
    return manual;
}

GPU::Current elegirRenderer(SolidWorks& sw) {
    const auto renderers = sw.candidatosRenderer();
    GPU::Current tempCurrent;
    cout << "Available renderers:\n";
    cout << "0. Enter manually\n";
    for (std::size_t i = 0; i < renderers.size(); i++) {
        cout << i + 1 << ". " << renderers[i].first << " (in " << renderers[i].second << ")\n";
    }
    while (true) {
        cout << "Select the renderer (or press Esc to cancel): ";
        string input = entradaTeclado(static_cast<int>(std::to_string(renderers.size()).length()), true);
        const std::size_t opcion = std::stoul(input);
        if (opcion >= 1 && opcion <= renderers.size()) {
            tempCurrent.renderer = renderers[opcion - 1].first;
            tempCurrent.origin = renderers[opcion - 1].second;
        }
        if (opcion == 0){
            tempCurrent.renderer = rendererManual();
            tempCurrent.origin = "Manual";
        }
        if (!tempCurrent.renderer.empty()) {
            return tempCurrent;
        }
        cout << "Invalid option. Please try again.\n";
    }
}

// Selecciona la marca de la GPU en caso de no poder determinarla.
GPU::Brand elegirMarca() {
    while (true) {
        cout << "Select the GPU brand:\n";
        cout << "1. NVIDIA\n";
        cout << "2. AMD\n";
        cout << "3. Intel\n";
        cout << "Select the GPU brand (or press Esc to cancel): ";
        switch (std::stoi(entradaTeclado(1, true))) {
        case 1:
            return GPU::Brand::NVIDIA;
        case 2:
            return GPU::Brand::AMD;
        case 3:
            return GPU::Brand::INTEL;
        default:
            cout << "Invalid option. Please try again. ESC to cancel.\n";
        }
    }
}

} // namespace

void mostrarVersionesInstaladas(const std::vector<std::pair<int, int>>& versiones) {
    if (versiones.empty()) {
        cout << "No SolidWorks versions found installed.\n";
        return;
    }

    // Guarda los atributos actuales de la consola. Sin consola (salida
    // redirigida) se usa el gris sobre negro por defecto.
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    const WORD saved_attributes = GetConsoleScreenBufferInfo(hConsole, &consoleInfo)
                                      ? consoleInfo.wAttributes
                                      : static_cast<WORD>(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    cout << "\n SW Installed v  | Compatibility\n";
    cout << " ----------------|---------------\n";
    for (const auto& [ano, compatibilidad] : versiones) {
        cout << " SolidWorks " << ano << " | ";
        // Cambia el color y muestra "[OK]", "[X]" o "[?]" utilizando un switch case
        switch (compatibilidad) {
            case 1:  // Compatible
                SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                cout << "[OK]";
                break;
            case 0:  // No compatible
                SetConsoleTextAttribute(hConsole, BACKGROUND_RED | BACKGROUND_INTENSITY);
                cout << "[X]";
                break;
            case 2:  // Versión futura
                SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | BACKGROUND_INTENSITY);
                cout << "[>" << SolidWorks::vMax << "]";
                break;
            default:
                SetConsoleTextAttribute(hConsole, saved_attributes);
                cout << "[?]";
                break;
        }
        // Restaura los atributos originales
        SetConsoleTextAttribute(hConsole, saved_attributes);
        cout << "\n";
    }
    cout << " ----------------|---------------\n";
}

bool preguntarModoGenerico(SolidWorks& sw, int version) {
    // Mismo mensaje con el que setVersion() rechaza la versión, antes de preguntar nada.
    if (sw.esCompatible(version) == 0) {
        throw std::runtime_error("The version of SolidWorks is not compatible.");
    }
    if (!sw.requiereGenerico(version)) {
        return false;
    }
    if (!sw.versionInstalada(version)) {
        cout << "SolidWorks " << version << " is not installed.\n";
    }
    if (sw.esCompatible(version) == 2) {
        if (sw.esDelFuturo(version)) {
            calc(99999);
            cout << "WHAT?!?!?! SolidWorks " << version << " really?!\nHow did you get this version? It's from the future!\n";
        } else {
            cout << "SolidWorks " << version << " is a future compatibility version.\n";
        }
    }
    cout << "Do you want to continue in generic mode? ";
    if (!yesOrNo()) {
        throw std::runtime_error("Installation canceled by the user.");
    }
    return true;
}

GPU::Current elegirCurrent(SolidWorks& sw) {
    GPU::Current current = sw.detectarCurrent();
    if (!current.renderer.empty()) {
        cout << "Renderer detected: " << current.renderer << ". Is this correct? ";
        if (!yesOrNo()) {
            current = GPU::Current{};
        }
    } else {
        cout << "Renderer not detected.\n";
    }
    if (current.renderer.empty()) {
        current = elegirRenderer(sw);
    }
    cout << "Renderer: " << current.renderer << "\n";
    return current;
}

void confirmarMarca(GPU& gpu) {
    if (gpu.getBrand() != GPU::Brand::UNKNOWN) {
        cout << "GPU Brand detected: " << GPU::brandToString(gpu.getBrand()) << "\n";
        return;
    }
    cout << "GPU Brand not detected. Please select the GPU brand manually.\n";
    gpu.setMarcaManual(elegirMarca());
}

std::vector<bool> preguntarOpcionesAvanzadas(const AdvanceMode& advMode, bool generico) {
    const std::vector<AdvanceMode::Opcion> opciones = advMode.opciones();
    std::size_t total = 0;
    for (const auto& opcion : opciones) {
        if (!(generico && opcion.soloInstalada)) {
            total++;
        }
    }
    std::vector<bool> elegidas(opciones.size(), false);
    std::size_t numero = 1;
    for (std::size_t i = 0; i < opciones.size(); i++) {
        if (generico && opciones[i].soloInstalada) {
            continue;
        }
        cout << "(" << numero++ << "/" << total << ") Enable " << opciones[i].nombre
             << (opciones[i].recomendada ? " (Recommended)" : "") << "? ";
        elegidas[i] = yesOrNo();
    }
    return elegidas;
}
