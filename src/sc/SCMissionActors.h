#pragma once
#include "precomp.h"

enum prog_op {
    OP_EXIT_PROG = 1,
    OP_SET_LABEL = 8,
    OP_GOTO_LABEL = 72,
    OP_IF_TARGET_IN_AREA = 146,
    OP_INSTANT_DESTROY_TARGET = 148,
    OP_SET_OBJ_TAKE_OFF = 161,
    OP_SET_OBJ_LAND = 162,
    OP_SET_OBJ_FLY_TO_WP = 165,
    OP_SET_OBJ_FLY_TO_AREA = 166,
    OP_SET_OBJ_DESTROY_TARGET = 167,
    OP_SET_OBJ_DEFEND_TARGET = 168,
    OP_SET_MESSAGE = 171,
    OP_DEACTIVATE_OBJ = 190,
};

class SCMissionActors {
public:

    std::string actor_name;
    uint8_t actor_id;
    RSProf *profile;
    std::vector<std::vector<PROG> *> prog;
    MISN_PART *object;
    SCPlane *plane;
    SCPilot *pilot;

    virtual bool execute();
    virtual bool takeOff(uint8_t arg); 
    virtual bool land(uint8_t arg);
    virtual bool flyToWaypoint(uint8_t arg);
    virtual bool flyToArea(uint8_t arg);
    virtual bool destroyTarget(uint8_t arg);
    virtual bool defendTarget(uint8_t arg);
    virtual bool deactivate(uint8_t arg);
    virtual bool setMessage(uint8_t arg);
};