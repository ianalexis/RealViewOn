#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <Windows.h>
#include <RtMidi.h>
#include "MidiFile.h"
#include "resource.h"

// Función para cargar el archivo MIDI incrustado como recurso
std::vector<unsigned char> loadEmbeddedMidi() {
    HMODULE hModule = GetModuleHandle(nullptr);
    if (!hModule) {
        throw std::runtime_error("Error obtaining executable module.");
    }

    // Cargar el recurso
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_MIDI_FILE), RT_RCDATA);
    if (!hResource) {
        throw std::runtime_error("MIDI resource not found.");
    }

    HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
    if (!hLoadedResource) {
        throw std::runtime_error("Error loading MIDI resource.");
    }

    DWORD resourceSize = SizeofResource(hModule, hResource);
    void* resourceData = LockResource(hLoadedResource);
    if (!resourceData) {
        throw std::runtime_error("Error locking MIDI resource.");
    }

    // Copiar los datos del recurso a un vector
    return std::vector<unsigned char>(static_cast<unsigned char*>(resourceData), static_cast<unsigned char*>(resourceData) + resourceSize);
}

// Función para enviar mensajes MIDI válidos
void sendMidiMessage(RtMidiOut& midiOut, const std::vector<unsigned char>& message) {
    if (message.size() <= 3 || (message[0] == 0xF0 && message.back() == 0xF7)) {
        // Mensaje estándar MIDI (máximo 3 bytes)
        midiOut.sendMessage(&message);
    } else {
        std::cerr << "" ;
    }
}

// Función para reproducir datos MIDI directamente desde memoria
void playMidi(const std::vector<unsigned char>& midiData) {
    try {
        // Crear RtMidiOut con manejo de errores silencioso
        RtMidiOut midiOut;

        // Configurar callback de error personalizado para suprimir mensajes automáticos
        midiOut.setErrorCallback([](RtMidiError::Type type, const std::string &errorText, void *userData) {
            // No hacer nada - suprimir salida de error automática
        });

        if (midiOut.getPortCount() == 0) {
            throw std::runtime_error("No MIDI ports available.");
        }

        // Abrir el primer puerto MIDI disponible
        try {
            midiOut.openPort(0);
        } catch (const RtMidiError& error) {
            // Convertir cualquier error de RtMidi en una excepción estándar
            throw std::runtime_error("RtMidi: " + error.getMessage());
        }

        // Usar un flujo de memoria para cargar el archivo MIDI
        std::stringstream midiStream(std::string(midiData.begin(), midiData.end()));

        // Usar la librería midi-file para interpretar los datos MIDI
        smf::MidiFile midiFile;
        midiFile.read(midiStream);
        midiFile.linkNotePairs();
        midiFile.doTimeAnalysis();
        midiFile.joinTracks();

        // Ajustar el volumen al 50%
        for (int track = 0; track < midiFile.getTrackCount(); ++track) {
            for (int event = 0; event < midiFile[track].size(); ++event) {
                auto& midiEvent = midiFile[track][event];
                if (midiEvent.isController() && midiEvent.getP1() == 7) { // Control Change, Volume
                    midiEvent.setP2(midiEvent.getP2() / 2);
                }
            }
        }

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
    catch (const RtMidiError& error) {
        // Convertir cualquier otro error de RtMidi en una excepción estándar
        throw std::runtime_error("RtMidi: " + error.getMessage());
    }
    catch (const std::exception& e) {
        // Re-lanzar excepciones estándar
        throw std::runtime_error(std::string(e.what()));
    }
    catch (...) {
        // Capturar cualquier otro tipo de excepción
        throw std::runtime_error("Unknown error during MIDI playback.");
    }
}