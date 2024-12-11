#include "Registro.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;

//Guarda el contenido generado en un archivo .reg y maneja posibles errores.
void guardarArchivoReg(int& version, const vector<string>& contenido) {
    std::ofstream regFile("RealViewEnable" + std::to_string(version) + ".reg");
    if (regFile.is_open()) {
        regFile << "Windows Registry Editor Version 5.00\n\n";
        for (const auto& line : contenido) {
            regFile << line << "\n";
        }
        regFile.close();
        cout << "File " << "RealViewEnable" + std::to_string(version) + ".reg created successfully." << std::endl;
    }
    else {
        throw std::runtime_error("Could not create the .reg file. \n Please ensure the program has write permissions in the folder and that no file with the same name is open.");
    }
}