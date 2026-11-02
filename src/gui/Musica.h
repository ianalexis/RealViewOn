#ifndef MUSICA_H
#define MUSICA_H
#include <Windows.h>
#include <atomic>
#include <thread>

// La música incrustada de la consola, en un hilo aparte y con la posibilidad de
// cortarla. Cuando termina sola, o si no hay salida MIDI, avisa a la ventana con
// el mensaje indicado: wParam vale 1 si terminó y 0 si no pudo sonar.
class Musica {
public:
    Musica() = default;
    Musica(const Musica&) = delete;
    Musica& operator=(const Musica&) = delete;
    ~Musica();

    // Empieza desde el principio; si ya estaba sonando, la corta antes.
    void reproducir(HWND ventana, UINT mensaje);
    // Corta y espera a que el hilo termine (menos de 50 ms).
    void detener();

private:
    std::thread hilo;
    std::atomic<bool> parar{false};
};

#endif // MUSICA_H
