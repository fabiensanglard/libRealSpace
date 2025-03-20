//
//  SCAnimationPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include "SCAnimationPlayer.h"
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

SCAnimationPlayer::SCAnimationPlayer(){
    this->fps_timer = SDL_GetTicks() / 10;
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
    if (this->mid.size() == 0) {
        for (int i = 0; i < midgames_files.size(); i++) {
            std::string file_path = "..\\..\\DATA\\MIDGAMES\\" + midgames_files[i] + ".PAK";
            TreEntry *entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(file_path.c_str());
            if (entry == nullptr) {
                continue;
            }
            PakArchive *arch = new PakArchive();
            arch->InitFromRAM(midgames_files[i].c_str(), entry->data, entry->size);
            this->mid.push_back(arch);
        }
    }
    if (this->midvoc.size() == 0) {

        for (int i = 0; i < midgames_files.size(); i++) {
            std::string file_path = "..\\..\\DATA\\MIDGAMES\\" + midgames_files[i] + "VOC.PAK";
            TreEntry *entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(file_path.c_str());
            if (entry == nullptr) {
                continue;
            }
            PakArchive *arch = new PakArchive();
            arch->InitFromRAM(midgames_files[i].c_str(), entry->data, entry->size);
            this->midvoc.push_back(arch);
        }
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
            {
                {
                    { &this->optShps, OptionShapeID::SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0},
                    { &this->midgames, 20, 0, 0, {0,0}, {0,0}, {0,0} }
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                }, 
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                90,
                0
            },
            {
                {
                    { &this->optShps, OptionShapeID::SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,0}, {0,0}, {0,0} ,0 }
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                }, 
                {
                    { this->mid[0], 3, 0, 0, {0,32}, {0,0}, {0,0} ,0 }
                },
                36,
                255,
                this->midvoc[0]->GetEntry(7)
            },
            {
                {
                    { &this->optShps, OptionShapeID::SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,0}, {0,0}, {0,0} ,0 }
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                }, 
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                100,
            },
            {
                {
                    { &this->optShps, OptionShapeID::SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,-32}, {0,-1} ,0},
                    { &this->midgames, 20, 0, 0, {0,0}, {0,-32}, {0,-1} ,0 },
                    { &this->midgames, 19, 1, 0, {0,151}, {0,119}, {0,-1} ,0 },
                    { &this->optShps, OptionShapeID::MOUTAINS_BG, 0, 0, {0,16}, {0,-16}, {0,-1} ,0 },
                    { &this->midgames, 19, 2, 0, {0,183}, {0,119}, {0,-2} ,0 },
                    { &this->midgames, 19, 3, 0, {0,186}, {0,90}, {0,-3} ,0 }
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                32
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 13, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                109,
                255,
                this->midvoc[0]->GetEntry(8),
                60,
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 20, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                39,
                255,
                this->midvoc[0]->GetEntry(9)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 20, 0, 0, {0,-10}, {0,0}, {0,0}  ,0}
                },
                39,
                255,
                this->midvoc[0]->GetEntry(9)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 3, 0, {0,-15}, {0,0}, {0,0} ,0 },
                    { this->mid[0],1, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0],1, 4, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                48
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 22, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                50,
                255,
                this->midvoc[0]->GetEntry(8)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 23, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                15,
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 23, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                39,
                255,
                this->midvoc[0]->GetEntry(9)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 4, 0, {0,-25}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 0, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                32,
                255,
                this->midvoc[0]->GetEntry(0)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 2, 0, {0,-25}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                25,
                255,
                this->midvoc[0]->GetEntry(4)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 20, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                39,
                255,
                this->midvoc[0]->GetEntry(10)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 16, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                19,
            },
            {
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 24, 0, 0, {0,36}, {0,0}, {0,0} ,1 }
                },
                5
            },
            {
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0},
                    { this->mid[0], 24, 1, 0, {0,36}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 24, 5, 0, {0,36}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                10
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 18, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                19,
                255,
                this->midvoc[0]->GetEntry(11)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 15, 0, 0, {0,32}, {0,0}, {0,0} , 0 }
                },
                23
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 21, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                13,
                255,
                this->midvoc[0]->GetEntry(12)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 29, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                12
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 35, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                13
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 30, 0, 0, {0,-10}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 17, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                49
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 31, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                14
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 32, 0, 0, {0,-10}, {0,0}, {0,0} ,0 }
                },
                15
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 4, 0, {0,-25}, {0,0}, {0,0} ,0 },
                    
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 0, 0, 0, {0,0}, {0,0}, {0,0} ,1 },
                },
                13,
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 4, 0, {0,-25}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 0, 1, 0, {0,0}, {0,0}, {0,0} , 0 },
                    { this->mid[0], 0, 14, 0, {0,0}, {0,0}, {0,0} , 0 },
                    
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                25,
                255,
                this->midvoc[0]->GetEntry(1)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 3, 0, {0,-15}, {0,0}, {0,0} ,0 },
                    { this->mid[0],1, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0],1, 4, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                35
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 2, 0, {0,-25}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {&this->midgames, 0,0,0,{0,0},{0,0},{0,0} ,1}
                },
                9
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 10, 0, 0, {0,32}, {0,0}, {0,0} ,1 }
                },
                19
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,110}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 2, 1, 0, {0,22}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 2, 2, 0, {0,22}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 19, 0, 0, {0,-10}, {0,0}, {0,0} ,1 }
                },
                56
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 4, 0, {0,-25}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 0, 1, 0, {0,0}, {0,0}, {0,0} , 0 },
                    { this->mid[0], 0, 14, 0, {0,0}, {0,0}, {0,0} , 0 },
                    
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                35,
                255,
                this->midvoc[0]->GetEntry(2)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 3, 0, {0,-15}, {0,0}, {0,0} ,0 },
                    { this->mid[0],1, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                25,
                255,
                this->midvoc[0]->GetEntry(5)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 4, 0, {0,-25}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 0, 1, 0, {0,0}, {0,0}, {0,0} , 0 },
                    { this->mid[0], 0, 14, 0, {0,0}, {0,0}, {0,0} , 0 },
                    
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                30,
                255,
                this->midvoc[0]->GetEntry(3)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 6, 3, 0, {0,-15}, {0,0}, {0,0} ,0 },
                    { this->mid[0],1, 1, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                65,
                255,
                this->midvoc[0]->GetEntry(6)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 25, 0, 0, {0,36}, {0,0}, {0,0} ,0 }
                },
                6
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-29}, {0,0}, {0,1} ,0 },
                    { &this->midgames, 20, 0, 0, {0,-19}, {0,10}, {0,1} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,1} ,0 },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,160}, {0,3} ,0 },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,160}, {0,3} ,0 },
                    { this->mid[0], 25, 7, 0, {0,36}, {0,80}, {0,3} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 33, 0, 0, {0,32}, {0,0}, {0,0} ,0 }
                },
                29
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,10}, {0,10}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 34, 0, 0, {0,32}, {0,0}, {0,0} ,0 }
                },
                200
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,10}, {0,10}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 9, 0, 0, {0,16}, {0,0}, {0,0} ,0 }
                },
                19,
                23,
                this->midvoc[0]->GetEntry(12)
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,10}, {0,10}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,0}, {0,0}, {0,0} ,0 },
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    { this->mid[0], 14, 0, 0, {0,16}, {0,0}, {0,0} ,0 }
                },
                10
            },
            {
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} ,0 },
                    { &this->midgames, 20, 0, 0, {0,0}, {0,10}, {0,0} ,0 },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,0}, {0,0}, {0,0} ,0 },
                    { this->mid[0], 14, 11, 0, {0,16}, {0,0}, {0,0} ,0 }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0} ,0}
                },
                64
            },
        }
    };

    for (auto sht: mid1Data.shots) {
        MIDGAME_SHOT *shot = new MIDGAME_SHOT();
    
        for (auto shot_bg: sht.background) {
            if (shot_bg.pak == nullptr) {
                continue;
            }
            MIDGAME_SHOT_BG *bg = new MIDGAME_SHOT_BG();
            RSImageSet *tmp_img = new RSImageSet();
            PakArchive *pk = new PakArchive();
            PakEntry *pe = shot_bg.pak->GetEntry(shot_bg.shape_id);
            pk->InitFromRAM("toto", pe->data, pe->size);
            tmp_img->InitFromPakArchive(pk,0);
            if (tmp_img->GetNumImages() == 0) {
                delete tmp_img;
                tmp_img = new RSImageSet();
                tmp_img->InitFromPakEntry(pe);
            }
            bg->palette = shot_bg.pal_id;
            bg->image = tmp_img;
            if (tmp_img->palettes.size()>0) {
                bg->pal = tmp_img->palettes[0];
            }
            bg->shapeid = shot_bg.sub_shape_id;
            bg->position_start = shot_bg.start;
            bg->position_end = shot_bg.end;
            bg->velocity = shot_bg.velocity;
            shot->background.push_back(bg);
        }
        for (auto sprt: sht.sprites) {
            if (sprt.pak == nullptr) {
                continue;
            }
            MIDGAME_SHOT_SPRITE *sprite = new MIDGAME_SHOT_SPRITE();
            RSImageSet *tmp_img = new RSImageSet();
            tmp_img->InitFromPakEntry(sprt.pak->GetEntry(sprt.shape_id));
            sprite->image = tmp_img;
            if (tmp_img->palettes.size()>0) {
                sprite->pal = tmp_img->palettes[0];
            }
            sprite->position_start = sprt.start;
            sprite->position_end = sprt.end;
            sprite->velocity = sprt.velocity;
            sprite->keep_first_frame = sprt.keep_first_frame;
            shot->sprites.push_back(sprite);
        }

        shot->nbframe = sht.nbframe;
        shot->music = sht.music;
        if (sht.sound != nullptr) {
            MIDGAME_SOUND *sound = new MIDGAME_SOUND();
            sound->data = sht.sound->data;
            sound->size = sht.sound->size;
            shot->sound = sound;
            shot->sound_time_code = sht.sound_time_code;
        }
        this->midgames_shots[1].push_back(shot);
    }


    this->palette = *VGA.GetPalette();
    this->shot_counter = 0;
    this->fps_counter = 0;
    this->fps = 1;
}

void SCAnimationPlayer::RunFrame(void){

    
    CheckKeyboard();
    VGA.Activate();
    VGA.GetFrameBuffer()->Clear();
    VGA.SetPalette(&this->palette);
    FrameBuffer *fb = VGA.GetFrameBuffer();
    fb->FillWithColor(0);
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - fps_timer > 6;
    if (fpsupdate) {
        fps_timer = (SDL_GetTicks() / 10);
    }
    

    MIDGAME_SHOT *shot = this->midgames_shots[1][shot_counter];
    if (this->current_music != shot->music && shot->music != 255) {
        Mixer.SwitchBank(0);
        Mixer.PlayMusic(shot->music);
        this->current_music = shot->music;
    }
    if (shot->sound != nullptr && (fps == shot->sound_time_code || shot->sound_time_code == 0)) {
        Mixer.PlaySoundVoc(shot->sound->data, shot->sound->size);
        shot->sound = nullptr;
    }
    for (auto bg : shot->background) {
        if (bg->image->GetNumImages()>0) {
            RLEShape *shp = bg->image->GetShape(bg->shapeid);
            FrameBuffer *texture = new FrameBuffer(320, 200);
            texture->FillWithColor(255);
            texture->DrawShape(shp);
            fb->blitWithMask(texture->framebuffer, bg->position_start.x, bg->position_start.y, 320, 200,255);
            if (fpsupdate && (bg->velocity.x != 0 || bg->velocity.y != 0)) {
                if (bg->position_start.x != bg->position_end.x) {
                    bg->position_start.x += bg->velocity.x;
                }  
                if (bg->velocity.y<0 && bg->position_start.y > bg->position_end.y) {
                    bg->position_start.y += bg->velocity.y;
                } else if (bg->velocity.y>0 && bg->position_start.y < bg->position_end.y) {
                    bg->position_start.y += bg->velocity.y;
                }
            }
            if (bg->palette != 0) {
                ByteStream paletteReader;
                paletteReader.Set(this->optPals.GetEntry(bg->palette)->data);
                this->palette.ReadPatch(&paletteReader);
                VGA.SetPalette(&this->palette);
            }
            if (bg->pal != nullptr) {
                this->palette.ReadPatch(bg->pal->GetColorPalette());
                VGA.SetPalette(&this->palette);
            }
        }
    }
    for (auto sprt: shot->sprites) {
        FrameBuffer *texture = new FrameBuffer(320, 200);
        texture->FillWithColor(255);
        if (sprt->keep_first_frame) {
            texture->DrawShape(sprt->image->GetShape(1));
        }
        if (fps<sprt->image->GetNumImages()) {
            texture->DrawShape(sprt->image->GetShape(fps));
        }
        int color = texture->framebuffer[0];
        fb->blitWithMask(texture->framebuffer, sprt->position_start.x, sprt->position_start.y, 320, 200,color);
        if (sprt->palette != 0) {
            ByteStream paletteReader;
            paletteReader.Set(this->optPals.GetEntry(sprt->palette)->data);
            this->palette.ReadPatch(&paletteReader);
            VGA.SetPalette(&this->palette);
        }
        if (sprt->pal != nullptr) {
            this->palette.ReadPatch(sprt->pal->GetColorPalette());
            VGA.SetPalette(&this->palette);
        }
    }
    
    fps_counter++;
    fps+=fpsupdate;
    
    if (fps > shot->nbframe) {
        fps = 1;
        shot_counter++;
        if (shot_counter>this->midgames_shots[1].size()-1) {
            shot_counter = 0;
            Game.StopTopActivity();
        }
    }
    for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT; i++)
            VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

    for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
        VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
    VGA.VSync();
    //this->RenderMenu();
}
void SCAnimationPlayer::RenderMenu() {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (ImGui::BeginMainMenuBar()) {
        ImGui::Text("Animation Player frame: %d, fps: %d, shot: %d", this->fps_counter, this->fps, this->shot_counter);
        ImGui::EndMainMenuBar();
    }
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}