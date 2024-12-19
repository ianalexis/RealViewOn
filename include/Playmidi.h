#pragma once

#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <windows.h>
#include <RtMidi.h>
#include "MidiFile.h"
#include "resource.h"

// Function to load embedded MIDI data
std::vector<unsigned char> loadEmbeddedMidi();

// Function to send a MIDI message
void sendMidiMessage(RtMidiOut& midiOut, const std::vector<unsigned char>& message);

// Function to play MIDI data
void playMidi(const std::vector<unsigned char>& midiData);