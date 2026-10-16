//
//  RSMusic.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
//  Chargement des musiques (utilitaire). Un fichier de musique (.ADL, .PAK) contient un ou
//  plusieurs jeux de pistes ; un .dat facultatif donne les transitions de chaque jeu (longueurs
//  de phrase, matrice, pistes de liaison) pour SCMusicSequencer. STRIKE.AD est la bibliotheque
//  de timbres du pilote AdLib (OPL simples + TVFX).
//  Structures : strike_commander_re/analysis/MUSIC_SYSTEM.md 2.1 a 2.4.
//
#pragma once
#include "precomp.h"
#include "SCMusicSet.h"
#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct MemMusic {
    uint8_t *data;
    size_t size;
};

// Un jeu de pistes charge depuis un fichier de musique.
struct RSMusicSet {
    SCMusicSet data;                // pistes, pistes de liaison, et donnees du .dat si hasDat
    std::vector<MemMusic *> tracks; // pistes principales (pointent dans data.tracks), pour GetMusic
    bool hasDat = false;            // transitions disponibles : RSMixer passe par le sequenceur
};

class RSMusic {
private:
    std::vector<MemMusic *> gameflow_music;
    AssetManager &assetManager = AssetManager::instance();
    std::vector<uint8_t> timbre_data;               // copie de la bibliotheque (timbres ne fait que pointer dessus)
    std::vector<uint8_t> sfx_timbre_data;
    std::string music_ext;
    std::string midgames_pak;
    std::string timbre_file;
    std::string sfx_ext;
    std::string sfx_timbre_file;
    bool loadTimbres(const std::string &file, SCTimbreLibrary &library, std::vector<uint8_t> &storage);
    std::vector<std::unique_ptr<RSMusicSet>> owned; // tous les jeux charges
    std::unordered_map<uint8_t, RSMusicSet *> bank_sets;
    RSMusicSet *newSet();
    void collectTracks(PakArchive *pak, const char *name, std::vector<std::vector<uint8_t>> &out, int depth);
    bool loadSetWithLinks(PakArchive *setpak, const char *name, RSMusicSet *set);

public:
    uint8_t bank{ 0 };
    std::unordered_map<uint8_t, std::vector<MemMusic *>> midgames_musics;
    std::unordered_map<uint8_t, std::vector<MemMusic *>> combat_musics;
    std::unordered_map<uint8_t, std::vector<MemMusic *>> gameflow_musics;
    std::unordered_map<uint8_t, std::vector<MemMusic *>> musics;
    std::unordered_map<uint8_t, std::vector<MemMusic *>> soundfx_musics;

    // Jeux charges, par nom de fichier (tel que passe a LoadMusicFile)
    std::unordered_map<std::string, std::vector<RSMusicSet *>> music_files;
    // "strike." + suffixe du pilote : STRIKE.AD (ADLIB.ADV), STRIKE.MT (MT-32) ; aucune en General MIDI
    // (Music_LoadTimbreFromLibrary_5A577). sfx_timbres : celle des effets (SOUNDFX).
    SCTimbreLibrary timbres;
    SCTimbreLibrary sfx_timbres;

    // Charge un fichier de musique et renvoie ses jeux (vide si le fichier est absent).
    // - Si les entrees de premier niveau sont des pistes (FORM), le fichier forme un seul jeu,
    //   une piste par entree.
    // - Sinon chaque entree de premier niveau est un jeu. Quand son entree 0 est une archive et
    //   les suivantes des pistes, c'est la structure de COMBAT.ADL : [0] pistes de liaison,
    //   [1..N] pistes principales. Sinon les pistes sont toutes les sequences FORM trouvees dans
    //   ses sous-archives.
    // datFile : le .dat qui donne les transitions, nullptr s'il n'y en a pas. L'enregistrement i
    // du .dat va avec le jeu i (AudioQueue_ProcessMain_AA84E : meme numero pour les deux fichiers).
    std::vector<RSMusicSet *> LoadMusicFile(const char *file, const char *datFile = nullptr);
    // La banque 'bank' sert les pistes du jeu 'set' (GetMusic) ; ses transitions si set->hasDat.
    void SetBank(uint8_t bank, RSMusicSet *set);
    // Fichiers d'un appareil : extension des musiques (ADL, ROL, MID), pak des cinematiques,
    // bibliotheque de timbres (vide : aucune), extension et bibliotheque des effets SOUNDFX.
    RSMusic(const std::string &music_ext, const std::string &midgames_pak, const std::string &timbre_file,
            const std::string &sfx_ext, const std::string &sfx_timbre_file);
    ~RSMusic();

    // Configuration de Strike Commander : fichiers, .dat et banques (voir RSMusic.cpp).
    void init();
    void SwitchBank(uint8_t bank);
    MemMusic *GetMusic(uint32_t index);
    // Transitions (.dat) de la banque courante, nullptr si elle n'en a pas
    SCMusicSet *GetMusicSet();
};
