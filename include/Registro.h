#ifndef REGISTRO_H
#define REGISTRO_H
#include <vector>
#include <string>

// Contenido completo del archivo .reg (cabecera más las líneas), sin escribirlo.
std::string armarArchivoReg(const std::string& version, const std::vector<std::string>& contenido, const std::string& RVO_VERSION);
// Escribe RealViewOn<version>.reg en la carpeta actual y devuelve la ruta completa.
std::string guardarArchivoReg(const std::string& version, const std::string& contenidoArchivo);
// Atajo: armarArchivoReg() + guardarArchivoReg().
std::string guardarArchivoReg(const std::string& version, const std::vector<std::string>& contenido, const std::string& RVO_VERSION);
// Exporta HKCU\SOFTWARE\SolidWorks a RVO_SWbackup_<fecha>.reg en la carpeta actual
// y devuelve el nombre del archivo.
std::string guardarBackUp();

#endif // REGISTRO_H
