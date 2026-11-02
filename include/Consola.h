#ifndef CONSOLA_H
#define CONSOLA_H
#include <utility>
#include <vector>
#include "AdvanceMode.h"
#include "GPU.h"
#include "SolidWorks.h"

// Preguntas de la versión de consola. El core no pregunta nada: estas funciones
// muestran lo que detecta y piden por teclado lo que falta. La GUI resuelve lo
// mismo con su ventana.

// Tabla de versiones instaladas con su compatibilidad.
void mostrarVersionesInstaladas(const std::vector<std::pair<int, int>>& versiones);
// Si la versión no está instalada o es futura, avisa y pregunta si seguir en modo
// genérico. Devuelve el modo elegido; lanza si la versión no es compatible o si el
// usuario cancela.
bool preguntarModoGenerico(SolidWorks& sw, int version);
// Confirma el renderer detectado, o pide elegir uno de la lista o escribirlo.
GPU::Current elegirCurrent(SolidWorks& sw);
// Informa la marca detectada, o la pide si no se pudo deducir.
void confirmarMarca(GPU& gpu);
// Pregunta por cada opción disponible. El resultado va a AdvanceMode::armarOpciones().
std::vector<bool> preguntarOpcionesAvanzadas(const AdvanceMode& advMode, bool generico);

#endif // CONSOLA_H
