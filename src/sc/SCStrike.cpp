//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include <cctype> // Include the header for the toupper function

#define SC_WORLD 1100

char* strtoupper(char* dest, const char* src) {
	char* result = dest;
	while (*dest++ = toupper(*src++));
	return result;
}

SCStrike::SCStrike(){

}

SCStrike::~SCStrike(){

}
void SCStrike::CheckKeyboard(void) {
    //Keyboard
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event* event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        case SDLK_z:
            //this->camera->MoveForward();
            this->newPosition.z+=100;
            break;
        case SDLK_s:
            //this->camera->MoveBackward();
            this->newPosition.z-=100;
            break;
        case SDLK_q:
            //this->camera->MoveStrafRight();
            this->newPosition.x+=100;
            break;
        case SDLK_d:
            //this->camera->MoveStrafLeft();
            this->newPosition.x-=100;
            break;
        case SDLK_e:
            this->yaw+=1;
            printf("yaw: %f\n",yaw);
            break;
        case SDLK_a:
            this->yaw-=1;
            printf("yaw: %f\n",yaw);
            break;
        case SDLK_UP:
            printf("UP\n");
            this->camera->MoveForward();
            this->newPosition = this->camera->GetPosition();
            break;
        case SDLK_DOWN:
            printf("DOWN\n");
            this->camera->MoveBackward();
            this->newPosition = this->camera->GetPosition();
            break;
        default:
            break;
        }
    }
}
void SCStrike::Init(void ){
    area.InitFromPAKFileName("ARENA.PAK");
    counter = 0;
    camera = Renderer.GetCamera();
    
    newPosition.x=  4100;
    newPosition.y= 100;
    newPosition.z=  3000;
    this->yaw = 0.0f;
    camera->SetPosition(&newPosition);
}

void SCStrike::RunFrame(void){
    this->CheckKeyboard();
    camera->SetPosition(&this->newPosition);
    camera->Rotate(0.0f,this->yaw*(M_PI/180.0f),0.0f);
    Renderer.RenderWorldSolid(&area,BLOCK_LOD_MAX,400);
}
