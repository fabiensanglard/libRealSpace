//
//  RSMusic.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "precomp.h"
#include <vector>
#include <stdint.h>
#include "SDL_mixer_ext.h"


class RSMusic{
    std::vector<uint8_t *> musics;

public:
    void Init();
    void PlayMusic(int id);    
};

