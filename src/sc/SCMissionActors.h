#pragma once
#include "precomp.h"

class SCMission;

enum prog_op {
    OP_EXIT_PROG = 1,
    OP_SET_LABEL = 8,
    OP_CALL_LABEL = 16,
    OP_GOTO_LABEL_IF_TRUE = 72,
    OP_GOTO_LABEL_IF_FALSE = 70,
    OP_IF_TARGET_IN_AREA = 146,
    OP_INSTANT_DESTROY_TARGET = 148,
    OP_SET_OBJ_TAKE_OFF = 161,
    OP_SET_OBJ_LAND = 162,
    OP_SET_OBJ_FLY_TO_WP = 165,
    OP_SET_OBJ_FLY_TO_AREA = 166,
    OP_SET_OBJ_DESTROY_TARGET = 167,
    OP_SET_OBJ_DEFEND_TARGET = 168,
    OP_SET_OBJ_FOLLOW_ALLY = 170,
    OP_SET_MESSAGE = 171,
    OP_DEACTIVATE_OBJ = 190,
    OP_85_UNKNOWN = 85,
    OP_ACTIVATE_OBJ = 144,
};

class SCMissionActors {
public:

    std::string actor_name;
    uint8_t actor_id;
    RSProf *profile{nullptr};
    std::vector<PROG> prog;
    MISN_PART *object{nullptr};
    SCPlane *plane{nullptr};
    SCPilot *pilot{nullptr};
    SCMission *mission{nullptr};
    prog_op current_objective;
    Vector3D formation_pos_offset{150.0f, 0.0f, 0.0f};
    Vector3D attack_pos_offset{0.0f, 0.0f, -200.0f};
    bool is_active{false};
    bool taken_off{false};

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