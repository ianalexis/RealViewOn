#pragma once
#include <string>

class SolidWorks
{
	std::string obtenerRenderer(); // Lee el valor 'renderer' actual desde el registro.
	std::string obtenerRegBase(); // Obtiene la ruta base del registro para enviarle al completador de contenido de la GPU.

};

