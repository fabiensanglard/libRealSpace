//
//  SCFontManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


RSFontManager::RSFontManager(){
    
}

RSFontManager::~RSFontManager(){
    
}

void RSFontManager::Init(void){
    
    const char* fontPath = "..\\..\\DATA\\GAMEFLOW\\BWFONT.SHP";
    
    const char* trePath = "GAMEFLOW.TRE";
    TreArchive GAMEFLOW;
    GAMEFLOW.InitFromFile(trePath);
    
    
    //Find the sounds PAKS.
    
    TreEntry* convFontEntry = GAMEFLOW.GetEntryByName(fontPath);
    
    
    PakArchive fontArch;
    fontArch.InitFromRAM("BWFONT.SHP",convFontEntry->data,convFontEntry->size);
    fontArch.List(stdout);
    
    
    font.InitFromPAK(&fontArch);

}

RSFont* RSFontManager::GetFont(const char* name){
    return &font;
}

