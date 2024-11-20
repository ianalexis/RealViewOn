#pragma once

#ifndef REGISTRO_H
#define REGISTRO_H
#include <string>

std::wstring construirRutaRegistro(const std::string& version);
std::string obtenerRenderer(const std::wstring& rutaRegistro);
std::string crearContenidoReg(const std::string& version, const std::string& renderer);
void guardarArchivoReg(const std::string& contenido);


#endif // REGISTRO_H