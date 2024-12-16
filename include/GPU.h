#pragma once
#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;

class GPU {
    public:
    struct Current {
        string renderer;
        string vendor;
        string workarounds;
    };

    GPU(Current current); // Constructor

    vector<string> completarContenidoReg(const vector<string>& regBase);

    enum class Brand {
        NVIDIA,
        AMD,
        INTEL,
        UNKNOWN
    };


    string brandToString(GPU::Brand brand); // Convierte la marca de GPU a string.

private:
    void setBrand(string v,string r); // Declarar setBrand
    vector<string> completarContenidoRegNVIDIA(const vector<string>& regBase); // Completa el contenido del archivo .reg para GPUs NVIDIA.
    vector<string> completarContenidoRegAMD(const vector<string>& regBase); // Completa el contenido del archivo .reg para GPUs AMD.
    vector<string> completarContenidoRegIntel(const vector<string>& regBase); // Completa el contenido del archivo .reg en modo genérico.
    void buscarEnRenderMap(string buscado); // Declarar buscarEnRenderMap
    Brand selecectBrandManual(); // Selecciona la marca de la GPU en caso de no poder determinarla.

    Brand brand = Brand::UNKNOWN;
    string renderer;
    string workarounds;

    struct BrandKeys {
        string brandKey;
        string glKey;
    };

    const std::map<Brand, BrandKeys> brandKeysMap = {
        {Brand::NVIDIA, {"40000", "70408"}},
        {Brand::AMD, {"4000085", "32408"}},
        {Brand::INTEL, {"4080080", "30008"}}
    };

    vector<string> regFull;

    // Mapeo de palabras clave a marcas de GPU EN MAYÚSCULAS.
    const std::map<string, Brand> rendererMap = {
        // Vendor, Brand
        {"NVIDIA", Brand::NVIDIA},
        {"AMD", Brand::AMD},
        {"INTEL", Brand::INTEL},
        {"ATI", Brand::AMD},
        {"Advanced Micro Devices", Brand::AMD},
        // Renderer, Brand. Orden String largo a corto. Orden Brand AMD INTEL NVIDIA.
        {"POWERCOLOR", Brand::AMD},
        {"VISIONTEK", Brand::AMD},
        {"SAPPHIRE", Brand::AMD},
        {"GIGABYTE", Brand::AMD},
        {"GEFORCE", Brand::NVIDIA},
        {"FIREPRO", Brand::AMD},
        {"RADEON", Brand::AMD},
        {"QUADRO", Brand::NVIDIA},
        {"ZOTAC", Brand::NVIDIA},
        {"TITAN", Brand::NVIDIA},
        {"TESLA", Brand::NVIDIA},
        {"RYZEN", Brand::AMD},
        {"PALIT", Brand::NVIDIA},
        {"GALAX", Brand::NVIDIA},
        {"VEGA", Brand::AMD},
        {"IRIS", Brand::INTEL},
        {"EVGA", Brand::NVIDIA},
        {"ASUS", Brand::AMD},
        {"XFX", Brand::AMD},
        {"UHD", Brand::INTEL},
        {"RTX", Brand::NVIDIA},
        {"PNY", Brand::NVIDIA},
        {"MSI", Brand::AMD},
        {"HIS", Brand::AMD},
        {"GTX", Brand::NVIDIA},
        {"GTS", Brand::NVIDIA},
        {"XE", Brand::INTEL},
        {"RX", Brand::AMD},
        {"R9", Brand::AMD},
        {"R7", Brand::AMD},
        {"R5", Brand::AMD},
        {"MX", Brand::NVIDIA},
        {"HD", Brand::AMD},
        {"GT", Brand::NVIDIA}
    };
};