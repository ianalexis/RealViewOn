#pragma once
#include <string>
#include <vector>
#include <map>

class GPU {
public:
    GPU(const std::string& renderer); // Constructor
    std::vector<std::string> completarContenidoReg(const std::vector<std::string>& regBase);

private:
    enum class Brand {
        NVIDIA,
        AMD,
        INTEL,
        UNKNOWN
    };

    void setBrand(); // Declarar setBrand
    std::vector<std::string> completarContenidoRegNVIDIA(const std::vector<std::string>& regBase); // Completa el contenido del archivo .reg para GPUs NVIDIA.
    std::vector<std::string> completarContenidoRegAMD(const std::vector<std::string>& regBase); // Completa el contenido del archivo .reg para GPUs AMD.
    std::vector<std::string> completarContenidoRegIntel(const std::vector<std::string>& regBase); // Completa el contenido del archivo .reg en modo genérico.
    Brand selecectBrand(); // Selecciona la marca de la GPU en caso de no poder determinarla.

    Brand brand = Brand::UNKNOWN;
    std::string renderer;
    const std::string brandKey = "40000"; // TODO: PROBAR SI EL VALOR NO CAMBIA POR VERSION DE SW O GPU.
    const std::string glKey = "70408"; //TODO: PROBAR SI EL VALOR NO CAMBIA POR VERSION DE SW O GPU.
    std::vector<std::string> regFull;

    // Mapeo de palabras clave a marcas de GPU
    const std::map<std::string, Brand> rendererMap = {
        {"GeForce", Brand::NVIDIA},
        {"Nvidia", Brand::NVIDIA},
        {"PNY", Brand::NVIDIA},
        {"EVGA", Brand::NVIDIA},
        {"Galax", Brand::NVIDIA},
        {"Zotac", Brand::NVIDIA},
        {"Palit", Brand::NVIDIA},
        {"RX", Brand::AMD},
        {"AMD", Brand::AMD},
        {"Sapphire", Brand::AMD},
        {"PowerColor", Brand::AMD},
        {"XFX", Brand::AMD},
        {"HIS", Brand::AMD},
        {"VisionTek", Brand::AMD}
    };
};