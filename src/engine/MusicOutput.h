//
//  MusicOutput.h
//  libRealSpace
//
//  Destination for the music (not for the sound effects). The sequencer and the mixer only talk to
//  this interface; a new output path is a new subclass, no signature has to change.
//    OplOutput           built-in OPL3 emulation (default)
//    MidiHardwareOutput  raw MIDI device (General MIDI or Roland MT-32), see MusicOutputs.h
//    SoftSynthOutput     SoundFont software synth (General MIDI), see MusicOutputs.h
//
#pragma once
#include <cstddef>
#include <cstdint>
#include "AILXmidiDriver.h"
#include "../realspace/SCMusicSet.h"

class MusicOutput {
public:
    virtual ~MusicOutput() = default;

    // Channel message. true = consumed, the OPL driver does not get it.
    virtual bool send(int status, int d1, int d2) {
        (void)status;
        (void)d1;
        (void)d2;
        return false;
    }
    // SysEx events inside a tune are only forwarded if this returns true.
    virtual bool forwardsSysex() const { return false; }
    virtual void sysex(const uint8_t *data, size_t size) {
        (void)data;
        (void)size;
    }

    // Timbre handling (AIL_timbre_request). An output that owns its timbres (MT-32 upload)
    // brings its own library and installs the timbres itself instead of the OPL driver.
    virtual bool ownsTimbres() const { return false; }
    virtual const SCTimbreLibrary *timbreLibrary() const { return nullptr; }
    virtual bool hasTimbre(int bank, int patch) const {
        (void)bank;
        (void)patch;
        return false;
    }
    // rec == nullptr: timbre not in the library, mark it as done
    virtual void installTimbre(int bank, int patch, const uint8_t *rec) {
        (void)bank;
        (void)patch;
        (void)rec;
    }

    // Software outputs mix into the audio stream (interleaved stereo int16, added to the OPL output).
    virtual void render(int16_t *out, int frames) {
        (void)out;
        (void)frames;
    }
};

// Default: nothing overridden, the music is played by the OPL3 emulation.
class OplOutput : public MusicOutput {};
