//
//  SCDogFightMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

static const uint8_t PAK_ID_PALETTE          = 7;
static const uint8_t PAK_ID_BACKGROUND       = 6;
static const uint8_t PAK_ID_TITLE            = 1 ;
static const uint8_t PAK_ID_BUTTONS          = 3 ;

SCDogFightMenu::SCDogFightMenu(){
    
}

SCDogFightMenu::~SCDogFightMenu(){
    
}

void SCDogFightMenu::Init( ){
    
    TreArchive tre ;
    tre.InitFromFile("GAMEFLOW.TRE");
    
    TreEntry* objViewIFF = NULL;//Assets.tres[]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.IFF");
    TreEntry* objViewPAK = NULL;//tre.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.PAK");
    
    
    IffLexer objToDisplay;
    objToDisplay.InitFromRAM(objViewIFF->data, objViewIFF->size);
    objToDisplay.List(stdout);
    
    PakArchive assets;
    assets.InitFromRAM("OBJVIEW.PAK",objViewPAK->data, objViewPAK->size);
    assets.List(stdout);
    
    PakEntry* paletteEntry = assets.GetEntry(PAK_ID_PALETTE);
    //ParsePalette(paletteEntry);
    renderer.SetPalette(&this->palette);
    
    this->palette.Diff(renderer.GetDefaultPalette());
    
    PakEntry* entry0 = assets.GetEntry(PAK_ID_BACKGROUND);
    PakArchive file0;
    file0.InitFromRAM("OBJVIEW.PAK: file PAK_ID_MENU_DYNAMC",entry0->data, entry0->size);
    file0.List(stdout);
    //showAllImage(&file0);
}




void SCDogFightMenu::RunFrame(void){
    
 
}