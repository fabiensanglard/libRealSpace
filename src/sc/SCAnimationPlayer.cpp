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
                {
                    { &this->optShps, OptionShapeID::SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,0}, {0,0} },
                    { &this->midgames, 20, 0, 0, {0,0}, {0,0}, {0,0} }
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                }, 
                {
                    { this->mid[0], 3, 0, 0, {0,32}, {0,0}, {0,0} }
                },
                15
            },
            {   // Shot 2
                {
                    { &this->optShps, OptionShapeID::SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,0}, {0,-32}, {0,-1} },
                    { &this->midgames, 20, 0, 0, {0,0}, {0,-32}, {0,-1} },
                    { &this->midgames, 19, 1, 0, {0,151}, {0,119}, {0,-1} },
                    { &this->optShps, OptionShapeID::MOUTAINS_BG, 0, 0, {0,16}, {0,-16}, {0,-1} },
                    { &this->midgames, 19, 2, 0, {0,151}, {0,119}, {0,-2} },
                    { &this->midgames, 19, 3, 0, {0,151}, {0,90}, {0,-3} }
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                },
                {
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                },
                32
            },
            {   // Shot 3
                {
                    { &this->optShps, SKY, 0, OPTPALS_PAK_SKY_PALETTE_PATCH_ID, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 20, 0, 0, {0,-32}, {0,0}, {0,0} },
                    { &this->midgames, 19, 1, 0, {0,119}, {0,0}, {0,0} },
                    { &this->optShps, MOUTAINS_BG, 0, 0, {0,-16}, {0,0}, {0,0} },
                    { &this->midgames, 19, 2, 0, {0,119}, {0,0}, {0,0} },
                    { &this->midgames, 19, 3, 0, {0,90}, {0,0}, {0,0} }
                },
                { 
                    {nullptr, 0,0,0,{0,0},{0,0},{0,0}}
                },
                {
                    { this->mid[0], 13, 0, 0, {0,-10}, {0,0}, {0,0} }
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
            if (tmp_img->GetNumImages() == 0) {
                delete tmp_img;
                tmp_img = new RSImageSet();
                tmp_img->InitFromPakEntry(pe);
            }
            bg->image = tmp_img;
            bg->shapeid = shot_bg.sub_shape_id;
            bg->position_start = shot_bg.start;
            bg->position_end = shot_bg.end;
            bg->velocity = shot_bg.velocity;
            shot->background.push_back(bg);
        }
        if (sht.sprites.size()>0 && sht.sprites[0].pak != nullptr) {
            MIDGAME_SHOT_SPRITE *sprite = new MIDGAME_SHOT_SPRITE();
            RSImageSet *tmp_img = new RSImageSet();
            tmp_img->InitFromPakEntry(sht.sprites[0].pak->GetEntry(sht.sprites[0].shape_id));
            sprite->image = tmp_img;
            sprite->position_start = sht.sprites[0].start;
            sprite->position_end = sht.sprites[0].end;
            sprite->velocity = sht.sprites[0].velocity;
            shot->sprites = sprite;
        }
        shot->nbframe = sht.nbframe;
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
            RLEShape *shp = bg->image->GetShape(bg->shapeid);
            FrameBuffer *texture = new FrameBuffer(320, 200);
            texture->FillWithColor(255);
            texture->DrawShape(shp);
            fb->blitWithMask(texture->framebuffer, bg->position_start.x, bg->position_start.y, 320, 200,255);
            if (fps_counter%5==0 && (bg->velocity.x != 0 || bg->velocity.y != 0)) {
                if (bg->position_start.x != bg->position_end.x) {
                    bg->position_start.x += bg->velocity.x;
                }  
                if (bg->velocity.y<0 && bg->position_start.y > bg->position_end.y) {
                    bg->position_start.y += bg->velocity.y;
                } else if (bg->velocity.y>0 && bg->position_start.y < bg->position_end.y) {
                    bg->position_start.y += bg->velocity.y;
                }
            }
            
        }
    }
    if (shot->sprites != nullptr) {
        FrameBuffer *texture = new FrameBuffer(320, 200);
        texture->FillWithColor(255);
        texture->DrawShape(shot->sprites->image->GetShape(fps));
        fb->blitWithMask(texture->framebuffer, shot->sprites->position_start.x, shot->sprites->position_start.y, 320, 200,255);
    }

    //VGA.GetFrameBuffer()->DrawShape(shot->foreground->GetShape(0));
    fps_counter++;
    if (fps_counter%5==0) {
        fps++;
        
        if (shot->sprites!=nullptr && fps > shot->sprites->image->GetNumImages()-1) {
            fps = 0;
            shot_counter++;
            if (shot_counter>this->midgames_shots[1].size()-1) {
                shot_counter = 0;
                Game.StopTopActivity();
            }
        } else if (shot->sprites==nullptr && fps > shot->nbframe) {
            fps = 0;
            shot_counter++;
            if (shot_counter>this->midgames_shots[1].size()-1) {
                shot_counter = 0;
                Game.StopTopActivity();
            }
        }
    }
    for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT; i++)
            VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

    for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
        VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
    Mouse.Draw();
    VGA.VSync();
}