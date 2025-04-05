#include "precomp.h"

SCMission::SCMission(std::string mission_name, std::map<std::string, RSEntity *> *objCache) {
    this->mission_name = mission_name;
    this->obj_cache = objCache;
    this->loadMission();
}
SCMission::~SCMission() {
    this->cleanup();
}
void SCMission::cleanup() {
    if (this->mission != nullptr) {
        delete this->mission;
        this->mission = nullptr;
    }
    if (this->area != nullptr) {
        delete this->area;
        this->area = nullptr;
    }
    
    for (auto actor : this->actors) {
        delete actor;
    }
    this->actors.clear();
    this->actors.shrink_to_fit();
    for (auto wp : this->waypoints) {
        delete wp;
    }
    this->waypoints.clear();
    this->waypoints.shrink_to_fit();
    
}
RSProf *SCMission::LoadProfile(std::string name) {
    RSProf *profile = new RSProf();
    std::string filename = "..\\..\\DATA\\INTEL\\"+ name + ".IFF";
    TreEntry *profile_tre = Assets.GetEntryByName(filename);
    if (profile_tre != nullptr) {
        profile->InitFromRAM(profile_tre->data, profile_tre->size);
    } else {
        printf("Unable to load profile %s\n", name.c_str());
        return nullptr;
    }
    return profile;
}
void SCMission::loadMission() {
    std::string miss_file_name = "..\\..\\DATA\\MISSIONS\\" + this->mission_name; 
    
    TreEntry *mission_tre = Assets.GetEntryByName(miss_file_name.c_str());
    this->mission = new RSMission();
    this->mission->InitFromRAM(mission_tre->data, mission_tre->size);


    std::string area_filename = "..\\..\\DATA\\MISSIONS\\"+this->mission->mission_data.world_filename + ".IFF";
    std::transform(area_filename.begin(), area_filename.end(), area_filename.begin(), ::toupper);   
    this->world = new RSWorld();
    TreEntry *treEntry = NULL;
    treEntry = Assets.GetEntryByName(area_filename.c_str());
    if (treEntry != NULL) {
        this->world->InitFromRAM(treEntry->data, treEntry->size);
    }
    std::string area_fn = this->world->tera+".PAK";
    std::transform(area_fn.begin(), area_fn.end(), area_fn.begin(), ::toupper);
    this->area = new RSArea(&Assets);
    this->area->InitFromPAKFileName(area_fn.c_str());
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
                }
                if (cast->actor == "TEAM0") {
                    cast->actor = GameState.wingman;
                }
                actor->actor_name = cast->actor;
                actor->actor_id = part->id;
                actor->object = part;
                actor->profile = this->LoadProfile(cast->actor);
                actor->mission = this;
                if (actor->object->on_is_activated != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_is_activated]) {
                        actor->on_is_activated.push_back(op);
                    }
                }
                if (actor->object->on_is_destroyed != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_is_destroyed]) {
                        actor->on_is_destroyed.push_back(op);
                    }
                }
                if (actor->object->on_missions_init != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_missions_init]) {
                        actor->on_mission_start.push_back(op);
                    }
                }
                if (actor->object->on_mission_update != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_mission_update]) {
                        actor->on_update.push_back(op);
                    }
                }
                
                if (actor->profile != nullptr && actor->profile->ai.isAI) {
                    if (actor->profile->ai.goal.size() > 0) {
                        actor->pilot = new SCPilot();
                        actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                part->position.y, part->position.z);
                        actor->plane->azimuthf = (360 - part->azymuth) * 10.0f;
                        actor->plane->yaw = (360 - part->azymuth) * (float) M_PI / 180.0f;
                        actor->plane->object = part;
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
                    actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                part->position.y, part->position.z);
                    actor->plane->azimuthf = (360 - part->azymuth) * 10.0f;
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
        for (auto prg_id: area_actor.progs_id) {
            if (prg_id != 255 && prg_id != 0 && prg_id < this->mission->mission_data.prog.size()) {
                for (auto prg: *this->mission->mission_data.prog[prg_id]) {
                    actor->prog.push_back(prg);
                }
            }
        }
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
        SCProg *p = new SCProg(this->player, this->player->on_is_activated, this);
        p->execute();
    }
    if (this->player->on_mission_start.size() > 0 && this->player->prog_executed == false) {
        SCProg *p = new SCProg(this->player, this->player->on_mission_start, this);
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
RSEntity * SCMission::LoadEntity(std::string name) {
    std::string tmpname = "..\\..\\DATA\\OBJECTS\\" + name + ".IFF";
    RSEntity *objct = new RSEntity(&Assets);
    TreEntry *entry = Assets.GetEntryByName((char *)tmpname.c_str());
    if (entry != nullptr) {
        objct->InitFromRAM(entry->data, entry->size);
        return objct;
    }
    return nullptr;
}
void SCMission::update() {
    uint8_t area_id = this->getAreaID({this->player->plane->x, this->player->plane->y, this->player->plane->z});
    if (area_id != this->current_area_id) {
        this->current_area_id = area_id;
        for (auto scene: this->mission->mission_data.scenes) {
            if (scene->area_id == area_id-1 && scene->on_leaving != -1) {
                if (scene->on_leaving < this->mission->mission_data.prog.size()) {
                    std::vector<PROG> prog;
                    for (auto prg: *this->mission->mission_data.prog[scene->on_leaving]) {
                        prog.push_back(prg);
                    }
                    SCProg *p = new SCProg(this->player, prog, this);
                    p->execute();
                }
            }
        }
    }
    for (auto scene: this->mission->mission_data.scenes) {
        if (scene->area_id == area_id-1 || scene->area_id == -1) {
            if (scene->is_active == 0) {
                if (scene->on_mission_update != -1) {
                    if (scene->on_mission_update < this->mission->mission_data.prog.size()) {
                        std::vector<PROG> prog;
                        for (auto prg: *this->mission->mission_data.prog[scene->on_mission_update]) {
                            prog.push_back(prg);
                        }
                        SCProg *p = new SCProg(this->player, prog, this);
                        p->execute();
                    }
                }
                continue;
            }
            for (auto cast: scene->cast) {
                int i=0;
                for (auto part: this->mission->mission_data.parts) {
                    if (i == cast) {
                        for (auto actor: this->actors) {
                            if (actor->actor_name == "PLAYER") {
                                continue;
                            }
                            if (actor->actor_id == part->id && actor->is_active == false) {
                                actor->is_active = true;
                                
                                if (scene->area_id != -1) {
                                    Vector3D correction;
                                    if (actor->object->unknown2 == 0) {
                                        correction = this->mission->mission_data.areas[scene->area_id]->position;
                                    } else if (actor->object->unknown2 == 1) {
                                        correction = {
                                            this->player->plane->x,
                                            this->player->plane->y,
                                            this->player->plane->z
                                        };
                                    }
                                    actor->object->position += correction;
                                    if (actor->plane != nullptr) {
                                        actor->plane->on_ground = false;
                                        actor->plane->x = actor->object->position.x;
                                        actor->plane->y = actor->object->position.y;
                                        actor->plane->z = actor->object->position.z;
                                    }
                                }
                                
                                if (actor->on_is_activated.size() > 0) {
                                    SCProg *p = new SCProg(actor, actor->on_is_activated, this);
                                    p->execute();
                                }
                                break;
                            }
                        }
                        break;
                    }
                    i++;
                }
            }
            if (scene->on_is_activated != -1) {
                if (scene->on_is_activated < this->mission->mission_data.prog.size()) {
                    std::vector<PROG> prog;
                    for (auto prg: *this->mission->mission_data.prog[scene->on_is_activated]) {
                        prog.push_back(prg);
                    }
                    SCProg *p = new SCProg(this->player, prog, this);
                    p->execute();
                }
            }
            scene->is_active = 0;
        }
    }
    for (auto ai_actor : this->actors) {
        if (ai_actor->object->alive == false && ai_actor->is_destroyed == false) {
            ai_actor->is_destroyed = true;
            if (ai_actor->on_is_destroyed.size() > 0) {
                SCProg *p = new SCProg(ai_actor, ai_actor->on_is_destroyed, this);
                p->execute();
            }
            if (ai_actor->object->member_name_destroyed != "") {
                ai_actor->object->entity = LoadEntity(ai_actor->object->member_name_destroyed);
            }
        }
        if (ai_actor->profile == nullptr) {
            continue;
        }
        if (ai_actor->profile->radi.info.callsign == "Strike Base") {
            SCProg *p = new SCProg(ai_actor, ai_actor->on_update, this);
            p->execute();
            continue;
        }
        if (ai_actor->is_active == false) {
            continue;
        }
        if (ai_actor->on_update.size() > 0 && ai_actor->is_destroyed == false) {
            SCProg *p = new SCProg(ai_actor, ai_actor->on_update, this);
            p->execute();
        }
        if (ai_actor->pilot == nullptr) {
            continue;
        }
        if (ai_actor->plane == nullptr) {
            continue;
        }
        if (ai_actor->plane->status == 0) {
            continue;
        }
        ai_actor->plane->Simulate();
        ai_actor->pilot->AutoPilot();
        
        Vector3D npos;
        ai_actor->plane->getPosition(&npos);
        
        ai_actor->object->position.x = npos.x;
        ai_actor->object->position.z = npos.z;
        ai_actor->object->position.y = npos.y;
        ai_actor->object->azymuth = 360 - (uint16_t)(ai_actor->plane->azimuthf / 10.0f);
        ai_actor->object->roll = (uint16_t)(ai_actor->plane->twist / 10.0f);
        ai_actor->object->pitch = (uint16_t)(ai_actor->plane->elevationf / 10.0f);
        if (ai_actor->is_destroyed == true && ai_actor->object->position.y < 0) {
            ai_actor->object->alive = false;
            ai_actor->is_active = false;
        }
    }
    if (this->player->plane->landed) {
        this->mission_ended = true;
    }
}


void SCMission::executeProg(std::vector<PROG> *prog) {
    
}
uint8_t SCMission::getAreaID(Vector3D position) {
    uint8_t area_id = 255;
    for (auto ar: this->mission->mission_data.areas) {
        if (ar->position.x - ar->AreaWidth / 2 <= position.x && ar->position.x + ar->AreaWidth / 2 >= position.x) {
            if (ar->position.z - ar->AreaWidth / 2 <= position.z && ar->position.z + ar->AreaWidth / 2 >= position.z) {
                area_id = ar->id;
            }
        }
    }
    return area_id;
}