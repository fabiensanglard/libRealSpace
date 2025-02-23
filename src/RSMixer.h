//
//  RSMixer.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <SDL2/SDL_mixer_ext.h>

class RSMixer {
private:
    int initted;
public:
    RSMixer();
    ~RSMixer();
    void Init();
    void PlayMusic(uint32_t index);
    void SwitchBank(uint8_t bank);
    void StopMusic();
    RSMusic *music;
};