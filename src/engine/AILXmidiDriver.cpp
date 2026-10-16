//
//  AILXmidiDriver.cpp
//  libRealSpace
//
#include "AILXmidiDriver.h"
#include "AILAdlibTables.h"
#include "AILMidi.h"
#include <cstring>
#include <vector>

using std::memcmp;
using std::memset;
using namespace AILMidi;

#define QUANT_TIME_16 0x208D5 // 16 000 000 / 120
#define XMI_NSEQS AILXmidiDriver::NSEQS
#define XMI_MAX_NOTES XmidiSequence::MAX_NOTES
#define XMI_FOR_NEST XmidiSequence::FOR_NEST
// index (x 16 canaux) des controleurs journalises, dans l'ordre de ADL_CTRL_LOGGED
enum {
    C_PV = 0,
    C_MODUL = 16,
    C_PAN = 32,
    C_EXP = 48,
    C_SUS = 64,
    C_PBS = 80,
    C_LOCK = 96,
    C_PROT = 112,
    C_VPROT = 128
};
// lock_status[canal] (XMIDI.ASM : 'bit 7: locked')
const uint8_t LOCKED = 0x80;    // canal pris par CHAN_LOCK
const uint8_t PROTECTED = 0x40; // canal reserve par CHAN_PROTECT
const uint8_t NONE = 0xFF;      // valeur de journal / note / canal : absente

namespace {
    uint32_t be32(const uint8_t *p);
    int tag(const uint8_t *p, const char *t);
    uint32_t vln(const uint8_t *b, size_t len, size_t *pos);
    long findSeq(const uint8_t *b, size_t len, int num);
    void rewindSeq(XmidiSequence *s);
    size_t sysex(XmidiSequence *s);
    void grad(int *percent, int target, uint32_t *accum, uint32_t period);
} // namespace
namespace {
    uint32_t be32(const uint8_t *p) {
        return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
    }
} // namespace
namespace {
    int tag(const uint8_t *p, const char *t) {
        return memcmp(p, t, 4) == 0;
    }
} // namespace

/* VLN XMIDI (7 bits par octet, bit 7 = suite) */
namespace {
    uint32_t vln(const uint8_t *b, size_t len, size_t *pos) {
        uint32_t v = 0;
        while (*pos < len) {
            uint8_t c = b[(*pos)++];
            v = (v << 7) | (c & 0x7F);
            if (!(c & 0x80)) {
                break;
            }
        }
        return v;
    }
} // namespace

/* ---- findSeq : FORM XMID numero num dans un FORM XMID ou un CAT XMID ---- */
namespace {
    long findSeq(const uint8_t *b, size_t len, int num) {
        size_t p = 0;
        for (;;) {
            if (p + 12 > len) {
                return -1;
            }
            int is_cat = tag(b + p, "CAT ");
            int is_form = tag(b + p, "FORM");
            if (!is_cat && !is_form) {
                return -1;
            }
            if (tag(b + p + 8, "XMID")) {
                break;
            }
            p += be32(b + p + 4) + 8;
        }
        if (tag(b + p, "FORM")) {
            return num == 0 ? (long)p : -1;
        }
        size_t end = p + 8 + be32(b + p + 4);
        if (end > len) {
            end = len;
        }
        p += 12;
        int n = 0;
        while (p + 12 <= end) {
            if (tag(b + p + 8, "XMID")) {
                if (n == num) {
                    return (long)p;
                }
                n++;
            }
            p += be32(b + p + 4) + 8;
        }
        return -1;
    }
} // namespace

/* ---- rewindSeq, version ADLIB.ADV (sub_2DFA) ---- */
namespace {
void rewindSeq(XmidiSequence *s) {
    for (int i = 0; i < XMI_FOR_NEST; i++) {
        s->for_cnt[i] = -1;
    }
    for (int c = 0; c < 16; c++) {
        s->chan_map[c] = (uint8_t)c;
        s->chan_program[c] = s->chan_pitch_l[c] = s->chan_pitch_h[c] = s->chan_indirect[c] = NONE;
    }
    memset(s->chan_controls, NONE, sizeof(s->chan_controls));
    memset(s->note_chan, NONE, sizeof(s->note_chan));
    s->interval_cnt = 0;
    s->note_count = 0;
    s->vol_percent = s->vol_target = 100;
    s->tempo_percent = s->tempo_target = 100;
    s->tempo_error = 0;
    s->beat_count = 0;
    s->measure_count = 0;
    s->time_fraction = QUANT_TIME_16;
    s->beat_fraction = QUANT_TIME_16;
    s->time_numerator = 4;
    s->time_per_beat = 0x7A1200; /* 500 000 us/temps * 16 */
}
} // namespace

void AILXmidiDriver::flushChannelNotes(int chan) {
    for (int h = 0; h < XMI_NSEQS; h++) {
        XmidiSequence *s = &seq[h];
        if (!s->used || !s->note_count) {
            continue;
        }
        for (int i = 0; i < XMI_MAX_NOTES; i++) {
            if (s->note_chan[i] != chan) {
                continue;
            }
            s->note_chan[i] = NONE;
            int m = s->chan_map[chan];
            active_notes[m]--;
            send(NOTE_OFF | m, s->note_num[i], 0);
            s->note_count--;
        }
    }
}

void AILXmidiDriver::flushNoteQueue(XmidiSequence *s) {
    for (int i = 0; i < XMI_MAX_NOTES; i++) {
        if (s->note_chan[i] == NONE) {
            continue;
        }
        int m = s->chan_map[s->note_chan[i]];
        s->note_chan[i] = NONE;
        active_notes[m]--;
        send(NOTE_OFF | m, s->note_num[i], 0);
    }
    s->note_count = 0;
}

int AILXmidiDriver::lockChannel() {
    int best = -1;
    for (int mask = LOCKED | PROTECTED;; mask = LOCKED) {
        unsigned cl = 0xFFFF;
        for (int c = 8; c >= 1; c--) {
            if (lock_status[c] & mask) {
                continue;
            }
            if (active_notes[c] >= cl) {
                continue;
            }
            cl = active_notes[c];
            best = c;
        }
        if (best >= 0 || mask == LOCKED) {
            break;
        }
    }
    if (best < 0) {
        return 0;
    }
    send(CONTROL_CHANGE | best, SUSTAIN, 0);
    flushChannelNotes(best);
    active_notes[best] = 0;
    lock_status[best] |= LOCKED;
    return best + 1;
}

void AILXmidiDriver::releaseChannel(int chan1) {
    int c = chan1 - 1;
    if (c < 0 || c > 15 || !(lock_status[c] & LOCKED)) {
        return;
    }
    lock_status[c] &= (uint8_t)~LOCKED;
    active_notes[c] = 0;
    send(CONTROL_CHANGE | c, SUSTAIN, 0);
    send(CONTROL_CHANGE | c, ALL_NOTES_OFF, 0);
    for (int i = 0; i < 9; i++) {
        uint8_t v = global_controls[i * 16 + c];
        if (v != NONE) {
            send(CONTROL_CHANGE | c, ADL_CTRL_LOGGED[i], v);
        }
    }
    if (global_program[c] != NONE) {
        send(PROGRAM_CHANGE | c, global_program[c], 0);
    }
    if (global_pitch_l[c] != NONE && global_pitch_h[c] != NONE) {
        send(PITCH_BEND | c, global_pitch_l[c], global_pitch_h[c]);
    }
}

void AILXmidiDriver::resetSequence(XmidiSequence *s) {
    for (int c = 0; c < 16; c++) {
        if ((int8_t)s->chan_controls[C_SUS + c] >= SWITCH_ON) {
            global_controls[C_SUS + c] = 0;
            send(CONTROL_CHANGE | c, SUSTAIN, 0);
        }
        if ((int8_t)s->chan_controls[C_LOCK + c] >= SWITCH_ON) {
            flushChannelNotes(c);
            releaseChannel(s->chan_map[c] + 1);
            s->chan_map[c] = (uint8_t)c;
        }
        if ((int8_t)s->chan_controls[C_PROT + c] >= SWITCH_ON) {
            lock_status[c] &= (uint8_t)~PROTECTED;
        }
        if ((int8_t)s->chan_controls[C_VPROT + c] >= SWITCH_ON) {
            send(CONTROL_CHANGE | c, VOICE_PROTECT, 0);
        }
    }
}

void AILXmidiDriver::send(int st, int d1, int d2) {
    device->send(st, d1, d2);
}

void AILXmidiDriver::xmidiVolume(XmidiSequence *s) /* sub_311D */
{
    for (int c = 0; c < 16; c++) {
        uint8_t pv = s->chan_controls[C_PV + c];
        if (pv == NONE) {
            continue;
        }
        unsigned v = (unsigned)pv * (unsigned)s->vol_percent / 100u;
        if (v >= 127) {
            v = 127;
        }
        global_controls[C_PV + c] = (uint8_t)v;
        if (lock_status[c] & LOCKED) {
            continue;
        }
        send(CONTROL_CHANGE | s->chan_map[c], PART_VOLUME, (int)v);
    }
}

/* ---- XMIDI_control, version ADLIB.ADV (sub_317C) ---- */
void AILXmidiDriver::xmidiControl(XmidiSequence *s, int chan, int con, int val) {
    if (s->chan_indirect[chan] != NONE) {
        s->chan_indirect[chan] = NONE; /* table de controle : NULL dans le jeu */
    }
    if (ctrl_hash[con] != NONE) {
        int i = ctrl_hash[con] + chan;
        global_controls[i] = (uint8_t)val;
        s->chan_controls[i] = (uint8_t)val;
    }
    switch (con) {
    case PART_VOLUME:
        if (s->vol_percent != 100) {
            unsigned v = (unsigned)val * (unsigned)s->vol_percent / 100u;
            if (v >= 127) {
                v = 127;
            }
            val = (int)v;
            global_controls[C_PV + chan] = (uint8_t)val;
        }
        break;
    case CLEAR_BEAT_BAR:
        s->beat_count = 0;
        s->measure_count = 0;
        s->beat_fraction = s->time_fraction;
        return;
    case CALLBACK_TRIG:
        return; /* pas de fonction de rappel */
    case FOR_LOOP:
        for (int i = 0; i < XMI_FOR_NEST; i++) {
            if (s->for_cnt[i] == -1) {
                s->for_cnt[i] = val;
                s->for_ptrs[i] = s->evnt_ptr;
                break;
            }
        }
        return;
    case NEXT_LOOP:
        if (val < SWITCH_ON) {
            return;
        }
        for (int i = XMI_FOR_NEST - 1; i >= 0; i--) {
            if (s->for_cnt[i] == -1) {
                continue;
            }
            if (s->for_cnt[i] != 0 && --s->for_cnt[i] == 0) {
                s->for_cnt[i] = -1;
                return;
            }
            s->evnt_ptr = s->for_ptrs[i];
            return;
        }
        return;
    case CHAN_PROTECT:
        lock_status[chan] |= PROTECTED;
        if (val < SWITCH_ON) {
            lock_status[chan] &= (uint8_t)~PROTECTED;
        }
        return;
    case CHAN_LOCK:
        if (val >= SWITCH_ON) {
            int c = lockChannel() - 1;
            s->chan_map[chan] = (uint8_t)(c == -1 ? chan : c);
        } else {
            flushChannelNotes(chan);
            releaseChannel(s->chan_map[chan] + 1);
            s->chan_map[chan] = (uint8_t)chan;
        }
        return;
    case INDIRECT_C_PFX:
        s->chan_indirect[chan] = (uint8_t)val;
        return;
    default:
        break;
    }
    if (lock_status[chan] & LOCKED) {
        return;
    }
    send(CONTROL_CHANGE | s->chan_map[chan], con, val);
}

void AILXmidiDriver::restoreSequence(XmidiSequence *s) {
    for (int c = 0; c < 16; c++) {
        uint8_t l = s->chan_controls[C_LOCK + c];
        if (l == NONE || (int8_t)l < SWITCH_ON) {
            continue;
        }
        int m = lockChannel() - 1;
        s->chan_map[c] = (uint8_t)(m == -1 ? c : m);
    }
    for (int i = 0; i < 9; i++) {
        int con = ADL_CTRL_LOGGED[i];
        if (con == CHAN_LOCK) {
            continue;
        }
        for (int c = 0; c < 16; c++) {
            uint8_t v = s->chan_controls[i * 16 + c];
            if (v != NONE) {
                xmidiControl(s, c, con, v);
            }
        }
    }
    for (int c = 0; c < 16; c++) {
        if (s->chan_pitch_l[c] != NONE && s->chan_pitch_h[c] != NONE) {
            send(PITCH_BEND | s->chan_map[c], s->chan_pitch_l[c], s->chan_pitch_h[c]);
        }
        if (s->chan_program[c] != NONE) {
            send(PROGRAM_CHANGE | s->chan_map[c], s->chan_program[c], 0);
        }
    }
}

void AILXmidiDriver::init(MusicDevice *driver) {
    device = driver;
    for (auto &s : seq) {
        memset(&s, 0, sizeof s);
    }
    current = 0;
    memset(active_notes, 0, sizeof active_notes);
    memset(lock_status, 0, sizeof lock_status);
    memset(global_controls, NONE, sizeof(global_controls));
    memset(global_program, NONE, sizeof(global_program));
    memset(global_pitch_l, NONE, sizeof(global_pitch_l));
    memset(global_pitch_h, NONE, sizeof(global_pitch_h));
    memset(ctrl_hash, NONE, sizeof(ctrl_hash));
    for (int i = 0; i < 9; i++) {
        ctrl_hash[ADL_CTRL_LOGGED[i]] = (uint8_t)(i * 16);
    }
    for (int i = 0; i < 9; i++) { /* valeurs initiales, canaux 1..9 */
        uint8_t v = ADL_CTRL_DEFAULT[i];
        if (v == NONE) {
            continue;
        }
        for (int c = 1; c <= 9; c++) {
            global_controls[i * 16 + c] = v;
            send(CONTROL_CHANGE | c, ADL_CTRL_LOGGED[i], v);
        }
    }
    for (int c = 1; c <= 9; c++) {
        global_pitch_l[c] = PITCH_CENTER_L;
        global_pitch_h[c] = PITCH_CENTER_H;
        send(PITCH_BEND | c, PITCH_CENTER_L, PITCH_CENTER_H);
        uint8_t prg = ADL_PRG_DEFAULT[c - 1];
        if (prg != NONE) {
            global_program[c] = prg;
            send(PROGRAM_CHANGE | c, prg, 0);
        }
    }
}

int AILXmidiDriver::registerSequence(const uint8_t *data, size_t len, int num) {
    int h;
    for (h = 0; h < XMI_NSEQS; h++) {
        if (!seq[h].used) {
            break;
        }
    }
    if (h == XMI_NSEQS) {
        return -1;
    }
    long f = findSeq(data, len, num);
    if (f < 0) {
        return -1;
    }
    XmidiSequence *s = &seq[h];
    memset(s, 0, sizeof(*s));
    s->base = data;
    s->len = len;
    size_t p = (size_t)f + 12;
    size_t end = (size_t)f + 8 + be32(data + f + 4);
    if (end > len) {
        end = len;
    }
    while (p + 8 <= end) {
        if (tag(data + p, "TIMB")) {
            s->timb = p;
        } else if (tag(data + p, "EVNT")) {
            s->evnt = p;
            break;
        }
        p += be32(data + p + 4) + 8;
    }
    if (!s->evnt) {
        return -1;
    }
    s->used = 1;
    s->status = SEQ_STOPPED;
    rewindSeq(s);
    return h;
}

void AILXmidiDriver::release(int h) {
    if (h < 0 || !seq[h].used) {
        return;
    }
    if (seq[h].status == SEQ_PLAYING) {
        seq[h].post_release = 1;
        return;
    }
    seq[h].used = 0;
}

void AILXmidiDriver::stop(int h) {
    if (h < 0 || !seq[h].used) {
        return;
    }
    XmidiSequence *s = &seq[h];
    if (s->status != SEQ_PLAYING) {
        return;
    }
    flushNoteQueue(s);
    resetSequence(s);
    s->status = SEQ_STOPPED;
}

void AILXmidiDriver::start(int h) {
    if (h < 0 || !seq[h].used) {
        return;
    }
    XmidiSequence *s = &seq[h];
    if (s->status == SEQ_PLAYING) {
        stop(h);
    }
    rewindSeq(s);
    s->evnt_ptr = s->evnt + 8;
    s->status = SEQ_PLAYING;
    s->seq_started = 1;
}

void AILXmidiDriver::resume(int h) {
    if (h < 0 || !seq[h].used) {
        return;
    }
    XmidiSequence *s = &seq[h];
    if (s->status != SEQ_STOPPED || !s->seq_started) {
        return;
    }
    restoreSequence(s);
    s->status = SEQ_PLAYING;
}

int AILXmidiDriver::status(int h) {
    return (h < 0 || !seq[h].used) ? -1 : seq[h].status;
}
int AILXmidiDriver::barCount(int h) {
    return (h < 0 || !seq[h].used) ? -1 : seq[h].measure_count;
}
int AILXmidiDriver::beatCount(int h) {
    return (h < 0 || !seq[h].used) ? -1 : seq[h].beat_count;
}
int AILXmidiDriver::relVolume(int h) {
    return (h < 0 || !seq[h].used) ? -1 : seq[h].vol_percent;
}

void AILXmidiDriver::setRelVolume(int h, int vol, int ms) {
    if (h < 0 || !seq[h].used) {
        return;
    }
    XmidiSequence *s = &seq[h];
    s->vol_target = vol;
    if (ms == 0) {
        s->vol_percent = vol;
        xmidiVolume(s);
        return;
    }
    int delta = s->vol_target - s->vol_percent;
    if (delta < 0) {
        delta = -delta;
    }
    if (!delta) {
        return;
    }
    uint32_t period = (uint32_t)(10u * (unsigned)ms) / (uint32_t)delta;
    s->vol_period = period ? period : 1;
    s->vol_accum = 0;
}

/* get_request (fonction 0x9B, 0x1CC7) : premier timbre du chunk TIMB absent du cache */
int AILXmidiDriver::timbreRequest(int h) {
    if (h < 0 || !seq[h].used || !seq[h].timb) {
        return 0xFFFF;
    }
    const uint8_t *t = seq[h].base + seq[h].timb;
    int n = t[8] | (t[9] << 8);
    for (int i = 0; i < n; i++) {
        int patch = t[10 + 2 * i];
        int bank = t[11 + 2 * i];
        bool present = device->hasTimbre(bank, patch);
        if (!present) {
            return (bank << 8) | patch;
        }
    }
    return 0xFFFF;
}

/* ---- XMIDI_note_on (sub_3360) ---- */
size_t AILXmidiDriver::noteOn(XmidiSequence *s) {
    const uint8_t *b = s->base;
    size_t p = s->evnt_ptr;
    int chan = b[p] & CHANNEL_MASK;
    int note = b[p + 1];
    int vel = b[p + 2];
    size_t q = p + 3;
    uint32_t dur = vln(b, s->len, &q);
    size_t len = q - p;
    if (lock_status[chan] & LOCKED) {
        return len;
    }
    int slot = 0;
    for (int i = 0; i < XMI_MAX_NOTES; i++) {
        if (s->note_chan[i] == NONE) {
            slot = i;
            s->note_count++;
            break;
        }
    }
    s->note_chan[slot] = (uint8_t)chan;
    s->note_num[slot] = (uint8_t)note;
    s->note_time[slot] = (int32_t)dur - 1;
    int m = s->chan_map[chan];
    active_notes[m]++;
    send(NOTE_ON | m, note, vel);
    return len;
}

/* ---- XMIDI_meta, version ADLIB.ADV (sub_3418) ---- */
size_t AILXmidiDriver::meta(XmidiSequence *s, int h) {
    const uint8_t *b = s->base;
    size_t p = s->evnt_ptr;
    int type = b[p + 1];
    size_t q = p + 2;
    uint32_t dlen = vln(b, s->len, &q);
    size_t ev_len = (q - p) + dlen;
    const uint8_t *d = b + q;
    if (type == META_END_OF_TRACK) {
        resetSequence(s);
        s->status = SEQ_DONE;
        if (s->post_release) {
            s->used = 0;
        }
        (void)h;
    } else if (type == META_TIME_SIGNATURE) {
        s->time_numerator = d[0];
        int cl = d[1] - 2;
        int32_t tf;
        if (cl >= 0) {
            tf = QUANT_TIME_16 * (1 << cl);
        } else {
            tf = QUANT_TIME_16 >> (-cl);
        }
        s->time_fraction = tf;
        s->beat_fraction = tf;
    } else if (type == META_TEMPO) {
        uint32_t us = ((uint32_t)d[0] << 16) | ((uint32_t)d[1] << 8) | d[2];
        s->time_per_beat = (int32_t)(us << 4);
    }
    return ev_len;
}

namespace {
size_t sysex(XmidiSequence *s) {
    size_t p = s->evnt_ptr;
    size_t q = p + 1;
    uint32_t n = vln(s->base, s->len, &q);
    return (q - p) + n;
}
} // namespace

namespace {
void grad(int *percent, int target, uint32_t *accum, uint32_t period) {
    int up = *percent < target;
    uint32_t a = *accum + 83; /* QUANT_TIME / 100 */
    int cx = -1;
    for (;;) {
        cx++;
        *accum = a;
        if ((int32_t)(a - period) < 0) {
            break;
        }
        a -= period;
    }
    if (!cx) {
        return;
    }
    int v = up ? *percent + cx : *percent - cx;
    if (up ? v > target : v < target) {
        v = target;
    }
    *percent = v;
}
} // namespace

void AILXmidiDriver::serve() {
    for (int h = 0; h < XMI_NSEQS; h++) {
        XmidiSequence *s = &seq[h];
        if (!s->used || s->status != SEQ_PLAYING) {
            continue;
        }
        current = h;
        int ax = s->tempo_error + s->tempo_percent;
        s->tempo_error = ax;
        ax -= 100;
        int done = 0;
        while (ax >= 0) {
            s->tempo_error = ax;
            int32_t bf = s->beat_fraction + s->time_fraction;
            if (bf >= s->time_per_beat) {
                bf -= s->time_per_beat;
                if (++s->beat_count >= s->time_numerator) {
                    s->beat_count = 0;
                    s->measure_count++;
                }
            }
            s->beat_fraction = bf;

            if (s->note_count) {
                for (int i = 0; i < XMI_MAX_NOTES && s->note_count; i++) {
                    if (s->note_chan[i] == NONE) {
                        continue;
                    }
                    if (--s->note_time[i] >= 0) {
                        continue;
                    }
                    int m = s->chan_map[s->note_chan[i]];
                    s->note_chan[i] = NONE;
                    active_notes[m]--;
                    send(NOTE_OFF | m, s->note_num[i], 0);
                    s->note_count--;
                }
            }
            if (--s->interval_cnt <= 0) {
                for (;;) {
                    if (s->evnt_ptr >= s->len) {
                        s->status = SEQ_DONE;
                        break;
                    }
                    const uint8_t *b = s->base;
                    size_t p = s->evnt_ptr;
                    int st = b[p];
                    if (st < NOTE_OFF) {
                        s->evnt_ptr++;
                        s->interval_cnt = st;
                        break;
                    } /* < 0x80 : intervalle */
                    int op = st & STATUS_MASK;
                    int ch = st & CHANNEL_MASK;
                    int d1 = (p + 1 < s->len) ? b[p + 1] : 0;
                    int d2 = (p + 2 < s->len) ? b[p + 2] : 0;
                    size_t sz;
                    if (op == SYSEX) {
                        if (st == META) {
                            sz = meta(s, h);
                        } else {
                            sz = sysex(s);
                            if (st == 0xF0 && sz > 2 && p + sz <= s->len) {
                                size_t q = p + 1;
                                uint32_t n = vln(s->base, s->len, &q);
                                std::vector<uint8_t> m;
                                m.push_back(0xF0);
                                m.insert(m.end(), b + q, b + q + n);
                                if (m.back() != 0xF7) {
                                    m.push_back(0xF7);
                                }
                                device->sysex(m.data(), m.size());
                            }
                        }
                    } else if (op == PITCH_BEND) {
                        s->chan_pitch_l[ch] = (uint8_t)d1;
                        s->chan_pitch_h[ch] = (uint8_t)d2;
                        global_pitch_l[ch] = (uint8_t)d1;
                        global_pitch_h[ch] = (uint8_t)d2;
                        sz = 3;
                        if (!(lock_status[ch] & LOCKED)) {
                            send(op | s->chan_map[ch], d1, d2);
                        }
                    } else if (op == CHANNEL_PRESSURE) {
                        sz = 2;
                        if (!(lock_status[ch] & LOCKED)) {
                            send(op | s->chan_map[ch], d1, d2);
                        }
                    } else if (op == PROGRAM_CHANGE) {
                        s->chan_program[ch] = (uint8_t)d1;
                        global_program[ch] = (uint8_t)d1;
                        sz = 2;
                        if (!(lock_status[ch] & LOCKED)) {
                            send(op | s->chan_map[ch], d1, d2);
                        }
                    } else if (op == CONTROL_CHANGE) {
                        xmidiControl(s, ch, d1, d2);
                        sz = 3;
                    } else if (op == POLY_PRESSURE) {
                        sz = 3;
                        if (!(lock_status[ch] & LOCKED)) {
                            send(op | s->chan_map[ch], d1, d2);
                        }
                    } else {
                        sz = noteOn(s); /* NOTE_ON (XMIDI : pas de NOTE_OFF) */
                    }
                    s->evnt_ptr += sz;
                    if (!s->used || s->status != SEQ_PLAYING) {
                        done = 1;
                        break;
                    }
                }
                if (done) {
                    break;
                }
            }
            ax = s->tempo_error - 100;
        }
        if (done) {
            continue;
        }
        if (s->tempo_percent != s->tempo_target) {
            grad(&s->tempo_percent, s->tempo_target, &s->tempo_accum, s->tempo_period);
        }
        if (s->vol_percent != s->vol_target) {
            grad(&s->vol_percent, s->vol_target, &s->vol_accum, s->vol_period);
            xmidiVolume(s);
        }
    }
    device->serve();
}
