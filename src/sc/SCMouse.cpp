//
//  SCMouse.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

const char* const CURSOR_SHAPE_PATH = "..\\..\\DATA\\FONTS\\MOUSE.SHP";

SCMouse::SCMouse() :
  currentMode(INVISIBLE)
{
    
}

SCMouse::~SCMouse(){
    
}

void SCMouse::Init(void){
    
    //Find the two cursor (arrow and visor)
    TreArchive* cursorArchive = Assets.tres[AssetManager::TRE_MISC];
    
    if (cursorArchive == NULL){
        //This is no a recoverable error condition
        Game.Terminate("SCMouse: Unable to open Cursor SHAPES.");
    }
        
    
    TreEntry*   cursorShape   = cursorArchive->GetEntryByName(CURSOR_SHAPE_PATH);
    
}

bool SCMouse::IsVisible(void){
    return this->visible;
}