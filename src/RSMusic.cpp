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
        musics.push_back(music);
    }
}

MemMusic *RSMusic::GetMusic(uint32_t index) { return musics[index]; }