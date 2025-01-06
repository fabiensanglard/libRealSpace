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
void SCMission::loadMission() {
    std::string miss_file_name = "..\\..\\DATA\\MISSIONS\\" + this->mission_name; 
    
    TreEntry *mission_tre = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName(miss_file_name.c_str());

    this->mission = new RSMission();
    this->mission->tre = Assets.tres[AssetManager::TRE_OBJECTS];
    this->mission->objCache = this->obj_cache;
    this->mission->InitFromRAM(mission_tre->data, mission_tre->size);


    std::string area_filename = "..\\..\\DATA\\MISSIONS\\"+this->mission->mission_data.world_filename + ".IFF";
    std::transform(area_filename.begin(), area_filename.end(), area_filename.begin(), ::toupper);   
    this->world = new RSWorld();
    TreArchive *tre = new TreArchive();
    tre->InitFromFile("MISSIONS.TRE");
    TreEntry *treEntry = NULL;
    treEntry = tre->GetEntryByName(area_filename.c_str());
    if (treEntry != NULL) {
        this->world->InitFromRAM(treEntry->data, treEntry->size);
    }
    std::string area_fn = this->world->tera+".PAK";
    std::transform(area_fn.begin(), area_fn.end(), area_fn.begin(), ::toupper);
    this->area = new RSArea();
    this->area->InitFromPAKFileName(area_fn.c_str());

    int cpt_actor=0;
    for (auto part : mission->mission_data.parts) {
        int search_id = 0;
        for (auto cast : mission->mission_data.casting) {
            if (part->id == search_id) {
                SCMissionActors *actor = new SCMissionActors();
                if (cast->actor == "PLAYER") {
                    actor = new SCMissionActorsPlayer();
                }
                actor->actor_name = cast->actor;
                actor->actor_id = part->id;
                actor->object = part;
                actor->profile = cast->profile;
                actor->mission = this;
                for (auto prg_id: actor->object->progs_id) {
                    if (prg_id != 255 && prg_id < this->mission->mission_data.prog.size()) {
                        for (auto op: *this->mission->mission_data.prog[prg_id]) {
                            actor->prog.push_back(op);
                        }
                    }
                }
                if (cast->profile != nullptr && cast->profile->ai.isAI) {
                    if (cast->profile->ai.goal.size() > 0) {
                        actor->pilot = new SCPilot();
                        actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                 23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                 part->position.y, part->position.z);
                        actor->plane->azimuthf = (360 - part->azymuth) * 10.0f;
                        actor->plane->yaw = (360 - part->azymuth) * (float) M_PI / 180.0f;
                        actor->plane->object = part;
                        if (this->area->getY(part->position.x, part->position.z) == part->position.y) {
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
                } else if (cast->profile != nullptr && cast->actor == "PLAYER") {
                    actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                 23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                 part->position.y, part->position.z);
                    actor->plane->azimuthf = (360 - part->azymuth) * 10.0f;
                    actor->plane->simple_simulation = false;
                    actor->plane->yaw = (360 - part->azymuth) * (float) M_PI / 180.0f;
                    actor->plane->object = part;
                    this->actors.push_back(actor);
                    this->player = actor;
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
    
    if (this->player->prog.size() > 0) {
        SCProg *p = new SCProg(this->player, this->player->prog, this);
        p->execute();
    }
    for (auto spot: this->mission->mission_data.spots) {
        if (spot->area_id != -1) {
            AREA *ar = this->mission->mission_data.areas[spot->area_id];
            spot->position += ar->position;
        }
    }
    
}

void SCMission::update() {
    uint8_t area_id = this->getAreaID({this->player->plane->x, this->player->plane->y, this->player->plane->z});

    for (auto scene: this->mission->mission_data.scenes) {

        if (scene->area_id == area_id-1 || scene->area_id == -1) {
            if (scene->is_active == 0) {
                continue;
            }
            for (auto cast: scene->cast) {
                int i=0;
                for (auto part: this->mission->mission_data.parts) {
                    if (i == cast) {
                        for (auto actor: this->actors) {
                            if (actor->actor_id == part->id) {
                                actor->is_active = true;
                                break;
                            }
                        }
                        break;
                    }
                    i++;
                }
            }
            std::vector<PROG> prog;
            for (auto prg_id: scene->progs_id) {
                if (prg_id != 255 && prg_id-1 < this->mission->mission_data.prog.size()) {
                    for (auto prg: *this->mission->mission_data.prog[prg_id]) {
                        prog.push_back(prg);
                    }
                }
            }
            if (prog.size() > 0) {
                SCProg *p = new SCProg(this->player, prog, this);
                p->execute();
            }
            scene->is_active = 0;
        }
    }
    for (auto ai_actor : this->actors) {
        if (ai_actor->is_active == false) {
            continue;
        }
        if (ai_actor->pilot == nullptr) {
            continue;
        }
        if (ai_actor->plane == nullptr) {
            continue;
        }
        if (ai_actor->prog.size() > 0) {
            SCProg *p = new SCProg(ai_actor, ai_actor->prog, this);
            p->execute();
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