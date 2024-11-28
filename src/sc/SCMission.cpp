#include "precomp.h"

SCMission::SCMission(std::string mission_name, std::map<std::string, RSEntity *> *objCache) {
    this->mission_name = mission_name;
    this->obj_cache = objCache;
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
    for (auto team : this->teams) {
        for (auto actor : team) {
            delete actor;
        }
    }
    this->teams.clear();
    this->teams.shrink_to_fit();
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
                if (cast->profile != nullptr && cast->profile->ai.isAI && cast->profile->ai.goal.size() > 0) {
                    
                } else if (cast->profile != nullptr && cast->actor == "PLAYER") {
                    this->player = actor;
                }
                break;
            }
            search_id++;
        }
    }
}