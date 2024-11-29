#pragma once
#include <string>
#include <vector>

using std::string;
using std::vector;

class SolidWorks {
public:
    SolidWorks(); // Constructor
    void obtenerVersionesInstaladas(); // Devuelve una lista de versiones de SolidWorks instaladas y si son compatibles.
    string obtenerRenderer(); // Lee el valor 'renderer' actual desde el registro.
    vector<string> obtenerRegBase(); // Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
    void setVersion(int v);
    int esCompatible(int v);
    void setGenerico(bool g);

private:
    int obtenerAnoActual(); // Obtiene el año actual del sistema operativo + 1.
    string obtenerRenderRaiz();
    string obtenerRendererAno();
    string obtenerRendererGenerico();
    string obtenerRegBaseRaiz();
    string obtenerRegBaseAno();
    bool versionInstalada(int v);
    std::string rendererManual();
    std::string elegirRenderer(std::vector<std::pair<std::string, std::string>> renderers);

    static const int vMin = 2010; // Versión mínima soportada
    static const int vMax = 2024; // Versión máxima soportada
    static const int vCambioRaiz = 2022; // Año de cambio de ubicación del registro
    int anoActual; // Año actual + 1
    std::wstring swRegRuta = L"SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    int swVersion = 0;
    vector<std::pair<int, int>> versiones;
    bool generico = false;
    string renderer;
    vector<string> regBase;
};