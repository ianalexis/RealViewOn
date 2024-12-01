//#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <RtMidi.h>
#include "MidiFile.h" 
#include "resource.h"

std::vector<unsigned char> loadEmbeddedMidi();
void sendMidiMessage(RtMidiOut& midiOut, const std::vector<unsigned char>& message);
void playMidi(const std::vector<unsigned char>& midiData);