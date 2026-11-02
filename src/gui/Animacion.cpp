#include "Animacion.h"
#include <Windows.h>
#include <gdiplus.h>
#include <objidl.h>
#include <shlwapi.h>
#include <cstring>
#include <vector>

Animacion::~Animacion() {
    liberar();
}

void Animacion::iniciar(HWND controlDestino, int id) {
    liberar();
    control = controlDestino;

    HMODULE modulo = GetModuleHandleW(nullptr);
    HRSRC recurso = FindResource(modulo, MAKEINTRESOURCE(id), RT_RCDATA);
    HGLOBAL cargado = recurso != nullptr ? LoadResource(modulo, recurso) : nullptr;
    const void* datos = cargado != nullptr ? LockResource(cargado) : nullptr;
    if (datos == nullptr) {
        return;
    }

    Gdiplus::GdiplusStartupInput entrada;
    if (Gdiplus::GdiplusStartup(&tokenGdiplus, &entrada, nullptr) != Gdiplus::Ok) {
        tokenGdiplus = 0;
        return;
    }
    // Copia los datos del recurso a un stream en memoria, que es lo que lee GDI+.
    flujo = SHCreateMemStream(static_cast<const BYTE*>(datos), SizeofResource(modulo, recurso));
    if (flujo == nullptr) {
        return;
    }
    imagen = Gdiplus::Image::FromStream(flujo);
    if (imagen == nullptr || imagen->GetLastStatus() != Gdiplus::Ok) {
        return;
    }

    // Demoras de cada cuadro, en centésimas de segundo. Como los navegadores,
    // una demora de 0 o 1 se toma como 100 ms.
    const UINT cuadros = imagen->GetFrameCount(&Gdiplus::FrameDimensionTime);
    const UINT tamano = imagen->GetPropertyItemSize(PropertyTagFrameDelay);
    std::vector<BYTE> buffer(tamano);
    auto* propiedad = reinterpret_cast<Gdiplus::PropertyItem*>(buffer.data());
    const bool hayDemoras = tamano > 0 && imagen->GetPropertyItem(PropertyTagFrameDelay, tamano, propiedad) == Gdiplus::Ok;
    for (UINT i = 0; i < cuadros; i++) {
        UINT centesimas = 10;
        if (hayDemoras && (i + 1) * sizeof(UINT) <= propiedad->length) {
            std::memcpy(&centesimas, static_cast<const BYTE*>(propiedad->value) + i * sizeof(UINT), sizeof(UINT));
        }
        demoras.push_back(centesimas <= 1 ? 100 : centesimas * 10);
    }
    cuadro = 0;
    // Una imagen fija no necesita timer.
    if (demoras.size() > 1) {
        programarSiguiente();
    }
    InvalidateRect(control, nullptr, FALSE);
}

void Animacion::programarSiguiente() {
    // El id del timer es la propia instancia, para recuperarla en alVencerTimer().
    SetTimer(control, reinterpret_cast<UINT_PTR>(this), demoras[cuadro], alVencerTimer);
}

void CALLBACK Animacion::alVencerTimer(HWND, UINT, UINT_PTR id, DWORD) {
    auto* animacion = reinterpret_cast<Animacion*>(id);
    // El GIF repite sin fin, que es lo que indica su bloque NETSCAPE2.0.
    animacion->cuadro = (animacion->cuadro + 1) % static_cast<UINT>(animacion->demoras.size());
    animacion->programarSiguiente();
    InvalidateRect(animacion->control, nullptr, FALSE);
}

void Animacion::dibujar(const DRAWITEMSTRUCT& item, COLORREF fondo) {
    const int ancho = item.rcItem.right - item.rcItem.left;
    const int alto = item.rcItem.bottom - item.rcItem.top;
    // Se arma el cuadro en memoria y se copia de una vez: dibujando directo, el
    // fondo y el GIF parpadean en cada cuadro.
    HDC memoria = CreateCompatibleDC(item.hDC);
    HBITMAP lienzo = CreateCompatibleBitmap(item.hDC, ancho, alto);
    HGDIOBJ anterior = SelectObject(memoria, lienzo);
    const RECT todo = {0, 0, ancho, alto};
    HBRUSH pincel = CreateSolidBrush(fondo);
    FillRect(memoria, &todo, pincel);
    DeleteObject(pincel);
    if (imagen != nullptr && !demoras.empty()) {
        imagen->SelectActiveFrame(&Gdiplus::FrameDimensionTime, cuadro);
        Gdiplus::Graphics grafico(memoria);
        grafico.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        grafico.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        // Cuadrado, centrado y del mayor tamaño que entre en el control.
        const int lado = ancho < alto ? ancho : alto;
        grafico.DrawImage(imagen, (ancho - lado) / 2, (alto - lado) / 2, lado, lado);
    }
    BitBlt(item.hDC, item.rcItem.left, item.rcItem.top, ancho, alto, memoria, 0, 0, SRCCOPY);
    SelectObject(memoria, anterior);
    DeleteObject(lienzo);
    DeleteDC(memoria);
}

void Animacion::liberar() {
    if (control != nullptr) {
        KillTimer(control, reinterpret_cast<UINT_PTR>(this));
    }
    delete imagen;
    imagen = nullptr;
    if (flujo != nullptr) {
        flujo->Release();
        flujo = nullptr;
    }
    // La imagen se borra antes de apagar GDI+.
    if (tokenGdiplus != 0) {
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
    }
    demoras.clear();
    control = nullptr;
}
