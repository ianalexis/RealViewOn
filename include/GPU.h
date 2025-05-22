#ifndef GPU_H
#define GPU_H
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
        string origin;
    };

    GPU(Current current);

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
    void setBrWorkarounds(string w); // Eliminar calificación extra
    Brand buscarEnRenderMap(string buscado); // Declarar buscarEnRenderMap
    Brand selecectBrandManual(); // Selecciona la marca de la GPU en caso de no poder determinarla.
    void setBaseData(Current current); // Setea la baseData de la GPU.
    vector<string> completarRealViewEnabler(string reg);
    vector<string> completarPerformanceFix(string reg);
    string baseDataToString(); // Convierte la baseData a string.


    Brand brand = Brand::UNKNOWN;
    string renderer;
    string brWorkarounds;
    Current baseData;
    struct BrandData {
        string brPath; // Brand Path para el fix.
        string brandKey; // Clave para el fix. Va en la carpeta de la marca.
        string brandKeyAlt; // Clave alternativa para el fix.
        string brandBaseAvoid; // Clave raiz de la marca para evitar (quizas en un futuro cambiar para que sea un blacklist).
        string glPath; // GL Path para el RealView.
        string glKey; // Clave para el RealView. Va en Gl2Shaders.
        string glKeyAlt; // Clave alternativa para el Gl2Shaders.
    };

    const std::map<Brand, BrandData> brandKeysMap = {
        {Brand::NVIDIA, {
            "NVIDIA Corporation",
            "4000000",
            "4000000, 2000001, 2000000, 6000000, 53001001, 53001000, 52040001, 52000000, 46000000, 44000100, 4000100, 40000, 2141001, 16000000, 14000100, 14000000, 12000001, 12000000, 2501001",
            "2501001",
            "NV40",
            "32408",
            "32408, 30408, 20008, 31408, 33408, 10008, 20408, 22408, 208, 5C, 30008, 7C, 20208, 108, 8, 1"}},
        {Brand::AMD, {
            "Advanced Micro Devices",
            "4000085", //Quizas sea 84000005
            "84000085, 84000084, 84000005, 84000004, 4000085, 4000005, 52400C84",
            "52400c84",
            "R420",
            "32408", //TODO: Probar con 30008
            "30008, 0, 32408, 20008, 40, 8, 22408"}},
        {Brand::INTEL, {
            "Intel",
            "4080080",
            "4000080, 4080080, 10000010, 4000480, 4000000, 10080080, 10000480, 10000080, 10",
            "10",
            "Other",
            "30008", //TODO: Probar con 30408 ya que 30008 no existe en intel.
            "30408, 8, 20408, 20008, 1"}}
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

#endif  // GPU_H