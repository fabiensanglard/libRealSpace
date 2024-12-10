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

struct SCMissionWaypoint {
    SPOT *spot;
    std::string *message;
};

class SCMission {
    enum prog_opcode {
        PROG_OP_SET_MESSAGE = 171,
        PROG_OP_SET_SPOT = 165,
        PROG_OP_SET_SPOT_2 = 166,
        PROG_OP_SET_PRIMARY_TARGET = 167
    };
private:
    std::string mission_name;
    std::map<std::string, RSEntity *> *obj_cache;

public:    
    std::vector<SCMissionActors *> actors;
    std::vector<SCMissionActors *> enemies;
    std::vector<SCMissionActors *> friendlies;
    std::vector<SCMissionWaypoint *> waypoints;

    SCMissionActors *player;
    RSArea *area{nullptr};
    RSMission *mission{nullptr};
    RSWorld *world{nullptr};
    
    SCMission(std::string mission_name, std::map<std::string, RSEntity *> *objCache);
    ~SCMission();
    void loadMission();
    void cleanup();
    void update();
    void executeProg(std::vector<PROG> *prog);
};