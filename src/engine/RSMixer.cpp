#include "RSMixer.h"
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include "Config.hpp"

static RSMixer *g_rsmixer_instance = nullptr;

namespace {
std::mutex g_mixMutex;
}

static void ChannelFinishedCallback(int ch) {
    // Capture locale de l'instance pour éviter les accès à une instance détruite
    RSMixer *inst = g_rsmixer_instance;
    if (!inst || inst->shuttingDown) {
        return;
    }

    // Limiter la durée du verrouillage
    Mix_Chunk *chunkToFree = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_mixMutex);
        auto it = inst->channelChunks.find(ch);
        if (it != inst->channelChunks.end()) {
            chunkToFree = it->second;
            inst->channelChunks.erase(it);
        }
    }

    // Libérer la ressource hors du mutex
    if (chunkToFree) {
        Mix_FreeChunk(chunkToFree);
    }
}

RSMixer::RSMixer() {
    this->shuttingDown = false;
    this->music = nullptr;
    g_rsmixer_instance = this;
}

void RSMixer::init() {
    if (this->has_been_initialized) {
        return;
    }
    this->has_been_initialized = true;
    this->initted = Mix_Init(0);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        printf("Erreur d'initialisation audio: %s\n", Mix_GetError());
        return;
    }
    int freq = 44100;
    int chans = 2;
    Uint16 fmt = AUDIO_S16SYS;
    if (Mix_QuerySpec(&freq, &fmt, &chans)) {
        this->sampleRate = freq;
        this->audioFormat = fmt;
        this->audioChannels = chans;
    }
    if (this->audioFormat != AUDIO_S16SYS && this->audioFormat != AUDIO_F32SYS) {
        printf("RSMixer: format audio 0x%04X non gere, musique muette\n", this->audioFormat);
    }
    this->music = new RSMusic();
    {
        Config &mcfg = Config::getInstance();
        int src = mcfg.getInt("Sound", "midi_music", 0);
        if (mcfg.getString("Sound", "midi_port", "").empty() || src < 0 || src > 2) {
            src = 0;
        }
        this->music->midiMusic = src;
    }
    this->music->init();
    this->isplaying = false;
    {
        std::lock_guard<std::recursive_mutex> lock(engineMutex);
        OPL3_Reset(&chip, (uint32_t)this->sampleRate);
        adl.init([this](uint16_t reg, uint8_t val) {
            OPL3_WriteReg(&chip, reg, val);
        });
        xmi.init(&adl);
        sequencer.init(&xmi, &this->music->timbres);
        Config &cfg = Config::getInstance();
        std::string midiPort = cfg.getString("Sound", "midi_port", "");
        midi.mt32 = (this->music->midiMusic == 2);
        if (!midiPort.empty() && midi.open(midiPort.c_str())) {
            const bool dbg = cfg.getInt("Sound", "midi_debug", 0) != 0;
            if (this->music->midiMusic == 2) {
                mt32.out = &midi;
                sequencer.mt = &mt32;
                sequencer.mtLib = &this->music->timbresMt;
                xmi.sysexTap = [this](const uint8_t *p, size_t n) { midi.raw(p, n); };
            }
            xmi.midiTap = [this, dbg](int st, int d1, int d2) -> bool {
                for (int c = 0; c < SFX_CHANNELS; c++) {
                    if (sfxHandle[c] >= 0 && sfxHandle[c] == xmi.current) {
                        return false; // FM-Effekte (SOUNDFX.ADL) bleiben auf OPL
                    }
                }
                if (dbg && (st & 0xF0) == 0xC0) {
                    printf("MIDI ch%d program %d\n", (st & 0x0F) + 1, d1);
                }
                midi.send(st, d1, d2);
                return true;
            };
        }
    }
    Mix_HookMusic(RSMixer::musicHook, this);
    Mix_ChannelFinished(ChannelFinishedCallback);
}

RSMixer::~RSMixer() {
    // Signaler arrêt avec un atomic bool serait préférable
    shuttingDown = true;

    // Plus de rendu musical : le crochet est retire avant tout le reste
    Mix_HookMusic(nullptr, nullptr);

    // Bloquer les opérations pendant la destruction
    std::lock_guard<std::mutex> lock(g_mixMutex);

    // Désenregistrer immédiatement le callback
    Mix_ChannelFinished(nullptr);

    // Stopper tous les canaux pour éviter callbacks tardifs
    Mix_HaltChannel(-1);

    // Nettoyer les ressources audio
    for (auto &kv : channelChunks) {
        if (kv.second) {
            Mix_FreeChunk(kv.second);
        }
    }
    channelChunks.clear();

    if (this->music) {
        delete this->music;
        this->music = nullptr;
    }

    // Plus personne ne doit utiliser l'instance
    if (g_rsmixer_instance == this) {
        g_rsmixer_instance = nullptr;
    }

    Mix_CloseAudio();
    Mix_Quit();
}

// ---------------------------------------------------------------------------------------
// Rendu : appele par SDL_mixer dans le fil audio, a la place de la lecture Mix_Music.
// ---------------------------------------------------------------------------------------

void RSMixer::musicHook(void *udata, Uint8 *stream, int len) {
    RSMixer *self = static_cast<RSMixer *>(udata);
    if (!self || self->shuttingDown) {
        memset(stream, 0, (size_t)len);
        return;
    }
    int sampleBytes = (self->audioFormat == AUDIO_F32SYS) ? 4 : 2;
    int frames = len / (sampleBytes * self->audioChannels);
    if (self->audioFormat != AUDIO_S16SYS && self->audioFormat != AUDIO_F32SYS) {
        memset(stream, 0, (size_t)len);
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(self->engineMutex);
    self->mixBuffer.resize((size_t)frames * 2);
    self->render(self->mixBuffer.data(), frames);
    const int16_t *src = self->mixBuffer.data();
    const int vol = self->musicVolume;
    for (int f = 0; f < frames; f++) {
        int l = src[2 * f] * vol / MIX_MAX_VOLUME;
        int r = src[2 * f + 1] * vol / MIX_MAX_VOLUME;
        if (self->audioFormat == AUDIO_S16SYS) {
            int16_t *out = reinterpret_cast<int16_t *>(stream) + f * self->audioChannels;
            if (self->audioChannels == 1) {
                out[0] = (int16_t)((l + r) / 2);
            } else {
                out[0] = (int16_t)l;
                out[1] = (int16_t)r;
                for (int c = 2; c < self->audioChannels; c++) {
                    out[c] = 0;
                }
            }
        } else {
            float *out = reinterpret_cast<float *>(stream) + f * self->audioChannels;
            if (self->audioChannels == 1) {
                out[0] = (l + r) / 65536.0f;
            } else {
                out[0] = l / 32768.0f;
                out[1] = r / 32768.0f;
                for (int c = 2; c < self->audioChannels; c++) {
                    out[c] = 0;
                }
            }
        }
    }
}

// Pas du pilote (120 Hz) : interpreteur XMIDI + voix, puis fin de piste isolee / bouclage.
void RSMixer::serveDriver() {
    xmi.serve();
    if (musicHandle >= 0 && xmi.status(musicHandle) == SEQ_DONE) {
        if (loopsLeft < 0 || loopsLeft > 1) {
            if (loopsLeft > 0) {
                loopsLeft--;
            }
            xmi.start(musicHandle);
        } else {
            pushEvent(SCMusicEvent::TRACK_FINISHED, isolatedTrack, -1, -1, xmi.barCount(musicHandle));
            xmi.release(musicHandle);
            musicHandle = -1;
            isplaying = false;
        }
    }
    for (int c = 0; c < SFX_CHANNELS; c++) {
        if (sfxHandle[c] >= 0 && xmi.status(sfxHandle[c]) == SEQ_DONE) {
            xmi.release(sfxHandle[c]);
            sfxHandle[c] = -1;
        }
    }
}

// Meme decoupage que le lecteur sc_player : le sequenceur a 20 Hz, le pilote a 120 Hz,
// les echantillons OPL entre deux pas.
void RSMixer::render(int16_t *out, int frames) {
    const double drvStep = 120.0 / sampleRate;
    const double gameStep = 20.0 / sampleRate;
    int done = 0;
    while (done < frames) {
        double toDrv = (1.0 - accDriver) / drvStep;
        double toGame = (1.0 - accGame) / gameStep;
        int chunk = (int)(toDrv < toGame ? toDrv : toGame);
        if (chunk < 1) {
            chunk = 1;
        }
        if (chunk > frames - done) {
            chunk = frames - done;
        }
        OPL3_GenerateStream(&chip, out + 2 * done, (uint32_t)chunk);
        done += chunk;
        accDriver += chunk * drvStep;
        accGame += chunk * gameStep;
        if (accGame >= 1.0) {
            accGame -= 1.0;
            sequencer.tick();
            drainSequencer();
            if (sequenced && !sequencer.active()) {
                sequenced = false; // arret (fondu termine)
                isplaying = false;
            }
        }
        if (accDriver >= 1.0) {
            accDriver -= 1.0;
            serveDriver();
        }
    }
}

// ---------------------------------------------------------------------------------------
// Pistes isolees
// ---------------------------------------------------------------------------------------

void RSMixer::pushEvent(SCMusicEvent::Type type, int track, int fromTrack, int link, int measure) {
    SCMusicEvent e;
    e.type = type;
    e.track = track;
    e.fromTrack = fromTrack;
    e.link = link;
    e.measure = measure;
    if (events.size() >= SCMusicSequencer::MAX_EVENTS) {
        events.pop_front();
    }
    events.push_back(e);
}

void RSMixer::drainSequencer() {
    SCMusicEvent e;
    while (sequencer.pollEvent(e)) {
        pushEvent(e.type, e.track, e.fromTrack, e.link, e.measure);
    }
}

bool RSMixer::pollMusicEvent(SCMusicEvent &e) {
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    drainSequencer();
    if (events.empty()) {
        return false;
    }
    e = events.front();
    events.pop_front();
    return true;
}

uint32_t RSMixer::getRequestedMusicID() {
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    if (this->sequenced) {
        return sequencer.requested == 0xFFFF ? UINT32_MAX : (uint32_t)sequencer.requested;
    }
    return this->isplaying ? this->current_music : UINT32_MAX;
}

bool RSMixer::isInTransition() {
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    return this->sequenced && sequencer.state == 3;
}

int RSMixer::getMeasure() {
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    if (this->sequenced) {
        return sequencer.measure();
    }
    return musicHandle >= 0 ? xmi.barCount(musicHandle) : 0;
}

void RSMixer::stopMusicLocked() {
    if (musicHandle >= 0) {
        pushEvent(SCMusicEvent::MUSIC_STOPPED, isolatedTrack, -1, -1, xmi.barCount(musicHandle));
        xmi.stop(musicHandle);
        xmi.release(musicHandle);
        musicHandle = -1;
    }
    this->isplaying = false;
}

// Banque avec un .dat : la piste est demandee au sequenceur, qui attend la barre de mesure
// et joue la piste de liaison du .dat avant de passer a la piste demandee.
void RSMixer::playSequenced(const SCMusicSet *set, uint32_t index) {
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    if (musicHandle >= 0) {
        stopMusicLocked();
    }
    sequencer.setMusicSet(set); // arrete la musique si le jeu de pistes change
    sequencer.request((int)index);
    drainSequencer();
    this->sequenced = true;
    this->isplaying = true;
    this->current_music = index;
    this->currentMusicMemPtr = this->music->GetMusic(index);
}

void RSMixer::playMusic(uint32_t index, int loop) {
    if (shuttingDown || !this->music) {
        return;
    }
    const SCMusicSet *set = this->music->GetMusicSet();
    if (set != nullptr && (int)index < set->trackCount) {
        playSequenced(set, index);
        return;
    }
    if (this->isplaying && !this->sequenced && this->current_music == index) {
        return;
    }
    MemMusic *mus = this->music->GetMusic(index);
    if (mus == nullptr) {
        printf("No music found for index %d in bank %d\n", index, this->music->bank);
        return;
    }
    playIsolated(mus, loop, (int)index);
}

void RSMixer::playMusic(MemMusic *mus, int loop) {
    if (shuttingDown || !this->music || !mus) {
        return;
    }
    int index = -1; // position dans la banque courante, pour les evenements
    auto it = this->music->musics.find(this->music->bank);
    if (it != this->music->musics.end()) {
        for (size_t i = 0; i < it->second.size(); i++) {
            if (it->second[i] == mus) {
                index = (int)i;
                break;
            }
        }
    }
    const SCMusicSet *set = this->music->GetMusicSet();
    if (set != nullptr && index >= 0) {
        playMusic((uint32_t)index, loop);
        return;
    }
    if (this->isplaying && !this->sequenced && this->currentMusicMemPtr == mus) {
        return;
    }
    playIsolated(mus, loop, index);
}

// Banque sans .dat : la piste joue seule.
void RSMixer::playIsolated(MemMusic *mus, int loop, int index) {
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    stopMusicLocked();
    sequencer.stop(false); // une seule musique a la fois, comme le jeu
    drainSequencer();
    this->sequenced = false;
    this->currentMusicMemPtr = mus;
    this->current_music = index < 0 ? UINT32_MAX : (uint32_t)index;
    this->isolatedTrack = index;
    int err = 0;
    musicHandle = SCMusicSequencer::registerAndStart(&xmi, sequencer.mt ? sequencer.mtLib : &this->music->timbres, mus->data, mus->size, &err, sequencer.mt);
    if (musicHandle < 0) {
        printf("Error loading music (error %d)\n", err);
        return;
    }
    loopsLeft = loop < 0 ? -1 : (loop == 0 ? 1 : loop);
    this->isplaying = true;
    pushEvent(SCMusicEvent::TRACK_STARTED, index, -1, -1, 0);
}

void RSMixer::stopMusic(bool fade) {
    if (shuttingDown) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    stopMusicLocked();
    sequencer.stop(fade);
    drainSequencer();
    if (!fade) {
        this->sequenced = false;
    }
    this->currentMusicMemPtr = nullptr;
    this->current_music = UINT32_MAX;
}

uint32_t RSMixer::getMusicID() {
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    if (this->sequenced) {
        if (!sequencer.active() || sequencer.finished()) {
            return UINT32_MAX;
        }
        return (uint32_t)sequencer.current;
    }
    return this->isplaying ? this->current_music : UINT32_MAX;
}

// ---------------------------------------------------------------------------------------
// Effets XMIDI
// ---------------------------------------------------------------------------------------

int RSMixer::playSoundFx(MemMusic *fx, int volume) {
    if (shuttingDown || !fx || !this->music) {
        return -1;
    }
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    int c = 0;
    while (c < SFX_CHANNELS && sfxHandle[c] >= 0) {
        c++; // SoundFX_FindFreeChannel_598A6
    }
    if (c == SFX_CHANNELS) {
        return -1;
    }
    int err = 0;
    int h = SCMusicSequencer::registerAndStart(&xmi, &this->music->timbres, fx->data, fx->size, &err);
    if (h < 0) {
        return -1;
    }
    xmi.setRelVolume(h, volume, 0);
    sfxHandle[c] = h;
    return c;
}

void RSMixer::setSoundFxVolume(int fxChannel, int volume) {
    if (fxChannel < 0 || fxChannel >= SFX_CHANNELS) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    if (sfxHandle[fxChannel] >= 0) {
        xmi.setRelVolume(sfxHandle[fxChannel], volume, 0);
    }
}

void RSMixer::stopSoundFx(int fxChannel) {
    if (fxChannel < 0 || fxChannel >= SFX_CHANNELS) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    if (sfxHandle[fxChannel] >= 0) {
        xmi.stop(sfxHandle[fxChannel]);
        xmi.release(sfxHandle[fxChannel]);
        sfxHandle[fxChannel] = -1;
    }
}

bool RSMixer::isSoundFxPlaying(int fxChannel) {
    if (fxChannel < 0 || fxChannel >= SFX_CHANNELS) {
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(engineMutex);
    return sfxHandle[fxChannel] >= 0;
}

// ---------------------------------------------------------------------------------------
// VOC (inchange : canaux SDL_mixer)
// ---------------------------------------------------------------------------------------

void RSMixer::playSoundVoc(uint8_t *data, size_t vocSize) {
    if (shuttingDown || !data) {
        return;
    }

    // Ne pas stocker le pointeur data, faire une copie si nécessaire
    SDL_RWops *rw = SDL_RWFromConstMem(data, static_cast<int>(vocSize));
    if (!rw) {
        printf("Erreur création RWops: %s\n", SDL_GetError());
        return;
    }

    Mix_Chunk *chunk = Mix_LoadWAV_RW(rw, 1); // 1 = SDL_RWops sera automatiquement libéré
    if (!chunk) {
        printf("Erreur chargement VOC: %s\n", Mix_GetError());
        return;
    }

    int ch = Mix_PlayChannel(-1, chunk, 0);
    if (ch == -1) {
        printf("Erreur lecture son: %s\n", Mix_GetError());
        Mix_FreeChunk(chunk);
        return;
    }

    // Ajouter le chunk à la map avec protection mutex
    {
        std::lock_guard<std::mutex> lock(g_mixMutex);
        channelChunks[ch] = chunk;
    }

    channel = ch;
}

void RSMixer::playSoundVoc(uint8_t *data, size_t vocSize, int channel, int loop) {
    if (shuttingDown) {
        return;
    }
    // Ne pas stocker le pointeur data, faire une copie si nécessaire
    SDL_RWops *rw = SDL_RWFromConstMem(data, static_cast<int>(vocSize));
    if (!rw) {
        printf("Erreur création RWops: %s\n", SDL_GetError());
        return;
    }

    Mix_Chunk *chunk = Mix_LoadWAV_RW(rw, 1); // 1 = SDL_RWops sera automatiquement libéré
    if (!chunk) {
        printf("Erreur chargement VOC: %s\n", Mix_GetError());
        return;
    }
    int result = Mix_PlayChannel(channel, chunk, loop);
    if (result == -1) {
        printf("Error playing VOC sound on channel %d: %s\n", channel, Mix_GetError());
        Mix_FreeChunk(chunk);
        return;
    }
    // Ajouter le chunk à la map avec protection mutex
    {
        std::lock_guard<std::mutex> lock(g_mixMutex);
        channelChunks[channel] = chunk;
    }
}

void RSMixer::stopSound() {
    if (shuttingDown) {
        return;
    }
    Mix_HaltChannel(-1);
    std::lock_guard<std::mutex> lock(g_mixMutex);
    for (auto &kv : channelChunks) {
        if (kv.second) {
            Mix_FreeChunk(kv.second);
        }
    }
    channelChunks.clear();
}

void RSMixer::stopSound(int chanl) {
    if (shuttingDown) {
        return;
    }
    Mix_HaltChannel(chanl);
    std::lock_guard<std::mutex> lock(g_mixMutex);
    auto it = channelChunks.find(chanl);
    if (it != channelChunks.end()) {
        if (it->second) {
            Mix_FreeChunk(it->second);
        }
        channelChunks.erase(it);
    }
}

bool RSMixer::isSoundPlaying() {
    if (shuttingDown) {
        return false;
    }
    return Mix_Playing(channel) != 0;
}

bool RSMixer::isSoundPlaying(int chanl) {
    if (shuttingDown) {
        return false;
    }
    return Mix_Playing(chanl) != 0;
}
void RSMixer::setVolume(int volume, int channel) {
    if (channel == -1) {
        // Mix_VolumeMusic n'agit pas sur Mix_HookMusic : gain applique dans musicHook
        std::lock_guard<std::recursive_mutex> lock(engineMutex);
        musicVolume = volume < 0 ? 0 : (volume > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : volume);
    } else {
        Mix_Volume(channel, volume);
    }
}
void RSMixer::switchBank(uint8_t bank) {
    this->music->SwitchBank(bank);
}
