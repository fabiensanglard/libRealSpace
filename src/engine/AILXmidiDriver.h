//
//  AILXmidiDriver.h
//  libRealSpace
//
//  Interpreteur XMIDI du pilote ADLIB.ADV (shell XMIDI.ASM d'AIL 2.0), service a 120 Hz.
//  Porte d'apres strike_commander_re/analysis/ail_sources/XMIDI.ASM, avec les ecarts de la
//  version compilee dans ADLIB.ADV (lus dans adlib.asm) :
//   - rewind (sub_2DFA) : compteur de mesures a 0 (et non -1), fraction de temps = QUANT_TIME_16 ;
//   - signature rythmique (sub_3418) : ne remet pas le temps a 0, n'incremente pas la mesure ;
//   - CLEAR_BEAT_BAR (sub_317C) : fraction de temps = fraction par intervalle ;
//   - service (0x35B1) : le temps est avance AVANT la file de notes et les evenements ;
//   - get_bar_count / get_beat_count renvoient les compteurs bruts.
//
#pragma once
#include <cstdint>
#include <cstddef>
#include "AILAdlibDriver.h"
#include <functional>

enum {
    SEQ_STOPPED = 0,
    SEQ_PLAYING = 1,
    SEQ_DONE = 2
};

struct XmidiSequence {
    static const int MAX_NOTES = 32;
    static const int FOR_NEST = 4;
    int used;
    const uint8_t *base; // donnees XMIDI (FORM XDIR + CAT XMID, ou FORM XMID)
    size_t len;
    size_t timb; // offsets des chunks TIMB / EVNT (0 si absent)
    size_t evnt;
    size_t evnt_ptr;
    int seq_started;
    int status;
    int post_release;
    int interval_cnt;
    int note_count;
    int vol_percent;
    int vol_target;
    uint32_t vol_accum;
    uint32_t vol_period;
    int tempo_error;
    int tempo_percent;
    int tempo_target;
    uint32_t tempo_accum;
    uint32_t tempo_period;
    int beat_count;
    int measure_count;
    int time_numerator;
    int32_t time_fraction;
    int32_t beat_fraction;
    int32_t time_per_beat;
    size_t for_ptrs[FOR_NEST];
    int for_cnt[FOR_NEST];
    uint8_t chan_map[16];
    uint8_t chan_program[16];
    uint8_t chan_pitch_l[16];
    uint8_t chan_pitch_h[16];
    uint8_t chan_indirect[16];
    uint8_t chan_controls[9 * 16]; // ctrl_log : [controle*16 + canal]
    uint8_t note_chan[MAX_NOTES];
    uint8_t note_num[MAX_NOTES];
    int32_t note_time[MAX_NOTES];
};

class AILXmidiDriver {
public:
    static const int NSEQS = 8;
    void init(AILAdlibDriver *driver);                              // init_driver
    int registerSequence(const uint8_t *data, size_t len, int num); // register_seq, -1 si echec
    void release(int h);
    void start(int h);
    void stop(int h);
    void resume(int h);
    int status(int h);
    int barCount(int h);
    int beatCount(int h);
    void setRelVolume(int h, int vol, int ms);
    int relVolume(int h);
    int timbreRequest(int h, const std::function<bool(int, int)> *have = nullptr); // (banque << 8) | patch du premier timbre absent, ou 0xFFFF
    // optional: bekommt SysEx-Events aus dem Stueck (F0 ... F7). Kein Warten im Callback!
    std::function<void(const uint8_t *, size_t)> sysexTap;
    void serve();             // a appeler a 120 Hz ; appelle aussi AILAdlibDriver::serve()

    AILAdlibDriver *adl = nullptr;
    XmidiSequence seq[NSEQS];
    int current = 0;
    uint8_t ctrl_hash[256];
    uint8_t global_controls[9 * 16];
    uint8_t global_program[16];
    uint8_t global_pitch_l[16];
    uint8_t global_pitch_h[16];
    uint8_t active_notes[16];
    uint8_t lock_status[16];

    // optional: gibt true zurueck, wenn die Nachricht an ein externes MIDI-Geraet ging
    // (dann wird sie nicht an den OPL-Treiber weitergegeben)
    std::function<bool(int, int, int)> midiTap;

private:
    void send(int st, int d1, int d2);
    void flushChannelNotes(int chan);
    void flushNoteQueue(XmidiSequence *s);
    int lockChannel();
    void releaseChannel(int chan1);
    void resetSequence(XmidiSequence *s);
    void xmidiVolume(XmidiSequence *s);
    void xmidiControl(XmidiSequence *s, int chan, int con, int val);
    void restoreSequence(XmidiSequence *s);
    size_t noteOn(XmidiSequence *s);
    size_t meta(XmidiSequence *s, int h);
};
