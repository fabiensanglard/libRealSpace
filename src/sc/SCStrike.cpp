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
// Function to check if two line segments intersect.
#include <optional>
#include <cmath>
#define SC_WORLD 1100
#define AUTOPILOTE_TIMEOUT 400
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
    if (this->camera_mode == View::AUTO_PILOT) {
        return;
    }
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
            nav_screen->Init();
            nav_screen->SetName((char *)this->current_mission->world->tera.c_str());
            nav_screen->mission = this->current_mission;
            nav_screen->missionObj = this->current_mission->mission = this->current_mission->mission;
            nav_screen->current_nav_point = &this->nav_point_id;
            Game.AddActivity(nav_screen);
        } break;
        case SDLK_a:
        {
            Vector2D destination = {this->current_mission->waypoints[this->nav_point_id]->spot->position.x,
                                    this->current_mission->waypoints[this->nav_point_id]->spot->position.z};

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
                    if (pointInRect(origine) || pointInRect(destination)) {
                        printf("Collision detected with area: %s\n", area->AreaName);
                        continue;
                    }

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
                this->autopilot_target_azimuth = atan2(destination.y - this->player_plane->z, destination.x - this->player_plane->x);
                this->player_plane->x = selectedPoint.x;
                this->player_plane->z = selectedPoint.y;
                float desty = this->current_mission->area->getY(this->player_plane->x, this->player_plane->z);
                if (this->player_plane->y < desty) {
                    this->player_plane->y += desty;    
                }
            } else {
                printf("No intersection points found in path.\n");
                this->autopilot_target_azimuth = atan2(destination.y - this->player_plane->z, destination.x - this->player_plane->x);
                this->player_plane->x = destination.x;
                this->player_plane->z = destination.y;
                float desty = this->current_mission->area->getY(this->player_plane->x, this->player_plane->z);
                if (this->player_plane->y < desty) {
                    this->player_plane->y = desty;    
                }
            }
            this->autopilot_target_azimuth = this->autopilot_target_azimuth * 180.0f / (float)M_PI;
            this->autopilot_target_azimuth -= 360;
            this->autopilot_target_azimuth += 90;
            if (this->autopilot_target_azimuth > 360) {
                this->autopilot_target_azimuth -= 360;
            }
            while (this->autopilot_target_azimuth < 0) {
                this->autopilot_target_azimuth += 360;
            }
            
            this->player_plane->ptw.Identity();
            this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);
            this->player_plane->ptw.rotateM(0, 0, 1, 0);
            this->player_plane->ptw.rotateM(0, 1, 0, 0);
            this->player_plane->ptw.rotateM(0, 0, 0, 1);
            this->player_plane->Simulate();
            int team_number = 1;
            for (auto team: this->current_mission->friendlies) {
                if (team->is_active) {
                    if (team->plane != nullptr) {
                        team->plane->x = this->player_plane->x + 50+team_number*5;
                        team->plane->z = this->player_plane->z;
                        team->plane->y = this->player_plane->y;
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
                    this->player_plane->Shoot(this->player_plane->selected_weapon, target);
                }
            }
            break;
        case SDLK_t:
            this->current_target = (this->current_target + 1) % this->current_mission->enemies.size();
            this->target = this->current_mission->enemies[this->current_target];
            this->cockpit->target = this->target->object;
            break;
        default:
            break;
        }
    }
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
void SCStrike::Init(void) {
    this->mouse_control = false;
    this->SetMission("TEMPLATE.IFF");
    this->pilote_lookat = {0, 0};
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
void SCStrike::SetMission(char const *missionName) {
    if (this->current_mission != nullptr) {
        this->current_mission->cleanup();
        delete this->current_mission;
        this->current_mission = nullptr;
    } 
    this->miss_file_name = missionName;
    this->current_mission = new SCMission(missionName, &objectCache);
    ai_planes.clear();
    ai_planes.shrink_to_fit();
    
    MISN_PART *playerCoord = this->current_mission->player->object;
    this->area = *this->current_mission->area;
    newPosition.x = playerCoord->position.x;
    newPosition.z = playerCoord->position.z;
    newPosition.y = playerCoord->position.y;

    camera = Renderer.GetCamera();
    camera->SetPosition(&newPosition);
    this->current_target = 0;
    this->target = this->current_mission->enemies[this->current_target];
    this->nav_point_id = 0;
    this->player_plane = this->current_mission->player->plane;
    this->player_plane->azimuthf = (360 - playerCoord->azymuth) * 10.0f;
    this->player_plane->yaw = (360 - playerCoord->azymuth) * (float) M_PI / 180.0f;
    this->player_plane->object = playerCoord;
    if (this->area.getY(newPosition.x, newPosition.z) < newPosition.y) {
        this->player_plane->SetThrottle(100);
        this->player_plane->SetWheel();
        this->player_plane->vz = -20;
        this->player_plane->Simulate();
    }
    this->player_plane->InitLoadout();
    this->player_prof = this->current_mission->player->profile;
    for (auto actor: this->current_mission->actors) {
        SCAiPlane *aiPlane = new SCAiPlane();
        if (actor->plane != nullptr) {
            aiPlane->plane = actor->plane;
            aiPlane->plane->InitLoadout();
            aiPlane->pilot = actor->pilot;
            aiPlane->object = actor->object;
            this->ai_planes.push_back(aiPlane);
        }
    }
    this->cockpit = new SCCockpit();
    this->cockpit->Init();
    this->cockpit->player_plane = this->player_plane;
    this->cockpit->current_mission = this->current_mission;
    this->cockpit->nav_point_id = &this->nav_point_id;
    Mixer.SwitchBank(2);
    Mixer.PlayMusic(this->current_mission->mission->mission_data.tune+1);
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
void SCStrike::RunFrame(void) {
    this->CheckKeyboard();
    if (!this->pause_simu && this->camera_mode!=View::AUTO_PILOT) {
        this->mfd_timeout--;
        this->player_plane->Simulate();
        this->current_mission->update();
        if (this->current_mission->mission_ended) {
            GameState.missions_flags.clear();
            GameState.missions_flags.shrink_to_fit();
            for (auto flags: this->current_mission->mission->mission_data.flags) {
                GameState.missions_flags.push_back(flags);
            }
            GameState.mission_flyed_success[GameState.mission_flyed] = this->current_mission->mission->mission_data.flags[0];
            Game.StopTopActivity();
        }
    }
    this->player_plane->getPosition(&newPosition);
    if (this->player_plane->object != nullptr) {
        this->player_plane->object->position.x = newPosition.x;
        this->player_plane->object->position.z = newPosition.z;
        this->player_plane->object->position.y = newPosition.y;
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
    switch (this->camera_mode) {
    case View::AUTO_PILOT: {
        if (this->autopilot_timeout > -AUTOPILOTE_TIMEOUT) {
            this->autopilot_timeout-=2;
            Vector3D pos = {this->newPosition.x +5, this->newPosition.y + 5, 
                this->newPosition.z - (autopilot_timeout-(autopilot_timeout/2))};
            camera->SetPosition(&pos);
            camera->LookAt(&this->newPosition);
        } else {
            this->player_plane->ptw.Identity();
            this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);
            this->player_plane->ptw.rotateM(this->autopilot_target_azimuth*(float) M_PI /180.0f, 0, 1, 0);
            this->player_plane->Simulate();
            this->camera_mode = View::FRONT;
        }
    } break;
    case View::FRONT: {
        Vector3D pos = {this->newPosition.x, this->newPosition.y, this->newPosition.z+10};
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
        Vector3D pos = {target->object->position.x + this->camera_pos.x, target->object->position.y + this->camera_pos.y,
                        target->object->position.z + this->camera_pos.z};
        Vector3D targetPos = {target->object->position.x, target->object->position.y, target->object->position.z};
        camera->SetPosition(&pos);
        camera->LookAt(&targetPos);
    }
    break;
    case View::OBJECT: {
        Vector3D obj_pos = this->current_mission->actors[this->current_object_to_view]->object->position;
        Vector3D pos = {obj_pos.x + this->camera_pos.x, obj_pos.y + this->camera_pos.y,
                        obj_pos.z + this->camera_pos.z};
        camera->SetPosition(&pos);
        camera->LookAt(&obj_pos);
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
        Vector3D pos = {this->newPosition.x, this->newPosition.y + 1, this->newPosition.z + 1};
        Vector3D targetPos = {target->object->position.x, target->object->position.y,target->object->position.z };
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
    if (this->show_bbox) {
        for (auto rrarea: this->current_mission->mission->mission_data.areas) {
            Renderer.RenderLineCube(rrarea->position, rrarea->AreaWidth);
        }
    }
    
    for (auto actor: this->current_mission->actors) {
        if (actor->plane != nullptr) {
            if (actor->plane != this->player_plane) {
                actor->plane->Render();
                BoudingBox *bb = actor->plane->object->entity->GetBoudingBpx();
                Vector3D position = {actor->plane->x, actor->plane->y, actor->plane->z};
                if (this->show_bbox) {
                    Renderer.RenderBBox(position, bb->min, bb->max);
                    Renderer.RenderBBox(position+actor->formation_pos_offset, bb->min, bb->max);
                    Renderer.RenderBBox(position+actor->attack_pos_offset, bb->min, bb->max);
                }
                if (actor->plane->object->alive == false) {
                     actor->plane->RenderSmoke();
                }
            }   
        } else if (actor->object->entity != nullptr) {
            glPushMatrix();

            glTranslatef(static_cast<GLfloat>(actor->object->position.x), static_cast<GLfloat>(actor->object->position.y),
                        static_cast<GLfloat>(actor->object->position.z));

            float model_view_mat[4][4];
            glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)model_view_mat);
            glRotatef((360.0f - (float)actor->object->azymuth + 90.0f), 0, 1, 0);
            glRotatef((float)actor->object->pitch, 0, 0, 1);
            glRotatef(-(float)actor->object->roll, 1, 0, 0);
            Renderer.DrawModel(actor->object->entity, LOD_LEVEL_MAX);
            glPopMatrix();
        } else {
            printf("Actor has no plane or object\n");
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
    case View::OBJECT:
    case View::AUTO_PILOT:
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
        cockpit_rotation.translateM(this->player_plane->x, (this->player_plane->y) - 2.0f,
                                    this->player_plane->z);
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


/**
 * Renders the in-game menu using the ImGui library. This menu provides various 
 * options and displays related to the simulation, such as controls for simulation 
 * settings, camera views, cockpit information, mission details, and AI pilot 
 * settings. It updates the menu states and handles user interactions with 
 * different menu items dynamically based on the current game context.
 */
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
    static bool show_music_player = false;
    static bool show_ai = false;
    static bool go_to_nav = false;
    static int altitude = 0;
    static int azimuth = 0;
    static int throttle = 0;
    static int speed = 0;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("SCStrike")) {
            ImGui::MenuItem("Mission", NULL, &show_mission);
            ImGui::MenuItem("Mission part and Area", NULL, &show_mission_parts_and_areas);
            ImGui::MenuItem("Music", NULL, &show_music_player);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Simulation", NULL, &show_simulation);
            ImGui::MenuItem("Simulation config", NULL, &show_simulation_config);
            ImGui::MenuItem("Camera", NULL, &show_camera);
            ImGui::MenuItem("Cockpit", NULL, &show_cockpit);
            ImGui::MenuItem("Show BBox", NULL, &this->show_bbox);
            ImGui::MenuItem("Ai pilot", NULL, &show_ai);
            ImGui::EndMenu();
        }
        int sceneid = -1;
        Vector3D plane_player_pos = {this->player_plane->x, this->player_plane->y, this->player_plane->z};
        uint8_t area_id = this->current_mission->getAreaID(plane_player_pos);
        ImGui::Text("Speed %d\tAltitude %.0f\tHeading %.0f\tTPS: %03d\tArea %s\tfilename: %s\tArea ID: %d",
                    this->player_plane->airspeed, this->newPosition.y,
                    360 - (this->player_plane->azimuthf / 10.0f), this->player_plane->tps,
                    this->current_mission->mission->mission_data.name.c_str(), this->miss_file_name.c_str(),area_id);
        ImGui::EndMainMenuBar();
    }
    if (show_ai) {
        ImGui::Begin("AI");
        for (auto ai_actor: this->current_mission->actors) {
            if (ai_actor->pilot != nullptr) {
                ImGui::Text("AI %s", ai_actor->object->member_name.c_str());
                ImGui::SameLine();
                ImGui::Text("Speed %d\tAltitude %.0f\tHeading %.0f", ai_actor->plane->airspeed, ai_actor->plane->y,
                            360 - (ai_actor->plane->azimuthf / 10.0f));
                ImGui::SameLine();
                if (ai_actor->pilot != nullptr) {
                    ImGui::Text("target heading %.0f", ai_actor->pilot->target_azimut);
                    ImGui::SameLine();
                    ImGui::Text("AI OBJ %d", ai_actor->current_objective);
                    ImGui::SameLine();
                    ImGui::Text("AI Target %d", ai_actor->current_target);
                }
                ImGui::SameLine();
                ImGui::Text("X %.0f Y %.0f Z %.0f", ai_actor->plane->x, ai_actor->plane->y, ai_actor->plane->z);
            }
        }
        ImGui::End();
    }
    if (show_music_player) {
        ImGui::Begin("Music Player");
        if (ImGui::BeginCombo("Music list", 0, 0)) {
            for (int i = 0; i < Mixer.music->musics[2].size(); i++) {
                if (ImGui::Selectable(std::to_string(i).c_str(), false))
                   Mixer.PlayMusic(i);
            }
            ImGui::EndCombo();
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
            this->pilot.target_azimut = (float) azimuth;
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
            go_to_nav = false;
        }
        if (go_to_nav) {
            this->pilot.plane = this->player_plane;
            this->pilot.target_speed = -speed;
            this->pilot.SetTargetWaypoint(this->current_mission->waypoints[this->nav_point_id]->spot->position);
            this->pilot.AutoPilot();
        }
        ImGui::SameLine();
        if (ImGui::Button("Go To Waypoint")) {
            this->pilot.plane = this->player_plane;
            this->pilot.SetTargetWaypoint(this->current_mission->waypoints[this->nav_point_id]->spot->position);
            go_to_nav = !go_to_nav;
            this->autopilot = false;
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
        ImGui::Text("Target %s", this->current_mission->actors[this->current_target]->actor_name.c_str());
        Vector3D targetPos = {this->target->object->position.x, this->target->object->position.y, this->target->object->position.z};
        Vector3D playerPos = {this->newPosition.x, this->newPosition.y, this->newPosition.z};

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
                    ImGui::Text("Target X:%.0f\tY:%.0f\tZ:%.0f", missils->target->object->position.x, missils->target->object->position.y, missils->target->object->position.z);    
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
        ImGui::Text("Mission %s", this->current_mission->mission->mission_data.name.c_str());
        ImGui::Text("Area %s", this->current_mission->mission->mission_data.world_filename.c_str());
        ImGui::Text("Player Coord %.0f %.0f %.0f", this->current_mission->mission->getPlayerCoord()->position.x, this->current_mission->mission->getPlayerCoord()->position.y,
                    this->current_mission->mission->getPlayerCoord()->position.z);
        ImGui::Text("Messages %d", this->current_mission->mission->mission_data.messages.size());
        if (ImGui::TreeNode("Messages")) {
            for (auto msg : this->current_mission->mission->mission_data.messages) {
                ImGui::Text("- %s", msg->c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Areas")) {
            for (auto area : this->current_mission->mission->mission_data.areas) {
                if (ImGui::TreeNode((void *)(intptr_t)area->id, "Area id %d", area->id)) {
                    ImGui::Text("Area name %s", area->AreaName);
                    ImGui::Text("Area x %.0f y %.0f z %.0f", area->position.x, area->position.y, area->position.z);
                    ImGui::Text("Area width %d height %d", area->AreaWidth, area->AreaHeight);
                    for (auto ub: area->unknown_bytes) {
                        ImGui::Text("ub %d", ub);
                    }
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
                    ImGui::Text("Object x %.3f y %.3f z %.3f", obj->position.x, obj->position.y, obj->position.z);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("SPOT")) {
            for (auto spot : this->current_mission->mission->mission_data.spots) {
                if (ImGui::TreeNode((void *)(intptr_t)spot, "Spot %d", spot->id)) {
                    ImGui::Text("Spot area_id %d", spot->area_id);
                    ImGui::Text("Spot x %.0f y %.0f z %.0f", spot->position.x, spot->position.y, spot->position.z);
                    ImGui::Text("Spot from file x %.0f y %.0f z %.0f", spot->origin.x, spot->origin.y, spot->origin.z);
                    ImGui::Text("u1 %d", spot->unknown1);
                    ImGui::Text("u2 %d", spot->unknown2);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("PROG")) {
            int id=1;
            for (auto prog : this->current_mission->mission->mission_data.prog) {
                if (ImGui::TreeNode((void *)(intptr_t)prog, "Prog %d", id)) {
                    for (auto op : *prog) {
                        ImGui::Text("opcode:[%d]\t\targ:[%d]", op.opcode, op.arg);
                    }
                    ImGui::TreePop();
                }
                id++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Flags")) {
            int cpt_flags = 0;
            for (auto flag : this->current_mission->mission->mission_data.flags) {
                ImGui::Text("Flag %d : %d", cpt_flags, flag);
                cpt_flags++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Scenes")) {
            int cpt_scenes = 0;
            for (auto scene : this->current_mission->mission->mission_data.scenes) {
                if (ImGui::TreeNode((void *)(intptr_t)scene, "Scene %d", cpt_scenes)) {
                    if (scene->area_id >= 0) {
                        ImGui::Text("Scene area_id %s", this->current_mission->mission->mission_data.areas[scene->area_id]->AreaName);
                    } else {
                        ImGui::Text("Scene area_id %d", scene->area_id);
                    }
                    ImGui::Text("IS ACTIVE %d", scene->is_active);
                    ImGui::Text("Is COORD RELATIVE TO AREA %d", scene->is_coord_on_area);
                    for (auto cast : scene->cast) {
                        ImGui::Text("Cast %d", cast);
                    }
                    for (auto prog : scene->progs_id) {
                        ImGui::Text("Prog %d", prog);
                    }
                    for (auto b: scene->unknown_bytes) {
                        ImGui::Text("ub %d", b);
                    }
                    ImGui::TreePop();
                }
                cpt_scenes++;
            }
            ImGui::TreePop();
        }
        ImGui::Text("Mission Parts %d", this->current_mission->mission->mission_data.parts.size());
        if (ImGui::TreeNode("Parts")) {
            for (auto part : this->current_mission->mission->mission_data.parts) {
                if (ImGui::TreeNode((void *)(intptr_t)part->id, "Parts id %d, area id %d", part->id, part->area_id)) {
                    ImGui::Text("u1 %d", part->unknown1);
                    ImGui::Text("u2 %d", part->unknown2);
                    ImGui::Text("x %.0f y %.0f z %.0f", part->position.x, part->position.y, part->position.z);
                    ImGui::Text("azymuth %d", part->azymuth);
                    ImGui::Text("u3 %d", part->unknown3);
                    ImGui::Text("Name %s", part->member_name.c_str());
                    ImGui::Text("Destroyed %s", part->member_name_destroyed.c_str());
                    for (auto obj : part->unknown_bytes) {
                        ImGui::Text("ub %d", obj);
                    }
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
        if (ImGui::TreeNode("Mission Actors")) {
            for (auto actor : current_mission->actors) {
                if (ImGui::TreeNode((void *)(intptr_t)actor, "Actor %d", actor->actor_id)) {
                    if (actor->profile != nullptr) {
                        ImGui::Text("Name %s", actor->profile->radi.info.name.c_str());
                        ImGui::Text("CallSign %s", actor->profile->radi.info.callsign.c_str());
                        ImGui::Text("Is AI %d", actor->profile->ai.isAI);
                        if (ImGui::TreeNode("MSGS")) {
                            for (auto msg : actor->profile->radi.msgs) {
                                ImGui::Text("- [%d]: %s", msg.first, msg.second.c_str());
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("ASKS")) {
                            for (auto msg : actor->profile->radi.asks) {
                                ImGui::Text("- [%s]: %s", msg.first.c_str(), msg.second.c_str());
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("AI MVRS")) {
                            for (auto ai : actor->profile->ai.mvrs) {
                                ImGui::Text("NODE [%d] - [%d]", ai.node_id, ai.value);
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("AI GOAL")) {
                            for (auto goal : actor->profile->ai.goal) {
                                ImGui::Text("[%d]", goal);
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("AI ATRB")) {
                            ImGui::Text("TH %d", actor->profile->ai.atrb.TH);
                            ImGui::Text("CN %d", actor->profile->ai.atrb.CN);
                            ImGui::Text("VB %d", actor->profile->ai.atrb.VB);
                            ImGui::Text("LY %d", actor->profile->ai.atrb.LY);
                            ImGui::Text("FL %d", actor->profile->ai.atrb.FL);
                            ImGui::Text("AG %d", actor->profile->ai.atrb.AG);
                            ImGui::Text("AA %d", actor->profile->ai.atrb.AA);
                            ImGui::Text("SM %d", actor->profile->ai.atrb.SM);
                            ImGui::Text("AR %d", actor->profile->ai.atrb.AR);
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("Score")) {
                            ImGui::Text("Score %d", actor->score);
                            ImGui::Text("Plane %d", actor->plane_down);
                            ImGui::Text("Ground %d", actor->ground_down);
                            ImGui::TreePop();
                        }
                    } else {
                        ImGui::Text("Name %s", actor->actor_name.c_str());
                        ImGui::Text("No profile");
                    }
                    if (ImGui::TreeNode("On is activated")) {
                        int cpt=0;
                        if (actor->on_is_activated.size() > 0) {
                            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_activated, "Prog %d", cpt)) {
                                for (auto opcodes: actor->on_is_activated) {
                                    ImGui::Text("OPCODE [%d]\t\tARG [%d]", opcodes.opcode, opcodes.arg);
                                }
                                ImGui::TreePop();
                            }
                            cpt++;
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("On update")) {
                        int cpt=0;
                        if (actor->on_update.size() > 0) {
                            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_update, "Prog %d", cpt)) {
                                for (auto opcodes: actor->on_update) {
                                    ImGui::Text("OPCODE [%d]\t\tARG [%d]", opcodes.opcode, opcodes.arg);
                                }
                                ImGui::TreePop();
                            }
                            cpt++;
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("On is destroyed")) {
                        int cpt=0;
                        if (actor->on_is_destroyed.size() > 0) {
                            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_destroyed, "Prog %d", cpt)) {
                                for (auto opcodes: actor->on_is_destroyed) {
                                    ImGui::Text("OPCODE [%d]\t\tARG [%d]", opcodes.opcode, opcodes.arg);
                                }
                                ImGui::TreePop();
                            }
                            cpt++;
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("On mission start")) {
                        int cpt=0;
                        if (actor->on_mission_start.size() > 0) {
                            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_destroyed, "Prog %d", cpt)) {
                                for (auto opcodes: actor->on_mission_start) {
                                    ImGui::Text("OPCODE [%d]\t\tARG [%d]", opcodes.opcode, opcodes.arg);
                                }
                                ImGui::TreePop();
                            }
                            cpt++;
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Casting")) {
            for (auto cast : this->current_mission->mission->mission_data.casting) {
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
