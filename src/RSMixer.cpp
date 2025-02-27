#include "precomp.h"
#include "RSMixer.h"

RSMixer::RSMixer() {
    
}
void RSMixer::Init() {
    this->initted = Mix_Init(MIX_INIT_MID);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    this->music = new RSMusic();
    this->music->Init();
    this->isplaying = false;
    
}
RSMixer::~RSMixer() { Mix_Quit(); }

void RSMixer::PlayMusic(uint32_t index, int loop) {
    if (this->music->bank == 1 && index == 0) {
        Mix_ADLMIDI_setSetDefaults();
    } else {
        Mix_ADLMIDI_setCustomBankFile("./assets/STRIKE.wopl");
    }
    MemMusic *mus = this->music->GetMusic(index);
    SDL_RWops *rw = SDL_RWFromConstMem(mus->data, (int) mus->size);
    Mix_Music *music = Mix_LoadMUSType_RW(rw, MUS_ADLMIDI, 1);
    
    int error = Mix_PlayMusic(music, loop);
    this->current_music = index;
    if (error == -1) {
        printf("Error playing music: %s\n", Mix_GetError());
    }
}
void RSMixer::SwitchBank(uint8_t bank) { this->music->SwitchBank(bank); }
void RSMixer::StopMusic() { 
    Mix_HaltMusic();
    this->isplaying = false;
}