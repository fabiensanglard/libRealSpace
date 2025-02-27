#include "precomp.h"

RSMixer::RSMixer() {
    
}
void RSMixer::Init() {
    this->initted = Mix_Init(MIX_INIT_MID);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    this->music = new RSMusic();
    this->music->Init();
}
RSMixer::~RSMixer() { Mix_Quit(); }

void RSMixer::PlayMusic(uint32_t index) {
    MemMusic *mus = this->music->GetMusic(index);
    SDL_RWops *rw = SDL_RWFromConstMem(mus->data, (int) mus->size);
    //Mix_ADLMIDI_setCustomBankFile("G:\\DOS\\DATA\\SOUND\\STRIKE.AD");
    Mix_Music *music = Mix_LoadMUSType_RW(rw, MUS_MID, 1);
    int error = Mix_PlayMusic(music, 1);
    if (error == -1) {
        printf("Error playing music: %s\n", Mix_GetError());
    }
}
void RSMixer::SwitchBank(uint8_t bank) { this->music->SwitchBank(bank); }
void RSMixer::StopMusic() { 
    Mix_HaltMusic();
}