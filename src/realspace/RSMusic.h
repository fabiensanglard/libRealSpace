//
//  RSMusic.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "SDL_mixer_ext.h"
#include "precomp.h"
#include <stdint.h>
#include <vector>

struct MemMusic {
    uint8_t *data;
    size_t size;
};

class RSMusic {
private:
    std::vector<MemMusic *> gameflow_music;
public:
    uint8_t bank{0};
    std::map<uint8_t, std::vector<MemMusic *>> musics;
    void init(AssetManager *assetManager);
    void SwitchBank(uint8_t bank);
    MemMusic *GetMusic(uint32_t index);
};
