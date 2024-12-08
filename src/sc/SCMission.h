#pragma once
#include "precomp.h"
struct SCAiPlane {
    SCPlane *plane;
    SCPilot *pilot;
    MISN_PART *object;
    RSProf *ai;
    std::string name;
};

struct SCMissionActors {
    std::string actor_name;
    uint8_t actor_id;
    RSProf *profile;
    std::vector<std::vector<PROG> *> prog;
    MISN_PART *object;
    SCPlane *plane;
    SCPilot *pilot;
};

class SCMission {
private:
    std::string mission_name;
    
    
    
    std::map<std::string, RSEntity *> *obj_cache;

public:    
    std::vector<SCMissionActors *> actors;
    std::vector<SCMissionActors *> enemies;
    std::vector<SCMissionActors *> friendlies;
    SCMissionActors *player;
    RSArea *area{nullptr};
    RSMission *mission{nullptr};
    RSWorld *world{nullptr};
    
    SCMission(std::string mission_name, std::map<std::string, RSEntity *> *objCache);
    ~SCMission();
    void loadMission();
    void cleanup();
    void update();
};