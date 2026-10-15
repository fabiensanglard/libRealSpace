#include "PSMission.h"
#include "PSMissionParser.h"

PSMission::PSMission() {
    this->last_time = SDL_GetTicks();
    this->last_tick = 0;
    this->tick_counter = 0;
    this->tps = 0;
}
PSMission::PSMission(std::string mission_name, std::unordered_map<std::string, RSEntity *> *objCache) {
    this->mission_name = mission_name;
    this->obj_cache = objCache;
    this->last_time = SDL_GetTicks();
    this->last_tick = 0;
    this->tick_counter = 0;
    this->tps = 0;
    this->loadMission();
}

PSMission::~PSMission() {}

void PSMission::loadMission() {
    std::string miss_file_name = Assets.mission_root_path + this->mission_name; 
    std::transform(miss_file_name.begin(), miss_file_name.end(), miss_file_name.begin(), ::toupper);
    TreEntry *mission_tre = Assets.GetEntryByName(miss_file_name.c_str());
    this->mission = new PSMissionParser();
    this->mission->InitFromRAM(mission_tre->data, mission_tre->size);


    std::string area_filename = Assets.mission_root_path+this->mission->mission_data.world_filename + ".IFF";
    std::transform(area_filename.begin(), area_filename.end(), area_filename.begin(), ::toupper);   
    this->world = new RSWorld();
    TreEntry *treEntry = NULL;
    treEntry = Assets.GetEntryByName(area_filename.c_str());
    if (treEntry != NULL) {
        this->world->InitFromRAM(treEntry->data, treEntry->size);
    }
    std::string area_fn = "..\\..\\DATA\\TERRAIN\\"+this->world->tera+".ZIP";
    std::transform(area_fn.begin(), area_fn.end(), area_fn.begin(), ::toupper);
    this->area = new RSArea();
    this->area->InitFromZipFileName(area_fn);

    Renderer.InvalidateAABBCache();           // ou InvalidateAABBCache(oldArea);
    Renderer.PrecomputeAABBs(this->area, 0, 1);  // optionnel: pré-calc LOD 0-1
    for (int i=0; i<256; i++) {
        this->mission->mission_data.flags.push_back(0);
    }
    for (auto &area_entity: this->area->objects) {
        area_entity.entity = LoadEntity(area_entity.name);
    }
    int cpt_actor=0;
    for (auto part : mission->mission_data.parts) {
        int search_id = 0;
        if (part->entity == nullptr) {
            part->entity = LoadEntity(part->member_name);
        }
        for (auto cast : mission->mission_data.casting) {
            if (part->id == search_id) {
                SCMissionActors *actor = new SCMissionActors();
                if (cast->actor == "PLAYER") {
                    actor = new SCMissionActorsPlayer();
                    if (part->area_id != 255 && part->unknown2 == 0) {
                        Vector3D correction = this->mission->mission_data.areas[part->area_id]->position;
                        part->position.x += correction.x;
                        part->position.y += correction.y;
                        part->position.z += correction.z;
                    }
                } else if (cast->actor == "STRIBASE") {
                    actor = new SCMissionActorsStrikeBase();
                }
                if (cast->actor == "TEAM0") {
                    cast->actor = GameState.wingman;
                }
                actor->team_id = part->unknown_bytes[2];
                actor->actor_name = cast->actor;
                actor->actor_id = part->id;
                actor->object = part;
                if (part->entity == nullptr) {
                    actor->health = 50;
                } else {
                    actor->health = part->entity->health;
                }
                actor->profile = this->LoadProfile(cast->actor);
                actor->mission = this;
                if (actor->object->on_is_activated != 255 && this->mission->mission_data.prog.size() > actor->object->on_is_activated) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_is_activated]) {
                        actor->on_is_activated.push_back(op);
                    }
                }
                if (actor->object->on_is_destroyed != 255 && this->mission->mission_data.prog.size() > actor->object->on_is_destroyed) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_is_destroyed]) {
                        actor->on_is_destroyed.push_back(op);
                    }
                }
                if (actor->object->on_missions_init != 255 && this->mission->mission_data.prog.size() > actor->object->on_missions_init) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_missions_init]) {
                        actor->on_mission_start.push_back(op);
                    }
                }
                if (actor->object->on_mission_update != 255 && this->mission->mission_data.prog.size() > actor->object->on_mission_update) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_mission_update]) {
                        actor->on_update.push_back(op);
                    }
                }
                
                if (actor->profile != nullptr && actor->profile->ai.isAI && actor->object->entity != nullptr && actor->object->entity->jdyn != nullptr && cast->actor != "PLAYER") {
                    if (actor->profile->ai.goal.size() > 0) {
                        actor->pilot = new SCPilot();
                        actor->pilot->actor = actor;
                        BoudingBox *bb = actor->object->entity->GetBoudingBpx();
                        
                        actor->plane = new SCJdynPlane(
                            10.0f,
                            -7.0f,
                            40.0f,
                            40.0f,
                            30.0f,
                            100.0f,
                            actor->object->entity->wing_area,
                            (float) actor->object->entity->weight_in_kg,
                            (float) actor->object->entity->jdyn->fuel_capacity,
                            (float) actor->object->entity->thrust_in_newton,
                            sqrtf(actor->object->entity->wing_area / ((float)M_PI * 0.83f * actor->object->entity->jdyn->induced_drag_k)),
                            actor->object->entity->jdyn->induced_drag_k,
                            (int) actor->object->entity->jdyn->stall_speed_ms,
                            this->area,
                            part->position.x,
                            part->position.y,
                            part->position.z
                        );
                        actor->plane->yaw = (360 - part->azymuth) * 10.0f;
                        
                        part->weapon_load.shrink_to_fit();
                        if (part->weapon_load.size() > 0) {
                            std::string weapon_path_name = Assets.object_root_path + part->weapon_load + ".IFF";
                            std::transform(weapon_path_name.begin(), weapon_path_name.end(), weapon_path_name.begin(), ::toupper);
                            TreEntry *weapon_entry = Assets.GetEntryByName(weapon_path_name.c_str());
                            part->entity->weaps.clear();
                            part->entity->weaps.shrink_to_fit();
                            part->entity->parseREAL_OBJT_JETP(weapon_entry->data, weapon_entry->size);   
                        }
                        actor->plane->object = part;
                        actor->plane->InitLoadout();
                        actor->plane->pilot = actor;
                        if (abs(this->area->getY(part->position.x, part->position.z)-part->position.y) <= 10 ) {
                            part->position.y = this->area->getY(part->position.x, part->position.z);
                            actor->plane->on_ground = true;
                        } else {
                            actor->plane->on_ground = false;
                        }
                        if (part->position.y < this->area->getY(part->position.x, part->position.z)) {
                            actor->plane->on_ground = true;
                        }
                        if (!actor->plane->on_ground) {
                            actor->plane->SetThrottle(100);
                            actor->pilot->target_climb = (int) (part->position.y);
                            actor->plane->vz = -20;
                            actor->pilot->target_azimut = actor->plane->azimuthf / 10.0f;
                            actor->pilot->target_speed = -20;
                        } else {
                            actor->plane->SetThrottle(0);
                            actor->pilot->target_climb = 0;
                            actor->plane->vz = 0;
                            actor->pilot->target_azimut = actor->plane->azimuthf / 10.0f;
                            actor->pilot->target_speed = 0;
                        }
                        actor->pilot->plane = actor->plane;
                    }
                    this->actors.push_back(actor);
                } else if (actor->profile != nullptr && cast->actor == "PLAYER") {
                    /*actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                part->position.y, part->position.z);*/
                    BoudingBox *bb = actor->object->entity->GetBoudingBpx();
                    actor->plane = new SCJdynPlane(
                        10.0f,
                        -7.0f,
                        40.0f,
                        40.0f,
                        30.0f,
                        100.0f,
                        actor->object->entity->wing_area,
                        (float) actor->object->entity->weight_in_kg,
                        (float) actor->object->entity->jdyn->fuel_capacity,
                        (float) actor->object->entity->thrust_in_newton,
                        sqrtf(actor->object->entity->wing_area / ((float)M_PI * 0.83f * actor->object->entity->jdyn->induced_drag_k)),
                        actor->object->entity->jdyn->induced_drag_k,
                        (int) actor->object->entity->jdyn->stall_speed_ms,
                        this->area,
                        part->position.x,
                        part->position.y,
                        part->position.z
                    );
                    actor->plane->yaw = (360 - part->azymuth) * 10.0f;
                    actor->plane->simple_simulation = false;
                    actor->plane->yaw = (360 - part->azymuth) * (float) M_PI / 180.0f;
                    actor->plane->object = part;
                    actor->plane->pilot = actor;
                    this->actors.push_back(actor);
                    this->player = actor;
                } else {
                    this->actors.push_back(actor);
                }
                cpt_actor++;
                break;
            }
            search_id++;
        }
    }
    for (auto area_actor: this->area->objects) {
        SCMissionActors *actor = new SCMissionActors();
        MISN_PART *part = new MISN_PART();
        part->id = cpt_actor++;
        part->member_name = area_actor.name;
        part->member_name_destroyed = area_actor.destroyedName;
        part->entity = area_actor.entity;
        part->position = area_actor.position;
        actor->actor_name = area_actor.name;
        actor->plane = nullptr;
        actor->pilot = nullptr;
        actor->actor_id = part->id;
        actor->object = part;
        actor->profile = nullptr;
        actor->is_active = true;
        actor->is_hidden = false;
        if (area_actor.entity == nullptr) {
            continue;
        }
        actor->mission = this;
        this->actors.push_back(actor);
    }
    for (auto member: this->mission->mission_data.team) {
        int id = this->mission->mission_data.parts[member]->id;
        for (auto actor: this->actors) {
            if (actor->actor_id == id) {
                this->friendlies.push_back(actor);
            }
        }
    }
    for (auto enemis : this->actors) {
        if (std::find(this->friendlies.begin(), this->friendlies.end(), enemis) == this->friendlies.end()) {
            this->enemies.push_back(enemis);
        }
    }
    if (this->player->on_is_activated.size() > 0) {
        SCProg *p = new SCProg(this->player, this->player->on_is_activated, this, this->player->object->on_is_activated);
        p->execute();
    }
    if (this->player->on_mission_start.size() > 0 && this->player->prog_executed == false) {
        SCProg *p = new SCProg(this->player, this->player->on_mission_start, this, this->player->object->on_missions_init);
        p->execute();
        this->player->prog_executed = true;
    }
    for (auto spot: this->mission->mission_data.spots) {
        spot->origin = {spot->position.x, spot->position.y, spot->position.z};
        if (spot->area_id != -1) {
            AREA *ar = this->mission->mission_data.areas[spot->area_id];
            spot->position += ar->position;
        }
    }
}
void PSMission::update() {
    SCMission::update();
}