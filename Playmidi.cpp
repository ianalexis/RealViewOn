#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <RtMidi.h>
#include "MidiFile.h" 
#include "resource.h"

// Función para cargar el archivo MIDI incrustado como recurso
std::vector<unsigned char> loadEmbeddedMidi() {
    HMODULE hModule = GetModuleHandle(nullptr);
    if (!hModule) {
        throw std::runtime_error("Error al obtener el módulo del ejecutable.");
    }

    // Cargar el recurso
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_MIDI_FILE), RT_RCDATA);
    if (!hResource) {
        throw std::runtime_error("Recurso MIDI no encontrado.");
    }

    HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
    if (!hLoadedResource) {
        throw std::runtime_error("Error al cargar el recurso MIDI.");
    }

    DWORD resourceSize = SizeofResource(hModule, hResource);
    void* resourceData = LockResource(hLoadedResource);
    if (!resourceData) {
        throw std::runtime_error("Error al bloquear el recurso MIDI.");
    }

    // Copiar los datos del recurso a un vector
    return std::vector<unsigned char>(static_cast<unsigned char*>(resourceData),
        static_cast<unsigned char*>(resourceData) + resourceSize);
}

// Función para enviar mensajes MIDI válidos
void sendMidiMessage(RtMidiOut& midiOut, const std::vector<unsigned char>& message) {
    if (message.size() <= 3 || message[0] == 0xF0 && message.back() == 0xF7) {
        // Mensaje estándar MIDI (máximo 3 bytes)
        midiOut.sendMessage(&message);
    } else {
        std::cerr << "" ;
    }
}

// Función para reproducir datos MIDI directamente desde memoria
void playMidi(const std::vector<unsigned char>& midiData) {
    try {
        RtMidiOut midiOut;
        if (midiOut.getPortCount() == 0) {
            std::cerr << "No hay puertos MIDI disponibles." << std::endl;
            return;
        }

        // Abrir el primer puerto MIDI disponible
        midiOut.openPort(0);

        // Usar un flujo de memoria para cargar el archivo MIDI
        std::stringstream midiStream(std::string(midiData.begin(), midiData.end()));

        // Usar la librería midi-file para interpretar los datos MIDI
        smf::MidiFile midiFile;
        midiFile.read(midiStream);
        midiFile.linkNotePairs();
        midiFile.doTimeAnalysis();
        midiFile.joinTracks();

        // Tiempo inicial de reproducción
        auto startTime = std::chrono::steady_clock::now();

        // Iterar sobre los eventos y reproducirlos
        for (size_t i = 0; i < midiFile[0].size(); ++i) {
            auto& event = midiFile[0][i];

            // Tiempo absoluto esperado para este evento (en segundos)
            double eventTime = event.seconds;

            // Esperar hasta el momento exacto para reproducir el evento
            auto currentTime = std::chrono::steady_clock::now();
            double elapsedSeconds = std::chrono::duration<double>(currentTime - startTime).count();

            if (elapsedSeconds < eventTime) {
                // Esperar la diferencia entre el tiempo actual y el tiempo del evento
                auto delayMs = static_cast<int>((eventTime - elapsedSeconds) * 1000);
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }

            // Crear mensaje MIDI
            std::vector<unsigned char> message(event.begin(), event.end());
            if (!message.empty()) {
                sendMidiMessage(midiOut, message);
            }
        }

       // std::cout << "Reproducción completada correctamente." << std::endl;

    }
    catch (RtMidiError& error) {
        error.printMessage();
    }
}

/*

int main() {
    try {
        std::cout << "Cargando archivo MIDI incrustado..." << std::endl;
        std::vector<unsigned char> midiData = loadEmbeddedMidi();

        std::cout << "Reproduciendo archivo MIDI..." << std::endl;
        playMidi(midiData);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
*/