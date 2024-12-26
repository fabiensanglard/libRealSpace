#pragma once

#include "precomp.h"


class SCProg {
    SCMissionActors *actor;
    std::vector<PROG> prog;
    SCMission *mission;

    std::map<uint8_t, size_t> labels;
    
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

    SCProg(SCMissionActors *profile, std::vector<PROG> prog, SCMission *mission) {
        this->actor = profile;
        this->prog = prog;
        this->mission = mission;
    };
    void execute();
};