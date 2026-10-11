//
//  SCMusicSequencer.h
//  libRealSpace
//
//  Sequenceur musical du jeu (seg121-125), porte d'apres le code :
//    Music_RequestTune_5A984, Music_SequencerTickISR_5940B (20 Hz, 4 etats),
//    Music_TuneTransitionResolve_595C2, Music_TuneTransitionCommit_5974D,
//    Music_ChannelRegisterSequence_59FF5, Music_ChannelStopSequence_59F1D,
//    Music_InstallTimbre_5A62A, Music_StopWithFade_AB1EF.
//  Voir strike_commander_re/analysis/MUSIC_SYSTEM.md section 4.
//
//  Ne garde que ce que donne le .dat : attente de la barre de mesure, position dans la
//  phrase, matrice et pistes de liaison. Le retour automatique apres une ponctuation
//  (pistes 0x10 a 0x12, piste de reprise word_7085B, cas 5 / 8 / 0x15 et "ennemi proche")
//  n'est PAS ici : c'est la mission qui choisit la piste suivante (MUSIC_SYSTEM.md 4.4).
//
#pragma once
#include "AILXmidiDriver.h"
#include "../realspace/SCMusicSet.h"
#include "Mt32Timbres.h"
#include <deque>

// Evenement du sequenceur, pour que le jeu (la mission) reagisse : reconnaitre une ponctuation,
// redemander une piste, etc. Le sequenceur ne sait pas ce que represente une piste.
struct SCMusicEvent {
    enum Type {
        TRANSITION_STARTED, // la barre est atteinte, la piste de liaison du .dat demarre
        TRACK_STARTED,      // la piste demandee demarre (apres la liaison, ou bascule directe)
        TRACK_FINISHED,     // une piste qui ne boucle pas s'est terminee
        MUSIC_STOPPED       // arret effectif (fondu termine compris)
    };
    Type type = TRACK_STARTED;
    int track = -1;     // piste concernee (TRANSITION_STARTED : piste demandee)
    int fromTrack = -1; // piste quittee (TRANSITION_STARTED, TRACK_STARTED), -1 sinon
    int link = -1;      // TRANSITION_STARTED : numero de la piste de liaison (0 = premiere)
    int measure = 0;    // mesure de la piste principale au moment de l'evenement
};

class SCMusicSequencer {
public:
    struct Channel {
        int handle = -1;
        int isLink = 0;
        int index = 0;
    }; // 5BE3h / 5BF5h

    void init(AILXmidiDriver *xmi, const SCTimbreLibrary *lib);
    void setMusicSet(const SCMusicSet *set); // jeu de pistes utilise (COMBAT, ...)
    void request(int tune);                  // Music_RequestTune_5A984
    void tick();                             // a appeler a 20 Hz
    void stop(bool fade);                    // Music_StopWithFade_AB1EF
    int measure();                           // AIL_measure_count du canal principal
    // La piste demandee est jouee jusqu'au bout et ne boucle pas (aucune autre demande en attente)
    bool finished();
    // Evenement suivant (le plus ancien), false si la file est vide. Appeler sous le meme verrou
    // que tick() : la file est remplie depuis le fil audio.
    bool pollEvent(SCMusicEvent &e);
    static const size_t MAX_EVENTS = 64; // au-dela, les plus anciens sont perdus
    bool active() const {
        return requested != 0xFFFF || fading;
    }

    // Music_ChannelRegisterSequence_59FF5 : enregistre la sequence, installe ses timbres
    // (AIL_timbre_request -> Music_InstallTimbre_5A62A), puis AIL_start_sequence.
    // Renvoie le handle, ou -1. Utilisable aussi pour jouer une piste isolee.
    static int registerAndStart(AILXmidiDriver *xmi, const SCTimbreLibrary *lib,
                                const uint8_t *data, size_t size, int *error, Mt32Uploader *mt = nullptr);

    // etat (public pour l'affichage)
    Channel mainCh;
    Channel linkCh;
    Mt32Uploader *mt = nullptr;             // gesetzt: Musik geht an den MT-32
    const SCTimbreLibrary *mtLib = nullptr; // STRIKE.MT
    int requested = 0xFFFF; // word_70859 (0xFFFF = arret)
    int current = 0;        // byte_72C90
    int state = 0;          // byte_70858
    int measureAtReq = 0;   // word_72C91 (mesure, puis position dans la phrase)
    int mainPlaying = 0;    // byte_72CA1
    int error = 0;          // byte_7084A
    bool fading = false;
    int lastMatrix = -1;
    int lastPos = -1;
    int lastValue = -1;
    int lastLink = -1;

private:
    AILXmidiDriver *xmi = nullptr;
    const SCTimbreLibrary *lib = nullptr;
    const SCMusicSet *set = nullptr;
    std::deque<SCMusicEvent> events;
    bool finishReported = false;
    bool hasPlayed() const;
    void emit(SCMusicEvent::Type type, int track, int fromTrack, int link, int measure);
    void startMain(int fromTrack); // demarre la piste 'current' sur le canal principal + evenement
    void chanStop(Channel &c);
    void chanPlay(Channel &c, const std::vector<uint8_t> *b, int isLink, int index);
    const std::vector<uint8_t> *track(int i);
    bool seqDone(Channel &c);
    void resolve();
    void commit();
};
