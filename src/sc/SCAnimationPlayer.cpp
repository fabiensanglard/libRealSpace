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

    MIDGAME_DATA mid1Data = {
        {
            {   // Shot 1
                {   // Background
                    { &this->optShps, OptionShapeID::SKY, 255, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 20, 255, 0, {0,-32}, {0,0}, {0,0} }
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                }, 
                {   // Forground
                    { this->mid[0], 3, 255, 0, {0,-32}, {0,0}, {0,0} }
                },
                15
            },
            {   // Shot 2
                {   // Background
                    { &this->optShps, OptionShapeID::SKY, 255, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 20, 255, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 19, 0, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->optShps, OptionShapeID::MOUTAINS_BG, 255, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 19, 1, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 19, 2, 0, {0,-32}, {0,0}, {0,0} }
                },
                {   // Middle (vide)
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                },
                {   // Forground (vide)
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                },
                15
            },
            {   // Shot 3
                {   // Background
                    { &this->optShps, SKY, 255, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 20, 255, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 19, 0, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->optShps, MOUTAINS_BG, 255, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 19, 1, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 19, 2, 0, {0,-32}, {0,0}, {0,0} }
                },
                {   // Middle (vide)
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                },
                {   // Forground
                    { this->mid[0], 13, 255, 0, {0,-32}, {0,0}, {0,0} }
                },
                15
            }
        }
    };

    for (auto sht: mid1Data.shots) {
        MIDGAME_SHOT *shot = new MIDGAME_SHOT();
    
        for (auto shot_bg: sht.background) {
            MIDGAME_SHOT_BG *bg = new MIDGAME_SHOT_BG();
            RSImageSet *tmp_img = new RSImageSet();
            PakArchive *pk = new PakArchive();
            PakEntry *pe = shot_bg.pak->GetEntry(shot_bg.shape_id);
            pk->InitFromRAM("toto", pe->data, pe->size);
            tmp_img->InitFromPakArchive(pk,0);
            bg->image = tmp_img;
            bg->position_start = shot_bg.start;
            bg->position_end = shot_bg.end;
            bg->velocity = shot_bg.velocity;
            shot->background.push_back(bg);
        }
        if (sht.forground.size()>0 && sht.forground[0].pak != nullptr) {
            RSImageSet *tmp_img = new RSImageSet();
            tmp_img->InitFromPakEntry(sht.forground[0].pak->GetEntry(sht.forground[0].shape_id));
            shot->foreground = tmp_img;
        }
        this->midgames_shots[1].push_back(shot);
    }


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
    FrameBuffer *fb = VGA.GetFrameBuffer();
    fb->FillWithColor(0);
    

    MIDGAME_SHOT *shot = this->midgames_shots[1][shot_counter];

    for (auto bg : shot->background) {
        if (bg->image->GetNumImages()>0) {
            for (auto shp: bg->image->shapes) {
                FrameBuffer *texture = new FrameBuffer(320, 200);
                texture->FillWithColor(255);
                texture->DrawShape(shp);
                fb->blitWithMask(texture->framebuffer, bg->position_start.x, bg->position_start.y, 320, 200,255);
            }
        }
    }
    if (shot->foreground != nullptr) {
        FrameBuffer *texture = new FrameBuffer(320, 200);
        texture->FillWithColor(255);
        texture->DrawShape(shot->foreground->GetShape(fps));
        fb->blitWithMask(texture->framebuffer, 0, 0, 320, 200,255);
    }

    //VGA.GetFrameBuffer()->DrawShape(shot->foreground->GetShape(0));
    fps_counter++;
    if (fps_counter%5==0) {
        fps++;
        if (shot->foreground==nullptr || fps > shot->foreground->GetNumImages()-1) {
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