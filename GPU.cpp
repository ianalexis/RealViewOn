#include "GPU.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>

using std::string;


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

string GPU::completarContenidoReg(const std::vector<string>& regBase) {
    regFull = "";
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

string GPU::completarContenidoRegNVIDIA(const std::vector<string>& regBase) {
    for (const auto& reg : regBase) {
        regFull.append(reg + "\\NVIDIA Corporation\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey + "\n");
        regFull.append(reg + "\\Gl2Shaders\\NV40\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey + "\n");
    }
    return regFull;
}

// Completa el contenido del archivo .reg para GPUs AMD.

string GPU::completarContenidoRegAMD(const std::vector<string>& regBase) {
    for (const auto& reg : regBase) {
        regFull.append(reg + "\\ATI Technologies Inc.\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey + "\n");
        regFull.append(reg + "\\Gl2Shaders\\RV900\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey + "\n"); //TODO: Segun https://github.com/TrevorAvrett/SolidWorks-RealView-Enabler/blob/master/Source%20Code%20(c%23) es R420
    }
    return regFull;
}

// Completa el contenido del archivo .reg en modo genérico (INTEL). TODO: Revisar con placas raras?

string GPU::completarContenidoRegGenerico(const std::vector<string>& regBase) {
    for (const auto& reg : regBase) {
        regFull.append(reg + "\\Intel\\" + renderer + "]\n\"Workarounds\"=dword:" + brandKey + "\n");
        regFull.append(reg + "\\Gl2Shaders\\Other\\" + renderer + "]\n\"Workarounds\"=dword:" + glKey + "\n");
    }
    return regFull;
}