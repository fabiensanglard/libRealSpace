//
//  SCRegister.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

SCRegister::SCRegister(){
    
}

SCRegister::~SCRegister(){
    
}

void SCRegister::CheckKeyboard(void){
    //Keyboard
    static bool shift = false;
    SDL_Event keybEvents[5];
    int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_GETEVENT,SDL_KEYUP,SDL_KEYUP);
    for(int i= 0 ; i < numKeybEvents ; i++){
        SDL_Event *ev = &keybEvents[i];
        switch (ev->key.keysym.sym) {
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                shift = false;
            break;
        }
    }
    numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYDOWN, SDL_KEYDOWN);
    for(int i= 0 ; i < numKeybEvents ; i++){
        SDL_Event* event = &keybEvents[i];
        switch (event->key.keysym.sym) {
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                shift = true;
            break;
            case SDLK_TAB:
                if (this->current_entry == &GameState.player_callsign){
                    this->current_entry = &GameState.player_name;
                } else if (this->current_entry == &GameState.player_firstname){
                    this->current_entry = &GameState.player_callsign;
                } else if (this->current_entry == &GameState.player_name){
                    this->current_entry = &GameState.player_firstname;
                }
            break;
            case SDLK_DELETE:
            case SDLK_BACKSPACE:
                if(this->current_entry->size() > 0){
                    *this->current_entry = this->current_entry->substr(0,this->current_entry->size()-1);
                }
            break;
            case SDLK_SPACE:
                *this->current_entry += ' ';
            break;
            case SDLK_a:
            case SDLK_b:
            case SDLK_c:
            case SDLK_d:
            case SDLK_e:
            case SDLK_f:
            case SDLK_g:
            case SDLK_h:
            case SDLK_i:
            case SDLK_j:
            case SDLK_k:
            case SDLK_l:
            case SDLK_m:
            case SDLK_n:
            case SDLK_o:
            case SDLK_p:
            case SDLK_q:
            case SDLK_r:
            case SDLK_s:
            case SDLK_t:
            case SDLK_u:
            case SDLK_v:
            case SDLK_w:
            case SDLK_x:
            case SDLK_y:
            case SDLK_z:
            {
                if (shift) {
                    *this->current_entry += char(event->key.keysym.sym - 'a' + 'A');
                } else {
                    *this->current_entry += char(event->key.keysym.sym);
                }
                break;
            }
            case SDLK_0:
            case SDLK_1:
            case SDLK_2:
            case SDLK_3:
            case SDLK_4:
            case SDLK_5:
            case SDLK_6:
            case SDLK_7:
            case SDLK_8:
            case SDLK_9:
            {
                *this->current_entry += char(event->key.keysym.sym);
                break;
            }
            case SDLK_RETURN :{
                if (GameState.player_callsign.size() > 0 && GameState.player_firstname.size() > 0 && GameState.player_name.size() > 0) {
                    Stop();
                } else {
                    if (this->current_entry == &GameState.player_callsign){
                        this->current_entry = &GameState.player_firstname;
                    } else if (this->current_entry == &GameState.player_firstname){
                        this->current_entry = &GameState.player_name;
                    } else if (this->current_entry == &GameState.player_name){
                        this->current_entry = &GameState.player_callsign;
                    }
                }
                break;
            }
            
            default:
                break;
        }
    }
}

void SCRegister::Init( ){
    
    //Load book
    TreEntry* entryMountain = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    PakArchive pak;
    pak.InitFromRAM("",entryMountain->data,entryMountain->size);
    
    PakArchive bookPak;
    bookPak.InitFromRAM("subPak board",
                            pak.GetEntry(OptionShapeID::START_GAME_REGISTRATION)->data ,
                            pak.GetEntry(OptionShapeID::START_GAME_REGISTRATION)->size);
    book.Init(bookPak.GetEntry(0)->data, bookPak.GetEntry(0)->size);

    
    
    //Load palette
    VGAPalette* rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
    

    TreEntry* palettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    PakArchive palettesPak;
    palettesPak.InitFromRAM("OPTSHPS.PAK",palettesEntry->data,palettesEntry->size);
    
     ByteStream paletteReader;
    paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_STARTGAME_REGISTRATION)->data);
    this->palette.ReadPatch(&paletteReader);

    this->font = FontManager.GetFont("..\\..\\DATA\\FONTS\\SM-FONT.SHP");
    this->current_entry = &GameState.player_name;
    
}

void SCRegister::RunFrame(void){
    CheckButtons();
    CheckKeyboard();
    
    VGA.Activate();
    FrameBuffer *fb = VGA.GetFrameBuffer();
    fb->Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    fb->DrawShape(&book);
    fb->PrintText(this->font, {172, 104}, GameState.player_firstname, 0);
    fb->PrintText(this->font, {88, 104}, GameState.player_name, 0);
    fb->PrintText(this->font, {122, 110}, GameState.player_callsign, 0);
    Point2D pos;
    if (this->current_entry == &GameState.player_callsign){
        pos = {122, 110};
    } else if (this->current_entry == &GameState.player_firstname){
        pos = {172, 104};
    } else if (this->current_entry == &GameState.player_name){
        pos = {88, 104};
    }
    pos.x = pos.x + ((this->current_entry->size())*5+2);
    fb->PrintText(this->font, pos, "_", 0);
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();

}
