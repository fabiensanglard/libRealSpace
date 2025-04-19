#pragma once
#include "precomp.h"

class SCMission;


class SCMissionActors {
public:
    int score{0};
    int plane_down{0};
    int ground_down{0};
    std::string actor_name;
    uint8_t actor_id;
    RSProf *profile{nullptr};
    std::vector<PROG> prog;
    std::vector<PROG> on_mission_start;
    std::vector<PROG> on_is_activated;
    std::vector<PROG> on_is_destroyed;
    std::vector<PROG> on_update;
    MISN_PART *object{nullptr};
    SCPlane *plane{nullptr};
    SCPilot *pilot{nullptr};
    SCMission *mission{nullptr};
    prog_op current_objective;
    Vector3D formation_pos_offset{150.0f, 0.0f, 0.0f};
    Vector3D attack_pos_offset{0.0f, 0.0f, -1000.0f};
    bool is_active{false};
    bool taken_off{false};
    bool is_destroyed{false};
    bool prog_executed{false};
    int current_target{0};
    bool current_command_executed{false};
    prog_op current_command;
    uint8_t current_command_arg;
    virtual bool execute();
    virtual bool takeOff(uint8_t arg); 
    virtual bool land(uint8_t arg);
    virtual bool flyToWaypoint(uint8_t arg);
    virtual bool flyToArea(uint8_t arg);
    virtual bool destroyTarget(uint8_t arg);
    virtual bool defendTarget(uint8_t arg);
    virtual bool deactivate(uint8_t arg);
    virtual bool setMessage(uint8_t arg);
    virtual bool followAlly(uint8_t arg);
    virtual bool ifTargetInSameArea(uint8_t arg);
    virtual bool activateTarget(uint8_t arg);
    virtual int getDistanceToTarget(uint8_t arg);
    virtual int getDistanceToSpot(uint8_t arg);
private:
    Vector3D target_position{0.0f, 0.0f, 0.0f};
    int target_position_update{0};
};

class SCMissionActorsPlayer : public SCMissionActors {
public:
    bool takeOff(uint8_t arg); 
    bool land(uint8_t arg);
    bool flyToWaypoint(uint8_t arg);
    bool flyToArea(uint8_t arg);
    bool destroyTarget(uint8_t arg);
    bool defendTarget(uint8_t arg);
    bool setMessage(uint8_t arg);
};

class SCMissionActorsStrikeBase : public SCMissionActors {
public:
    bool setMessage(uint8_t arg);
};