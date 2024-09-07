//
//  SCZone.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 22/08/2024.
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
        this->onclick(this->sprite->efect, this->id);
    }
}

void SCZone::Draw(void) {
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks() / 10);
    }
    VGA.DrawShape(this->sprite->img->GetShape(this->sprite->img->sequence[0]));
    if (this->sprite->img->GetNumImages() > 1 && this->sprite->frames != nullptr) {
        VGA.DrawShape(this->sprite->img->GetShape(this->sprite->frames->at(this->sprite->frameCounter)));
        this->sprite->frameCounter =
            (this->sprite->frameCounter + fpsupdate) % static_cast<uint8_t>(this->sprite->frames->size());

    } else if (this->sprite->img->sequence.size() > 1 && this->sprite->frames == nullptr &&
                this->sprite->cliked == false) {

        if (this->sprite->frameCounter >= this->sprite->img->sequence.size()) {
            this->sprite->frameCounter = 1;
        }
        VGA.DrawShape(this->sprite->img->GetShape(this->sprite->img->sequence[this->sprite->frameCounter]));
        this->sprite->frameCounter += fpsupdate;
    } else if (this->sprite->img->sequence.size() > 1 && this->sprite->frames == nullptr &&
                this->sprite->cliked == true) {
        VGA.DrawShape(this->sprite->img->GetShape(this->sprite->img->sequence[this->sprite->frameCounter]));
    }

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
