#pragma once
#include "MidiOut.h"
#include <cstdint>
#include <map>
#include <vector>
#include <utility>

// Laedt Timbres aus STRIKE.MT (Record: u16 LE 0x00F8 + 246 Byte) per SysEx in den MT-32.
// Speicherplatz n liegt bei Adresse 08 00 00 + n*256, Patch p wird bei 05 00 00 + p*8 zugewiesen.
class Mt32Uploader {
public:
    MidiOut *out = nullptr;

    bool has(int bank, int patch) const {
        auto it = slotOf.find(patch);
        return it != slotOf.end() && it->second.first == bank;
    }

    void install(int bank, int patch, const uint8_t *rec) {
        if (!out || !rec || (rec[0] | (rec[1] << 8)) != 248) {
            slotOf[patch] = { bank, -1 }; // als erledigt markieren, sonst fragt der Sequenzer endlos nach
            return;
        }
        int slot;
        auto it = slotOf.find(patch);
        if (it != slotOf.end() && it->second.second >= 0) {
            slot = it->second.second; // Patch hat schon einen Platz: ueberschreiben
        } else {
            if (next >= 64) { // MT-32 hat 64 Plaetze: von vorn anfangen
                slotOf.clear();
                next = 0;
            }
            slot = next++;
        }
        const uint8_t *t = rec + 2;
        static const int pos[5] = { 0, 14, 72, 130, 188 };
        static const int len[5] = { 14, 58, 58, 58, 58 };
        for (int k = 0; k < 5; k++) {
            out->sysex(sx(0x08, slot * 256 + pos[k], t + pos[k], len[k]));
        }
        const uint8_t pd[2] = { 0x02, (uint8_t)slot };
        out->sysex(sx(0x05, patch * 8, pd, 2));
        slotOf[patch] = { bank, slot };
    }

private:
    std::map<int, std::pair<int, int>> slotOf; // Patch -> (Bank, Slot)
    int next = 0;
    static std::vector<uint8_t> sx(int base, int offset, const uint8_t *p, size_t n) {
        std::vector<uint8_t> m = { 0xF0, 0x41, 0x10, 0x16, 0x12,
            (uint8_t)(base + ((offset >> 14) & 0x7F)),
            (uint8_t)((offset >> 7) & 0x7F), (uint8_t)(offset & 0x7F) };
            for (size_t i = 0; i < n; i++) {
                m.push_back(p[i]);
            }
            int sum = 0;
            for (size_t i = 5; i < m.size(); i++) {
                sum += m[i];
            }
            m.push_back((uint8_t)((128 - (sum & 0x7F)) & 0x7F));
            m.push_back(0xF7);
            return m;
    }
};
