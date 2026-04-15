#include "Teclado.h"
#include <string>
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <conio.h>
#include "Misc.h"

using std::string;
using std::cout;
using std::cin;

// Captura una entrada de teclado del usuario con un límite caracteres (<= 0 para indefinido) y la opción de aceptar solo números. Si recibe escape cierra el programa.
string entradaTeclado(int caracteres, bool number) {
    string entrada = "";
    bool done = false;
    while (!done) {
        int ch = _getch();
        if (ch == 0 || ch == 224) {
            // Teclas extendidas (flechas, F-keys): descartar escaneo secundario.
            _getch();
            continue;
        }

        if (ch == 27) { // Escape presionado
            cout << "\nExiting the program...\n";
            std::exit(0);
        } else if (ch == '\b' && !entrada.empty()) { // Retroceso
            entrada.pop_back();
            cout << "\b \b"; // Borrar en pantalla
        } else if (caracteres <= 0 && ch == '\r') { // Enter presionado
            done = true;
        } else {
            const unsigned char inputChar = static_cast<unsigned char>(ch);
            bool aceptarCaracter = false;

            if (number) {
                aceptarCaracter = std::isdigit(inputChar) != 0;
            } else if (caracteres <= 0) {
                aceptarCaracter = std::isprint(inputChar) != 0;
            } else {
                aceptarCaracter = std::isalnum(inputChar) != 0;
            }

            if (aceptarCaracter) {
                entrada += static_cast<char>(ch);
                cout << static_cast<char>(ch); // Mostrar en pantalla
            }

            if (aceptarCaracter && caracteres > 0 && entrada.length() >= static_cast<size_t>(caracteres)) {
                done = true;
            }
        }
    }
    cout << std::endl;
    if (number && !entrada.empty()) {
        const int value = std::stoi(entrada);
        if (numberTextMap.find(value) != numberTextMap.end()) {
            calc(value);
        }
    }
    return entrada;
}

// Solicita, rebice y traduce una respuesta de sí o no del usuario.
bool yesOrNo() {
    char opcion;
    while (true) {
        cout << "[Y]es/[N]o: ";
        opcion = entradaTeclado(1,false).at(0);
        if (opcion == 'y' || opcion == 'Y') {
            return true;
        }
        else if (opcion == 'n' || opcion == 'N') {
            return false;
        }
        else {
            cout << "Invalid option. ";
        }
    }
}

void calc(int n) {
    cout << numberTextMap[n];
    cout << std::endl;
}
