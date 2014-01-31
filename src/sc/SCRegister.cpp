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

void SCRegister::Init( ){
    
    //Load book
    TreEntry* entryMountain = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    PakArchive pak;
    pak.InitFromRAM("",entryMountain->data,entryMountain->size);
    
    
    //The board is within an other pak within MAINMENU.PAK !!!!
    PakArchive bookPak;
    bookPak.InitFromRAM("subPak board",
                            pak.GetEntry(OptionShapeID::START_GAME_REGISTRATION)->data ,
                            pak.GetEntry(OptionShapeID::START_GAME_REGISTRATION)->size);
    book.Init(bookPak.GetEntry(0)->data, bookPak.GetEntry(0)->size);

    
    
    //Load palette
    VGAPalette* rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
    
#define PAK 30
    TreEntry* palettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    PakArchive palettesPak;
    palettesPak.InitFromRAM("OPTSHPS.PAK",palettesEntry->data,palettesEntry->size);
    
     ByteStream paletteReader;
    paletteReader.Set(palettesPak.GetEntry(PAK)->data);
    this->palette.ReadPatch(&paletteReader);

    

    
}

void SCRegister::RunFrame(void){
    CheckButtons();
    
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
