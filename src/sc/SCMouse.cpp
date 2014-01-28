//
//  SCMouse.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

const char* const CURSOR_SHAPE_PATH = "..\\..\\DATA\\MOUSE.SHP";

SCMouse::SCMouse() :
  mode(CURSOR)
{
    
}

SCMouse::~SCMouse(){
    
}

void SCMouse::Init(void){
    
    
    TreEntry* cursorShape   = Assets.tres[AssetManager::TRE_MISC]->GetEntryByName(CURSOR_SHAPE_PATH);
    
    PakArchive cursors ;
    cursors.InitFromRAM("MOUSE.SHP",cursorShape->data,cursorShape->size);
    
    RLEShape* shape;
    
    for (int i = 0 ; i < 4; i++) {
        shape = new RLEShape();
        shape->Init(cursors.GetEntry(i)->data, cursors.GetEntry(i)->size);
        appearances[i] = shape;
    }
}

void SCMouse::Draw(void){
    
    if (! IsVisible())
        return;
    
    if (mode == CURSOR){
        appearances[1]->SetPosition(&position);
        VGA.DrawShape(appearances[1]);
    }
    
    if (mode == VISOR){
        appearances[0]->SetPosition(&position);
        VGA.DrawShape(appearances[0]);
        
    }
    //If the mouse is over a clickable button, the current appearance has already been selected.
    
}
