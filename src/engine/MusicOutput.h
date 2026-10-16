//
//  MusicOutput.h
//  libRealSpace
//
//  Device side of the XMIDI driver: what AILXmidiDriver calls on the hardware, like the device
//  part of an AIL 2.0 driver (ADLIB.ADV, MT32MPU.ADV...). One implementation per device, chosen
//  with the files it plays (RSMixer::init):
//    OplDevice           .ADL + STRIKE.AD, built-in OPL3 emulation
//    Mt32Device          .ROL + STRIKE.MT, MIDI port (MusicOutputs.h)
//    GeneralMidiDevice   .MID, MIDI port or SoundFont (MusicOutputs.h)
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "AILAdlibDriver.h"
#include "../commons/Nuked-OPL3/opl3.h"

class MusicDevice {
public:
    virtual ~MusicDevice() = default;
    virtual void send(int status, int d1, int d2) = 0;
    virtual void sysex(const uint8_t *data, size_t size) {
        (void)data;
        (void)size;
    }
    // AIL_timbre_request / AIL_install_timbre. rec == nullptr: timbre absent from the library.
    virtual bool hasTimbre(int bank, int patch) = 0;
    virtual void installTimbre(int bank, int patch, const uint8_t *rec) {
        (void)bank;
        (void)patch;
        (void)rec;
    }
    // 120 Hz service, after the XMIDI events of the interval
    virtual void serve() {}
    // Software devices add their output to an interleaved stereo int16 buffer
    virtual void render(int16_t *out, int frames) {
        (void)out;
        (void)frames;
    }
};

class OplDevice : public MusicDevice {
public:
    void init(int sampleRate) {
        OPL3_Reset(&chip, (uint32_t)sampleRate);
        adl.init([this](uint16_t reg, uint8_t val) {
            OPL3_WriteReg(&chip, reg, val);
        });
    }
    void send(int status, int d1, int d2) override { adl.send(status, d1, d2); }
    bool hasTimbre(int bank, int patch) override { return adl.timbreStatus(bank, patch); }
    void installTimbre(int bank, int patch, const uint8_t *rec) override {
        if (rec != nullptr) {
            adl.installTimbre(bank, patch, rec);
        }
    }
    void serve() override { adl.serve(); }
    void render(int16_t *out, int frames) override {
        buffer.resize((size_t)frames * 2);
        OPL3_GenerateStream(&chip, buffer.data(), (uint32_t)frames);
        for (size_t i = 0; i < buffer.size(); i++) {
            int v = out[i] + buffer[i];
            if (v > 32767) {
                v = 32767;
            }
            if (v < -32768) {
                v = -32768;
            }
            out[i] = (int16_t)v;
        }
    }
    AILAdlibDriver adl;

private:
    opl3_chip chip;
    std::vector<int16_t> buffer;
};
