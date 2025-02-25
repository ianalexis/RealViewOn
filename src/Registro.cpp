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
void guardarArchivoReg(string& version, const vector<string>& contenido, const string& RVO_VERSION) {
    std::string filePath = "RealViewEnable" + version + ".reg";
    std::string fullPath = std::filesystem::current_path().string() + "\\" + filePath;
    std::ofstream regFile(filePath);
    if (regFile.is_open()) {
        std::string fileContent = "Windows Registry Editor Version 5.00\n;# Created with: RealViewOn v" + RVO_VERSION + " #\n;## Base Data: ##\n; - **SW Target:** " + version + "\n";
        for (const auto& line : contenido) {
            fileContent += line + "\n";
        }
        // Remove trailing newlines
        while (!fileContent.empty() && fileContent.back() == '\n') {
            fileContent.pop_back();
        }
        regFile << fileContent;
        regFile.close();
        cout << "\nFile " << filePath << " created successfully at:\n" << fullPath << std::endl;
    }
    else {
        throw std::runtime_error("Could not create the file at\n" + fullPath + ". \nPlease ensure the program has write permissions in the folder and that no file with the same name is open.");
    }
}