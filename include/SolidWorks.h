#ifndef SOLIDWORKS_H
#define SOLIDWORKS_H
#include <string>
#include <utility>
#include <vector>
#include "GPU.h"

class SolidWorks {
public:
    SolidWorks(); // Constructor
    void obtenerVersionesInstaladas(); // Devuelve una lista de versiones de SolidWorks instaladas y si son compatibles.
    GPU::Current obtenerCurrent(); // Lee el valor 'renderer' actual desde el registro.
    std::vector<std::string> obtenerRegBase(); // Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
    void setVersion(int v);
    int esCompatible(int v);
    void setGenerico(bool g);
    bool getGenerico();
    std::string versions = std::to_string(vMin) + ", " + std::to_string(vCambioRaiz) + ", " + std::to_string(vMax); // Versión máxima de SolidWorks soportada.

private:
    friend struct SolidWorksTestAccess; // Solo para tests unitarios (tests/test_access.h). No usar en producción.

    int obtenerAnoActual(); // Obtiene el año actual del sistema operativo + 1.
    bool versionInstalada(int v);
    GPU::Current obtenerCurrent(std::string path);
    GPU::Current obtenerCurrentRaiz();
    GPU::Current obtenerCurrentAno();
    std::vector<std::pair<std::string, std::string>> obtenerRendererGenerico();
    std::vector<std::pair<std::string, std::string>> windowsDisplayAdapters(); // Obtiene los adaptadores de Windows.
    std::string obtenerRegBaseAllowList();
    std::string obtenerRegBaseAno();
    GPU::Current elegirRenderer();
    std::string rendererManual();

    static const int vMin = 2010; // Versión mínima soportada
    static const int vCambioRaiz = 2023; // Año de cambio de ubicación del registro
    static const int vMax = 2026; // Versión máxima soportada

    int anoActual; // Año actual + 1
    std::string swRegRuta = "SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    int swVersion = 0;
    std::vector<std::pair<int, int>> versiones;
    bool generico = false;
    GPU::Current current;
    std::vector<std::string> regBase;
};

#endif // SOLIDWORKS_H
