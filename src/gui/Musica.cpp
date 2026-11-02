#include "Musica.h"
#include <Windows.h>
#include <exception>
#include <thread>
#include "Playmidi.h"

Musica::~Musica() {
    detener();
}

void Musica::reproducir(HWND ventana, UINT mensaje) {
    detener();
    parar = false;
    hilo = std::thread([this, ventana, mensaje]() {
        WPARAM termino = 1;
        try {
            playMidi(loadEmbeddedMidi(), &parar);
        } catch (const std::exception&) {
            termino = 0; // Típicamente: no hay ningún puerto MIDI.
        }
        // Si se cortó a pedido, quien la cortó ya lo sabe.
        if (!parar) {
            PostMessageW(ventana, mensaje, termino, 0);
        }
    });
}

void Musica::detener() {
    parar = true;
    if (hilo.joinable()) {
        hilo.join();
    }
}
