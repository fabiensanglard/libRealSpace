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
#include <tuple>
#define SC_WORLD 1100

SCStrike::SCStrike() {
    this->camera_mode = 0;
    this->camera_pos = {16, 2, -29};
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
        this->pilote_lookat.x = ((Screen.width / 360) * msx) / 6;
        this->pilote_lookat.y = ((Screen.height / 360) * msy) / 6;
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
        switch (event->key.keysym.scancode) {
        case SDL_SCANCODE_MINUS:
            this->player_plane->SetThrottle(this->player_plane->GetThrottle() - 1);
            break;
        case SDL_SCANCODE_EQUALS:
            this->player_plane->SetThrottle(this->player_plane->GetThrottle() + 1);
            break;
        case SDL_SCANCODE_LEFTBRACKET:
            this->cockpit->radar_zoom -= 1;
            if (this->cockpit->radar_zoom < 1) {
                this->cockpit->radar_zoom = 1;
            }
            break;
        case SDL_SCANCODE_RIGHTBRACKET:
            this->cockpit->radar_zoom += 1;
            if (this->cockpit->radar_zoom > 4) {
                this->cockpit->radar_zoom = 4;
            }
            break;
        case SDL_SCANCODE_W:
            if (this->cockpit->show_weapons) {
                this->player_plane->selected_weapon = (this->player_plane->selected_weapon+1) % 5;
                this->mfd_timeout = 400;
            } else {
                this->cockpit->show_weapons = !this->cockpit->show_weapons;
                this->mfd_timeout = 400;
            }
            break;
        default:
            break;
        }
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        case SDLK_F1:
            this->camera_mode = View::FRONT;
            break;
        case SDLK_F2:
            this->camera_mode = View::FOLLOW;
            this->camera_pos = {-2, 4, -43};
            break;
        case SDLK_F3:
            this->camera_mode = View::RIGHT;
            this->pilote_lookat.x = 90;
            break;
        case SDLK_F4:
            this->camera_mode = View::LEFT;
            this->pilote_lookat.x = 270;
            break;
        case SDLK_F5:
            this->camera_mode = View::REAR;
            this->pilote_lookat.x = 180;
            break;
        case SDLK_F6:
            this->mouse_control = false;
            this->camera_mode = View::REAL;
            break;
        case SDLK_F7:
            this->camera_mode = View::TARGET;
            break;
        case SDLK_F8:
            this->camera_mode = View::MISSILE_CAM;
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
        case SDLK_p:
            this->pause_simu = !this->pause_simu;
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
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(10);    
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 1;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_2:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(20);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 2;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_3:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(30);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 3;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_4:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(40);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 4;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_5:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(50);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 5;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_6:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(60);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 6;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_7:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(70);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 7;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_8:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(80);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 8;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_9:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(90);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 9;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
            break;
        case SDLK_0:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(100);
            } else {
                if (this->cockpit->comm_target == 0) {
                    this->cockpit->comm_target = 10;
                } else {
                    // send message 1 to ai comm_target
                    this->cockpit->comm_target = 0;
                    this->cockpit->show_comm = false;
                }
            }
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
        case SDLK_c:
            this->cockpit->show_comm = !this->cockpit->show_comm;
            this->cockpit->comm_target = 0;
            this->mfd_timeout = 400;
            break;
        case SDLK_r:
            this->cockpit->show_radars = !this->cockpit->show_radars;
            break;
        case SDLK_y:
            if (this->camera_mode != View::EYE_ON_TARGET) {
                this->camera_mode = View::EYE_ON_TARGET;
            } else {
                this->camera_mode = View::FRONT;
            }

            break;
        case SDLK_n: {
            SCNavMap *nav_screen = new SCNavMap();
            nav_screen->Init();
            nav_screen->SetName((char *)this->missionObj->mission_data.world_filename.c_str());
            nav_screen->missionObj = this->missionObj;
            nav_screen->current_nav_point = &this->nav_point_id;
            Game.AddActivity(nav_screen);
        } break;
        case SDLK_a:
            this->player_plane->x = this->missionObj->mission_data.areas[this->nav_point_id]->XAxis / COORD_SCALE;
            this->player_plane->z = -this->missionObj->mission_data.areas[this->nav_point_id]->YAxis / COORD_SCALE;
            this->player_plane->ptw.Identity();
            this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);
            break;
        case SDLK_SPACE:
            {
                MISN_PART *target = this->missionObj->mission_data.parts[this->current_target];
                this->player_plane->Shoot(this->player_plane->selected_weapon, target);
            }
            break;
        case SDLK_t:
            this->current_target = (this->current_target + 1) % this->missionObj->mission_data.parts.size();
            break;
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

void SCStrike::SetMission(char const *missionName) {
    ai_planes.clear();
    ai_planes.shrink_to_fit();
    sprintf(missFileName, "..\\..\\DATA\\MISSIONS\\%s", missionName);

    TreEntry *mission = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName(missFileName);

    missionObj = new RSMission();
    missionObj->tre = Assets.tres[AssetManager::TRE_OBJECTS];
    missionObj->objCache = &objectCache;
    missionObj->InitFromRAM(mission->data, mission->size);

    char filename[13];
    sprintf(filename, "%s.PAK", missionObj->mission_data.world_filename.c_str());
    area.InitFromPAKFileName(filename);

    if (area.objects.size() == 0) {
        sprintf(filename, "%s.PAK", missionObj->mission_data.name.c_str());
        area.InitFromPAKFileName(filename);
    }
    MISN_PART *playerCoord = missionObj->getPlayerCoord();

    newPosition.x = (float)playerCoord->x;
    newPosition.z = (float)-playerCoord->y;
    newPosition.y = (float)playerCoord->z;

    camera = Renderer.GetCamera();
    camera->SetPosition(&newPosition);
    yaw = 0.0f;
    this->current_target = 0;
    this->nav_point_id = 0;
    this->player_plane =
        new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f, 23000.0f, 32.0f, .93f, 120,
                    &this->area, newPosition.x, newPosition.y, newPosition.z);
    this->player_plane->azimuthf = (360 - playerCoord->azymuth) * 10.0f;
    this->player_plane->object = playerCoord;
    if (this->area.getY(newPosition.x, newPosition.z) < newPosition.y) {
        this->player_plane->SetThrottle(100);
        this->player_plane->SetWheel();
        this->player_plane->vz = -20;
        this->player_plane->Simulate();
    }
    std::map<int, std::vector<std::tuple<std::string, int>>> weap_map = {
        {12, {{"0", 1000}}},
        {1, {{"4", 1}, {"1", 1}, {"2", 1}}},
        {2, {{"4", 1}, {"1", 1}, {"2", 1}}},
        {3, {{"2", 3}, {"3", 3}}},
        {4, {{"2", 2}, {"3", 2}}},
        {5, {{"2", 6}, {"3", 6}}},
        {6, {{"2", 3}, {"3", 6}}},
        {7, {{"2", 3}, {"3", 3}}},
        {8, {{"2", 1}, {"3", 1}}},
        {9, {{"1", 1}, {"2", 2}}}
    };
    
    for (auto loadout: playerCoord->entity->weaps) {
        int plane_wp_loadout = loadout->nb_weap;
        for (auto hpts: weap_map.at(loadout->objct->wdat->weapon_id)) {
            int cpt=0;
            int next_hp = 0;
            if (plane_wp_loadout == 0) {
                break;
            }
            for (auto plane_hpts: player_plane->object->entity->hpts) {
                if ((plane_hpts->id == std::stoi(std::get<0>(hpts))) && (next_hp == 0 || next_hp == plane_hpts->id)) {
                    
                    int nb_weap = std::get<1>(hpts) - ((loadout->nb_weap /2) - std::get<1>(hpts));
                    plane_wp_loadout = plane_wp_loadout - nb_weap;
                    SCWeaponLoadoutHardPoint *weap = new SCWeaponLoadoutHardPoint();
                    weap->objct = loadout->objct;
                    weap->nb_weap = std::get<1>(hpts);
                    weap->hpts_type = std::stoi(std::get<0>(hpts));
                    weap->name = loadout->name;
                    weap->position = {(float) plane_hpts->x, (float) plane_hpts->z, (float) plane_hpts->y};
                    weap->hud_pos = {0, 0};
                    if (this->player_plane->weaps_load[cpt] == nullptr) {
                        this->player_plane->weaps_load[cpt] = weap;
                    }
                    if (next_hp == 0) {
                        next_hp = std::stoi(std::get<0>(hpts));
                    } else {
                        next_hp = 0;
                    }
                }
                cpt++;
            }
        }
    }
    for (auto part : missionObj->mission_data.parts) {
        int search_id = 0;
        for (auto cast : missionObj->mission_data.casting) {
            if (part->id == search_id) {
                if (cast->profile != nullptr && cast->profile->ai.isAI && cast->profile->ai.goal.size() > 0) {
                    SCAiPlane *aiPlane = new SCAiPlane();
                    aiPlane->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                 23000.0f, 32.0f, .93f, 120, &this->area, (float)part->x,
                                                 (float)part->z, (float)-part->y);
                    aiPlane->plane->azimuthf = (360 - part->azymuth) * 10.0f;
                    aiPlane->pilot = new SCPilot();
                    aiPlane->ai = cast->profile;
                    aiPlane->name = cast->actor;

                    if (this->area.getY((float)part->x, (float)-part->y) == part->z) {
                        aiPlane->plane->on_ground = true;
                    } else {
                        aiPlane->plane->on_ground = false;
                    }
                    if (part->z < this->area.getY((float)part->x, (float)-part->y)) {
                        aiPlane->plane->on_ground = true;
                    }
                    if (!aiPlane->plane->on_ground) {
                        aiPlane->plane->SetThrottle(100);
                        aiPlane->pilot->target_climb = (int) ((float) part->z * 3.6f);
                        aiPlane->plane->vz = -20;
                    } else {
                        aiPlane->pilot->target_climb = 25000;
                    }

                    aiPlane->pilot->plane = aiPlane->plane;
                    aiPlane->pilot->target_azimut = (int) (aiPlane->plane->azimuthf / 10.0f);

                    aiPlane->pilot->target_speed = -20;
                    aiPlane->plane->object = part;
                    aiPlane->object = part;
                    this->ai_planes.push_back(aiPlane);
                } else if (cast->profile != nullptr && cast->actor == "PLAYER") {
                    this->player_prof = cast->profile;
                }
                break;
            }
            search_id++;
        }
    }
}
void SCStrike::RunFrame(void) {
    this->CheckKeyboard();
    if (!this->pause_simu) {
        this->mfd_timeout--;
        this->player_plane->Simulate();
        for (auto aiPlane : this->ai_planes) {
            aiPlane->plane->Simulate();
            aiPlane->pilot->AutoPilot();
            Vector3D npos;
            aiPlane->plane->getPosition(&npos);
            if (aiPlane->object->area_id != 255) {
                AREA *ar = this->missionObj->mission_data.areas[aiPlane->object->area_id];

                float minX = (float) ar->XAxis-(ar->AreaWidth/2);
                float minY = (float) ar->YAxis-(ar->AreaWidth/2);
                float maxX = (float) ar->XAxis+(ar->AreaWidth/2);
                float maxY = (float) ar->YAxis+(ar->AreaWidth/2);

                if (npos.x < minX || npos.x > maxX || npos.z < minY || npos.z > maxY) {
                    aiPlane->pilot->target_azimut = aiPlane->pilot->target_azimut + 180;
                    if (aiPlane->pilot->target_azimut > 360) {
                        aiPlane->pilot->target_azimut = aiPlane->pilot->target_azimut - 360;
                    }
                }
            }
            aiPlane->object->x = (long)npos.x;
            aiPlane->object->z = (uint16_t)npos.y;
            aiPlane->object->y = -(long)npos.z;
            aiPlane->object->azymuth = 360 - (uint16_t)(aiPlane->plane->azimuthf / 10.0f);
            aiPlane->object->roll = (uint16_t)(aiPlane->plane->twist / 10.0f);
            aiPlane->object->pitch = (uint16_t)(aiPlane->plane->elevationf / 10.0f);
        }
    }
    this->player_plane->getPosition(&newPosition);
    if (this->player_plane->object != nullptr) {
        this->player_plane->object->x = (long)newPosition.x;
        this->player_plane->object->z = (uint16_t)newPosition.y;
        this->player_plane->object->y = (long)newPosition.z;
    }
    this->cockpit->pitch = this->player_plane->elevationf/10.0f;
    this->cockpit->roll = this->player_plane->twist/10.0f;
    this->cockpit->yaw = this->player_plane->azimuthf/10.0f;
    this->cockpit->speed = (float) this->player_plane->airspeed;
    this->cockpit->throttle = this->player_plane->GetThrottle();
    this->cockpit->altitude = this->player_plane->y;
    this->cockpit->heading = this->player_plane->azimuthf/10.0f;
    this->cockpit->gear = this->player_plane->GetWheel();
    this->cockpit->flaps = this->player_plane->GetFlaps()>0;
    this->cockpit->airbrake = this->player_plane->GetSpoilers()>0;
    this->cockpit->target = this->missionObj->mission_data.parts[this->current_target];
    this->cockpit->player = this->player_plane->object;
    this->cockpit->weapoint_coords.x = this->missionObj->mission_data.areas[this->nav_point_id]->XAxis;
    this->cockpit->weapoint_coords.y = -this->missionObj->mission_data.areas[this->nav_point_id]->YAxis;
    this->cockpit->parts = this->missionObj->mission_data.parts;
    this->cockpit->ai_planes = this->ai_planes;
    this->cockpit->player_prof = this->player_prof;
    this->cockpit->player_plane = this->player_plane;

    if (this->mfd_timeout <= 0) {
        if (this->cockpit->show_comm) {
            this->cockpit->show_comm = false;
        }
        if (this->cockpit->show_weapons) {
            this->cockpit->show_weapons = false;
        }
    }
    switch (this->camera_mode) {

    case View::FRONT: {
        Vector3D pos = {this->newPosition.x, this->newPosition.y + 3, this->newPosition.z - 2};
        camera->SetPosition(&pos);
        camera->ResetRotate();
        camera->Rotate((-0.1f * this->player_plane->elevationf + 10) * ((float)M_PI / 180.0f),
                       (-0.1f * this->player_plane->azimuthf) * ((float)M_PI / 180.0f),
                       (-0.1f * (float)this->player_plane->twist) * ((float)M_PI / 180.0f));
    } break;
    case View::FOLLOW: {
        Vector3D pos = {this->newPosition.x + this->camera_pos.x, this->newPosition.y + this->camera_pos.y,
                        this->newPosition.z + this->camera_pos.z};
        camera->SetPosition(&pos);
        camera->LookAt(&this->newPosition);
    } break;
    case View::RIGHT:
    case View::LEFT:
    case View::REAR:
        camera->SetPosition(&this->newPosition);
        camera->ResetRotate();
        camera->Rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);
        camera->Rotate((-0.1f * this->player_plane->elevationf) * ((float)M_PI / 180.0f),
                       (-0.1f * this->player_plane->azimuthf) * ((float)M_PI / 180.0f),
                       (-0.1f * (float)this->player_plane->twist) * ((float)M_PI / 180.0f));
        break;
    case View::TARGET: {
        MISN_PART *target = this->missionObj->mission_data.parts[this->current_target];
        Vector3D pos = {target->x + this->camera_pos.x, target->z + this->camera_pos.y,
                        -target->y + this->camera_pos.z};
        Vector3D targetPos = {(float)target->x, (float)target->z, -(float)target->y};
        camera->SetPosition(&pos);
        camera->LookAt(&targetPos);
    }
    break;
    case View::MISSILE_CAM: {
        if (this->player_plane->weaps_object.size() == 0) {
            this->camera_mode = View::FRONT;
            break;
        }
        SCSimulatedObject *missile = this->player_plane->weaps_object.back();
        if (missile == nullptr) {
            this->camera_mode = View::FRONT;
            break;
        }
        Vector3D missil_pos{0,0,0};
        missile->GetPosition(&missil_pos);
        Vector3D mpos = {
            missil_pos.x+this->camera_pos.x,
            missil_pos.y+this->camera_pos.y,
            missil_pos.z+this->camera_pos.z
        };
        camera->SetPosition(&mpos);
        camera->LookAt(&missil_pos);
    }
    break;
    case View::EYE_ON_TARGET: {
        MISN_PART *target = this->missionObj->mission_data.parts[this->current_target];
        Vector3D pos = {this->newPosition.x, this->newPosition.y + 1, this->newPosition.z + 1};
        Vector3D targetPos = {(float)target->x, (float)target->z, (float)-target->y};
        camera->SetPosition(&pos);
        camera->LookAt(&targetPos);
    } break;
    case View::REAL:
    default: {
        Vector3D pos = {this->newPosition.x, this->newPosition.y + 1, this->newPosition.z + 1};
        camera->SetPosition(&pos);
        camera->ResetRotate();

        camera->Rotate(-this->pilote_lookat.y * ((float)M_PI / 180.0f), 0.0f, 0.0f);
        camera->Rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);

        camera->Rotate(((-this->player_plane->elevationf / 10.0f)) * ((float)M_PI / 180.0f),
                       (-this->player_plane->azimuthf / 10.0f) * ((float)M_PI / 180.0f),
                       (-(float)this->player_plane->twist / 10.0f) * ((float)M_PI / 180.0f));
    } break;
    }

    Renderer.RenderWorldSolid(&area, BLOCK_LOD_MAX, 400);
    
    Renderer.RenderMissionObjects(missionObj);
    for (auto aiPlane : this->ai_planes) {
        if (aiPlane->object->alive == false) {
            aiPlane->plane->RenderSmoke();
        }
    }
    this->player_plane->RenderSimulatedObject();
    this->cockpit->cam = camera;
    switch (this->camera_mode) {
    case View::FRONT:
        this->cockpit->Render(0);
        break;
    case View::MISSILE_CAM:
    case View::TARGET:
    case View::FOLLOW:
        this->player_plane->Render();
        break;
    case View::RIGHT:
        this->cockpit->Render(1);
        break;
    case View::LEFT:
        this->cockpit->Render(2);
        break;
    case View::REAR:
        this->cockpit->Render(3);
        break;
    case View::EYE_ON_TARGET:
    case View::REAL:
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
    static bool show_simulation = false;
    static bool show_camera = false;
    static bool show_cockpit = false;
    static bool show_mission = false;
    static bool show_mission_parts_and_areas = false;
    static bool show_simulation_config = false;
    static bool azymuth_control = false;
    static bool show_ai = false;
    static int altitude = 0;
    static int azimuth = 0;
    static int throttle = 0;
    static int speed = 0;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("SCStrike")) {
            ImGui::MenuItem("Simulation", NULL, &show_simulation);
            ImGui::MenuItem("Simulation config", NULL, &show_simulation_config);
            ImGui::MenuItem("Camera", NULL, &show_camera);
            ImGui::MenuItem("Cockpit", NULL, &show_cockpit);
            ImGui::MenuItem("Mission", NULL, &show_mission);
            ImGui::MenuItem("Mission part and Area", NULL, &show_mission_parts_and_areas);
            ImGui::MenuItem("Ai pilot", NULL, &show_ai);
            ImGui::EndMenu();
        }
        int sceneid = -1;
        ImGui::Text("Speed %d\tAltitude %.0f\tHeading %.0f\tTPS: %03d\tArea %s\tfilename: %s",
                    this->player_plane->airspeed, this->newPosition.y * 3.6,
                    360 - (this->player_plane->azimuthf / 10.0f), this->player_plane->tps,
                    missionObj->mission_data.name.c_str(), missFileName);
        ImGui::EndMainMenuBar();
    }
    if (show_ai) {
        ImGui::Begin("AI");
        for (auto aiPlane : ai_planes) {
            ImGui::Text("Plane %s", aiPlane->object->member_name.c_str());
            ImGui::SameLine();
            ImGui::Text("Speed %d\tAltitude %.0f\tHeading %.0f", aiPlane->plane->airspeed, aiPlane->plane->y * 3.6,
                        360 - (aiPlane->plane->azimuthf / 10.0f), aiPlane->plane->tps);
            ImGui::SameLine();
            ImGui::Text("X %d Y %d Z %d", aiPlane->object->x, aiPlane->object->y, aiPlane->object->z);
        }
        ImGui::End();
    }
    if (show_simulation_config) {
        ImGui::Begin("Simulation Config");
        ImGui::DragInt("set altitude", &altitude, 100, 0, 30000);
        ImGui::DragInt("set throttle", &throttle, 10, 0, 100);
        ImGui::DragInt("set azimuth", &azimuth, 1, 0, 360);
        ImGui::DragInt("set speed", &speed, 2, 0, 30);
        if (ImGui::Button("set")) {
            this->player_plane->vz = (float) -speed;
            this->player_plane->y = (float) altitude;
            this->player_plane->last_py = this->player_plane->y;
            this->player_plane->rollers = 0;
            this->player_plane->roll_speed = 0;
            this->player_plane->elevation_speedf = 0;
            this->player_plane->elevator = 0;
            this->player_plane->SetThrottle(throttle);
            this->player_plane->ptw.Identity();
            this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);

            this->player_plane->ptw.rotateM(tenthOfDegreeToRad(azimuth * 10.0f), 0, 1, 0);
            this->player_plane->ptw.rotateM(tenthOfDegreeToRad(this->player_plane->elevationf), 1, 0, 0);
            this->player_plane->ptw.rotateM(tenthOfDegreeToRad(this->player_plane->twist), 0, 0, 1);
            this->player_plane->Simulate();

            this->pause_simu = true;
        }
        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->autopilot) {
            this->pilot.target_speed = -speed;
            this->pilot.target_climb = altitude;
            this->pilot.target_azimut = azimuth;
            this->pilot.AutoPilot();
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Autopilot")) {
            this->pilot.plane = this->player_plane;
            this->autopilot = !this->autopilot;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        float azimut_diff = azimuth - (360 - (this->player_plane->azimuthf / 10.0f));

        if (azimut_diff > 180.0f) {
            azimut_diff -= 360.0f;
        } else if (azimut_diff < -180.0f) {
            azimut_diff += 360.0f;
        }

        const float max_twist_angle = 80.0f;
        const float Kp = 3.0f;

        float target_twist_angle = Kp * azimut_diff;
        float current_twist = 360 - this->player_plane->twist / 10.0f;

        if (current_twist > 180.0f) {
            current_twist -= 360.0f;
        } else if (current_twist < -180.0f) {
            current_twist += 360.0f;
        }

        if (target_twist_angle > 180.0f) {
            target_twist_angle -= 360.0f;
        } else if (target_twist_angle < -180.0f) {
            target_twist_angle += 360.0f;
        }

        if (target_twist_angle > max_twist_angle) {
            target_twist_angle = max_twist_angle;
        } else if (target_twist_angle < -max_twist_angle) {
            target_twist_angle = -max_twist_angle;
        }

        ImGui::Text("Current diff %f ", current_twist);
        ImGui::Text("azymuth diff %f", azimut_diff);
        ImGui::Text("target twist %f", target_twist_angle);
        ImGui::Text("Twist to go %f", current_twist - target_twist_angle);

        if (azimut_diff > 0) {
            ImGui::Text("Go right");

            if (current_twist - target_twist_angle < 0) {
                ImGui::SameLine();
                ImGui::Text("Push RIGHT");
            } else {
                ImGui::SameLine();
                ImGui::Text("Let go the stick");
            }
            if (azymuth_control) {
                if (current_twist - target_twist_angle < 0) {
                    this->player_plane->control_stick_x = 50;
                } else {
                    this->player_plane->control_stick_x = 0;
                }
            }
        } else {

            ImGui::Text("Go left");
            if (current_twist - target_twist_angle > 0) {
                ImGui::SameLine();
                ImGui::Text("Push LEFT");
            } else {
                ImGui::SameLine();
                ImGui::Text("Let go the stick");
            }
            if (azymuth_control) {
                if (current_twist - target_twist_angle > 0) {
                    this->player_plane->control_stick_x = -50;
                } else {
                    this->player_plane->control_stick_x = 0;
                }
            }
        }

        ImGui::PushID(1);
        if (azymuth_control) {
            if (azimut_diff > 0) {

            } else {
            }
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Azymuth control")) {
            azymuth_control = !azymuth_control;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        Vector3D target = {this->player_plane->x, (float) altitude, (this->player_plane->z + 60 * this->player_plane->vz)};
        Vector3D current_position = {this->player_plane->x, this->player_plane->y, this->player_plane->z};

        float target_elevation = atan2(this->player_plane->z - (this->player_plane->z + (60 * this->player_plane->vz)),
                                       this->player_plane->y - altitude);
        target_elevation = target_elevation * 180.0f / (float)M_PI;

        if (target_elevation > 180.0f) {
            target_elevation -= 360.0f;
        } else if (target_elevation < -180.0f) {
            target_elevation += 360.0f;
        }
        target_elevation = target_elevation - 90.0f;

        ImGui::Text("Current elevation %.3f, target elevation %.3f", this->player_plane->elevationf / 10.0f,
                    target_elevation);
        ImGui::End();
    }
    if (show_simulation) {
        ImGui::Begin("Simulation");
        ImGui::Text("Speed %d, Altitude %.0f, Heading %.0f", this->player_plane->airspeed, this->newPosition.y,
                    this->player_plane->azimuthf);
        ImGui::Text("Throttle %d", this->player_plane->GetThrottle());
        ImGui::Text("Control Stick %d %d", this->player_plane->control_stick_x, this->player_plane->control_stick_y);
        float twist_diff = 360 - this->player_plane->twist / 10.0f;
        if (twist_diff > 180.0f) {
            twist_diff -= 360.0f;
        } else if (twist_diff < -180.0f) {
            twist_diff += 360.0f;
        }

        ImGui::Text("Elevation %.3f, Twist %.3f, RollSpeed %d", this->player_plane->elevationf, twist_diff,
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
    }
    if (show_camera) {
        ImGui::Begin("Camera");
        ImGui::Text("Tps %d", this->player_plane->tps);
        ImGui::Text("Camera mode %d", this->camera_mode);
        ImGui::Text("Position [%.3f,%.3f,%.3f]", this->camera_pos.x, this->camera_pos.y, this->camera_pos.z);
        ImGui::Text("Pilot lookat [%d,%d]", this->pilote_lookat.x, this->pilote_lookat.y);
        ImGui::End();
    }
    if (show_cockpit) {
        ImGui::Begin("Cockpit");
        ImGui::Text("Throttle %d", this->player_plane->GetThrottle());

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetWheel()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Gears")) {
            this->player_plane->SetWheel();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetFlaps()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Flaps")) {
            this->player_plane->SetFlaps();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetSpoilers()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Breaks")) {
            this->player_plane->SetSpoilers();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        if (this->mouse_control) {
            ImGui::SameLine();
            ImGui::Text("Mouse control enabled");
        }
        ImGui::Text("Target %s", this->missionObj->mission_data.parts[this->current_target]->member_name.c_str());
        MISN_PART *target = this->missionObj->mission_data.parts[this->current_target];
        Vector3D targetPos = {(float)target->x + 180000.0f, (float)target->z, -(float)target->y + 180000.0f};
        Vector3D playerPos = {this->newPosition.x + 180000.0f, this->newPosition.y, this->newPosition.z + 180000.0f};

        Camera totarget;
        totarget.SetPosition(&playerPos);
        totarget.LookAt(&targetPos);
        float raw;
        float yawn;
        float pitch;
        raw = 0.0f;
        yawn = 0.0f;
        pitch = 0.0f;

        totarget.GetOrientation().GetAngles(pitch, yawn, raw);

        Vector3D directionVector = {targetPos.x - playerPos.x, targetPos.y - playerPos.y, targetPos.z - playerPos.z};

        ImGui::Text("Target position [%.3f,%.3f,%.3f]", targetPos.x, targetPos.y, targetPos.z);
        ImGui::Text("Player position [%.3f,%.3f,%.3f]", playerPos.x, playerPos.y, playerPos.z);
        ImGui::Text("Azimuth to target %.3f",
                    (360 - (this->player_plane->azimuthf / 10.0f)) -
                        (atan2(targetPos.z - playerPos.z, targetPos.x - playerPos.x) * (180.0 / M_PI) + 90.0f));
        ImGui::Text("Q angles [%.3f,%.3f,%.3f] rel %.3f", pitch * 180.0 / M_PI, yawn * 180.0 / M_PI,
                    180.0f - (raw * 180.0 / M_PI),
                    (180.0f - (raw * 180.0 / M_PI)) - (360 - (this->player_plane->azimuthf / 10.0f)));
        if (ImGui::TreeNode("missiles")) {
            for (auto missils: this->player_plane->weaps_object) {
                ImGui::Text("VX:%.3f\tVY:%.3f\tVZ:%.3f\televation:%.3f\tazimut:%.3f\tspeed%.3f", 
                    missils->vx,
                    missils->vy,
                    missils->vz,
                    missils->elevationf,
                    missils->azimuthf,
                    Vector3D(missils->vx, missils->vy, missils->vz).Norm()
                );
                ImGui::Text("X:%.3f\tY:%.3f\tZ:%.3f", missils->x+ 180000.0f, missils->y, missils->z+ 180000.0f);
                if (missils->target != nullptr) {
                    ImGui::Text("Target X:%d\tY:%d\tZ:%d", missils->target->x+ 180000, missils->target->z, -(missils->target->y)+ 180000);    
                }
                
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }
    if (show_mission) {
        ImGui::Begin("Mission");
        static ImGuiComboFlags flags = 0;
        if (ImGui::BeginCombo("List des missions", mission_list[mission_idx], flags)) {
            for (int n = 0; n < SCSTRIKE_MAX_MISSIONS; n++) {
                const bool is_selected = (mission_idx == n);
                if (ImGui::Selectable(mission_list[n], is_selected))
                    mission_idx = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Load Mission")) {
            this->SetMission((char *)mission_list[mission_idx]);
        }

        ImGui::End();
    }

    if (show_mission_parts_and_areas) {
        ImGui::Begin("Mission Parts and Areas");
        ImGui::Text("Mission %s", missionObj->mission_data.name.c_str());
        ImGui::Text("Area %s", missionObj->mission_data.world_filename.c_str());
        ImGui::Text("Player Coord %d %d %d", missionObj->getPlayerCoord()->x, missionObj->getPlayerCoord()->y,
                    missionObj->getPlayerCoord()->z);
        ImGui::Text("Messages %d", missionObj->mission_data.messages.size());
        if (ImGui::TreeNode("Messages")) {
            for (auto msg : missionObj->mission_data.messages) {
                ImGui::Text("- %s", msg->c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Areas")) {
            for (auto area : missionObj->mission_data.areas) {
                if (ImGui::TreeNode((void *)(intptr_t)area->id, "Area id %d", area->id)) {
                    ImGui::Text("Area name %s", area->AreaName);
                    ImGui::Text("Area x %d y %d z %d", area->XAxis, area->YAxis, area->ZAxis);
                    ImGui::Text("Area width %d height %d", area->AreaWidth, area->AreaHeight);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Area Objects")) {
            for (int g = 0; g < area.objects.size(); g++) {
                auto obj = &area.objects.at(g);
                if (ImGui::TreeNode((void *)(intptr_t)g, "Object id %d", g)) {
                    ImGui::Text("Object name %s", obj->name);
                    ImGui::Text("Object x %d y %d z %d", obj->position[0], obj->position[1], obj->position[2]);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::Text("Mission Parts %d", missionObj->mission_data.parts.size());
        if (ImGui::TreeNode("Parts")) {
            for (auto part : missionObj->mission_data.parts) {
                if (ImGui::TreeNode((void *)(intptr_t)part->id, "Parts id %d, area id %d", part->id, part->area_id)) {
                    ImGui::Text("u1 %d", part->unknown1);
                    ImGui::Text("u2 %d", part->unknown2);
                    ImGui::Text("x %d y %d z %d", part->x, part->y, part->z);
                    ImGui::Text("azymuth %d", part->azymuth);
                    ImGui::Text("Name %s", part->member_name.c_str());
                    ImGui::Text("Destroyed %s", part->member_name_destroyed.c_str());
                    ImGui::Text("Weapon load %s", part->weapon_load.c_str());
                    if (ImGui::TreeNode("RS ENTITY")) {
                        ImGui::Text("weight %d", part->entity->weight_in_kg);
                        ImGui::Text("Thrust %d", part->entity->thrust_in_newton);
                        if (ImGui::TreeNode("childs")) {
                            for (auto child : part->entity->chld) {
                                ImGui::Text("%s - %d %d %d",child->name.c_str(), child->x, child->y, child->z);
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("weapons")) {
                            for (auto weapon : part->entity->weaps) {
                                ImGui::Text("%s - %d", weapon->name.c_str(), weapon->nb_weap);
                                if (weapon->objct != nullptr) {
                                    if (ImGui::TreeNode("weapons")) {
                                        if (weapon->objct->wdat != nullptr) {
                                            ImGui::Text("Weapon DATA %d, %d, %d, %d, %d", 
                                                weapon->objct->wdat->damage,
                                                weapon->objct->wdat->effective_range,
                                                weapon->objct->wdat->target_range,
                                                weapon->objct->wdat->tracking_cone,
                                                weapon->objct->wdat->radius
                                            );
                                        }
                                        if (weapon->objct->dynn_miss != nullptr) {
                                            ImGui::Text("Dynamics: %d, %d, %d, %d",
                                                weapon->objct->dynn_miss->velovity_m_per_sec,
                                                weapon->objct->dynn_miss->turn_degre_per_sec,
                                                weapon->objct->dynn_miss->proximity_cm,
                                                weapon->objct->weight_in_kg
                                            ); 
                                        }
                                        ImGui::TreePop();
                                    }
                                }
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("hpts")) {
                            for (auto hpt : part->entity->hpts) {
                                ImGui::Text("%d - %d %d %d",hpt->id,  hpt->x, hpt->y, hpt->z);
                            }
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Casting")) {
            for (auto cast : missionObj->mission_data.casting) {
                if (ImGui::TreeNode((void *)(intptr_t)cast, "Actor %s", cast->actor.c_str())) {
                    ImGui::Text("Name %s", cast->profile->radi.info.name.c_str());
                    ImGui::Text("CallSign %s", cast->profile->radi.info.callsign.c_str());
                    ImGui::Text("Is AI %d", cast->profile->ai.isAI);
                    if (ImGui::TreeNode("MSGS")) {
                        for (auto msg : cast->profile->radi.msgs) {
                            ImGui::Text("- [%d]: %s", msg.first, msg.second.c_str());
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("ASKS")) {
                        for (auto msg : cast->profile->radi.asks) {
                            ImGui::Text("- [%s]: %s", msg.first.c_str(), msg.second.c_str());
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("ASKS_VECTOR")) {
                        for (auto msg : cast->profile->radi.asks_vector) {
                            ImGui::Text("- %s", msg.c_str());
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}
