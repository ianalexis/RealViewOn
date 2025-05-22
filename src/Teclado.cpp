#include "Teclado.h"
#include <string>
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include "Misc.h"

using std::string;
using std::cout;
using std::cin;

// Captura una entrada de teclado del usuario con un límite caracteres (<= 0 para indefinido) y la opción de aceptar solo números. Si recibe escape cierra el programa.
string entradaTeclado(int caracteres, bool number) {
    string entrada = "";
    bool done = false;
    while (!done) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 27) { // Escape presionado
                cout << "\nExiting the program...\n";
                exit(0);
            } else if (ch == '\b' && !entrada.empty()) { // Retroceso
                entrada.pop_back();
                cout << "\b \b"; // Borrar en pantalla
            } else if (caracteres <= 0 && ch == '\r') { // Enter presionado
                done = true;
            } else if (caracteres <= 0 || (number && isdigit(ch)) || (!number && (isdigit(ch) || isalpha(ch)))) {
                entrada += ch;
                cout << ch; // Mostrar en pantalla
                if (caracteres > 0 && entrada.length() >= caracteres) {
                    done = true;
                }
            }
        }
        Sleep(50); // Pausa breve para evitar saturar el CPU
    }
    cout << std::endl;
    if (number && numberTextMap.find(std::stoi(entrada)) != numberTextMap.end()) {
        calc(std::stoi(entrada));
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
