//
//  RSMixer.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
//  Musique et effets XMIDI : chaine du jeu original, sans ADLMIDI ni banque WOPL.
//    SCMusicSequencer (20 Hz) -> AILXmidiDriver (120 Hz) -> AILAdlibDriver -> Nuked-OPL3,
//  rendue dans le flux de SDL_mixer par Mix_HookMusic. Les VOC restent sur les canaux SDL_mixer.
//
#pragma once
#include "../realspace/AssetManager.h"
#include "../realspace/RSMusic.h"
#include <SDL2/SDL_mixer_ext.h>
#include <deque>
#include <mutex>
#include <vector>
#include "../commons/Nuked-OPL3/opl3.h"
#include "AILAdlibDriver.h"
#include "AILXmidiDriver.h"
#include "SCMusicSequencer.h"
#include "MidiOut.h"
#include "Mt32Timbres.h"

class RSMixer {
    int initted;
    bool has_been_initialized{ false };
    bool isplaying;
    uint32_t current_music{ UINT32_MAX };
    int channel{ 0 };
    uint8_t *voc_data{ nullptr };

    // --- moteur musical (protege par engineMutex : le rendu tourne dans le fil audio) ---
    std::recursive_mutex engineMutex;
    opl3_chip chip;
    AILAdlibDriver adl;
    AILXmidiDriver xmi;
    MidiOut midi;
    Mt32Uploader mt32;
    SCMusicSequencer sequencer;
    int sampleRate{ 44100 };
    uint16_t audioFormat{ AUDIO_S16SYS };
    int audioChannels{ 2 };
    double accDriver{ 0 };
    double accGame{ 0 };
    int musicVolume{ MIX_MAX_VOLUME };
    int musicHandle{ -1 };             // piste isolee (playMusic) : handle XMIDI
    int loopsLeft{ 0 };                // -1 = sans fin
    static const int SFX_CHANNELS = 5; // objet effets du jeu : 5 canaux (MUSIC_SYSTEM.md 7.1)
    int sfxHandle[SFX_CHANNELS]{
        -1,
        -1,
        -1,
        -1,
        -1
    };
    std::vector<int16_t> mixBuffer;

    static void musicHook(void *udata, Uint8 *stream, int len);
    void render(int16_t *out, int frames); // appele sous engineMutex
    bool sequenced{ false };               // la musique courante passe par le sequenceur (banque avec .dat)
    int isolatedTrack{ -1 };               // piste isolee : index dans la banque (-1 si inconnu), pour les evenements
    std::deque<SCMusicEvent> events;
    void pushEvent(SCMusicEvent::Type type, int track, int fromTrack, int link, int measure);
    void drainSequencer(); // recopie les evenements du sequenceur dans 'events'
    void stopMusicLocked();
    void playSequenced(const SCMusicSet *set, uint32_t index);
    void playIsolated(MemMusic *mus, int loop, int index);
    void serveDriver();

public:
    RSMusic *music;
    bool shuttingDown = false;
    std::unordered_map<int, Mix_Chunk *> channelChunks;
    MemMusic *currentMusicMemPtr = nullptr;
    static RSMixer &getInstance() {
        static RSMixer instance;
        return instance;
    }
    RSMixer();
    ~RSMixer();
    void init();
    // Joue la piste index de la banque courante.
    // - Banque avec un .dat (COMBAT.ADL + COMBAT.DAT) : le changement de piste attend la barre
    //   de mesure suivante et passe par la piste de liaison que donne le .dat, s'il y en a une
    //   (Music_RequestTune_5A984). loop est ignore : les pistes bouclent d'elles-memes.
    // - Autre banque : piste jouee seule. loop : -1 = sans fin, n >= 1 = n fois (0 = une fois).
    void playMusic(uint32_t index, int loop = 1);
    // Meme chose ; si mus appartient a une banque avec un .dat, il passe par les transitions.
    void playMusic(MemMusic *mus, int loop = 1);
    void switchBank(uint8_t bank);
    // fade : fondu d'une seconde (Music_StopWithFade_AB1EF), sinon arret immediat
    void stopMusic(bool fade = false);
    // Piste en train de jouer (apres une transition), UINT32_MAX si aucune
    uint32_t getMusicID();
    // Piste demandee (celle vers laquelle une transition est en cours), UINT32_MAX si arret
    uint32_t getRequestedMusicID();
    // Une piste de liaison du .dat est en train de jouer
    bool isInTransition();
    // Mesure courante de la piste principale (compteur du pilote XMIDI)
    int getMeasure();

    // Evenements musicaux, a lire depuis la boucle de jeu (une fois par frame, jusqu'a false).
    // RSMixer ne sait pas ce que represente une piste : c'est au consommateur de reconnaitre
    // une ponctuation (0x10 a 0x12 dans COMBAT) et de demander la piste suivante.
    //   TRANSITION_STARTED : track = piste demandee, fromTrack = piste quittee, link = liaison
    //   TRACK_STARTED      : track = piste qui demarre, fromTrack = precedente (-1 au depart)
    //   TRACK_FINISHED     : track = piste qui ne boucle pas et vient de se terminer
    //   MUSIC_STOPPED      : arret effectif (stopMusic, fondu termine, ou autre musique lancee)
    // File limitee a SCMusicSequencer::MAX_EVENTS : au-dela, les plus anciens sont perdus.
    bool pollMusicEvent(SCMusicEvent &e);

    // Effets XMIDI (SOUNDFX.ADL) : 5 canaux comme le jeu. volume en pourcentage (0..100,
    // SoundFX_Play3D_59902 : 100 - distance/10). Renvoie le canal, -1 si aucun libre.
    int playSoundFx(MemMusic *fx, int volume = 100);
    void setSoundFxVolume(int fxChannel, int volume);
    void stopSoundFx(int fxChannel); // SoundFX_StopEffect_59A8A : Note Off seulement
    bool isSoundFxPlaying(int fxChannel);

    void stopSound();
    void stopSound(int chanl);
    void playSoundVoc(uint8_t *data, size_t vocSize);
    void playSoundVoc(uint8_t *data, size_t vocSize, int channel, int loop = 0);
    void setVolume(int volume, int channel = -1);
    bool isSoundPlaying();
    bool isSoundPlaying(int chanl);
};
