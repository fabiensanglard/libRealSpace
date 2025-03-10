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
        default:
            break;
        }
    }
}
void SCAnimationPlayer::Init(){
    TreEntry *midgames_entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(
        "..\\..\\DATA\\MIDGAMES\\MIDGAMES.PAK"
    );
    PakArchive *midgames_arch = new PakArchive();
    this->midgames.InitFromRAM("MIDGAMES.PAK", midgames_entry->data, midgames_entry->size);
    std::vector<std::string> midgames_files = {
        "MID1",
        "MID2",
        "MID3",
        "MID5",
        "MID12",
        "MID14",
        "MID15",
        "MID16",
        "MID17",
        "MID20",
        "MID36"
    };
    for (int i = 0; i < midgames_files.size(); i++) {
        std::string file_path = "..\\..\\DATA\\MIDGAMES\\" + midgames_files[i] + ".PAK";
        TreEntry *entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(file_path.c_str());
        PakArchive *arch = new PakArchive();
        arch->InitFromRAM(midgames_files[i].c_str(), entry->data, entry->size);
        this->mid.push_back(arch);
    }
    TreEntry *optShapEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(
        "..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK"
    );
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry *optPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(
        "..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK"
    );
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);

    MIDGAME_SHOT *shot = new MIDGAME_SHOT();
    RSImageSet *tmp_img = new RSImageSet();
    PakArchive *pk = new PakArchive();
    pk->InitFromRAM("toto", this->optShps.GetEntry(OptionShapeID::SKY)->data, this->optShps.GetEntry(OptionShapeID::SKY)->size);
    tmp_img->InitFromPakArchive(pk,0);
    shot->background.push_back(tmp_img);

    
    pk = new PakArchive();
    tmp_img = new RSImageSet();
    pk->InitFromRAM("toto", this->midgames.GetEntry(20)->data, this->midgames.GetEntry(20)->size);
    tmp_img->InitFromPakArchive(pk,0);
    shot->background.push_back(tmp_img);

    pk = new PakArchive();
    tmp_img = new RSImageSet();
    pk->InitFromRAM("toto", this->mid[0]->GetEntry(3)->data, this->mid[0]->GetEntry(3)->size);
    tmp_img->InitFromPakEntry(this->mid[0]->GetEntry(3));
    shot->foreground = tmp_img;
    this->midgames_shots[1].push_back(shot);

    shot = new MIDGAME_SHOT();
    tmp_img = new RSImageSet();
    pk = new PakArchive();
    pk->InitFromRAM("toto", this->optShps.GetEntry(OptionShapeID::SKY)->data, this->optShps.GetEntry(OptionShapeID::SKY)->size);
    tmp_img->InitFromPakArchive(pk,0);
    shot->background.push_back(tmp_img);

    
    pk = new PakArchive();
    tmp_img = new RSImageSet();
    pk->InitFromRAM("toto", this->midgames.GetEntry(20)->data, this->midgames.GetEntry(20)->size);
    tmp_img->InitFromPakArchive(pk,0);
    shot->background.push_back(tmp_img);


    /*tmp_img = new RSImageSet();
    pk = new PakArchive();
    pk->InitFromRAM("toto", this->optShps.GetEntry(OptionShapeID::MOUTAINS_BG)->data, this->optShps.GetEntry(OptionShapeID::MOUTAINS_BG)->size);
    tmp_img->InitFromPakArchive(pk,0);
    shot->background.push_back(tmp_img);*/

    
    tmp_img = new RSImageSet();
    tmp_img->InitFromPakEntry(this->midgames.GetEntry(19));
    shot->background.push_back(tmp_img);


    pk = new PakArchive();
    tmp_img = new RSImageSet();
    pk->InitFromRAM("toto", this->mid[0]->GetEntry(13)->data, this->mid[0]->GetEntry(3)->size);
    tmp_img->InitFromPakEntry(this->mid[0]->GetEntry(13));
    shot->foreground = tmp_img;

    this->midgames_shots[1].push_back(shot);

    VGAPalette *rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
    ByteStream paletteReader;
    paletteReader.Set(this->optPals.GetEntry(OPTPALS_PAK_SKY_PALETTE_PATCH_ID)->data); // Sky Good but not mountains
    this->palette.ReadPatch(&paletteReader);
}

void SCAnimationPlayer::RunFrame(void){
    static int fps_counter = 0;
    static int fps = 0;
    static int shot_counter = 0;
    CheckKeyboard();
    VGA.Activate();
    VGA.GetFrameBuffer()->Clear();
    VGA.SetPalette(&this->palette);
    VGA.GetFrameBuffer()->FillWithColor(0);

    MIDGAME_SHOT *shot = this->midgames_shots[1][shot_counter];

    for (auto bg : shot->background) {
        if (bg->GetNumImages()>0) {
            for (auto shp: bg->shapes) {
                VGA.GetFrameBuffer()->DrawShape(shp);
            }
        }
    }
    if (shot->foreground != nullptr) {
        VGA.GetFrameBuffer()->DrawShape(shot->foreground->GetShape(fps));
    }

    //VGA.GetFrameBuffer()->DrawShape(shot->foreground->GetShape(0));
    fps_counter++;
    if (fps_counter%5==0) {
        fps++;
        if (fps > shot->foreground->GetNumImages()-1) {
            fps = 0;
            shot_counter++;
            if (shot_counter>this->midgames_shots.size()) {
                Game.StopTopActivity();
            }
        }
    } 
    Mouse.Draw();
    VGA.VSync();
}