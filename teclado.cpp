#include "teclado.h"
#include <string>
#include <iostream>
#include <conio.h>
#include <Windows.h>

using std::string;
using std::cout;
using std::cin;

// Funcion que lee por teclado la cantidad de caracteres recibidas y chequea que no sea la tecla escape. En caso de ser escape hace EXIT.
string entradaTeclado(int caracteres) {
    string entrada = "";
    while (entrada.length() < caracteres) {
        // Detectar Escape
        if (_kbhit()) {
            char ch = _getch();

            if (ch == 27) { // Escape presionado
                cout << "\nSaliendo del programa...\n";
                exit(0);
            }
            //else if (ch == '\r') { // Enter presionpado
            //    //cout << "\n";
            //     // Terminar la entrada
            //                }
            else if (isdigit(ch) || isalpha(ch)) { // Agregar dígitos y letras válidos
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
    cout << "\n";
    return entrada;
}

bool yesOrNo() {
    char opcion;
    while (true) {
        opcion = entradaTeclado(1).at(0);
        if (opcion == 'y' || opcion == 'Y') {
            return true;
        }
        else if (opcion == 'n' || opcion == 'N') {
            return false;
        }
        else {
            cout << "Opción inválida. Intente nuevamente: ";
        }
    }
}