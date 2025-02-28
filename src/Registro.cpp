#include "Registro.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>

using std::string;
using std::vector;
using std::cout;

//Guarda el contenido generado en un archivo .reg y maneja posibles errores.
void guardarArchivoReg(const string& version, const vector<string>& contenido, const string& RVO_VERSION) {
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

void guardarBackUp() {
    // Nombre del archivo de respaldo con la fecha y hora actual
    SYSTEMTIME st;
    GetLocalTime(&st);
    char fileName[100];
    sprintf_s(fileName, "SWbackup_%02d-%02d-%02d_%02d-%02d.reg", st.wYear % 100, st.wMonth, st.wDay, st.wHour, st.wMinute);

    // Comando para exportar la clave del registro
    std::string command = "reg export HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks ";
    command += fileName;
    command += " /y > nul 2>&1";

    // Ejecutar el comando
    if (system(command.c_str()) == 0) {
        cout << "Backup created successfully: " << fileName << std::endl;
    } else {
        throw std::runtime_error("Failed to create backup.");
    }
}