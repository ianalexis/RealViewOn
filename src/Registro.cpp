#include "Registro.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

using std::string;
using std::vector;
using std::cout;

//Guarda el contenido generado en un archivo .reg y maneja posibles errores.
void guardarArchivoReg(int& version, const vector<string>& contenido, const string& RVO_VERSION) {//TODO: Modificar para que en lugar de ir escribiendo, primero pase por un string nuevo y limpie saltos de lineas al final para luego escribirlo todo de una.
    std::string filePath = "RealViewEnable" + std::to_string(version) + ".reg";
    std::string fullPath = std::filesystem::current_path().string() + "\\" + filePath;
    std::ofstream regFile(filePath);
    if (regFile.is_open()) {
        regFile << "Windows Registry Editor Version 5.00\n;# Created with: RealViewOn v" << RVO_VERSION << " #\n;## Base Data: ##\n; - **SW Target:** " << version << "\n";
        for (const auto& line : contenido) {
            regFile << line << "\n";
        }
        regFile.close();
        cout << "File " << filePath << " created successfully at:\n" << fullPath << std::endl;
    }
    else {
        throw std::runtime_error("Could not create the file at\n" + fullPath + ". \nPlease ensure the program has write permissions in the folder and that no file with the same name is open.");
    }
}