#include "precomp.h"

SCMission::SCMission(std::string mission_name, std::map<std::string, RSEntity *> *objCache) {
    this->mission_name = mission_name;
    this->obj_cache = objCache;
    this->LoadMission();
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
void SCMission::LoadMission() {
    this->cleanup();
    std::string miss_file_name = "..\\..\\DATA\\MISSIONS\\" + this->mission_name; 
    
    TreEntry *mission_tre = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName(miss_file_name.c_str());

    this->mission = new RSMission();
    this->mission->tre = Assets.tres[AssetManager::TRE_OBJECTS];
    this->mission->objCache = this->obj_cache;
    this->mission->InitFromRAM(mission_tre->data, mission_tre->size);

    std::string area_filename = this->mission->mission_data.world_filename + ".PAK";
    this->area = new RSArea();
    this->area->InitFromPAKFileName(area_filename.c_str());

    if (this->area->objects.size() == 0) {
        area_filename = this->mission->mission_data.name + ".PAK";
        this->area->InitFromPAKFileName(area_filename.c_str());
    }

    for (auto part : mission->mission_data.parts) {
        int search_id = 0;
        for (auto cast : mission->mission_data.casting) {
            if (part->id == search_id) {
                SCMissionActors *actor = new SCMissionActors();
                actor->actor_name = cast->actor;
                actor->actor_id = part->id;
                actor->object = part;
                actor->profile = cast->profile;
                
                actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                 23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                 part->position.y, part->position.z);
                actor->plane->azimuthf = (360 - part->azymuth) * 10.0f;
                if (cast->profile != nullptr && cast->profile->ai.isAI && cast->profile->ai.goal.size() > 0) {
                    actor->pilot = new SCPilot();
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
                        actor->pilot->target_climb = (int) (part->position.y * 3.6f);
                        actor->plane->vz = -20;
                    } else {
                        actor->pilot->target_climb = 25000;
                    }

                    actor->pilot->plane = actor->plane;
                    actor->pilot->target_azimut = (int) (actor->plane->azimuthf / 10.0f);

                    actor->pilot->target_speed = -20;
                    this->actors.push_back(actor);
                } else if (cast->profile != nullptr && cast->actor == "PLAYER") {
                    this->player = actor;
                }
                break;
            }
            search_id++;
        }
    }
}