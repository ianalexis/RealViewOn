#include "GPU.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>

using std::string;
using std::vector;

// Constructor
GPU::GPU(string r) {
    renderer = r;
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
        regFull = completarContenidoRegNVIDIA(regBase);
        break;
    case Brand::AMD:
        regFull = completarContenidoRegAMD(regBase);
        break;
    case Brand::UNKNOWN:
        regFull = completarContenidoRegGenerico(regBase);
        break;
    }
    return regFull;
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

vector<string> GPU::completarContenidoRegAMD(const vector<string>& regBase) {
    vector<string> result;
    for (const auto& reg : regBase) {
        result.push_back(reg + "\\ATI Technologies Inc.\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey);
        result.push_back(reg + "\\Gl2Shaders\\RV900\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey); //TODO: Segun https://github.com/TrevorAvrett/SolidWorks-RealView-Enabler/blob/master/Source%20Code%20(c%23) es R420
    }
    return result;
}

// Completa el contenido del archivo .reg en modo genérico (INTEL). TODO: Revisar con placas raras?
vector<string> GPU::completarContenidoRegGenerico(const vector<string>& regBase) {
    vector<string> result;
    for (const auto& reg : regBase) {
        result.push_back(reg + "\\Intel\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey);
        result.push_back(reg + "\\Gl2Shaders\\Other\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey);
    }
    return result;
}