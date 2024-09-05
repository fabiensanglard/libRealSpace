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
    SDL_Event keybEvents[5];
    int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYDOWN,SDL_KEYUP);
    for(int i= 0 ; i < numKeybEvents ; i++){
        SDL_Event* event = &keybEvents[i];
        switch (event->key.keysym.sym) {
            case SDLK_RETURN :{
                
                Stop();
                
                //Add both animation and next location on the stack.
                SCWildCatBase* baseLocation = new SCWildCatBase();
                baseLocation->Init();
                Game.AddActivity(baseLocation);
                
                SCAnimationPlayer* anim = new SCAnimationPlayer(0,0);
                anim->Init();
                Game.AddActivity(anim);
                
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

    

    
}

void SCRegister::RunFrame(void){
    CheckButtons();
    CheckKeyboard();
    
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(&book);

    
    DrawButtons();
    
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();

}
