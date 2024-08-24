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

    
    this->SetMission("TEMPLATE.IFF");
}

void SCStrike::SetMission(char *missionName) {
    char missFileName[33];
    sprintf_s(missFileName, "..\\..\\DATA\\MISSIONS\\%s", missionName);

    TreEntry* mission = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName(missFileName);
    IffLexer missionIFF;
    missionIFF.InitFromRAM(mission->data, mission->size);

    missionObj = new RSMission();
    missionObj->tre = Assets.tres[AssetManager::TRE_OBJECTS];
    missionObj->objCache = &objectCache;
    missionObj->InitFromIFF(&missionIFF);
    
    char filename[13];
    sprintf_s(filename, "%s.PAK", missionObj->getMissionAreaFile());
    area.InitFromPAKFileName(filename);

    PART* playerCoord = missionObj->getPlayerCoord();

    newPosition.x=  (float) playerCoord->XAxisRelative;
    newPosition.z=  (float) -playerCoord->YAxisRelative;
    newPosition.y= this->getY(newPosition.x, newPosition.z) + 10;
    camera = Renderer.GetCamera();
    camera->SetPosition(&newPosition);
    yaw = 0.0f;
}
float SCStrike::getY(float x, float z) {
	
	int centerX = 180000;
	int centerY = 180000;
	int blocX = (int)(x + centerX)/20000;
	int blocY = (int)(z + centerY)/20000;

	return (area.getGroundLevel(blocY * 18 + blocX, x, z));
}
void SCStrike::RunFrame(void){
    this->CheckKeyboard();
    camera->SetPosition(&this->newPosition);
    camera->Rotate(0.0f,this->yaw*(M_PI/180.0f),0.0f);
    Renderer.RenderWorldSolid(&area,BLOCK_LOD_MAX,400);
    Renderer.RenderMissionObjects(missionObj);
}
