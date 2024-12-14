//#pragma once
#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;

class GPU {
public:
    GPU(const string& renderer); // Constructor
    vector<string> completarContenidoReg(const vector<string>& regBase);

    enum class Brand {
        NVIDIA,
        AMD,
        INTEL,
        UNKNOWN
    };

    string brandToString(GPU::Brand brand); // Convierte la marca de GPU a string.

private:
    void setBrand(string r); // Declarar setBrand
    vector<string> completarContenidoRegNVIDIA(const vector<string>& regBase); // Completa el contenido del archivo .reg para GPUs NVIDIA.
    vector<string> completarContenidoRegAMD(const vector<string>& regBase); // Completa el contenido del archivo .reg para GPUs AMD.
    vector<string> completarContenidoRegIntel(const vector<string>& regBase); // Completa el contenido del archivo .reg en modo genérico.
    Brand selecectBrand(); // Selecciona la marca de la GPU en caso de no poder determinarla.

    Brand brand = Brand::UNKNOWN;
    string renderer;

    struct BrandKeys {
        string brandKey;
        string glKey;
    };

    const std::map<Brand, BrandKeys> brandKeysMap = {
        {Brand::NVIDIA, {"40000", "70408"}},
        {Brand::AMD, {"4000085", "32408"}},
        {Brand::INTEL, {"4080080", "30408"}}
    };

    vector<string> regFull;

    // Mapeo de palabras clave a marcas de GPU EN MAYÚSCULAS.
    const std::map<string, Brand> rendererMap = {
        {"GEFORCE", Brand::NVIDIA},
        {"NVIDIA", Brand::NVIDIA},
        {"QUADRO", Brand::NVIDIA},
        {"TESLA", Brand::NVIDIA},
        {"TITAN", Brand::NVIDIA},
        {"RTX", Brand::NVIDIA},
        {"GTX", Brand::NVIDIA},
        {"GT", Brand::NVIDIA},
        {"GTS", Brand::NVIDIA},
        {"MX", Brand::NVIDIA},
        {"PNY", Brand::NVIDIA},
        {"EVGA", Brand::NVIDIA},
        {"GALAX", Brand::NVIDIA},
        {"ZOTAC", Brand::NVIDIA},
        {"PALIT", Brand::NVIDIA},
        {"RADEON", Brand::AMD},
        {"FIREPRO", Brand::AMD},
        {"R9", Brand::AMD},
        {"R7", Brand::AMD},
        {"R5", Brand::AMD},
        {"HD", Brand::AMD},
        {"RX", Brand::AMD},
        {"RX", Brand::AMD},
        {"AMD", Brand::AMD},
        {"VEGA", Brand::AMD},
        {"RYZEN", Brand::AMD},
        {"RADEON", Brand::AMD},
        {"ASUS", Brand::AMD},
        {"MSI", Brand::AMD},
        {"GIGABYTE", Brand::AMD},
        {"SAPPHIRE", Brand::AMD},
        {"POWERCOLOR", Brand::AMD},
        {"XFX", Brand::AMD},
        {"HIS", Brand::AMD},
        {"VISIONTEK", Brand::AMD},
        {"INTEL", Brand::INTEL},
        {"IRIS", Brand::INTEL},
        {"UHD", Brand::INTEL},
        {"HD", Brand::INTEL},
        {"XE", Brand::INTEL}
    };
};