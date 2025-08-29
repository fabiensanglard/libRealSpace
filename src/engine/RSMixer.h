//
//  RSMixer.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "../realspace/AssetManager.h"
#include "../realspace/RSMusic.h"
#include <SDL2/SDL_mixer_ext.h>

class RSMixer {
    int initted;
    bool isplaying;
    uint32_t current_music{UINT32_MAX};
    int channel{0};
    uint8_t *voc_data{nullptr};
public:
    RSMixer();
    ~RSMixer();
    void init(AssetManager *amana);
    void PlayMusic(uint32_t index, int loop=1);
    void SwitchBank(uint8_t bank);
    void StopMusic();
    uint32_t GetMusicID() { return this->current_music; };
    void StopSound();
    void StopSound(int chanl);
    void PlaySoundVoc(uint8_t *data, size_t vocSize);
    void PlaySoundVoc(uint8_t *data, size_t vocSize, int channel, int loop=0);
    void setVolume(int volume, int channel = -1);
    bool IsSoundPlaying();
    bool IsSoundPlaying(int chanl);
    RSMusic *music;
};