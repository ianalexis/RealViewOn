#include "GPU.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include "teclado.h"

using std::string;
using std::vector;
using std::cout;

// Constructor
GPU::GPU(const std::string& r) {
    renderer = r;
    setBrand(r);
}

// Setea el fabricante de la GPU si el renderer contiene alguna palabra clave.
void GPU::setBrand(string r) {
    // Convierte el renderer a mayúsculas para comparar.
    std::transform(r.begin(), r.end(), r.begin(), ::toupper);
    
    for (const auto& pair : rendererMap) {
        string key = pair.first;
        // Convertir la clave a mayúsculas para comparar.
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        
        if (r.find(key) != string::npos) {
            brand = pair.second;
            cout << "Marca de GPU detectada: " << brandToString(brand) << "\n";
            return;
        }
    }
    brand = selecectBrand();
}

std::string GPU::brandToString(GPU::Brand brand) {
    switch (brand) {
    case GPU::Brand::NVIDIA:
        return "NVIDIA";
    case GPU::Brand::AMD:
        return "AMD";
    case GPU::Brand::INTEL:
        return "Intel";
    default:
        return "Desconocida";
    }
}

// Selecciona la marca de la GPU en caso de no poder determinarla.
GPU::Brand GPU::selecectBrand() {
    cout << "Seleccione la marca de la GPU:\n";
    cout << "1. NVIDIA\n";
    cout << "2. AMD\n";
    cout << "3. Intel\n";
    //cout << "4. Otra\n";
    cout << "Seleccione la marca de la GPU (o presione Esc para cancelar): ";
    string input = entradaTeclado(1);
    int opcion = std::stoi(input);
    switch (opcion) {
    case 1:
        return Brand::NVIDIA;
    case 2:
        return Brand::AMD;
    case 3:
        return Brand::INTEL;
    //case 4:
    //    return Brand::UNKNOWN;
    default:
        cout << "Opción inválida. Intente nuevamente.\n";
        return selecectBrand();
    }
}

vector<string> GPU::completarContenidoReg(const vector<string>& regBase) {
    regFull.clear();
    switch (brand) {
    case Brand::NVIDIA:
        return completarContenidoRegNVIDIA(regBase);
    case Brand::AMD:
        return completarContenidoRegAMD(regBase);
    case Brand::INTEL:
        return completarContenidoRegIntel(regBase);
    default: // Marca desconocida
        throw std::invalid_argument("Marca de GPU desconocida para " + renderer);
    }
}

// Completa el contenido del archivo .reg para GPUs NVIDIA.
vector<string> GPU::completarContenidoRegNVIDIA(const vector<string>& regBase) {
    vector<string> result;
    for (const auto& reg : regBase) {
        result.push_back(reg + "\\NVIDIA Corporation\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey);
        result.push_back(reg + "\\Gl2Shaders\\NV40\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey);
    }
    return result;
}

// Completa el contenido del archivo .reg para GPUs AMD.
vector<string> GPU::completarContenidoRegAMD(const vector<string>& regBase) { //TODO: Segun https://github.com/TrevorAvrett/SolidWorks-RealView-Enabler/blob/master/Source%20Code%20(c%23) es R420
    vector<string> result;
    for (const auto& reg : regBase) {
        result.push_back(reg + "\\ATI Technologies Inc.\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey);
        result.push_back(reg + "\\Gl2Shaders\\RV900\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey);
    }
    return result;
}

// Completa el contenido del archivo .reg en modo genérico (INTEL).
vector<string> GPU::completarContenidoRegIntel(const vector<string>& regBase) { // TODO: Revisar con placas raras?
    vector<string> result;
    for (const auto& reg : regBase) {
        result.push_back(reg + "\\Intel\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey);
        result.push_back(reg + "\\Gl2Shaders\\Other\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey);
    }
    return result;
}