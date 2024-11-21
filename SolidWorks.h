#pragma once
#include <string>
#include <vector>
class SolidWorks
{
public:
    SolidWorks(); // Constructor
    std::vector<std::pair<std::string, bool>> obtenerVersionesInstaladas(); // Devuelve una lista de versiones de SolidWorks instaladas y si son compatibles.
    std::string obtenerRenderer(); // Lee el valor 'renderer' actual desde el registro.
    std::string obtenerRegBase(); // Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.
    int obtenerAnoActual(); // Obtiene el año actual del sistema operativo + 1.
};