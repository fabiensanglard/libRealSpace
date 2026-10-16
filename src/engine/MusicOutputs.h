//
//  MusicOutputs.h
//  libRealSpace
//
//  MIDI devices, see MusicOutput.h.
//
#pragma once
#include <cstdio>
#include <memory>
#include "MusicOutput.h"
#include "MidiOut.h"
#include "Mt32Timbres.h"
#include "SoftSynth.h"

// Roland MT-32 on a MIDI port: .ROL tunes, timbres of STRIKE.MT sent as SysEx
class Mt32Device : public MusicDevice {
public:
    explicit Mt32Device(bool debug) : debugLog(debug) {
        midi.mt32 = true;
        uploader.out = &midi;
    }
    bool open(const char *path) { return midi.open(path); }
    void send(int status, int d1, int d2) override {
        if (debugLog && (status & 0xF0) == 0xC0) {
            std::printf("MIDI ch%d program %d\n", (status & 0x0F) + 1, d1);
        }
        midi.send(status, d1, d2);
    }
    void sysex(const uint8_t *data, size_t size) override { midi.raw(data, size); }
    bool hasTimbre(int bank, int patch) override { return uploader.has(bank, patch); }
    void installTimbre(int bank, int patch, const uint8_t *rec) override { uploader.install(bank, patch, rec); }

private:
    bool debugLog;
    MidiOut midi;
    Mt32Uploader uploader;
};

// General MIDI: .MID tunes, standard programs (no timbre library), on a MIDI port or a SoundFont
class GeneralMidiDevice : public MusicDevice {
public:
    explicit GeneralMidiDevice(bool debug) : debugLog(debug) {}
    bool openPort(const char *path) {
        usePort = midi.open(path);
        return usePort;
    }
    bool openSoundFont(const char *path, int sampleRate) { return soft.open(path, sampleRate); }
    void send(int status, int d1, int d2) override {
        if (debugLog && (status & 0xF0) == 0xC0) {
            std::printf("MIDI ch%d program %d\n", (status & 0x0F) + 1, d1);
        }
        if (usePort) {
            midi.send(status, d1, d2);
        } else {
            soft.send(status, d1, d2);
        }
    }
    bool hasTimbre(int bank, int patch) override {
        (void)bank;
        (void)patch;
        return true;
    }
    void render(int16_t *out, int frames) override {
        if (!usePort) {
            soft.render(out, frames);
        }
    }

private:
    bool debugLog;
    bool usePort{ false };
    MidiOut midi;
    SoftSynth soft;
};
