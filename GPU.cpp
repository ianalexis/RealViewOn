#include "GPU.h"
#include <string>
#include <iostream>
#include <vector>

using std::string;
using std::vector;

// Constructor
GPU::GPU(const string& r) : renderer(r) {
    setBrand();
}

// Setea el fabricante de la GPU si el renderer contiene alguna palabra clave.
void GPU::setBrand() {
    for (const auto& pair : rendererMap) {
        if (renderer.find(pair.first) != string::npos) {
            brand = pair.second;
            break;
        }
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