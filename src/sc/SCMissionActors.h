#pragma once
#include "precomp.h"

class SCMission;

enum prog_op {
    OP_EXIT_PROG = 1,
    OP_EXEC_SUB_PROG = 2,
    OP_SET_LABEL = 8,
    OP_INVERT_FLAG = 9,
    OP_CALL_LABEL = 16,
    OP_SELECT_FLAG = 17,
    OP_SAVE_VALUE_TO_FLAG = 20,
    OP_ADD_3_TO_FLAG = 35,
    OP_ADD_VALUE_TO_WORK = 46, 
    OP_CMP_GREATER_EQUAL_THAN = 64,
    OP_GET_FLAG_ID = 69,
    OP_GOTO_IF_CURRENT_COMMAND_IN_PROGRESS = 70,
    OP_GOTO_LABEL_IF_TRUE = 72,
    OP_GOTO_IF_FALSE_73 = 73,
    OP_GOTO_IF_TRUE_74 = 74,
    OP_EXECUTE_CALL = 79,
    OP_STORE_CALL_TO_VALUE = 80,
    OP_SET_FLAG_TO_TRUE = 82,
    OP_ADD_1_TO_FLAG = 85,
    OP_ACTIVATE_OBJ = 144,
    OP_IF_TARGET_IN_AREA = 146,
    OP_INSTANT_DESTROY_TARGET = 148,
    OP_DIST_TO_TARGET = 149,
    OP_SET_OBJ_TAKE_OFF = 161,
    OP_SET_OBJ_LAND = 162,
    OP_SET_OBJ_FLY_TO_WP = 165,
    OP_SET_OBJ_FLY_TO_AREA = 166,
    OP_SET_OBJ_DESTROY_TARGET = 167,
    OP_SET_OBJ_DEFEND_TARGET = 168,
    OP_SET_OBJ_FOLLOW_ALLY = 170,
    OP_SET_MESSAGE = 171,
    OP_DEACTIVATE_OBJ = 190,
    OP_SELECT_FLAG_208 = 208,
};


class SCMissionActors {
public:

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
    Vector3D attack_pos_offset{0.0f, 0.0f, -200.0f};
    bool is_active{false};
    bool taken_off{false};
    bool is_destroyed{false};
    bool prog_executed{false};
    bool current_command_executed{false};

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