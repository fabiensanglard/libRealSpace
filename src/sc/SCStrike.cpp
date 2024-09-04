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
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

#define SC_WORLD 1100

SCStrike::SCStrike() {
    this->camera_mode = 0;
    this->camera_pos = {0, 0, 0};
    this->counter = 0;
}

SCStrike::~SCStrike() {}
void SCStrike::CheckKeyboard(void) {
    // Keyboard
    SDL_Event keybEvents[1];

    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    int upEvents = SDL_PeepEvents(keybEvents, 1, SDL_GETEVENT, SDL_KEYUP, SDL_KEYUP);
    int msx = 0;
    int msy = 0;
    SDL_GetMouseState(&msx, &msy);
    msy = msy - (Screen.height / 2);
    msx = msx - (Screen.width / 2);
    if (this->mouse_control) {
        this->player_plane->control_stick_x = msx;
        this->player_plane->control_stick_y = msy;
    }
    if (this->camera_mode == 5) {
        this->pilote_lookat.x = ((Screen.width / 360) * msx) * .2;
        this->pilote_lookat.y = ((Screen.height / 360) * msy) * .2;
    }
    for (int i = 0; i < upEvents; i++) {
        SDL_Event *event = &keybEvents[i];
        switch (event->key.keysym.sym) {
        case SDLK_UP:
            if (!this->mouse_control)
                this->player_plane->control_stick_y = 0;
            break;
        case SDLK_DOWN:
            if (!this->mouse_control)
                this->player_plane->control_stick_y = 0;
            break;
        case SDLK_LEFT:
            if (!this->mouse_control)
                this->player_plane->control_stick_x = 0;
            break;
        case SDLK_RIGHT:
            if (!this->mouse_control)
                this->player_plane->control_stick_x = 0;
            break;
        default:
            break;
        }
    }
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        case SDLK_F1:
            this->camera_mode = 0;
            break;
        case SDLK_F2:
            this->camera_mode = 1;
            this->camera_pos = {-2, 4, -43};
            break;
        case SDLK_F3:
            this->camera_mode = 2;
            this->pilote_lookat.x = 90;
            break;
        case SDLK_F4:
            this->camera_mode = 3;
            this->pilote_lookat.x = 270;
            break;
        case SDLK_F5:
            this->camera_mode = 4;
            this->pilote_lookat.x = 180;
            break;
        case SDLK_F6:
            this->camera_mode = 5;
            break;
        case SDLK_KP_8:
            this->camera_pos.z += 1;
            break;
        case SDLK_KP_2:
            this->camera_pos.z -= 1;
            break;
        case SDLK_KP_4:
            this->camera_pos.x -= 1;
            break;
        case SDLK_KP_6:
            this->camera_pos.x += 1;
            break;
        case SDLK_KP_7:
            this->camera_pos.y += 1;
            break;
        case SDLK_KP_9:
            this->camera_pos.y -= 1;
            break;
        case SDLK_m:
            this->mouse_control = !this->mouse_control;
            break;
        case SDLK_UP:
            if (!this->mouse_control)
                this->player_plane->control_stick_y = -100;
            break;
        case SDLK_DOWN:
            if (!this->mouse_control)
                this->player_plane->control_stick_y = 100;
            break;
        case SDLK_LEFT:
            if (!this->mouse_control)
                this->player_plane->control_stick_x = -50;
            break;
        case SDLK_RIGHT:
            if (!this->mouse_control)
                this->player_plane->control_stick_x = 50;
            break;
        case SDLK_1:
            if (!this->mouse_control)
                this->player_plane->SetThrottle(10);
            break;
        case SDLK_2:
            this->player_plane->SetThrottle(20);
            break;
        case SDLK_3:
            this->player_plane->SetThrottle(30);
            break;
        case SDLK_4:
            this->player_plane->SetThrottle(40);
            break;
        case SDLK_5:
            this->player_plane->SetThrottle(50);
            break;
        case SDLK_6:
            this->player_plane->SetThrottle(60);
            break;
        case SDLK_7:
            this->player_plane->SetThrottle(70);
            break;
        case SDLK_8:
            this->player_plane->SetThrottle(80);
            break;
        case SDLK_9:
            this->player_plane->SetThrottle(90);
            break;
        case SDLK_0:
            this->player_plane->SetThrottle(100);
            break;
        case SDLK_MINUS:
            this->player_plane->SetThrottle(this->player_plane->GetThrottle() - 1);
            break;
        case SDLK_PLUS:
            this->player_plane->SetThrottle(this->player_plane->GetThrottle() + 1);
            break;
        case SDLK_l:
            this->player_plane->SetWheel();
            break;
        case SDLK_f:
            this->player_plane->SetFlaps();
            break;
        case SDLK_b:
            this->player_plane->SetSpoilers();
            break;
        case SDLK_n: {
            SCNavMap *nav_screen = new SCNavMap();
            char *arena_name = new char[8];
            char *arena_file_name = this->missionObj->getMissionAreaFile();
            for (int i = 0; i < 8; i++) {
                arena_name[i] = toupper(arena_file_name[i]);
            }

            nav_screen->Init();
            nav_screen->SetName(arena_name);
            Game.AddActivity(nav_screen);
        } break;
        default:
            break;
        }
    }
}
void SCStrike::Init(void) {
    this->mouse_control = false;
    this->SetMission("TEMPLATE.IFF");
    this->cockpit = new SCCockpit();
    this->cockpit->Init();
    this->pilote_lookat = {0, 0};
}

void SCStrike::SetMission(char *missionName) {
    char missFileName[33];
    sprintf_s(missFileName, "..\\..\\DATA\\MISSIONS\\%s", missionName);

    TreEntry *mission = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName(missFileName);
    IffLexer missionIFF;
    missionIFF.InitFromRAM(mission->data, mission->size);

    missionObj = new RSMission();
    missionObj->tre = Assets.tres[AssetManager::TRE_OBJECTS];
    missionObj->objCache = &objectCache;
    missionObj->InitFromIFF(&missionIFF);

    char filename[13];
    sprintf_s(filename, "%s.PAK", missionObj->getMissionAreaFile());
    area.InitFromPAKFileName(filename);

    PART *playerCoord = missionObj->getPlayerCoord();

    newPosition.x = (float)playerCoord->XAxisRelative;
    newPosition.z = (float)-playerCoord->YAxisRelative;
    newPosition.y = this->area.getY(newPosition.x, newPosition.z);

    camera = Renderer.GetCamera();
    camera->SetPosition(&newPosition);
    yaw = 0.0f;

    this->player_plane =
        new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f, 23000.0f, 32.0f, .93f, 120,
                    9.0f, 18.0f, &this->area, newPosition.x, newPosition.y, newPosition.z);
    this->player_plane->azimuthf = 1800.0f;
    for (auto obj : missionObj->missionObjects) {
        if (strcmp(obj->MemberName, "F-16DES") == 0) {
            this->player_plane->object = obj;
        }
    }
}
void SCStrike::RunFrame(void) {
    this->CheckKeyboard();
    this->player_plane->Simulate();
    this->player_plane->getPosition(&newPosition);
    if (this->player_plane->object != nullptr) {
        this->player_plane->object->XAxisRelative = (long)newPosition.x;
        this->player_plane->object->YAxisRelative = (long)newPosition.y;
        this->player_plane->object->ZAxisRelative = (long)newPosition.z;
    }

    switch (this->camera_mode) {

    case 0:
        camera->SetPosition(&this->newPosition);
        camera->ResetRotate();
        camera->Rotate((-0.1f * this->player_plane->elevationf) * ((float)M_PI / 180.0f),
                       (-0.1f * this->player_plane->azimuthf) * ((float)M_PI / 180.0f),
                       (-0.1f * (float)this->player_plane->twist) * ((float)M_PI / 180.0f));
        break;
    case 1: {
        Vector3D pos = {this->newPosition.x + this->camera_pos.x, this->newPosition.y + this->camera_pos.y,
                        this->newPosition.z + this->camera_pos.z};
        camera->SetPosition(&pos);
        camera->LookAt(&this->newPosition);
    } break;
    case 2:
    case 3:
    case 4:
        camera->SetPosition(&this->newPosition);
        camera->ResetRotate();
        camera->Rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);
        camera->Rotate((-0.1f * this->player_plane->elevationf) * ((float)M_PI / 180.0f),
                       (-0.1f * this->player_plane->azimuthf) * ((float)M_PI / 180.0f),
                       (-0.1f * (float)this->player_plane->twist) * ((float)M_PI / 180.0f));
        break;
    case 5:
    default:
        camera->SetPosition(&this->newPosition);
        camera->ResetRotate();

        camera->Rotate(-this->pilote_lookat.y * ((float)M_PI / 180.0f), 0.0f, 0.0f);
        camera->Rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);

        camera->Rotate((-this->player_plane->elevationf / 10.0f) * ((float)M_PI / 180.0f),
                       (-this->player_plane->azimuthf / 10.0f) * ((float)M_PI / 180.0f),
                       (-(float)this->player_plane->twist / 10.0f) * ((float)M_PI / 180.0f));
        break;
    }

    Renderer.RenderWorldSolid(&area, BLOCK_LOD_MAX, 400);
    Renderer.RenderMissionObjects(missionObj);
    switch (this->camera_mode) {
    case 0:
        this->cockpit->Render(0);
        break;
    case 1:
        this->player_plane->Render();
        break;
    case 2:
        this->cockpit->Render(1);
        break;
    case 3:
        this->cockpit->Render(2);
        break;
    case 4:
        this->cockpit->Render(3);
        break;
    case 5:
        glPushMatrix();
        Matrix cockpit_rotation;
        cockpit_rotation.Clear();
        cockpit_rotation.Identity();
        cockpit_rotation.translateM(this->player_plane->x * COORD_SCALE, (this->player_plane->y * COORD_SCALE) - 2.0f,
                                    this->player_plane->z * COORD_SCALE);
        cockpit_rotation.rotateM(((this->player_plane->azimuthf + 900) / 10.0f) * ((float)M_PI / 180.0f), 0.0f, 1.0f,
                                 0.0f);
        cockpit_rotation.rotateM((this->player_plane->elevationf / 10.0f) * ((float)M_PI / 180.0f), 0.0f, 0.0f, 1.0f);
        cockpit_rotation.rotateM(-(this->player_plane->twist / 10.0f) * ((float)M_PI / 180.0f), 1.0f, 0.0f, 0.0f);

        glMultMatrixf((float *)cockpit_rotation.v);
        glDisable(GL_CULL_FACE);
        Renderer.DrawModel(&this->cockpit->cockpit->REAL.OBJS, LOD_LEVEL_MAX);
        glPopMatrix();
        glEnable(GL_CULL_FACE);

        glPushMatrix();
        Matrix rotation;
        rotation.Clear();
        rotation.Identity();
        rotation.translateM(newPosition.x, newPosition.y, newPosition.z);

        rotation.rotateM(-0.1f * (this->player_plane->azimuthf + 900.0f) * ((float)M_PI / 180.0f), 0.0f, 1.0f, 0.0f);
        rotation.rotateM(-0.1f * (this->player_plane->elevationf) * ((float)M_PI / 180.0f), 0.0f, 0.0f, 1.0f);
        rotation.rotateM(-0.1f * (this->player_plane->twist) * ((float)M_PI / 180.0f), 1.0f, 0.0f, 0.0f);

        glMultMatrixf((float *)rotation.v);
        glPopMatrix();
        break;
    }
    this->RenderMenu();
}

void SCStrike::RenderMenu() {
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Simulation");
    ImGui::Text("Speed %d, Altitude %.0f, Heading %.0f", this->player_plane->airspeed, this->newPosition.y,
                this->player_plane->azimuthf);
    ImGui::Text("Throttle %d", this->player_plane->GetThrottle());
    ImGui::Text("Control Stick %d %d", this->player_plane->control_stick_x, this->player_plane->control_stick_y);
    ImGui::Text("Elevation %.3f, Twist %.3f, RollSpeed %d", this->player_plane->elevationf, this->player_plane->twist,
                this->player_plane->roll_speed);
    ImGui::Text("Y %.3f, On ground %d", this->player_plane->y, this->player_plane->on_ground);
    ImGui::Text("flight [roller:%4f, elevator:%4f, rudder:%4f]", this->player_plane->rollers,
                this->player_plane->elevator, this->player_plane->rudder);
    ImGui::Text("Acceleration (vx,vy,vz) [%.3f ,%.3f ,%.3f ]", this->player_plane->vx, this->player_plane->vy,
                this->player_plane->vz);
    ImGui::Text("Acceleration (ax,ay,az) [%.3f ,%.3f ,%.3f ]", this->player_plane->ax, this->player_plane->ay,
                this->player_plane->az);
    ImGui::Text("Lift %.3f", this->player_plane->lift);
    ImGui::Text("%.3f %.3f %.3f %.3f %.3f %.3f", this->player_plane->uCl, this->player_plane->Cl,
                this->player_plane->Cd, this->player_plane->Cdc, this->player_plane->kl, this->player_plane->qs);
    ImGui::Text("Gravity %.3f", this->player_plane->gravity);
    ImGui::Text("ptw");
    for (int o = 0; o < 4; o++) {
        ImGui::Text("PTW[%d]=[%f,%f,%f,%f]", o, this->player_plane->ptw.v[o][0], this->player_plane->ptw.v[o][1],
                    this->player_plane->ptw.v[o][2], this->player_plane->ptw.v[o][3]);
    }
    ImGui::Text("incremental");
    for (int o = 0; o < 4; o++) {
        ImGui::Text("INC[%d]=[%f,%f,%f,%f]", o, this->player_plane->incremental.v[o][0],
                    this->player_plane->incremental.v[o][1], this->player_plane->incremental.v[o][2],
                    this->player_plane->incremental.v[o][3]);
    }
    ImGui::End();
    ImGui::Begin("Camera");
    ImGui::Text("Tps %d", this->player_plane->tps);
    ImGui::Text("Camera mode %d", this->camera_mode);
    ImGui::Text("Position [%.3f,%.3f,%.3f]", this->camera_pos.x, this->camera_pos.y, this->camera_pos.z);
    ImGui::Text("Pilot lookat [%d,%d]", this->pilote_lookat.x, this->pilote_lookat.y);
    ImGui::End();
    ImGui::Begin("Cockpit");
    if (this->player_plane->GetWheel()) {
        ImGui::Text("Wheel down");
    } else {
        ImGui::Text("Wheel up");
    }
    if (this->player_plane->GetFlaps()) {
        ImGui::Text("Flaps");
    }
    if (this->player_plane->GetSpoilers()) {
        ImGui::Text("Breaks");
    }
    ImGui::End();
    ImGui::Begin("Mission");
    ImGui::Text("Mission %s", missionObj->getMissionName());
    ImGui::Text("Area %s", missionObj->getMissionAreaFile());
    ImGui::Text("Player Coord %d %d", missionObj->getPlayerCoord()->XAxisRelative,
                missionObj->getPlayerCoord()->YAxisRelative);
    static ImGuiComboFlags flags = 0;
    if (ImGui::BeginCombo("List des missions", mission_list[mission_idx], flags)) {
        for (int n = 0; n < SCSTRIKE_MAX_MISSIONS; n++) {
            const bool is_selected = (mission_idx == n);
            if (ImGui::Selectable(mission_list[n], is_selected))
                mission_idx = n;

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button("Load Mission")) {
        this->SetMission((char *)mission_list[mission_idx]);
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}
