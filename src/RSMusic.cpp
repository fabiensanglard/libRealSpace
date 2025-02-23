//
//  RSMusic.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
void RSMusic::Init() {
    TreArchive *tre = new TreArchive();
    tre->InitFromFile("GAMEFLOW.TRE");
    PakArchive *pak = new PakArchive();
    TreEntry *entry = tre->GetEntryByName("..\\..\\DATA\\MIDGAMES\\AMUSIC.PAK");
    pak->InitFromRAM("..\\..\\DATA\\MIDGAMES\\AMUSIC.PAK", entry->data, entry->size);

    for (size_t i = 0; i < pak->GetNumEntries(); i++) {
        PakEntry *e = pak->GetEntry(i);
        uint8_t *data = new uint8_t[e->size];
        memcpy(data, e->data, e->size);
        MemMusic *music = new MemMusic();
        music->data = data;
        music->size = e->size;
        musics[0].push_back(music);
    }

    tre->InitFromFile("SOUND.TRE");
    TreEntry *gameflow = tre->GetEntryByName("..\\..\\DATA\\SOUND\\GAMEFLOW.ADL");
    pak = new PakArchive();
    pak->InitFromRAM("..\\..\\DATA\\SOUND\\GAMEFLOW.ADL", gameflow->data, gameflow->size);
    for (size_t i = 1; i < pak->GetNumEntries(); i++) {
        PakEntry *e = pak->GetEntry(i);
        PakArchive *subpak = new PakArchive();
        subpak->InitFromRAM("..\\..\\DATA\\SOUND\\GAMEFLOW.ADL", e->data, e->size);
        for (size_t j = 0; j < subpak->GetNumEntries(); j++) {
            PakEntry *sub = subpak->GetEntry(j);
            uint8_t *data = new uint8_t[sub->size];
            memcpy(data, sub->data, sub->size);
            MemMusic *music = new MemMusic();
            music->data = data;
            music->size = sub->size;
            musics[1].push_back(music);
        }
    }

    TreEntry *combat = tre->GetEntryByName("..\\..\\DATA\\SOUND\\COMBAT.ADL");
    pak = new PakArchive();
    pak->InitFromRAM("..\\..\\DATA\\SOUND\\COMBAT.ADL", combat->data, combat->size);
    for (size_t i = 0; i < pak->GetNumEntries(); i++) {
        PakEntry *e = pak->GetEntry(i);
        PakArchive *subpak = new PakArchive();
        subpak->InitFromRAM("..\\..\\DATA\\SOUND\\COMBAT.ADL", e->data, e->size);
        for (size_t j = 0; j < subpak->GetNumEntries(); j++) {
            PakEntry *sub = subpak->GetEntry(j);
            PakArchive *subsubpak = new PakArchive();
            subsubpak->InitFromRAM("..\\..\\DATA\\SOUND\\COMBAT.ADL", sub->data, sub->size);
            for (size_t k = 0; k < subsubpak->GetNumEntries(); k++) {
                PakEntry *subsub = subpak->GetEntry(k);
                if (subsub != NULL) {
                    uint8_t *data = new uint8_t[subsub->size];
                    memcpy(data, subsub->data, subsub->size);
                    MemMusic *music = new MemMusic();
                    music->data = data;
                    music->size = subsub->size;
                    musics[2].push_back(music);    
                }
            }
        }
    }
}

void RSMusic::SwitchBank(uint8_t bank) {
    this->bank = bank;
}
MemMusic *RSMusic::GetMusic(uint32_t index) { return musics[bank][index]; }