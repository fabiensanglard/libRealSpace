//
//  WildCatBase.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"



SCWildCatBase::SCWildCatBase(){
    
}

SCWildCatBase::~SCWildCatBase(){
    
}

void SCWildCatBase::Init( ){
    
    //Load book
    TreEntry* entryMountain = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    PakArchive pak;
    pak.InitFromRAM("",entryMountain->data,entryMountain->size);
    
    PakArchive bookPak;
    bookPak.InitFromRAM("subPak board",
                        pak.GetEntry(OptionShapeID::WILDCAT_HANGAR)->data ,
                        pak.GetEntry(OptionShapeID::WILDCAT_HANGAR)->size);
    hangar.Init(bookPak.GetEntry(0)->data, bookPak.GetEntry(0)->size);
    
    //Load vehicule
    PakArchive vehiculePak;
    vehiculePak.InitFromRAM("subPak board",
                        pak.GetEntry(OptionShapeID::WILDCAT_HANGAR_VEHICULE_F16)->data ,
                        pak.GetEntry(OptionShapeID::WILDCAT_HANGAR_VEHICULE_F16)->size);
    vehicule.Init(vehiculePak.GetEntry(0)->data, vehiculePak.GetEntry(0)->size);
    
    
    //Load palette
    VGAPalette* rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
    
#define PAK 5
    TreEntry* palettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    PakArchive palettesPak;
    palettesPak.InitFromRAM("OPTSHPS.PAK",palettesEntry->data,palettesEntry->size);
    
    ByteStream paletteReader;
    paletteReader.Set(palettesPak.GetEntry(PAK)->data);
    this->palette.ReadPatch(&paletteReader);
    

    
    
}

void SCWildCatBase::RunFrame(void){

    CheckButtons();
    
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(&hangar);
    VGA.DrawShape(&vehicule);
    
    DrawButtons();
    
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();

}

