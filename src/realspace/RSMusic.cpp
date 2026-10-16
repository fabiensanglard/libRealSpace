//
//  RSMusic.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

static bool isForm(const uint8_t *data, size_t size) {
    return data != NULL && size >= 4 && memcmp(data, "FORM", 4) == 0;
}

static bool isForm(const PakEntry *e) {
    return e != NULL && isForm(e->data, e->size);
}

// Configuration de Strike Commander. Pour donner des transitions a un fichier, il suffit de
// passer son .dat a LoadMusicFile.
void RSMusic::init() {
    LoadTimbres("..\\..\\DATA\\SOUND\\STRIKE.AD");



    if (midiMusic == 2) {
        if (TreEntry *mt = assetManager.GetEntryByName("..\\..\\DATA\\SOUND\\STRIKE.MT")) {
            mt_data.assign(mt->data, mt->data + mt->size);
            timbresMt.set(mt_data.data(), mt_data.size());
        } else {
            printf("RSMusic: Could not find STRIKE.MT\n");
        }
    }

    static const char *const exts[] = { "ADL", "MID", "ROL" };
    static const char *const midPaks[] = { "AMUSIC.PAK", "MMUSIC.PAK", "RMUSIC.PAK" };
    static const char *const names[] = { "AdLib", "General MIDI", "Roland MT-32" };
    const int src = (midiMusic >= 0 && midiMusic <= 2) ? midiMusic : 0;
    const char *ext = exts[src];
    const std::string sound = "..\\..\\DATA\\SOUND\\";
    const std::string mid = "..\\..\\DATA\\MIDGAMES\\";
    printf("RSMusic: music source %s (.%s / %s)\n", names[src], ext, midPaks[src]);

    // banque 0 : AMUSIC.PAK / MMUSIC.PAK / RMUSIC.PAK (pistes a plat, pas de .dat)
    std::vector<RSMusicSet *> amusic = LoadMusicFile((mid + midPaks[src]).c_str());

    if (!amusic.empty()) {
        SetBank(0, amusic[0]);
        midgames_musics[0] = amusic[0]->tracks;
    }

    // banque 1 : GAMEFLOW.ADL / GAMEFLOW.MID
    std::vector<RSMusicSet *> gameflow = LoadMusicFile((sound + "GAMEFLOW." + ext).c_str());
    for (size_t i = 0; i < gameflow.size(); i++) {
        if (i > 0) {
            gameflow_musics[i] = gameflow_musics[0];
        }
        for (MemMusic *m : gameflow[i]->tracks) {
            musics[1].push_back(m);
            gameflow_musics[i].push_back(m);
        }
    }

    // banque 2 : COMBAT.ADL / COMBAT.MID, Transitionen aus COMBAT.DAT (fuer alle Varianten gleich)
    std::vector<RSMusicSet *> combat = LoadMusicFile((sound + "COMBAT." + ext).c_str(), (sound + "COMBAT.DAT").c_str());
    for (size_t i = 0; i < combat.size(); i++) {
        combat_musics[i] = combat[i]->tracks;
    }
    if (!combat.empty()) {
        SetBank(2, combat[0]);
    }

    // Effekte: immer ADL (es gibt keine GM-Variante), laufen weiter ueber OPL
    std::vector<RSMusicSet *> soundfx = LoadMusicFile("..\\..\\DATA\\SOUND\\SOUNDFX.ADL");
    for (size_t i = 0; i < soundfx.size(); i++) {
        soundfx_musics[i] = soundfx[i]->tracks;
    }
}

bool RSMusic::LoadTimbres(const char *file) {
    TreEntry *lib = assetManager.GetEntryByName(file);
    if (lib == NULL) {
        // Sans bibliotheque, les sequences s'enchainent mais aucune note ne sonne.
        printf("RSMusic: Could not find %s (no timbre bank: music will be silent)\n", file);
        return false;
    }
    timbre_data.assign(lib->data, lib->data + lib->size);
    timbres.set(timbre_data.data(), timbre_data.size());
    return true;
}

RSMusic::~RSMusic() {
    for (std::unique_ptr<RSMusicSet> &set : owned) {
        for (MemMusic *m : set->tracks) {
            delete m;
        }
    }
}

RSMusicSet *RSMusic::newSet() {
    owned.push_back(std::unique_ptr<RSMusicSet>(new RSMusicSet()));
    return owned.back().get();
}

// Ajoute a out chaque sequence XMIDI (entree commencant par "FORM") trouvee dans pak,
// en descendant dans les sous-archives.
void RSMusic::collectTracks(PakArchive *pak, const char *name, std::vector<std::vector<uint8_t>> &out, int depth) {
    for (size_t j = 0; j < pak->GetNumEntries(); j++) {
        PakEntry *sub = pak->GetEntry(j);
        if (sub == NULL || sub->size == 0) {
            continue;
        }
        if (isForm(sub)) {
            out.emplace_back(sub->data, sub->data + sub->size);
        } else if (depth < 4) {
            PakArchive *subpak = new PakArchive();
            subpak->InitFromRAM(name, sub->data, sub->size);
            collectTracks(subpak, name, out, depth + 1);
        }
    }
}

// Structure de COMBAT.ADL (verifiee sur le vrai fichier) : [0] archive des pistes de liaison,
// [1..N] pistes principales (AudioQueue_LoadTrackTable_AACA6, AudioQueue_LoadTransitionTable_AAFA0).
// Renvoie false si le jeu n'a pas cette structure.
bool RSMusic::loadSetWithLinks(PakArchive *setpak, const char *name, RSMusicSet *set) {
    size_t n = setpak->GetNumEntries();
    if (n < 2 || isForm(setpak->GetEntry(0))) {
        return false;
    }
    for (size_t j = 1; j < n; j++) {
        if (!isForm(setpak->GetEntry(j))) {
            return false;
        }
    }
    for (size_t j = 1; j < n; j++) {
        PakEntry *t = setpak->GetEntry(j);
        set->data.tracks.emplace_back(t->data, t->data + t->size);
    }
    PakEntry *l = setpak->GetEntry(0);
    PakArchive *linkpak = new PakArchive();
    linkpak->InitFromRAM(name, l->data, l->size);
    for (size_t k = 0; k < linkpak->GetNumEntries(); k++) {
        PakEntry *lt = linkpak->GetEntry(k);
        if (lt == NULL) {
            set->data.linkTracks.emplace_back();
        } else {
            set->data.linkTracks.emplace_back(lt->data, lt->data + lt->size);
        }
    }
    return true;
}

std::vector<RSMusicSet *> RSMusic::LoadMusicFile(const char *file, const char *datFile) {
    std::vector<RSMusicSet *> result;
    TreEntry *entry = assetManager.GetEntryByName(file);
    if (entry == NULL) {
        printf("RSMusic: Could not find %s\n", file);
        return result;
    }
    PakArchive *datpak = NULL;
    if (datFile != NULL) {
        TreEntry *dat = assetManager.GetEntryByName(datFile);
        if (dat == NULL) {
            printf("RSMusic: Could not find %s (no transitions for %s)\n", datFile, file);
        } else {
            datpak = new PakArchive();
            datpak->InitFromRAM(datFile, dat->data, dat->size);
        }
    }
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM(file, entry->data, entry->size);

    if (pak->GetNumEntries() > 0 && isForm(pak->GetEntry(0))) {
        // pistes a plat : un seul jeu, une piste par entree (les numeros restent ceux du fichier)
        RSMusicSet *set = newSet();
        for (size_t i = 0; i < pak->GetNumEntries(); i++) {
            PakEntry *e = pak->GetEntry(i);
            if (e == NULL) {
                set->data.tracks.emplace_back();
            } else {
                set->data.tracks.emplace_back(e->data, e->data + e->size);
            }
        }
        result.push_back(set);
    } else {
        for (size_t i = 0; i < pak->GetNumEntries(); i++) {
            PakEntry *e = pak->GetEntry(i);
            RSMusicSet *set = newSet();
            result.push_back(set);
            if (e == NULL || e->size == 0) {
                continue;
            }
            PakArchive *setpak = new PakArchive();
            setpak->InitFromRAM(file, e->data, e->size);
            bool withLinks = loadSetWithLinks(setpak, file, set);
            if (!withLinks) {
                collectTracks(setpak, file, set->data.tracks, 0);
            }
            if (datpak == NULL) {
                continue;
            }
            PakEntry *rec = datpak->GetEntry(i);
            if (!withLinks || rec == NULL || !set->data.parseDat(rec->data, rec->size)) {
                printf("RSMusic: %s set %zu: no usable transitions in %s\n", file, i, datFile);
                continue;
            }
            if ((int)set->data.tracks.size() < set->data.trackCount) {
                printf("RSMusic: %s set %zu: %zu tracks, %s expects %d\n", file, i, set->data.tracks.size(),
                       datFile, set->data.trackCount);
            }
            set->hasDat = true;
        }
    }
    // MemMusic : vues sur les pistes du jeu (data.tracks n'est plus modifie)
    for (RSMusicSet *set : result) {
        for (std::vector<uint8_t> &t : set->data.tracks) {
            MemMusic *music = new MemMusic();
            music->data = t.data();
            music->size = t.size();
            set->tracks.push_back(music);
        }
    }
    music_files[file] = result;
    return result;
}

void RSMusic::SetBank(uint8_t bank, RSMusicSet *set) {
    if (set == NULL) {
        bank_sets.erase(bank);
        musics.erase(bank);
        return;
    }
    bank_sets[bank] = set;
    musics[bank] = set->tracks;
}

void RSMusic::SwitchBank(uint8_t bank) {
    if (this->bank == bank) {
        return;
    }
    this->bank = bank;
}

MemMusic *RSMusic::GetMusic(uint32_t index) {
    if (musics.find(bank) == musics.end()) {
        return NULL;
    }
    if (index >= musics[bank].size()) {
        return NULL;
    }
    return musics[bank][index];
}

SCMusicSet *RSMusic::GetMusicSet() {
    auto it = bank_sets.find(bank);
    if (it == bank_sets.end() || !it->second->hasDat) {
        return NULL;
    }
    return &it->second->data;
}
