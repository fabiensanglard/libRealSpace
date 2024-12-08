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
                actor->actor_name = cast->actor;
                actor->actor_id = part->id;
                actor->object = part;
                actor->profile = cast->profile;
                
                for (auto prg_id: actor->object->progs_id) {
                    if (prg_id != 255 && prg_id < this->mission->mission_data.prog.size()) {
                        std::vector<PROG> *prog = this->mission->mission_data.prog[prg_id];
                        actor->prog.push_back(prog);
                    }
                }
                if (cast->profile != nullptr && cast->profile->ai.isAI) {
                    if (cast->profile->ai.goal.size() > 0) {
                        actor->pilot = new SCPilot();
                        actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                 23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                 part->position.y, part->position.z);
                        actor->plane->azimuthf = (360 - part->azymuth) * 10.0f;
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
                        } else {
                            actor->pilot->target_climb = 10000;
                        }
                        actor->pilot->plane = actor->plane;
                        actor->pilot->target_azimut = (int) (actor->plane->azimuthf / 10.0f);
                        actor->pilot->target_speed = -20;
                    }
                    this->actors.push_back(actor);
                } else if (cast->profile != nullptr && cast->actor == "PLAYER") {
                    actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                 23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                 part->position.y, part->position.z);
                    actor->plane->azimuthf = (360 - part->azymuth) * 10.0f;
                    actor->plane->object = part;
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
                std::vector<PROG> *prog = this->mission->mission_data.prog[prg_id];
                actor->prog.push_back(prog);
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
}

void SCMission::update() {
    for (auto ai_actor : this->actors) {
        if (ai_actor->pilot == nullptr) {
            continue;
        }
        if (ai_actor->plane == nullptr) {
            continue;
        }
        ai_actor->plane->Simulate();
        ai_actor->pilot->AutoPilot();
        Vector3D npos;
        ai_actor->plane->getPosition(&npos);
        if (ai_actor->object->area_id != 255) {
            AREA *ar = this->mission->mission_data.areas[ai_actor->object->area_id];

            float minX = (float) ar->position.x-(ar->AreaWidth/2);
            float minZ = (float) ar->position.z-(ar->AreaWidth/2);
            float maxX = (float) ar->position.x+(ar->AreaWidth/2);
            float maxZ = (float) ar->position.z+(ar->AreaWidth/2);

            if (npos.x < minX || npos.x > maxX || npos.z < minZ || npos.z > maxZ) {
                ai_actor->pilot->target_azimut = ai_actor->pilot->target_azimut + 180;
                if (ai_actor->pilot->target_azimut > 360) {
                    ai_actor->pilot->target_azimut = ai_actor->pilot->target_azimut - 360;
                }
            }
        }
        ai_actor->object->position.x = npos.x;
        ai_actor->object->position.z = npos.z;
        ai_actor->object->position.y = npos.y;
        ai_actor->object->azymuth = 360 - (uint16_t)(ai_actor->plane->azimuthf / 10.0f);
        ai_actor->object->roll = (uint16_t)(ai_actor->plane->twist / 10.0f);
        ai_actor->object->pitch = (uint16_t)(ai_actor->plane->elevationf / 10.0f);
    }
}