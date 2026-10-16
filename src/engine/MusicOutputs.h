//
//  MusicOutputs.h
//  libRealSpace
//
//  Concrete music outputs, see MusicOutput.h.
//
#pragma once
#include <cstdio>
#include <memory>
#include "MusicOutput.h"
#include "MidiOut.h"
#include "Mt32Timbres.h"
#include "SoftSynth.h"

// Raw MIDI device (Linux). mt32Mode: Roland MT-32 (reset sequence, timbres from STRIKE.MT are
// uploaded via SysEx); otherwise General MIDI (GS reset).
class MidiHardwareOutput : public MusicOutput {
public:
    MidiHardwareOutput(bool mt32Mode, const SCTimbreLibrary *mt32Library, bool debug)
    : mt32(mt32Mode), library(mt32Library), debugLog(debug) {
        midi.mt32 = mt32Mode;
        uploader.out = &midi;
    }
    bool open(const char *path) { return midi.open(path); }

    bool send(int status, int d1, int d2) override {
        if (debugLog && (status & 0xF0) == 0xC0) {
            std::printf("MIDI ch%d program %d\n", (status & 0x0F) + 1, d1);
        }
        midi.send(status, d1, d2);
        return true;
    }
    bool forwardsSysex() const override { return mt32; }
    void sysex(const uint8_t *data, size_t size) override { midi.raw(data, size); }

    bool ownsTimbres() const override { return mt32; }
    const SCTimbreLibrary *timbreLibrary() const override { return library; }
    bool hasTimbre(int bank, int patch) const override { return uploader.has(bank, patch); }
    void installTimbre(int bank, int patch, const uint8_t *rec) override { uploader.install(bank, patch, rec); }

private:
    bool mt32;
    const SCTimbreLibrary *library;
    bool debugLog;
    MidiOut midi;
    Mt32Uploader uploader;
};

// General MIDI through a SoundFont (TinySoundFont), no hardware needed.
class SoftSynthOutput : public MusicOutput {
public:
    bool open(const char *soundFont, int sampleRate) { return soft.open(soundFont, sampleRate); }
    bool send(int status, int d1, int d2) override {
        if (!soft.active()) {
            return false;
        }
        soft.send(status, d1, d2);
        return true;
    }
    void render(int16_t *out, int frames) override { soft.render(out, frames); }

private:
    SoftSynth soft;
};
