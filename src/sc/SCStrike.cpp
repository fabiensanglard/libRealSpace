//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include <cctype> // Include the header for the toupper function

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
        default:
            break;
        }
    }
}
void SCStrike::Init(void ){
    TreEntry* mission = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName("..\\..\\DATA\\MISSIONS\\MISN-12A.IFF");
    IffLexer missionIFF;

    missionIFF.InitFromRAM(mission->data, mission->size);

    this->missionObj.tre = Assets.tres[AssetManager::TRE_OBJECTS];
    this->missionObj.objCache = &objectCache;
    this->missionObj.InitFromIFF(&missionIFF);

    char * mname = missionObj.getMissionAreaFile();
    char areaName[8 + 4 + 1];
    char fareaName[8 + 4 + 2];
    strtoupper(areaName, mname);
    sprintf_s(fareaName, "%s.PAK", areaName);
    Renderer.Init(1);
    this->area.tre = Assets.tres[AssetManager::TRE_OBJECTS];
	this->area.objCache = &objectCache;
	this->area.InitFromPAKFileName(fareaName);
}

void SCStrike::RunFrame(void){
    this->CheckKeyboard();
    Point3D* position = new Point3D({0,0,0});
    Renderer.setPlayerPosition(position);
    Renderer.RenderWorld(&area,BLOCK_LOD_MAX,400);

}
