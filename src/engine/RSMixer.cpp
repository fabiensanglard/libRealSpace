#include "RSMixer.h"

RSMixer::RSMixer() {
    
}
void RSMixer::init(AssetManager *amana) {
    this->initted = Mix_Init(MIX_INIT_MID);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    this->music = new RSMusic();
    this->music->init(amana);
    this->isplaying = false;
    
}
RSMixer::~RSMixer() { Mix_Quit(); }

void RSMixer::PlayMusic(uint32_t index, int loop) {
    if (this->current_music == index) {
        return;
    }
    Mix_ADLMIDI_setCustomBankFile("./assets/STRIKE.wopl");
    MemMusic *mus = this->music->GetMusic(index);
    SDL_RWops *rw = SDL_RWFromConstMem(mus->data, (int) mus->size);
    Mix_Music *music = Mix_LoadMUSType_RW(rw, MUS_ADLMIDI, 1);
    this->isplaying = true;
    int error = Mix_PlayMusic(music, loop);
    this->current_music = index;
    if (error == -1) {
        printf("Error playing music: %s\n", Mix_GetError());
    }
}
void RSMixer::SwitchBank(uint8_t bank) { 
    this->music->SwitchBank(bank);
}
void RSMixer::StopMusic() { 
    Mix_HaltMusic();
    this->isplaying = false;
}
void RSMixer::PlaySoundVoc(uint8_t *data, size_t vocSize) {
    if (voc_data == data) {
        return;
    }
    voc_data = data;
    SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(vocSize));
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1);
    if (!chunk) {
        printf("Error loading VOC sound: %s\n", Mix_GetError());
        return;
    }
    channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel == -1) {
        printf("Error playing VOC sound: %s\n", Mix_GetError());
    }
}
void RSMixer::PlaySoundVoc(uint8_t *data, size_t vocSize, int channel, int loop) {
    if (voc_data == data) {
        return;
    }
    voc_data = data;
    SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(vocSize));
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1);
    if (!chunk) {
        printf("Error loading VOC sound: %s\n", Mix_GetError());
        return;
    }
    // Stop any sound currently playing on this channel
    Mix_HaltChannel(channel);
    int result = Mix_PlayChannel(channel, chunk, loop);
    if (result == -1) {
        printf("Error playing VOC sound on channel %d: %s\n", channel, Mix_GetError());
    }
}
void RSMixer::StopSound() { Mix_HaltChannel(channel); };
void RSMixer::StopSound(int chanl) { Mix_HaltChannel(chanl); };
bool RSMixer::IsSoundPlaying() {
    return Mix_Playing(channel) != 0;
}
void RSMixer::setVolume(int volume, int channel) {
    if (channel == -1) {
        Mix_VolumeMusic(volume);
    } else {
        Mix_Volume(channel, volume);
    }
};