#pragma once
#include "precomp.h"
struct SCAiPlane {
    SCPlane *plane{nullptr};
    SCPilot *pilot{nullptr};
    MISN_PART *object{nullptr};
    RSProf *ai{nullptr};
    std::string name;
};

struct SCMissionWaypoint {
    SPOT *spot{nullptr};
    std::string *message{nullptr};
    std::string *objective{nullptr};
};

class SCProg;

class SCMission {
private:
    std::string mission_name;
    std::map<std::string, RSEntity *> *obj_cache{nullptr};
    RSEntity * LoadEntity(std::string name);
    RSProf * LoadProfile(std::string name);
public:    
    std::vector<SCMissionActors *> actors;
    std::vector<SCMissionActors *> enemies;
    std::vector<SCMissionActors *> friendlies;
    std::vector<SCMissionWaypoint *> waypoints;

    std::map<uint8_t, uint8_t> gameflow_registers;
    bool success{false};
    bool failure{false};
    bool mission_ended{false};
    uint8_t current_area_id{0};
    SCMissionActors *player{nullptr};
    RSArea *area{nullptr};
    RSMission *mission{nullptr};
    RSWorld *world{nullptr};
    std::vector<std::string*> radio_messages;
    
    SCMission(std::string mission_name, std::map<std::string, RSEntity *> *objCache);
    ~SCMission();
    void loadMission();
    void cleanup();
    void update();
    void executeProg(std::vector<PROG> *prog);
    uint8_t getAreaID(Vector3D position);
};