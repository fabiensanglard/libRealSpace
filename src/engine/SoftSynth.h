#pragma once
#include "third_party/tsf.h"
#include <cstdint>
#include <cstdio>
#include <mutex>

// General-MIDI-Software-Synthesizer (TinySoundFont, SoundFont .sf2).
// Die Implementierung von tsf wird in RSMixer.cpp erzeugt (TSF_IMPLEMENTATION).
class SoftSynth {
public:
    ~SoftSynth() { close(); }

    bool open(const char *path, int sampleRate) {
        close();
        std::lock_guard<std::mutex> lk(mtx);
        f = tsf_load_filename(path);
        if (!f) {
            std::printf("SoftSynth: cannot load SoundFont %s\n", path);
            return false;
        }
        tsf_set_output(f, TSF_STEREO_INTERLEAVED, sampleRate, 0.0f);
        tsf_channel_set_presetnumber(f, 9, 0, 1); // Kanal 10 = Schlagzeug
        std::printf("SoftSynth: SoundFont %s loaded\n", path);
        return true;
    }

    void close() {
        std::lock_guard<std::mutex> lk(mtx);
        if (f) {
            tsf_close(f);
            f = nullptr;
        }
    }

    bool active() const { return f != nullptr; }

    void send(int st, int d1, int d2) {
        std::lock_guard<std::mutex> lk(mtx);
        if (!f) {
            return;
        }
        const int ch = st & 0x0F;
        switch (st & 0xF0) {
        case 0x80:
            tsf_channel_note_off(f, ch, d1);
            break;
        case 0x90:
            if (d2 > 0) {
                tsf_channel_note_on(f, ch, d1, d2 / 127.0f);
            } else {
                tsf_channel_note_off(f, ch, d1);
            }
            break;
        case 0xB0:
            tsf_channel_midi_control(f, ch, d1, d2);
            break;
        case 0xC0:
            tsf_channel_set_presetnumber(f, ch, d1, ch == 9);
            break;
        case 0xE0:
            tsf_channel_set_pitchwheel(f, ch, (d1 & 0x7F) | ((d2 & 0x7F) << 7));
            break;
        default:
            break;
        }
    }

    // mischt additiv in einen int16-Stereo-Puffer
    void render(int16_t *out, int frames) {
        std::lock_guard<std::mutex> lk(mtx);
        if (f) {
            tsf_render_short(f, out, frames, 1);
        }
    }

private:
    std::mutex mtx;
    tsf *f = nullptr;
};
