#ifndef TEMA_OSCURO_H
#define TEMA_OSCURO_H
#include <Windows.h>
#include <CommCtrl.h>

// Modo claro/oscuro de la GUI. Sigue la configuración de Windows ("modo de las
// aplicaciones") salvo que se fuerce uno.
//
// Windows no tiene una API documentada de modo oscuro para los controles Win32,
// así que cada parte va por su lado:
// - Barra de título: DWMWA_USE_IMMERSIVE_DARK_MODE (documentado).
// - Fondo y texto de diálogos, estáticos, edits y listas: WM_CTLCOLOR*.
// - Botones, combos y barras de desplazamiento: los temas DarkMode_Explorer y
//   DarkMode_CFD de Windows. No están documentados, pero existen desde Windows 10
//   1809 y los usa el propio Explorador.
// - Casillas y radios: el tema dibuja su texto siempre negro, así que se pintan
//   con NM_CUSTOMDRAW (cuadro del tema, texto propio).
// En Windows 7, 8 y 10 anterior a 1809 queda siempre el aspecto claro.
namespace tema {

// Tono de un texto; el color concreto depende del modo.
enum class Tono { Normal, Tenue, Exito, Info, Error };

void inicializar(bool forzarOscuro, bool forzarClaro);
void liberar();
bool oscuro();
// Ante WM_SETTINGCHANGE "ImmersiveColorSet": vuelve a leer la configuración de
// Windows (salvo que el modo esté forzado). Devuelve true si cambió.
bool actualizarDesdeSistema();
// Aplica el modo actual a la ventana y a sus controles.
void aplicar(HWND ventana);
COLORREF color(Tono tono);
// Fondo de las ventanas en el modo actual.
COLORREF fondo();
// Para WM_CTLCOLOR*: el pincel del modo oscuro, o nullptr para dejar el
// comportamiento normal.
HBRUSH colorear(UINT msg, HDC dc, HWND control);
// Para NM_CUSTOMDRAW: dibuja casillas y radios en modo oscuro. En cualquier otro
// caso devuelve CDRF_DODEFAULT.
LRESULT dibujarCasilla(const NMCUSTOMDRAW& cd);

} // namespace tema

#endif // TEMA_OSCURO_H
