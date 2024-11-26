#pragma once
#include <string>
#include <vector>

class SolidWorks
{
public:
    SolidWorks(); // Constructor
    void obtenerVersionesInstaladas(); // Devuelve una lista de versiones de SolidWorks instaladas y si son compatibles.
    std::string obtenerRenderer(); // Lee el valor 'renderer' actual desde el registro.
    std::vector<std::string> obtenerRegBase(); // Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
    int obtenerAnoActual(); // Obtiene el año actual del sistema operativo + 1.
    void setVersion(int v);
    bool esCompatible(int v);
    void setGenerico(bool g);
    std::string obtenerRenderRaiz();
    std::string obtenerRendererAno();
    std::string obtenerRendererGenerico();
    std::string obtenerRegBaseRaiz(); // Declaración del método
    std::string obtenerRegBaseAno(); // Declaración del método
    bool versionInstalada(int v); // Declaración del método

private:
    int vMin = 2010; // Versión mínima soportada
    int vMax = 2024; // Versión máxima soportada
    int vCambioRaiz = 2022; // Año de cambio de ubicación del registro TODO: REVISAR, me confie por un comentario de reddit
    int anoActual = vMax; // Año actual + 1
    std::wstring swRegRuta = L"SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    int swVersion = 0;
    std::vector<std::pair<int, bool>> versiones;
    bool generico = false;
    std::string renderer;
    std::vector<std::string> regBase;
};