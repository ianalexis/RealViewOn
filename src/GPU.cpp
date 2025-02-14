#include "GPU.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include "Teclado.h"

using std::string;
using std::vector;
using std::cout;

// Constructor
GPU::GPU(Current current) {
    renderer = current.renderer;
    setBrand(current.vendor, renderer);
    setBrWorkarounds(current.workarounds);
}

// Setea el fabricante de la GPU si el renderer contiene alguna palabra clave.
void GPU::setBrand(string v,string r) {
    brand = Brand::UNKNOWN;
    buscarEnRenderMap(v);
    if (brand == Brand::UNKNOWN){
        buscarEnRenderMap(r);
    }
    if (brand == Brand::UNKNOWN){
        cout << "GPU Brand not detected. Please select the GPU brand manually.\n";
        brand = selecectBrandManual();
    }
}

void GPU::setBrWorkarounds(string w) {
    brWorkarounds = empty(w) ? brandKeysMap.at(brand).brandKey : w;
}

// Busca la marca de la GPU en el mapeo de palabras clave.
GPU::Brand GPU::buscarEnRenderMap(string buscado) {
    // Convierte el renderer a mayúsculas para comparar.
    Brand brandEncontrada = Brand::UNKNOWN;
    std::transform(buscado.begin(), buscado.end(), buscado.begin(), ::toupper);
    
    for (const auto& pair : rendererMap) {
        string key = pair.first;
        //cout << "Key buscada: " << key << "\n";
        // Convertir la clave a mayúsculas para comparar.
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        
        if (buscado.find(key) != string::npos) {
            brand = pair.second;
            cout << "GPU Brand detected: " << brandToString(brand) << "\n";
            return brandEncontrada;
        }
    }
    return brandEncontrada;
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
        return "Unknown";
    }
}

// Selecciona la marca de la GPU en caso de no poder determinarla.
GPU::Brand GPU::selecectBrandManual() {
    cout << "Select the GPU brand:\n";
    cout << "1. NVIDIA\n";
    cout << "2. AMD\n";
    cout << "3. Intel\n";
    //cout << "4. Otra\n";
    cout << "Select the GPU brand (or press Esc to cancel): ";
    string input = entradaTeclado(1, true);
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
        cout << "Invalid option. Please try again. ESC to cancel.\n";
        return selecectBrandManual();
    }
}

vector<string> GPU::completarContenidoReg(const vector<string>& regBase) {
    vector<string> result;
    for (const auto& reg : regBase) {
        result.push_back("\n;RealView Enabler:");
        result.push_back(reg + "\\Gl2Shaders\\"+ brandKeysMap.at(brand).glPath +"\\" + renderer + "]");
        result.push_back("\"Workarounds\"=dword:" + brandKeysMap.at(brand).glKey);
        result.push_back(";GL2Shaders Alternative Workarounds: " + brandKeysMap.at(brand).glKeyAlt + "\n");
        result.push_back(";Sketchs and visual errors for Performance for Brand " + brandKeysMap.at(brand).brPath);
        result.push_back(reg + "\\"+ brandKeysMap.at(brand).brPath +"\\" + renderer + "]");
        result.push_back("\"Workarounds\"=dword:" + brWorkarounds);
        result.push_back(";Brand Alternative Workarounds: " + brandKeysMap.at(brand).brandKeyAlt);
    }
    return result;
}
