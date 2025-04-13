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

void SCMouse::init(void){
    
    
    TreEntry* cursorShape   = Assets.GetEntryByName(CURSOR_SHAPE_PATH);
    
    PakArchive cursors ;
    cursors.InitFromRAM("MOUSE.SHP",cursorShape->data,cursorShape->size);
    
    RLEShape* shape;
    
    for (int i = 0 ; i < 4; i++) {
        shape = new RLEShape();
        shape->init(cursors.GetEntry(i)->data, cursors.GetEntry(i)->size);
        appearances[i] = shape;
    }
}

void SCMouse::Draw(void){
    
    if (! IsVisible())
        return;
    
    // We need to draw the cursor a little bit higher left than the mouse
    // position so it is centered.
    Point2D cursorPos = this->position;
    //printf("Cursor pos: %d, %d\n", cursorPos.x, cursorPos.y);
    cursorPos.x -= 4;
    cursorPos.y -= 4;
    
    
    //If the mouse is over a clickable button, the current appearance has already been selected.
    if (mode == CURSOR){
        appearances[1]->SetPosition(&cursorPos);
        VGA.GetFrameBuffer()->DrawShape(appearances[1]);
    }
    
    if (mode == VISOR){
        appearances[0]->SetPosition(&cursorPos);
        VGA.GetFrameBuffer()->DrawShape(appearances[0]);
        
    }
}

void SCMouse::FlushEvents(void){
    for (size_t i = 0 ; i < 3 ; i++) {
        buttons[i].event = MouseButton::NONE;
    }
}
