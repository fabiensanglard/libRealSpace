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
        this->fps = SDL_GetTicks() / 10;
}

SCZone::~SCZone(){
    
}

void SCZone::OnAction(void){
    if (this->sprite != nullptr && this->sprite->cliked) {
        this->sprite->active = true;
    } else if (this->onclick != nullptr) {
        if (this->sprite != nullptr) {
            this->onclick(this->sprite->efect, this->id);
        } else {
            this->onclick(nullptr, this->id);
        }
    }
}

bool  SCZone::IsActive(std::map<uint8_t, bool> *requierd_flags) {
    if (this->sprite == nullptr || this->sprite->requ == nullptr) {
        this->active = true;
        return this->active;
    }
    for (auto requ_flag: *this->sprite->requ) {
        switch (requ_flag->op) {
            case 0:
                if (requierd_flags->find(requ_flag->value) != requierd_flags->end()) {
                    this->active = !requierd_flags->at(requ_flag->value);
                } else if (active) {
                    this->active = true;
                }
                break;
            case 1:
                if (requierd_flags->find(requ_flag->value) != requierd_flags->end()) {
                    this->active = requierd_flags->at(requ_flag->value);
                } else if (active) {
                    this->active = false;
                }
                break;
            default:
                break;
        }
    }
    return this->active;
}
void SCZone::Draw(void) {
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks() / 10);
    }
    if (this->sprite != nullptr) {
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
                    this->sprite->cliked == true && this->sprite->frameCounter < this->sprite->img->sequence.size()) {
            VGA.DrawShape(this->sprite->img->GetShape(this->sprite->img->sequence[this->sprite->frameCounter]));
            if (this->sprite->active == true) {
                this->sprite->frameCounter += fpsupdate;
                if (this->sprite->frameCounter >= this->sprite->img->sequence.size()-1) {
                    if (this->onclick != nullptr) {
                        this->onclick(this->sprite->efect, this->id);
                    }
                }
            }
        }
    }
}
void SCZone::drawQuad() {
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
