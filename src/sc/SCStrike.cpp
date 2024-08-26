//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include <cctype> // Include the header for the toupper function
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#define SC_WORLD 1100


SCStrike::SCStrike(){

}

SCStrike::~SCStrike(){

}
void SCStrike::CheckKeyboard(void) {
    //Keyboard
    SDL_Event keybEvents[1];

    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    int upEvents = SDL_PeepEvents(keybEvents, 1, SDL_GETEVENT, SDL_KEYUP, SDL_KEYUP);
    int controlStickX = 0;
    int controlStickY = 0;
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event* event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        case SDLK_z:
            
            break;
        case SDLK_s:
            
            break;
        case SDLK_q:
            
            break;
        case SDLK_d:
            
            break;
        case SDLK_e:
            
            break;
        case SDLK_a:
            
            break;
        case SDLK_UP:
            controlStickY = 45;
            break;
        case SDLK_DOWN:
            controlStickY = -45;
            break;
        case SDLK_LEFT:
            controlStickX = -45;
            break;
        case SDLK_RIGHT:
            controlStickX = 45;
            break;
        case SDLK_1:
            this->playerplane->SetThrottle(10);
            break;
        case SDLK_2:
            this->playerplane->SetThrottle(20);
            break;
        case SDLK_3:
            this->playerplane->SetThrottle(30);
            break;
        case SDLK_4:
            this->playerplane->SetThrottle(40);
            break;
        case SDLK_5:
            this->playerplane->SetThrottle(50);
            break;
        case SDLK_6:
            this->playerplane->SetThrottle(60);
            break;
        case SDLK_7:
            this->playerplane->SetThrottle(70);
            break;
        case SDLK_8:
            this->playerplane->SetThrottle(80);
            break;
        case SDLK_9:
            this->playerplane->SetThrottle(90);
            break;
        case SDLK_0:
            this->playerplane->SetThrottle(100);
            break;
        case SDLK_MINUS:
            this->playerplane->SetThrottle(this->playerplane->GetThrottle()-1);
            break;
        case SDLK_PLUS:
            this->playerplane->SetThrottle(this->playerplane->GetThrottle()+1);
            break;
        default:
            break;
        }
    }
    this->playerplane->SetControlStick(controlStickX, controlStickY);
}
void SCStrike::Init(void ){

    this->SetMission("TEMPLATE.IFF");
    this->playerplane->SetThrottle(0);
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
    newPosition.y= this->area.getY(newPosition.x, newPosition.z) + 10000;
    camera = Renderer.GetCamera();
    camera->SetPosition(&newPosition);
    yaw = 0.0f;

    this->playerplane = new SCPlane(
        10.0f,
        -7.0f,
        40.0f,
        40.0f,
        30.0f,
        100.0f,
        0,
        390.0f,
        18000.0,
        8000.0f,
        23000.0,
        32.0f,
        .93f,
        2,
        2,
        120,
        9.0f,
        18.0f,
        &this->area,
        newPosition.x,
        newPosition.y,
        newPosition.z
    );
    this->playerplane->on_ground = 0;
}
void SCStrike::RunFrame(void){
    this->CheckKeyboard();
    this->playerplane->Simulate();
    this->playerplane->getPosition(&newPosition);
    camera->SetPosition(&this->newPosition);
    camera->Rotate(
        -this->playerplane->elevationf*((float)M_PI/1800.0f),
        this->playerplane->azimuthf*((float)M_PI/1800.0f),
        (float)this->playerplane->twist*((float)M_PI/1800.0f)
    );
    Renderer.RenderWorldSolid(&area,BLOCK_LOD_MAX,400);
    Renderer.RenderMissionObjects(missionObj);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Simulation"); 

    ImGui::Text("Speed %d, Altitude %.0f, Heading %.0f", 
        this->playerplane->airspeed,
        this->newPosition.y,
        this->playerplane->azimuthf
    );
    ImGui::Text("Throttle %d", this->playerplane->GetThrottle());
    ImGui::Text("Control Stick %d %d", this->playerplane->control_stick_x, this->playerplane->control_stick_y);
    ImGui::Text("Elevation %.0f, Twist %.0f", this->playerplane->elevationf, this->playerplane->twist);
    ImGui::Text("Y %.0f, On ground %d", this->playerplane->y, this->playerplane->on_ground);

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
