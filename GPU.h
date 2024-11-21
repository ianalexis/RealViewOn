#pragma once
#include <string>
#include <vector>
#include <map>

class GPU
{
public:
    GPU(std::string renderer); // Constructor
    std::string completarContenidoReg(const std::vector<std::string>& regBase);
    std::string completarContenidoRegNVIDIA(const std::vector<std::string>& regBase); // Completa el contenido del archivo .reg para GPUs NVIDIA.
    std::string completarContenidoRegAMD(const std::vector<std::string>& regBase); // Completa el contenido del archivo .reg para GPUs AMD.
    std::string completarContenidoRegGenerico(const std::vector<std::string>& regBase); // Completa el contenido del archivo .reg en modo genérico.
    void setBrand(); // Declarar setBrand

private:
    enum class Brand {
        NVIDIA,
        AMD,
        UNKNOWN
    };

    Brand brand = Brand::UNKNOWN;
    std::string renderer;
    std::string brandKey = "40000"; // TODO: PROBAR SI EL VALOR NO CAMBIA POR VERSION DE SW O GPU.
    std::string glKey = "70408"; //TODO: PROBAR SI EL VALOR NO CAMBIA POR VERSION DE SW O GPU.
    std::string regFull;

    // Mapeo de palabras clave a marcas de GPU
    std::map<std::string, Brand> rendererMap = {
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