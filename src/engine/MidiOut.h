#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>
#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#endif
#include <mutex>
#include <vector>

// Minimaler Raw-MIDI-Ausgang. Linux: ALSA-Geraeteknoten wie /dev/snd/midiC1D0.
class MidiOut {
public:
    ~MidiOut() { shut(); }
    bool mt32{ false }; // true: MT-32-Reset statt GS-Reset beim Oeffnen
    bool open(const char *path) {
        #if defined(__linux__)
        shut();
        fd = ::open(path, O_WRONLY | O_NONBLOCK);
        if (fd < 0) {
            std::printf("MidiOut: cannot open %s (%s)\n", path, strerror(errno));
            return false;
        }
        static const uint8_t gsReset[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };
       static const uint8_t mt32Reset[]    = { 0xF0,0x41,0x10,0x16,0x12,0x7F,0x00,0x00,0x01,0x00,0xF7 };
        static const uint8_t mt32Channels[] = { 0xF0,0x41,0x10,0x16,0x12,0x10,0x00,0x0D,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x36,0xF7 };
        static const uint8_t mt32Partials[] = { 0xF0,0x41,0x10,0x16,0x12,0x10,0x00,0x04,0x03,0x04,0x03,0x04,0x03,0x04,0x03,0x04,0x04,0x4C,0xF7 };
        static const uint8_t mt32Reverb[]   = { 0xF0,0x41,0x10,0x16,0x12,0x10,0x00,0x01,0x00,0x03,0x02,0x6A,0xF7 };
        if (mt32) {
            raw(mt32Reset, sizeof mt32Reset);
            usleep(500000); // MT-32 braucht nach dem Reset laenger
            raw(mt32Channels, sizeof mt32Channels);
            usleep(40000);
            raw(mt32Partials, sizeof mt32Partials);
            usleep(40000);
            raw(mt32Reverb, sizeof mt32Reverb);
            usleep(40000);
        } else {
            raw(gsReset, sizeof gsReset);
        }
        return true;
        #else
        (void)path;
        return false;
        #endif
    }
    void shut() {
        #if defined(__linux__)
        if (fd >= 0) {
            allNotesOff();
            ::close(fd);
            fd = -1;
        }
        #endif
    }
    void send(int status, int d1, int d2) {
        uint8_t m[3] = { (uint8_t)status, (uint8_t)(d1 & 0x7F), (uint8_t)(d2 & 0x7F) };
        int op = status & 0xF0;
        raw(m, (op == 0xC0 || op == 0xD0) ? 2 : 3);
    }
    void allNotesOff() {
        for (int c = 0; c < 16; c++) {
            send(0xB0 | c, 123, 0);
        }
    }
    void raw(const uint8_t *p, size_t n) {
        #if defined(__linux__)
        std::lock_guard<std::mutex> lk(mtx);
        writeAll(p, n);
        #else
        (void)p;
        (void)n;
        #endif
    }
    // SysEx senden und der Hardware Zeit geben (31250 Baud = ca. 0,32 ms pro Byte)
    void sysex(const std::vector<uint8_t> &m) {
        raw(m.data(), m.size());
        #if defined(__linux__)
        usleep((useconds_t)(m.size() * 320 + 20000));
        #endif
    }

private:
    std::mutex mtx;
    int fd = -1;
    #if defined(__linux__)
    void writeAll(const uint8_t *p, size_t n) {
        if (fd < 0) {
            return;
        }
        size_t done = 0;
        int tries = 0;
        while (done < n && tries < 2000) {
            ssize_t r = ::write(fd, p + done, n - done);
            if (r > 0) {
                done += (size_t)r;
                tries = 0;
            } else if (r < 0 && errno == EAGAIN) {
                usleep(1000);
                tries++;
            } else {
                break;
            }
        }
    }
    #endif
};
