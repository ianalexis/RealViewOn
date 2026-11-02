#ifndef ANIMACION_H
#define ANIMACION_H
#include <Windows.h>
#include <objidl.h>
#include <vector>

namespace Gdiplus {
class Image;
}

// GIF animado incrustado como recurso, dibujado con GDI+ (viene con Windows
// desde XP, sin dependencias) sobre un control estático SS_OWNERDRAW. El diálogo
// le pasa el WM_DRAWITEM de ese control a dibujar(); el ritmo lo marca un timer
// propio con las demoras de cada cuadro del GIF.
class Animacion {
public:
    Animacion() = default;
    Animacion(const Animacion&) = delete;
    Animacion& operator=(const Animacion&) = delete;
    ~Animacion();

    // Carga el recurso RCDATA `id` y empieza a animarlo en `control`. Si falla,
    // el control queda vacío y la ventana funciona igual.
    void iniciar(HWND control, int id);
    // Dibuja el cuadro actual sobre `fondo`, escalado al control.
    void dibujar(const DRAWITEMSTRUCT& item, COLORREF fondo);
    void liberar();

private:
    static void CALLBACK alVencerTimer(HWND ventana, UINT mensaje, UINT_PTR id, DWORD tiempo);
    void programarSiguiente();

    HWND control = nullptr;
    ULONG_PTR tokenGdiplus = 0;
    IStream* flujo = nullptr; // GDI+ lo lee mientras exista la imagen.
    Gdiplus::Image* imagen = nullptr;
    std::vector<UINT> demoras; // Milisegundos de cada cuadro.
    UINT cuadro = 0;
};

#endif // ANIMACION_H
