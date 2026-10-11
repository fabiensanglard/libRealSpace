//
//  SCMusicSequencer.cpp
//  libRealSpace
//
#include "SCMusicSequencer.h"
#include <cstdio>

void SCMusicSequencer::init(AILXmidiDriver *x, const SCTimbreLibrary *l) {
    xmi = x;
    lib = l;
    mainCh = Channel();
    linkCh = Channel();
    requested = 0xFFFF;
    current = 0;
    state = 0;
    measureAtReq = 0;
    mainPlaying = 0;
    error = 0;
    fading = false;
    lastMatrix = lastPos = lastValue = lastLink = -1;
    events.clear();
    finishReported = false;
}

void SCMusicSequencer::setMusicSet(const SCMusicSet *s) {
    if (set == s) {
        return;
    }
    stop(false);
    set = s;
}

void SCMusicSequencer::chanStop(Channel &c) // Music_ChannelStopSequence_59F1D
{
    if (c.handle < 0) {
        return;
    }
    xmi->stop(c.handle);
    xmi->release(c.handle);
    c.handle = -1;
}

int SCMusicSequencer::registerAndStart(AILXmidiDriver *xmi, const SCTimbreLibrary *lib,
                                       const uint8_t *data, size_t size, int *error, Mt32Uploader *mt) {
    if (!data) {
        if (error) {
            *error = 3;
        }
        return -1;
    }
    int h = xmi->registerSequence(data, size, 0);
    if (h < 0) {
        if (error) {
            *error = 3;
        }
        return -1;
    }
    std::function<bool(int, int)> mtHave;
    if (mt) {
        mtHave = [mt](int b, int p) { return mt->has(b, p); };
    }
    // Le jeu boucle sans fin sur un timbre introuvable ; ici on s'arrete et on le signale.
    for (int guard = 0; guard < 256; guard++) {
        int r = xmi->timbreRequest(h, mt ? &mtHave : nullptr);
        if (r == 0xFFFF) {
            break;
        }
        int bank = r >> 8;
        int patch = r & 0xFF;
        const uint8_t *t = lib ? lib->find(bank, patch) : nullptr; // Music_InstallTimbre_5A62A
        if (!t) {
            if (mt) {
                mt->install(bank, patch, nullptr); // als erledigt markieren (ROM-Klang), weiter
                continue;
            }
            if (error) {
                *error = 4;
            }
            std::printf("SCMusicSequencer : timbre absent de la bibliotheque (banque %d, patch %d)\n", bank, patch);
            break;
        }
        if (mt) {
            mt->install(bank, patch, t);
        } else {
            xmi->adl->installTimbre(bank, patch, t);
        }
    }
    xmi->start(h);
    return h;
}

void SCMusicSequencer::chanPlay(Channel &c, const std::vector<uint8_t> *b, int isLink, int index) {
    c.isLink = isLink;
    c.index = index;
    if (!b || b->empty()) {
        error = 3;
        return;
    }
    c.handle = registerAndStart(xmi, mt ? mtLib : lib, b->data(), b->size(), &error, mt);
}

const std::vector<uint8_t> *SCMusicSequencer::track(int i) {
    return (set && i >= 0 && i < (int)set->tracks.size()) ? &set->tracks[(size_t)i] : nullptr;
}

void SCMusicSequencer::request(int tune) {
    if (set && tune < set->trackCount) {
        requested = tune;
    } else if (tune != 0xFF) {
        std::printf("Invalid tune requested: %d\n", tune);
    }
}

int SCMusicSequencer::measure() {
    return mainCh.handle >= 0 ? xmi->barCount(mainCh.handle) : 0;
}

void SCMusicSequencer::emit(SCMusicEvent::Type type, int track, int fromTrack, int link, int measure) {
    SCMusicEvent e;
    e.type = type;
    e.track = track;
    e.fromTrack = fromTrack;
    e.link = link;
    e.measure = measure;
    if (events.size() >= MAX_EVENTS) {
        events.pop_front();
    }
    events.push_back(e);
}

bool SCMusicSequencer::pollEvent(SCMusicEvent &e) {
    if (events.empty()) {
        return false;
    }
    e = events.front();
    events.pop_front();
    return true;
}

bool SCMusicSequencer::hasPlayed() const {
    return requested != 0xFFFF || mainCh.handle >= 0 || linkCh.handle >= 0;
}

void SCMusicSequencer::startMain(int fromTrack) {
    chanPlay(mainCh, track(current), 0, current);
    mainPlaying = 1;
    finishReported = false;
    emit(SCMusicEvent::TRACK_STARTED, current, fromTrack, -1, 0);
}

bool SCMusicSequencer::finished() {
    return requested != 0xFFFF && state == 1 && seqDone(mainCh);
}

bool SCMusicSequencer::seqDone(Channel &c) {
    return c.handle < 0 ? true : xmi->status(c.handle) == SEQ_DONE;
}

void SCMusicSequencer::resolve() // Music_TuneTransitionResolve_595C2
{
    int bar = measure(); // mesure de la piste quittee, avant son arret
    int A = set->phraseLen[(size_t)current];
    int r = A ? (int)((int16_t)measureAtReq % A) : 0; // 'cwd / idiv bx'
    measureAtReq = r ? r + 1 : set->phraseLast[(size_t)current];
    uint8_t e = set->matrix[(size_t)(current * set->trackCount + requested)];
    uint8_t v = 0;
    if (e != 0xFF && e < set->linkEntries.size() && measureAtReq < (int)set->linkEntries[e].size()) {
        v = set->linkEntries[e][(size_t)measureAtReq];
    }
    lastMatrix = e;
    lastPos = measureAtReq;
    lastValue = v;
    lastLink = -1;

    if (v == 0) { // bascule directe
        int from = current;
        chanStop(mainCh);
        current = requested;
        startMain(from);
        state = 1;
        return;
    }
    if (v & 0x80) {
        v &= 0x7F; // la piste principale continue
    } else {
        chanStop(mainCh);
        mainPlaying = 0;
    }
    if (v > set->linkTracks.size()) { // refus
        error = 2;
        state = 1;
        requested = current;
        return;
    }
    lastLink = v - 1;
    emit(SCMusicEvent::TRANSITION_STARTED, requested, current, v - 1, bar);
    chanPlay(linkCh, &set->linkTracks[(size_t)(v - 1)], 1, v - 1);
    state = 3;
}

void SCMusicSequencer::commit() // Music_TuneTransitionCommit_5974D
{
    chanStop(linkCh);
    if (mainPlaying == 1) {
        chanStop(mainCh);
    }
    int from = current;
    current = requested & 0xFF;
    startMain(from);
}

void SCMusicSequencer::tick() // Music_SequencerTickISR_5940B
{
    if (fading) { // attente de la fin du fondu
        if (mainCh.handle < 0 || xmi->relVolume(mainCh.handle) == 0) {
            chanStop(mainCh);
            fading = false;
            emit(SCMusicEvent::MUSIC_STOPPED, current, -1, -1, 0);
        }
        return;
    }
    if (requested == 0xFFFF || !set) {
        return;
    }
    switch (state) {
    case 0:
        current = requested & 0xFF;
        startMain(-1);
        state = 1;
        break;
    case 1:
        if (current != requested) {
            measureAtReq = measure();
            state = 2;
        } else if (!finishReported && seqDone(mainCh)) {
            finishReported = true;
            emit(SCMusicEvent::TRACK_FINISHED, current, -1, -1, measure());
        }
        break;
    case 2:
        if (current == requested) {
            state = 1;
            break;
        }
        if (measure() != measureAtReq || seqDone(mainCh)) {
            resolve();
        }
        break;
    case 3:
        if (seqDone(linkCh)) {
            commit();
            state = 1;
        }
        break;
    default:
        error = 1;
        break;
    }
}

void SCMusicSequencer::stop(bool fade) // Music_StopWithFade_AB1EF
{
    bool wasPlaying = hasPlayed() && !fading;
    requested = 0xFFFF;
    state = 0;
    if (!xmi) {
        return;
    }
    chanStop(linkCh);
    if (fade && mainCh.handle >= 0) {
        xmi->setRelVolume(mainCh.handle, 0, 1000);
        fading = true;
    } else {
        chanStop(mainCh);
        fading = false;
        if (wasPlaying) {
            emit(SCMusicEvent::MUSIC_STOPPED, current, -1, -1, 0);
        }
    }
}
