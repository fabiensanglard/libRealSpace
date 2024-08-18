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
        this->label = nullptr;
        this->quad = nullptr;
        this->onclick = nullptr;
        this->dimension = {0,0};
        this->position  = {0,0};
        this->id = 0;
}

SCZone::~SCZone(){
    
}

void SCZone::OnAction(void){
    if (this->onclick != nullptr) {
        this->onclick(this->id);
    }
}

void SCZone::Draw(void) {
    if (this->quad != nullptr) {
        VGA.line(
            this->quad->at(0)->x,
            this->quad->at(0)->y,
            this->quad->at(1)->x,
            this->quad->at(1)->y,
            8
        );
        VGA.line(
            this->quad->at(1)->x,
            this->quad->at(1)->y,
            this->quad->at(2)->x,
            this->quad->at(2)->y,
            8
        );
        VGA.line(
            this->quad->at(2)->x,
            this->quad->at(2)->y,
            this->quad->at(3)->x,
            this->quad->at(3)->y,
            8
        );

        VGA.line(
            this->quad->at(3)->x,
            this->quad->at(3)->y,
            this->quad->at(0)->x,
            this->quad->at(0)->y,
            25
        );
    } else {
        VGA.rect_slow(
            this->position.x,
            this->position.y,
            this->position.x + this->dimension.x,
            this->position.y + this->dimension.y,
            8
        );
    }
}
