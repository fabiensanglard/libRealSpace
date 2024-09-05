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
    std::vector<MemMusic *> musics;

public:
    void Init();
    MemMusic *GetMusic(uint32_t index);
};
