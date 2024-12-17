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
    Brand buscarEnRenderMap(string buscado); // Declarar buscarEnRenderMap
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
        {Brand::AMD, {"4000085", "32408"}},//FIXME: Quizas brandKey sea 84000005 y el gl quizas 0 https://www.reddit.com/r/SolidWorks/comments/rii95z/comment/jsak9c7/
        {Brand::INTEL, {"4080080", "30008"}}
    };

    // Mapeo de palabras clave a marcas de GPU EN MAYÚSCULAS.
    const std::vector<std::pair<string, Brand>> rendererMap = {
        // Vendor, Brand
        {"NVIDIA", Brand::NVIDIA},
        {"INTEL", Brand::INTEL},
        {"AMD", Brand::AMD},
        {"ATI", Brand::AMD},
        {"Advanced Micro Devices", Brand::AMD},
        // Renderer, Brand. Orden String largo a corto. Orden Brand AMD INTEL NVIDIA.
        {"SAPPHIRE", Brand::AMD},
        {"GEFORCE", Brand::NVIDIA},
        {"FIREPRO", Brand::AMD},
        {"RADEON", Brand::AMD},
        {"QUADRO", Brand::NVIDIA},
        {"TITAN", Brand::NVIDIA},
        {"TESLA", Brand::NVIDIA},
        {"RYZEN", Brand::AMD},
        {"PALIT", Brand::NVIDIA},
        {"VEGA", Brand::AMD},
        {"IRIS", Brand::INTEL},
        {"UHD", Brand::INTEL},
        {"RTX", Brand::NVIDIA},
        {"PNY", Brand::NVIDIA},
        {"HIS", Brand::AMD},
        {"GTX", Brand::NVIDIA},
        {"GTS", Brand::NVIDIA},
        {"XE", Brand::INTEL},
        {"RX", Brand::AMD},
        {"R9", Brand::AMD},
        {"R7", Brand::AMD},
        {"R5", Brand::AMD},
        {"MX", Brand::NVIDIA},
        {"GT", Brand::NVIDIA}
    };
};