#ifndef PLAYMIDI_H
#define PLAYMIDI_H
#include <atomic>
#include <vector>

// Function to load embedded MIDI data
std::vector<unsigned char> loadEmbeddedMidi();

// Function to play MIDI data. Bloquea hasta que termina la canción, o hasta que
// `detener` pasa a true: en ese caso corta en menos de 50 ms y apaga las notas
// que estaban sonando.
void playMidi(const std::vector<unsigned char>& midiData, const std::atomic<bool>* detener = nullptr);

#endif // PLAYMIDI_H
