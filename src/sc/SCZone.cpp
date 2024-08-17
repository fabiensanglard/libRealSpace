//
//  Button.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

SCZone::SCZone():
    enabled(true){
    
}

SCZone::~SCZone(){
    
}

void SCZone::InitBehavior(ActionFunction fct, Point2D position,Point2D dimension){
    this->onClick = fct;
    this->position = position;
    this->dimension = dimension;
}

void SCZone::OnAction(void){
    this->onClick();
}

void SCZone::Draw(void) {
    //VGA.rect_slow()
}
