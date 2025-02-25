#pragma once
#include <string>
#include <vector>
#include "GPU.h"

using std::string;
using std::vector;

class SolidWorks {
public:
    SolidWorks(); // Constructor
    void obtenerVersionesInstaladas(); // Devuelve una lista de versiones de SolidWorks instaladas y si son compatibles.
    GPU::Current obtenerCurrent(); // Lee el valor 'renderer' actual desde el registro.
    vector<string> obtenerRegBase(); // Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
    void setVersion(int v);
    int esCompatible(int v);
    void setGenerico(bool g);
    bool getGenerico();
    string versions = std::to_string(vMin) + ", " + std::to_string(vCambioRaiz) + ", " + std::to_string(vMax); // Versión máxima de SolidWorks soportada.

private:
    int obtenerAnoActual(); // Obtiene el año actual del sistema operativo + 1.
    bool versionInstalada(int v);
    GPU::Current obtenerCurrent(string path);
    GPU::Current obtenerCurrentRaiz();
    GPU::Current obtenerCurrentAno();
    std::vector<std::pair<std::string, std::string>> obtenerRendererGenerico();
    std::vector<std::pair<std::string, std::string>> windowsDisplayAdapters(); // Obtiene los adaptadores de Windows.
    string obtenerRegBaseAllowList();
    string obtenerRegBaseAno();
    GPU::Current elegirRenderer();
    string rendererManual();

    static const int vMin = 2010; // Versión mínima soportada
    static const int vCambioRaiz = 2023; // Año de cambio de ubicación del registro
    static const int vMax = 2025; // Versión máxima soportada

    int anoActual; // Año actual + 1
    string swRegRuta = "SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    int swVersion = 0;
    vector<std::pair<int, int>> versiones;
    bool generico = false;
    GPU::Current current;
    vector<string> regBase;
};