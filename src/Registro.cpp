#include "Registro.h"
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>

using std::string;
using std::vector;

// Arma el contenido del .reg: cabecera, datos base y las líneas recibidas.
string armarArchivoReg(const string& version, const vector<string>& contenido, const string& RVO_VERSION) {
    string fileContent = "Windows Registry Editor Version 5.00\n;# Created with: RealViewOn v" + RVO_VERSION + " #\n;## Base Data: ##\n; - **SW Target:** " + version + "\n";
    for (const auto& line : contenido) {
        fileContent += line + "\n";
    }
    // Remove trailing newlines
    while (!fileContent.empty() && fileContent.back() == '\n') {
        fileContent.pop_back();
    }
    return fileContent;
}

//Guarda el contenido generado en un archivo .reg y maneja posibles errores.
string guardarArchivoReg(const string& version, const string& contenidoArchivo) {
    std::string filePath = "RealViewOn" + version + ".reg";
    std::string fullPath = std::filesystem::current_path().string() + "\\" + filePath;
    std::ofstream regFile(filePath);
    if (!regFile.is_open()) {
        throw std::runtime_error("Could not create the file at\n" + fullPath + ". \nPlease ensure the program has write permissions in the folder and that no file with the same name is open.");
    }
    regFile << contenidoArchivo;
    return fullPath;
}

string guardarArchivoReg(const string& version, const vector<string>& contenido, const string& RVO_VERSION) {
    return guardarArchivoReg(version, armarArchivoReg(version, contenido, RVO_VERSION));
}

string guardarBackUp() {
    // Nombre del archivo de respaldo con la fecha y hora actual
    SYSTEMTIME st;
    GetLocalTime(&st);
    char fileName[100];
    sprintf_s(fileName, "RVO_SWbackup_%02d-%02d-%02d_%02d-%02d.reg", st.wYear % 100, st.wMonth, st.wDay, st.wHour, st.wMinute);

    // reg.exe con su ruta completa y sin ventana. Con system() la GUI mostraba una
    // consola por un instante, y la búsqueda en el PATH podía encontrar otro reg.exe.
    wchar_t sistema[MAX_PATH];
    const UINT largo = GetSystemDirectoryW(sistema, MAX_PATH);
    if (largo == 0 || largo >= MAX_PATH) {
        throw std::runtime_error("Failed to create backup.");
    }
    const std::wstring regExe = std::wstring(sistema) + L"\\reg.exe";
    const string nombre = fileName; // Solo ASCII.
    std::wstring comando = L"\"" + regExe + L"\" export HKEY_CURRENT_USER\\SOFTWARE\\SolidWorks " +
                           std::wstring(nombre.begin(), nombre.end()) + L" /y";

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(regExe.c_str(), comando.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        throw std::runtime_error("Failed to create backup.");
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD salida = 1;
    GetExitCodeProcess(pi.hProcess, &salida);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (salida != 0) {
        throw std::runtime_error("Failed to create backup.");
    }
    return nombre;
}
