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
    setBaseData(current);
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
        baseData.vendor += brandToString(brand);
    }
}

void GPU::setBrWorkarounds(string w) {
    brWorkarounds = (w.empty() || w == brandKeysMap.at(brand).brandBaseAvoid) ? brandKeysMap.at(brand).brandKey : w;
}

void GPU::setBaseData(Current current) {
    baseData.renderer = !current.renderer.empty() ? "; - **Renderer:** " + current.renderer : "";
    baseData.vendor = !current.vendor.empty() ? "; - **Vendor:** " + current.vendor : "";
    baseData.workarounds = !current.workarounds.empty() ? "; - **Workarounds:** " + current.workarounds : "";
    baseData.origin = !current.origin.empty() ? "; - **Origin:** " + current.origin : "";
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
    if (!baseData.vendor.empty()){
        baseData.vendor += "\n";
    }
    baseData.vendor += ";   - Manual selected: ";
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

vector<string> GPU::completarRealViewEnabler(string reg){
    vector<string> rvEnabler;
    rvEnabler.push_back(reg + "\\Gl2Shaders\\"+ brandKeysMap.at(brand).glPath +"\\" + renderer + "]");
    rvEnabler.push_back("\"Workarounds\"=dword:" + brandKeysMap.at(brand).glKey);
    return rvEnabler;
}

vector<string> GPU::completarPerformanceFix(string reg){
    vector<string> performanceFix;
    performanceFix.push_back(reg + "\\"+ brandKeysMap.at(brand).brPath +"\\" + renderer + "]");
    performanceFix.push_back("\"Workarounds\"=dword:" + brWorkarounds);
    return performanceFix;
}

vector<string> GPU::completarContenidoReg(const vector<string>& regBase) {
    vector<string> result;
    result.push_back(baseDataToString());
    result.push_back("\n;## RealView Enabler: ##\n;```");// Completa con RealView Enabler
    for (const auto& reg : regBase) {
        for (const auto& rv : completarRealViewEnabler(reg)) {
            result.push_back(rv);
        }
    }
    result.push_back(";```\n;GL2Shaders Alternative Workarounds: " + brandKeysMap.at(brand).glKeyAlt + "\n\n");

    result.push_back(";## Performance ##\n; - Sketches and visual fixes for  " + brandKeysMap.at(brand).brPath + "\n;```"); // Completa con Performance Fix
    for (const auto& reg : regBase) { // Completa con Performance Fix
        for (const auto& pf : completarPerformanceFix(reg)) {
            result.push_back(pf);
        }
    }
    result.push_back(";```\n;Brand Alternative Workarounds: " + brandKeysMap.at(brand).brandKeyAlt);
    return result;
}

string GPU::baseDataToString() {
    string baseDataString = "";
    baseDataString += !baseData.renderer.empty() ? baseData.renderer + "\n" : "";
    baseDataString += !baseData.vendor.empty() ? baseData.vendor + "\n" : "";
    baseDataString += !baseData.workarounds.empty() ? baseData.workarounds + "\n" : "";
    baseDataString += !baseData.origin.empty() ? baseData.origin + "\n" : "";
    return baseDataString;
} 
