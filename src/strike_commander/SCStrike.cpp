//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include <cctype>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>
#include <tuple>
#include <optional>
#include <cmath>
#include "SCStrike.h"
#define SC_WORLD 1100
#define RENDER_DISTANCE 40000.0f
#define AUTOPILOTE_TIMEOUT 1000
#define AUTOPILOTE_SPEED 4
/**
 * @brief Constructor
 *
 * Initializes the SCStrike object with a default camera position and
 * other variables.
 */
SCStrike::SCStrike() {
    this->camera_mode = 0;
    this->camera_pos = {-169.0f, 79.0f, -189.0f};
    this->counter = 0;
}

SCStrike::~SCStrike() {}
/**
 * @brief Handle keyboard events
 *
 * This function peeks at the SDL event queue to check for keyboard events.
 * If a key is pressed, it updates the game state based on the key pressed.
 * The supported keys are escape, space, and return.
 *
 * @return None
 */
void SCStrike::checkKeyboard(void) {
    // Keyboard
    SDL_Event keybEvents[1];

    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    int upEvents = SDL_PeepEvents(keybEvents, 1, SDL_GETEVENT, SDL_KEYUP, SDL_KEYUP);
    int msx = 0;
    int msy = 0;
    SDL_GetMouseState(&msx, &msy);
    msy = msy - (Screen->height / 2);
    msx = msx - (Screen->width / 2);
    if (this->camera_mode == View::AUTO_PILOT) {
        return;
    }
    if (this->mouse_control) {
        this->player_plane->control_stick_x = msx;
        this->player_plane->control_stick_y = msy;
    }
    if (this->camera_mode == 5) {
        this->pilote_lookat.x = ((Screen->width / 360) * msx) / 6;
        this->pilote_lookat.y = ((Screen->height / 360) * msy) / 6;
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
            this->current_mission->mission_ended = true;
            break;
        }
        case SDLK_F1:
            this->camera_mode = View::FRONT;
            break;
        case SDLK_F2:
            if (this->camera_mode != View::FOLLOW) {
                this->follow_dynamic = false;
            }
            if (this->camera_mode == View::FOLLOW) {
                this->follow_dynamic = !this->follow_dynamic;
            }
            this->camera_mode = View::FOLLOW;
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
        case SDLK_F9:
            if (this->camera_mode != View::OBJECT) {
                this->camera_mode = View::OBJECT;
            } else {
                this->current_object_to_view = (this->current_object_to_view + 1) % this->current_mission->actors.size();
            }
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
        case SDLK_KP_PLUS:
            this->eye_y += 1;
            break;
        case SDLK_KP_MINUS:
            this->eye_y -= 1;
            break;
        case SDLK_m:
            this->mouse_control = !this->mouse_control;
            break;
        case SDLK_p:
            this->pause_simu = !this->pause_simu;
            break;
        case SDLK_UP:
            if (!this->mouse_control)
                this->player_plane->control_stick_y = -125;
            break;
        case SDLK_DOWN:
            if (!this->mouse_control)
                this->player_plane->control_stick_y = 125;
            break;
        case SDLK_LEFT:
            if (!this->mouse_control)
                this->player_plane->control_stick_x = -110;
            break;
        case SDLK_RIGHT:
            if (!this->mouse_control)
                this->player_plane->control_stick_x = 110;
            break;
        case SDLK_1:
            if (!this->cockpit->show_comm) {
                this->player_plane->SetThrottle(10);
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1);    
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1); 
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1); 
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1); 
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1); 
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1); 
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1);
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1);
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1);
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
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1);
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
            if (this->player_plane->GetThrottle() == 0) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL_SHUT_DOWN];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, 0);
            }
            break;
        case SDLK_PLUS:
            
            this->player_plane->SetThrottle(this->player_plane->GetThrottle() + 1);
            if (this->player_plane->GetThrottle() > 0 && this->player_plane->GetThrottle() < 60) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1);
            } else if (this->player_plane->GetThrottle() >= 60) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
                Mixer.PlaySoundVoc(engine->data, engine->size, 5, -1);
            }
            break;
        case SDLK_l:
            this->player_plane->SetWheel();
            if (this->player_plane->GetWheel()) {
                MemSound *gears = this->current_mission->sound.sounds[SoundEffectIds::GEARS_UP];
                Mixer.PlaySoundVoc(gears->data, gears->size, 4, 0);
            } else {
                MemSound *gears = this->current_mission->sound.sounds[SoundEffectIds::GEARS_DOWN];
                Mixer.PlaySoundVoc(gears->data, gears->size, 4, 0);
            }
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
            this->mfd_timeout = AUTOPILOTE_TIMEOUT;
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
            nav_screen->init();
            nav_screen->SetName((char *)this->current_mission->world->tera.c_str());
            nav_screen->mission = this->current_mission;
            nav_screen->missionObj = this->current_mission->mission = this->current_mission->mission;
            nav_screen->current_nav_point = &this->nav_point_id;
            Game->addActivity(nav_screen);
        } break;
        case SDLK_a:
        {
            Vector2D destination = {this->current_mission->waypoints[this->nav_point_id]->spot->position.x,
                                    this->current_mission->waypoints[this->nav_point_id]->spot->position.z};
            this->autopilot_target_azimuth = atan2(this->player_plane->x-destination.x, this->player_plane->z-destination.y);
            this->autopilot_target_azimuth = radToDegree(this->autopilot_target_azimuth);
            this->player_plane->yaw = 0.0f;
            this->player_plane->pitch = 0.0f;
            this->player_plane->roll = 0.0f;
            float dest_y = this->current_mission->waypoints[this->nav_point_id]->spot->position.y;
            Vector2D origine = {this->player_plane->x, this->player_plane->z};
            std::vector<Vector2D> path;
            for (auto area: this->current_mission->mission->mission_data.areas) {
                {
                    bool area_active = false;
                    for (auto scen: this->current_mission->mission->mission_data.scenes) {
                        if (scen->area_id == area->id-1) {
                            area_active = area_active || scen->is_active;
                        }
                    }
                    if (!area_active) {
                        continue;
                    }
                    printf("check collision with Areas: %s\n", area->AreaName);
                    // Assume each area is represented as an axis-aligned rectangle in the X-Z plane.
                    // Compute rectangle boundaries.
                    float halfWidth  = area->AreaWidth / 2.0f;
                    
                    float left   = area->position.x - halfWidth;
                    float right  = area->position.x + halfWidth;
                    float top    = area->position.z - halfWidth;
                    float bottom = area->position.z + halfWidth;

                    // Lambda to test if a point is inside the rectangle.
                    auto pointInRect = [&](const Vector2D &pt) -> bool {
                        return (pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom);
                    };
                    if (pointInRect(origine)) {
                        // we know we are in this area already
                        printf("We are in area: %s\n", area->AreaName);
                        continue;
                    }
                    // If either endpoint is inside, we have a collision.
                    /*if (pointInRect(origine) || pointInRect(destination)) {
                        printf("Collision detected with area: %s\n", area->AreaName);
                        continue;
                    }*/

                    auto doLinesIntersect = [&](const Vector2D &p1, const Vector2D &p2,
                                                  const Vector2D &p3, const Vector2D &p4) -> std::optional<Vector2D> {
                        float r_px = p2.x - p1.x;
                        float r_py = p2.y - p1.y;
                        float s_px = p4.x - p3.x;
                        float s_py = p4.y - p3.y;
                        float denominator = r_px * s_py - r_py * s_px;
                        if (fabs(denominator) < 1e-6f)
                            return std::nullopt; // lines are parallel

                        float t = ((p3.x - p1.x) * s_py - (p3.y - p1.y) * s_px) / denominator;
                        float u = ((p3.x - p1.x) * r_py - (p3.y - p1.y) * r_px) / denominator;

                        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
                            // Return the intersection point
                            return Vector2D { p1.x + t * r_px, p1.y + t * r_py };
                        }
                        return std::nullopt;
                    };

                    // Define rectangle edges (using two points per edge).
                    Vector2D r1 = {left, top};
                    Vector2D r2 = {right, top};
                    Vector2D r3 = {right, bottom};
                    Vector2D r4 = {left, bottom};

                    std::optional<Vector2D> i1 = doLinesIntersect(origine, destination, r1, r2);
                    std::optional<Vector2D> i2 = doLinesIntersect(origine, destination, r2, r3);
                    std::optional<Vector2D> i3 = doLinesIntersect(origine, destination, r3, r4);
                    std::optional<Vector2D> i4 = doLinesIntersect(origine, destination, r4, r1);

                    // Check if the segment (origin,destination) intersects any rectangle edge.
                    if (i1 != std::nullopt ||
                        i2 != std::nullopt ||
                        i3 != std::nullopt ||
                        i4 != std::nullopt)
                    {
                        printf("Collision detected with area: %s\n", area->AreaName);
                        //continue;
                    }
                    if (i1 != std::nullopt) {
                        path.push_back(i1.value());
                    }
                    if (i2 != std::nullopt) {
                        path.push_back(i2.value());
                    }
                    if (i3 != std::nullopt) {
                        path.push_back(i3.value());
                    }
                    if (i4 != std::nullopt) {
                        path.push_back(i4.value());
                    }
                }
            }
            if (!path.empty()) {
                Vector2D selectedPoint;
                float minDistSq = (std::numeric_limits<float>::max)();
                for (const auto &pt : path) {
                    float dx = pt.x - origine.x;
                    float dy = pt.y - origine.y;
                    float distSq = dx * dx + dy * dy;
                    if (distSq < minDistSq) {
                        minDistSq = distSq;
                        selectedPoint = pt;
                    }
                }
                printf("Selected point: (%.3f, %.3f)\n", selectedPoint.x, selectedPoint.y);
                
                
                this->player_plane->x = selectedPoint.x;
                this->player_plane->z = selectedPoint.y;
                
            } else {
                printf("No intersection points found in path.\n");
                
                this->player_plane->x = destination.x;
                this->player_plane->z = destination.y;
            }
            float dest_min_altitude = this->current_mission->area->getY(this->player_plane->x, this->player_plane->z);
            if (dest_y<dest_min_altitude) {
                this->player_plane->y += dest_min_altitude;
            } else {
                this->player_plane->y = dest_y;    
            }
            
            
            
            this->player_plane->ptw.Identity();
            this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);
            this->player_plane->ptw.rotateM(0, 0, 1, 0);
            this->player_plane->ptw.rotateM(0, 1, 0, 0);
            this->player_plane->ptw.rotateM(0, 0, 0, 1);
            this->player_plane->Simulate();
            Vector3D formation_pos_offset{80.0f, 0.0f, 40.0f};
            int team_number = 1;
            Vector3D prev={this->player_plane->x, this->player_plane->y, this->player_plane->z};
            for (auto team: this->current_mission->friendlies) {
                if (team->is_active) {
                    if (team->plane != nullptr && team->plane != this->player_plane) {
                        prev += formation_pos_offset;
                        team->taken_off = true;
                        team->plane->x = prev.x;
                        team->plane->z = prev.z;
                        team->plane->y = prev.y;
                        team->plane->yaw=0;
                        team->plane->pitch=0;
                        team->plane->roll=0;
                        team->plane->ptw.Identity();
                        team->plane->ptw.translateM(team->plane->x, team->plane->y, team->plane->z);
                        team->plane->Simulate();
                        team_number++;
                    }
                }
            }
            this->camera_mode = View::AUTO_PILOT;
            this->autopilot_timeout = 400;
            fflush(stdout);
            
        }
        break;
        
        case SDLK_SPACE:
            {
                if (target != nullptr) {
                    this->player_plane->Shoot(this->player_plane->selected_weapon, target, this->current_mission);
                }
            }
            break;
        case SDLK_t:
            {
                float minDistSq = (std::numeric_limits<float>::max)();
                int nearestIndex = -1;
                for (size_t i = 0; i < this->current_mission->enemies.size(); i++) {
                    if (i == this->current_target) {
                        continue;
                    }
                    auto enemy = this->current_mission->enemies[i];
                    if (this->current_mission->area->getY(enemy->object->position.x, enemy->object->position.z) > enemy->object->position.y) {
                        // under the ground
                        continue;
                    }
                    if (enemy->object->alive) {
                        float dx = enemy->object->position.x - this->player_plane->x;
                        float dy = enemy->object->position.y - this->player_plane->y;
                        float dz = enemy->object->position.z - this->player_plane->z;
                        float distSq = dx * dx + dy * dy + dz * dz;
                        if (distSq < minDistSq) {
                            minDistSq = distSq;
                            nearestIndex = static_cast<int>(i);
                        }
                    }
                }
                if (nearestIndex != -1) {
                    this->current_target = nearestIndex;
                    this->target = this->current_mission->enemies[nearestIndex];
                    this->cockpit->target = this->target->object;
                }
            }
            break;
        default:
            break;
        }
    }
    this->cockpit->mouse_control = this->mouse_control;
}
/**
 * SCStrike::Init
 *
 * Initialize the game state with default values and a starting mission.
 *
 * This function initializes the game state by setting the mouse control
 * flag to false, setting the mission to "TEMPLATE.IFF", initializing the
 * cockpit object, and setting the pilot's lookat position to the origin.
 */
void SCStrike::init(void) {
    this->mouse_control = false;
    this->pilote_lookat = {0, 0};
}

RSEntity * SCStrike::loadWeapon(std::string name) {
    std::string tmpname = Assets.object_root_path + name + ".IFF";
    RSEntity *objct = new RSEntity(&Assets);
    TreEntry *entry = Assets.GetEntryByName(tmpname);
    if (entry != nullptr) {
        objct->InitFromRAM(entry->data, entry->size);
        return objct;
    }
    return nullptr;
}

/**
 * SetMission
 *
 * Sets the current mission to the one specified in the missionName argument.
 *
 * This function initializes the game state by setting the mission to the one
 * specified in the missionName argument, clearing the list of AI planes,
 * initializing the cockpit object, and setting the pilot's lookat position to
 * the origin. It also loads the necessary data from the mission file, such as
 * the player's coordinates and the objects in the mission.
 *
 * @param[in] missionName The name of the mission file to load.
 */
void SCStrike::setMission(char const *missionName) {
    if (this->current_mission != nullptr) {
        this->current_mission->cleanup();
        delete this->current_mission;
        this->current_mission = nullptr;
    } 
    this->miss_file_name = missionName;
    this->current_mission = new SCMission(missionName, &object_cache);
    ai_planes.clear();
    ai_planes.shrink_to_fit();
    
    MISN_PART *playerCoord = this->current_mission->player->object;
    this->area = *this->current_mission->area;
    new_position.x = playerCoord->position.x;
    new_position.z = playerCoord->position.z;
    new_position.y = playerCoord->position.y;

    camera = Renderer.getCamera();
    camera->SetPosition(&new_position);
    this->current_target = 0;
    this->target = this->current_mission->enemies[this->current_target];
    this->nav_point_id = 0;
    this->player_plane = this->current_mission->player->plane;
    this->player_plane->yaw = (360 - playerCoord->azymuth) * 10.0f;
    this->player_plane->object = playerCoord;
    float ground = this->area.getY(new_position.x, new_position.z);
    if (ground < new_position.y) {
        this->player_plane->SetThrottle(100);
        this->player_plane->SetWheel();
        this->player_plane->vz = -20;
        this->player_plane->Simulate();
    }
    if (GameState.weapon_load_out.size()>1) {
        this->player_plane->object->entity->weaps.clear();
        std::map<weapon_type_shp_id, std::string> weapon_names = {
            {AIM9J, "SWINDERJ"},
            {AIM9M, "SWINDERM"},
            {AIM120, "AMRAAM"},
            {AGM65D, "AGM-65D"},
            {DURANDAL, "DURANDAL"},
            {MK20, "MK20"},
            {MK82, "MK82"},
            {GBU15, "GBU-15G"},
            {LAU3, "POD"}
        };
        RSEntity::WEAPS *weap = new RSEntity::WEAPS();
        weap->name = "20MM";
        weap->nb_weap = 1000;
        weap->objct = loadWeapon(weap->name);
        this->player_plane->object->entity->weaps.push_back(weap);
        std::map<weapon_type_shp_id, bool> loaded;
        for (int i=1; i<5; i++) {
            if (loaded.find(weapon_type_shp_id(GameState.weapon_load_out[i])) != loaded.end()) {
                continue;
            }
            RSEntity::WEAPS *weap = new RSEntity::WEAPS();
            weap->name = weapon_names[weapon_type_shp_id(GameState.weapon_load_out[i])];
            weap->nb_weap = GameState.weapon_load_out[GameState.weapon_load_out[i]];
            weap->objct = loadWeapon(weap->name);
            this->player_plane->object->entity->weaps.push_back(weap);
            loaded[weapon_type_shp_id(GameState.weapon_load_out[i])] = true;
        }
    }
    
    this->player_plane->InitLoadout();
    this->player_prof = this->current_mission->player->profile;
    this->cockpit = new SCCockpit();
    this->cockpit->init();
    this->cockpit->player_plane = this->player_plane;
    this->cockpit->current_mission = this->current_mission;
    this->cockpit->nav_point_id = &this->nav_point_id;
    Mixer.StopMusic();
    Mixer.SwitchBank(2);
    Mixer.PlayMusic(this->current_mission->mission->mission_data.tune+1);
}
void SCStrike::setCameraFront() {
    Vector3D pos = {this->new_position.x, this->new_position.y, this->new_position.z};
    camera->SetPosition(&pos);
    camera->ResetRotate();
    camera->Rotate(
        -tenthOfDegreeToRad(this->player_plane->elevationf),
        -tenthOfDegreeToRad(this->player_plane->azimuthf),
        -tenthOfDegreeToRad(this->player_plane->twist)
    );
}
void SCStrike::setCameraFollow(SCPlane *plane) {
    const float distanceBehind = -60.0f;
    float r_azim = tenthOfDegreeToRad(plane->azimuthf);
    float r_elev = tenthOfDegreeToRad(plane->elevationf-100.0f);
    float r_twist = tenthOfDegreeToRad(plane->twist);
    Vector3D camPos;
    camPos.x = plane->x - distanceBehind * cos(r_elev) * sin(r_azim);
    camPos.y = plane->y + distanceBehind * sin(r_elev);
    camPos.z = plane->z - distanceBehind * cos(r_elev) * cos(r_azim);
    camera->SetPosition(&camPos);

    Vector3D camLookAt = {plane->x, plane->y, plane->z};
    float cosT = cos(r_twist), sinT = sin(r_twist);
    float cosE = cos(r_elev), sinE = sin(r_elev);
    float cosA = cos(r_azim), sinA = sin(r_azim);

    // Compute the up vector as the second column of the composite rotation matrix
    // R = Ry(azim) * Rx(elev) * Rz(twist)
    Vector3D up;
    up.x = -cosA * sinT + sinA * sinE * cosT;
    up.y = cosE * cosT;
    up.z = sinA * sinT + cosA * sinE * cosT;
    if (follow_dynamic) {
        camera->LookAt(&camLookAt, &up);
    } else {
        camera->LookAt(&camLookAt);
    }
}
void SCStrike::setCameraRLR() {
    camera->SetPosition(&this->new_position);
    camera->ResetRotate();
    camera->Rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);
    camera->Rotate(
        -tenthOfDegreeToRad(this->player_plane->elevationf),
        -tenthOfDegreeToRad(this->player_plane->azimuthf),
        -tenthOfDegreeToRad(this->player_plane->twist)
    );
}
void SCStrike::setCameraLookat(Vector3D obj_pos) {
    Vector3D pos = {
        obj_pos.x + this->camera_pos.x,
        obj_pos.y + this->camera_pos.y,
        obj_pos.z + this->camera_pos.z
    };
    camera->SetPosition(&pos);
    camera->LookAt(&obj_pos);
}
/**
 * @brief Executes a single frame of the game simulation.
 *
 * This function performs the main loop operations for each frame of the game
 * simulation. It handles keyboard input check, updates the states of the player
 * plane and AI planes, adjusts the camera view based on the current mode, and
 * renders the game world and cockpit.
 *
 * Key operations include:
 * - Checking and processing keyboard inputs.
 * - Simulating the player and AI planes' movements and autopilot systems.
 * - Updating positions and orientations of planes and mission objects.
 * - Configuring the camera based on the selected view mode.
 * - Rendering the world, mission objects, and cockpit interface.
 * - Managing the display of cockpit elements like communication and weapons.
 */
void SCStrike::runFrame(void) {
    Mixer.setVolume(5,5);
    this->checkKeyboard();
    Renderer.setLight(&this->light);
    if (!this->pause_simu && this->camera_mode!=View::AUTO_PILOT) {
        this->mfd_timeout--;
        this->player_plane->Simulate();
        this->current_mission->update();
        if (this->current_mission->mission_ended) {
            GameState.missions_flags.clear();
            GameState.missions_flags.shrink_to_fit();
            // @TODO Rework this part when we will be dealing with the mission funds bonus
            for (auto flags: this->current_mission->gameflow_registers) {
                GameState.missions_flags.push_back(flags.second);
            }
            GameState.mission_flyed_success[GameState.mission_flyed] = this->current_mission->gameflow_registers[0];
            Renderer.clear();
            Screen->refresh();
            Renderer.clear();
            Game->stopTopActivity();
            return;
        }
    }
    this->player_plane->getPosition(&new_position);
    if (this->player_plane->object != nullptr) {
        this->player_plane->object->position.x = new_position.x;
        this->player_plane->object->position.z = new_position.z;
        this->player_plane->object->position.y = new_position.y;
    }
    this->cockpit->Update();
    if (this->mfd_timeout <= 0) {
        if (this->cockpit->show_comm) {
            this->cockpit->show_comm = false;
        }
        if (this->cockpit->show_weapons) {
            this->cockpit->show_weapons = false;
        }
    }

    if (this->target != nullptr) {
        if (this->target->object != nullptr) {
            Vector3D target_position = {
                this->target->object->position.x, 
                this->target->object->position.y, 
                this->target->object->position.z
            };
            this->setCameraLookat(target_position);
            Renderer.initRenderToTexture();
            Renderer.initRenderCameraView();
            Renderer.renderWorldToTexture(&area);
            
            this->player_plane->Render();
            if (this->target->plane != nullptr) {
                this->target->plane->Render();    
            } else {
                Renderer.drawModel(this->target->object->entity, target_position, {0.0f, 0.0f, 0.0f});
            }
            
            Renderer.getRenderToTexture();
        }
    }
    
    switch (this->camera_mode) {
    case View::AUTO_PILOT: {
        if (this->autopilot_timeout > -AUTOPILOTE_TIMEOUT) {
            this->autopilot_timeout -= AUTOPILOTE_SPEED;
            Vector3D pos = {this->new_position.x +5, this->new_position.y + 5, 
                this->new_position.z - (autopilot_timeout)};
            camera->SetPosition(&pos);
            camera->LookAt(&this->new_position);
        } else {
            this->player_plane->ptw.Identity();
            this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);
            this->player_plane->ptw.rotateM(degreeToRad(this->autopilot_target_azimuth), 0, 1, 0);
            this->player_plane->yaw = this->autopilot_target_azimuth * 10.0f;
            this->player_plane->Simulate();
            this->camera_mode = View::FRONT;
            for (auto team: this->current_mission->friendlies) {
                if (team->is_active) {
                    if (team->plane != nullptr && team->plane != this->player_plane) {                       
                        team->plane->yaw=this->autopilot_target_azimuth * 10.0f;
                    }
                }
            }
        }
    } break;
    case View::FRONT: {
        this->setCameraFront();
         // Apply a vertical offset to the projection matrix
    } break;
    case View::FOLLOW: {
        this->setCameraFollow(this->player_plane);
    } break;
    case View::RIGHT:
    case View::LEFT:
    case View::REAR:
        this->setCameraRLR();
        break;
    case View::TARGET: {
        setCameraLookat(this->target->object->position);
    }
    break;
    case View::OBJECT: {
        setCameraLookat(this->current_mission->actors[this->current_object_to_view]->object->position);
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
        const float distanceBehind = -60.0f;
        float r_azim = degreeToRad(missile->azimuthf);
        float r_elev = degreeToRad(missile->elevationf);
        float r_twist = 0.0f;
        Vector3D camPos;
        camPos.x = missil_pos.x - distanceBehind * cos(r_elev) * sin(r_azim);
        camPos.y = missil_pos.y + distanceBehind * sin(r_elev);
        camPos.z = missil_pos.z - distanceBehind * cos(r_elev) * cos(r_azim);
        camera->SetPosition(&camPos);
        camera->LookAt(&missil_pos);
    }
    break;
    case View::EYE_ON_TARGET: {
        Vector3D pos = {this->new_position.x, this->new_position.y + this->eye_y, this->new_position.z};
        float r_twist = tenthOfDegreeToRad(this->player_plane->twist);
        float r_elev  = tenthOfDegreeToRad(this->player_plane->elevationf);
        float r_azim  = tenthOfDegreeToRad(this->player_plane->azimuthf);
        float cosT = cos(r_twist), sinT = sin(r_twist);
        float cosE = cos(r_elev), sinE = sin(r_elev);
        float cosA = cos(r_azim), sinA = sin(r_azim);

        Vector3D camPos;
        camPos.x = this->new_position.x;
        camPos.y = this->new_position.y;
        camPos.z = this->new_position.z;
        
        // Compute the up vector as the second column of the composite rotation matrix
        // R = Ry(azim) * Rx(elev) * Rz(twist)
        Vector3D up;
        up.x = -cosA * sinT + sinA * sinE * cosT;
        up.y = cosE * cosT;
        up.z = sinA * sinT + cosA * sinE * cosT;
        
        Vector3D targetPos = {target->object->position.x, target->object->position.y,target->object->position.z };
        camera->SetPosition(&camPos);
        camera->LookAt(&targetPos, &up);
    } break;
    case View::REAL:
    default: {
        Vector3D pos = {this->new_position.x, this->new_position.y + this->eye_y, this->new_position.z};
        camera->SetPosition(&pos);
        camera->ResetRotate();

        camera->Rotate(-this->pilote_lookat.y * ((float)M_PI / 180.0f), 0.0f, 0.0f);
        camera->Rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);

        camera->Rotate(
            -tenthOfDegreeToRad(this->player_plane->elevationf),
            -tenthOfDegreeToRad(this->player_plane->azimuthf),
            -tenthOfDegreeToRad(this->player_plane->twist)
        );
    } break;
    }

    Renderer.bindCameraProjectionAndView(0.45f);
    Renderer.renderWorldSolid(&area, BLOCK_LOD_MAX, 400);

    glDisable(GL_TEXTURE_2D);
    if (this->show_bbox) {
        for (auto rrarea: this->current_mission->mission->mission_data.areas) {
            Renderer.renderLineCube(rrarea->position, rrarea->AreaWidth);
        }
    }
    
    for (auto actor: this->current_mission->actors) {
        if (actor->plane != nullptr) {
            if (actor->plane != this->player_plane) {
                Vector3D distance = {
                    actor->plane->x - this->player_plane->x,
                    actor->plane->y - this->player_plane->y,
                    actor->plane->z - this->player_plane->z
                };
                if (distance.Length() > RENDER_DISTANCE) {
                    continue; // Skip rendering if the plane is too far away
                }
                if (this->show_bbox) {
                    actor->plane->renderPlaneLined();
                    BoudingBox *bb = actor->plane->object->entity->GetBoudingBpx();
                    Vector3D position = {actor->plane->x, actor->plane->y, actor->plane->z};          
                    Vector3D orientation = {
                        actor->plane->azimuthf/10.0f + 90,
                        actor->plane->elevationf/10.0f,
                        -actor->plane->twist/10.0f
                    };
                    for (auto vertex: actor->plane->object->entity->vertices) {
                        if (vertex.x == bb->min.x) {
                            Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
                        }
                        if (vertex.x == bb->max.x) {
                            Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
                        }
                        if (vertex.z == bb->min.z) {
                            Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
                        }
                        if (vertex.z == bb->max.z) {
                            Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
                        }
                    }
                    Renderer.renderBBox(position, bb->min, bb->max);
                    Renderer.renderBBox(position+actor->formation_pos_offset, bb->min, bb->max);
                    Renderer.renderBBox(position+actor->attack_pos_offset, bb->min, bb->max);
                } else {
                    actor->plane->Render();
                }
                if (actor->plane->object->alive == false) {
                    actor->plane->RenderSmoke();
                }
            }   
        } else if (actor->object->entity != nullptr) {
            Vector3D actor_position = {actor->object->position.x, actor->object->position.y, actor->object->position.z};
            Vector3D actor_orientation = {(360.0f - static_cast<float>(actor->object->azymuth) + 90.0f), static_cast<float>(actor->object->pitch), -static_cast<float>(actor->object->roll)};
            Vector3D distance = {
                actor_position.x - this->player_plane->x,
                actor_position.y - this->player_plane->y,
                actor_position.z - this->player_plane->z
            };
            if (distance.Length() > RENDER_DISTANCE) {
                continue; // Skip rendering if the object is too far away
            }
            Renderer.drawModel(actor->object->entity, LOD_LEVEL_MAX, actor_position, actor_orientation);
            if (this->show_bbox) {
                if (actor->aiming_vector.x != 0.0f || actor->aiming_vector.y != 0.0f || actor->aiming_vector.z != 0.0f) {
                    Vector3D aim_pos = {actor->aiming_vector.x, actor->aiming_vector.y, actor->aiming_vector.z};
                    Renderer.drawLine(actor_position, aim_pos, {1.0f, 0.0f, 0.0f});
                }    
            }
            for (auto weapons : actor->weapons_shooted) {
                if (weapons->alive) {
                    weapons->Render();
                }
            }
            if (this->show_bbox) {
                BoudingBox *bb = actor->object->entity->GetBoudingBpx();
                Vector3D position = {actor->object->position.x, actor->object->position.y, actor->object->position.z};
                Renderer.renderBBox(position, bb->min, bb->max);
            }
            
        } else {
            printf("Actor has no plane or object\n");
        }
    }
    for (auto expl: this->current_mission->explosions) {
        if (expl->is_finished) {
            // Remove explosion when finished
            this->current_mission->explosions.erase(
                std::remove_if(this->current_mission->explosions.begin(),
                              this->current_mission->explosions.end(),
                              [](const auto& expl) { return expl->is_finished; }),
                this->current_mission->explosions.end());
            continue;
        }
        expl->render();
    }
    this->player_plane->RenderSimulatedObject();
    this->cockpit->cam = camera;
    switch (this->camera_mode) {
    case View::FRONT:
        this->cockpit->Render(0);
        break;
    case View::MISSILE_CAM:
    case View::TARGET:
    case View::OBJECT:
    case View::AUTO_PILOT:
    case View::FOLLOW:
        if (!this->show_bbox) {
            this->player_plane->Render();
        } else {
            this->player_plane->renderPlaneLined();
        }
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

        Vector3D cockpit_pos;
        cockpit_pos.x = this->camera->GetPosition().x;
        cockpit_pos.y = this->camera->GetPosition().y;
        cockpit_pos.z = this->camera->GetPosition().z;
        this->cockpit->RenderHUD();

        Vector3D cockpit_rot = {(this->player_plane->azimuthf+900)/10.0f, this->player_plane->elevationf/10.0f, -this->player_plane->twist/10.0f};
        Vector3D cockpit_ajustement = { 0.0f,-3.0f,0.0f};
        Texture *hud_texture = new Texture();
        hud_texture->animated = true;
        RSImage *hud_image = new RSImage();
        hud_image->palette = &this->cockpit->palette;
        hud_image->data = this->cockpit->hud_framebuffer->framebuffer;
        hud_image->width = this->cockpit->hud_framebuffer->width;
        hud_image->height = this->cockpit->hud_framebuffer->height;
        hud_texture->Set(hud_image);
        hud_texture->UpdateContent(hud_image);
        Renderer.drawModel(this->cockpit->cockpit->REAL.OBJS, LOD_LEVEL_MAX, cockpit_pos, cockpit_rot, cockpit_ajustement);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {5.8f, 2.0f, -1.22f}, 
                {5.8f, 2.0f, 1.35f},
                {6.0f, -0.8f, 1.35f},
                {6.0f, -0.8f, -1.22f}
            },
            hud_texture
        );
        Texture *mfd_right_texture = new Texture();
        mfd_right_texture->animated = true;
        RSImage *mfd_right_image = new RSImage();
        mfd_right_image->palette = &this->cockpit->palette;
        cockpit->RenderMFDSWeapon({0,0}, cockpit->mfd_right_framebuffer);
        mfd_right_image->data = this->cockpit->mfd_right_framebuffer->framebuffer;
        mfd_right_image->width = this->cockpit->mfd_right_framebuffer->width;
        mfd_right_image->height = this->cockpit->mfd_right_framebuffer->height;
        mfd_right_texture->Set(mfd_right_image);
        mfd_right_texture->UpdateContent(mfd_right_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.2f, -2.5f, -3.0f}, 
                {6.2f, -2.5f, -1.15f},
                {6.1f, -4.4f, -1.15f},
                {6.1f, -4.4f, -3.0f}
            },
            mfd_right_texture
        );
        cockpit->RenderMFDSRadar({0,0}, cockpit->radar_zoom*20000.0f, this->cockpit->radar_mode, cockpit->mfd_left_framebuffer);
        Texture *mfd_left_texture = new Texture();
        mfd_left_texture->animated = true;
        RSImage *mfd_left_image = new RSImage();
        mfd_left_image->palette = &this->cockpit->palette;
        mfd_left_image->data = this->cockpit->mfd_left_framebuffer->framebuffer;
        mfd_left_image->width = this->cockpit->mfd_left_framebuffer->width;
        mfd_left_image->height = this->cockpit->mfd_left_framebuffer->height;
        mfd_left_texture->Set(mfd_left_image);
        mfd_left_texture->UpdateContent(mfd_left_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.2f, -2.5f, 1.15f}, 
                {6.2f, -2.5f, 3.0f},
                {6.1f, -4.4f, 3.0f},
                {6.1f, -4.4f, 1.15f}
            },
            mfd_left_texture
        );
        cockpit->raws_framebuffer->FillWithColor(0);
        cockpit->RenderRAWS({0,0}, cockpit->raws_framebuffer);
        Texture *raws_texture = new Texture();
        raws_texture->animated = true;
        RSImage *raws_image = new RSImage();
        raws_image->palette = &this->cockpit->palette;
        raws_image->data = this->cockpit->raws_framebuffer->framebuffer;
        raws_image->width = this->cockpit->raws_framebuffer->width;
        raws_image->height = this->cockpit->raws_framebuffer->height;
        raws_texture->Set(raws_image);
        raws_texture->UpdateContent(raws_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.2f, -1.38f, -2.10f}, 
                {6.2f, -1.38f, -1.15f},
                {6.1f, -2.45f, -1.15f},
                {6.1f, -2.45f, -2.10f}
            },
            raws_texture
        );
        cockpit->target_framebuffer->FillWithColor(255);
        cockpit->RenderTargetWithCam({0,0}, cockpit->target_framebuffer);
        Texture *target_texture = new Texture();
        target_texture->animated = true;
        RSImage *target_image = new RSImage();
        target_image->palette = &this->cockpit->palette;
        target_image->data = this->cockpit->target_framebuffer->framebuffer;
        target_image->width = this->cockpit->target_framebuffer->width;
        target_image->height = this->cockpit->target_framebuffer->height;
        target_texture->Set(target_image);
        target_texture->UpdateContent(target_image);
        // Calculate a position in front of the camera
        // Create a fullscreen quad in front of the camera
        // Adjust for the camera Y-axis offset
        Vector3D quadPos = camera->GetPosition() + camera->GetForward() * 8.0f;
        
        // Set up vectors for billboard creation
        Vector3D forward = camera->GetForward();
        Vector3D right = camera->GetRight();
        Vector3D up = camera->GetUp();

        
        
        // Scale factors for quad size
        float width =4.92f;
        float height = 3.22f;
        // Compensate for the 0.45f Y-axis projection offset
        // We need to shift the entire quad upwards by adding an offset in the up direction
        float projectionYOffset = -1.5f;
        Vector3D offsetCompensation = up * projectionYOffset;
        
        // Apply the offset to the quad center position
        Vector3D adjustedQuadPos = quadPos + offsetCompensation;
        
        // Define the quad's corners in world space with the adjusted center position
        Vector3D topLeft = adjustedQuadPos + (up * height) - (right * width);
        Vector3D topRight = adjustedQuadPos + (up * height) + (right * width);
        Vector3D bottomRight = adjustedQuadPos - (up * height) + (right * width);
        Vector3D bottomLeft = adjustedQuadPos - (up * height) - (right * width);
        
        // Draw the quad with the target texture
        Renderer.drawTexturedQuad(
            {0, 0, 0}, // No position offset needed as we're using world coordinates
            {0, 0, 0}, // No rotation needed as we're manually setting vertices
            {
                topLeft,
                topRight,
                bottomRight,
                bottomLeft
            },
            target_texture
        );
        cockpit->alti_framebuffer->FillWithColor(0);
        cockpit->RenderAlti({0,0}, cockpit->alti_framebuffer);
        Texture *alti_texture = new Texture();
        alti_texture->animated = true;
        RSImage *alti_image = new RSImage();
        alti_image->palette = &this->cockpit->palette;
        alti_image->data = this->cockpit->alti_framebuffer->framebuffer;
        alti_image->width = this->cockpit->alti_framebuffer->width;
        alti_image->height = this->cockpit->alti_framebuffer->height;
        alti_texture->Set(alti_image);
        alti_texture->UpdateContent(alti_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.1f, -3.15f, 0.020f}, 
                {6.1f, -3.15f, 0.90f},
                {5.7f, -3.92f, 0.90f},
                {5.7f, -3.92f, 0.020f}
            },
            alti_texture
        );
        cockpit->speed_framebuffer->FillWithColor(0);
        cockpit->RenderSpeedOmetter({0,0}, cockpit->speed_framebuffer);
        Texture *speed_texture = new Texture();
        speed_texture->animated = true;
        RSImage *speed_image = new RSImage();
        speed_image->palette = &this->cockpit->palette;
        speed_image->data = this->cockpit->speed_framebuffer->framebuffer;
        speed_image->width = this->cockpit->speed_framebuffer->width;
        speed_image->height = this->cockpit->speed_framebuffer->height;
        speed_texture->Set(speed_image);
        speed_texture->UpdateContent(speed_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.1f, -3.15f, -0.90f}, 
                {6.1f, -3.15f, -0.020f},
                {5.7f, -3.92f, -0.020f},
                {5.7f, -3.92f, -0.90f}
            },
            speed_texture
        );
        break;
    }
}
