#include "GPU.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <vector>

using std::string;
using std::vector;

// Constructor
GPU::GPU(Current current) {
    setBaseData(current);
    renderer = current.renderer;
    workaroundsLeidos = current.workarounds;
    brand = detectarMarca(current.vendor, renderer);
    // Sin marca no hay claves de referencia: los workarounds se resuelven en
    // setMarcaManual().
    if (brand != Brand::UNKNOWN) {
        setBrWorkarounds(workaroundsLeidos);
    }
}

GPU::Brand GPU::getBrand() const {
    return brand;
}

void GPU::setMarcaManual(Brand marca) {
    if (marca == Brand::UNKNOWN) {
        throw std::invalid_argument("A GPU brand must be selected.");
    }
    brand = marca;
    if (!baseData.vendor.empty()) {
        baseData.vendor += "\n";
    }
    baseData.vendor += ";   - Manual selected: " + brandToString(marca);
    setBrWorkarounds(workaroundsLeidos);
}

// El vendor tiene prioridad: el renderer solo se mira si el vendor no coincide.
GPU::Brand GPU::detectarMarca(const string& vendor, const string& renderer) {
    const Brand porVendor = buscarMarca(vendor);
    return porVendor != Brand::UNKNOWN ? porVendor : buscarMarca(renderer);
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

// Pasa a mayúsculas acotando el char a unsigned char: std::toupper con un char
// negativo (nombres de GPU con acentos, ® o ™) es comportamiento indefinido.
static void aMayusculas(string& texto) {
    std::transform(texto.begin(), texto.end(), texto.begin(),
                   [](char c) { return static_cast<char>(std::toupper(static_cast<unsigned char>(c))); });
}

// Busca la marca de la GPU en el mapeo de palabras clave.
GPU::Brand GPU::buscarMarca(string buscado) {
    // Convierte el renderer a mayúsculas para comparar.
    aMayusculas(buscado);
    for (const auto& pair : rendererMap) {
        string key = pair.first;
        // Convertir la clave a mayúsculas para comparar.
        aMayusculas(key);
        if (buscado.find(key) != string::npos) {
            return pair.second;
        }
    }
    return Brand::UNKNOWN;
}

// Deja la marca en `brand` solo si encuentra una coincidencia.
void GPU::buscarEnRenderMap(string buscado) {
    const Brand encontrada = buscarMarca(buscado);
    if (encontrada != Brand::UNKNOWN) {
        brand = encontrada;
    }
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
    if (brand == Brand::UNKNOWN) {
        throw std::runtime_error("GPU brand not set.");
    }
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