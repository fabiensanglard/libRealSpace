//
//  SCAnimationPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include "SCAnimationPlayer.h"


SCAnimationPlayer::SCAnimationPlayer(){
    
}

SCAnimationPlayer::~SCAnimationPlayer(){
}

void SCAnimationPlayer::CheckKeyboard(void) {
    SDL_Event mouseEvents[5];
    int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP);
    for (int i = 0; i < numMouseEvents; i++) {
        SDL_Event *event = &mouseEvents[i];

        switch (event->type) {
        case SDL_MOUSEBUTTONUP:
            Game.StopTopActivity();
            break;
        default:
            break;
        }
    }
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        case SDLK_SPACE:
            this->current_img = this->current_img+1;
            printf("Current image: %d\n", this->current_img);
            _flushall();
            if (this->current_img > this->midgames->GetNumEntries()-1) {
                this->current_img = 0;
            }
            break;
        case SDLK_p:
            this->palette_id = this->palette_id+1;
            printf("Current palette: %d\n", this->palette_id);
            if (this->palette_id > this->midgames->GetNumEntries()-1) {
                this->palette_id = 0;
            }
            _flushall();
            break;
        case SDLK_o:
            this->image_offset += 1;
            if (this->image_offset > 9) {
                this->image_offset = 0;
            }
            printf("Current image offset: %d\n", this->image_offset);
            _flushall();
            break;
        case SDLK_a:
            {
                TreEntry *midgames_entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\MIDGAMES\\MIDGAMES.PAK");
                PakArchive *midgames_arch = new PakArchive();
                midgames_arch->InitFromRAM("MIDGAMES.PAK", midgames_entry->data, midgames_entry->size);
                this->midgames = midgames_arch;
                this->palette_id = 0;
                this->current_img = 0;
            }
        break;
        case SDLK_z:
            {
                TreEntry *midgames_entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\MIDGAMES\\MID1.PAK");
                PakArchive *midgames_arch = new PakArchive();
                midgames_arch->InitFromRAM("MID1.PAK", midgames_entry->data, midgames_entry->size);
                this->midgames = midgames_arch;
                this->palette_id = 0;
                this->current_img = 0;
            }
        break;
        case SDLK_e:
            {
                TreEntry *midgames_entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\MIDGAMES\\MID2.PAK");
                PakArchive *midgames_arch = new PakArchive();
                midgames_arch->InitFromRAM("MID2.PAK", midgames_entry->data, midgames_entry->size);
                this->midgames = midgames_arch;
                this->palette_id = 0;
                this->current_img = 0;
            }
        break;
        default:
            break;
        }
    }
}
void SCAnimationPlayer::Init(){
    TreEntry *midgames_entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\MIDGAMES\\MIDGAMES.PAK");
    PakArchive *midgames_arch = new PakArchive();
    midgames_arch->InitFromRAM("MIDGAMES.PAK", midgames_entry->data, midgames_entry->size);
    this->midgames = midgames_arch;
}

void SCAnimationPlayer::RunFrame(void){
    static int fps_counter = 0;
    static int sequence_counter = 0;
    CheckKeyboard();
    VGA.Activate();
    VGA.Clear();
    VGA.FillWithColor(0);
    RSImageSet *img = new RSImageSet();
    img->InitFromPakEntry(midgames->GetEntry(this->current_img));
    if (img->palettes.size() > 0) {
        VGA.SetPalette(img->palettes[0]->GetColorPalette());
    } else {
        img->InitFromPakEntry(midgames->GetEntry(this->palette_id));
        if (img->palettes.size() > 0) {
            VGA.SetPalette(img->palettes[0]->GetColorPalette());
        }
        img = nullptr;
    }
    if (img != nullptr && sequence_counter >= img->sequence.size()) {
        sequence_counter = 0;
    }
    if (img != nullptr && img->sequence.size() > 0) {
        VGA.DrawShape(img->GetShape(img->sequence[0]));
        VGA.DrawShape(img->GetShape(img->sequence[sequence_counter]));
    } else {
        PakArchive *test = new PakArchive();
        PakEntry *entry = midgames->GetEntry(this->current_img);
        test->InitFromRAM("test", entry->data, entry->size);
        if (test->GetNumEntries() != 0) {
            RLEShape *shape = new RLEShape();
            shape->Init(test->GetEntry(0)->data, test->GetEntry(0)->size);
            bool error = VGA.DrawShape(shape);
            if (error) {
                printf("Error: %d\n", error);
                _flushall();
            }
            
        } else {
            RLEShape *shape = new RLEShape();
            shape->Init(entry->data, entry->size);
            bool error = VGA.DrawShape(shape);
            if (error) {
                printf("Error: %d\n", error);
                _flushall();
            }
        }
        
    }
    //VGA.DrawShape(img->GetShape(img->sequence[sequence_counter]));
    if (fps_counter % 10 == 0) {
        sequence_counter++;
    }
    fps_counter++;
    Mouse.Draw();
    VGA.VSync();

}