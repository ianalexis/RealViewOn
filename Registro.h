#pragma once

#ifndef REGISTRO_H
#define REGISTRO_H
#include <string>

std::wstring construirRutaRegistro(const std::string& version); // Construye la ruta del registro basada en la versión de SolidWorks proporcionada. TODO: ELIMINAR, ESTO SE LO VA A OBTENER LA GPU DESDE EL SOLIDWORKS CORRESPONDIENTE.
std::string obtenerRenderer(const std::wstring& rutaRegistro); // Devuelve el valor 'renderer' correspondiente. TODO: ELIMINAR OBTENIENDO ESTA INFO DEL SOLIDWORKS CORRESPONDIENTE
std::string crearContenidoReg(const std::string& version, const std::string& renderer); // Genera el contenido del archivo .reg basado en la versión y el renderer. TODO: ELIMINAR OBTENIENDO ESTA INFO DE LA GPU CORRESPONDIENTE
void guardarArchivoReg(const std::string& contenido); // Guarda el archivo .reg en el directorio actual con el contenido proporcionado.


#endif // REGISTRO_H