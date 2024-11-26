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
        for (const auto& line : contenido) {
            regFile << line << "\n";
        }
        regFile.close();
        cout << "Archivo " << "RealViewEnable" + std::to_string(version) + ".reg creado exitosamente." << std::endl;
    }
    else {
        throw std::runtime_error("No se pudo crear el archivo .reg. \n Verifique que el programa tenga permisos de escritura en la carpeta y no haya un archivo con el mismo nombre abierto.");
    }
}

//Guarda en el registro directamente sin generar archivos de por medio.
void escribirRegDirecto(const vector<string>& contenido) { // TODO: Ver como implementar.
    for (const auto& line : contenido) {

    }
}