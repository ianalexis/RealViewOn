#ifndef SOLIDWORKS_H
#define SOLIDWORKS_H
#include <string>
#include <utility>
#include <vector>
#include "GPU.h"

// Detección de las versiones de SolidWorks y de su renderer. No pregunta ni
// imprime nada: las interfaces (consola y GUI) deciden qué mostrar y qué pedir.
class SolidWorks {
public:
    static const int vMin = 2010; // Versión mínima soportada
    static const int vCambioRaiz = 2023; // Año de cambio de ubicación del registro
    static const int vMax = 2026; // Versión máxima soportada

    SolidWorks(); // Constructor
    // Busca las versiones instaladas y devuelve (año, compatibilidad) de cada una.
    const std::vector<std::pair<int, int>>& obtenerVersionesInstaladas();
    // Comprueba si una versión es compatible. 0 = No compatible, 1 = Compatible, 2 = Versión futura.
    int esCompatible(int v);
    // Según la última llamada a obtenerVersionesInstaladas().
    bool versionInstalada(int v);
    // Sin instalar o versión futura: solo se puede seguir en modo genérico.
    bool requiereGenerico(int v);
    // Posterior al año actual + 1.
    bool esDelFuturo(int v);
    // Lanza si la versión no es compatible. El modo genérico lo decide quien llama.
    void setVersion(int v, bool generico);
    void setGenerico(bool g);
    bool getGenerico();
    // Renderer actual según la versión elegida, sin confirmar. Vacío si no hay.
    GPU::Current detectarCurrent();
    // Renderers del registro de SolidWorks más los adaptadores de Windows, como
    // (nombre, origen) y sin repetir nombres. Recorre todo HKCU\SOFTWARE\SolidWorks.
    std::vector<std::pair<std::string, std::string>> candidatosRenderer();
    std::vector<std::string> obtenerRegBase(); // Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
    std::string versions = std::to_string(vMin) + ", " + std::to_string(vCambioRaiz) + ", " + std::to_string(vMax); // Versiones sugeridas al pedir el año.

private:
    friend struct SolidWorksTestAccess; // Solo para tests unitarios (tests/test_access.h). No usar en producción.

    int obtenerAnoActual(); // Obtiene el año actual del sistema operativo + 1.
    GPU::Current obtenerCurrent(std::string path);
    GPU::Current obtenerCurrentRaiz();
    GPU::Current obtenerCurrentAno();
    std::vector<std::pair<std::string, std::string>> obtenerRendererGenerico();
    std::vector<std::pair<std::string, std::string>> windowsDisplayAdapters(); // Obtiene los adaptadores de Windows.
    std::string obtenerRegBaseAllowList();
    std::string obtenerRegBaseAno();

    int anoActual; // Año actual + 1
    std::string swRegRuta = "SOFTWARE\\SolidWorks\\SOLIDWORKS ";
    int swVersion = 0;
    std::vector<std::pair<int, int>> versiones;
    bool generico = false;
    std::vector<std::string> regBase;
};

#endif // SOLIDWORKS_H
