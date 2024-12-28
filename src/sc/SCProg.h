#pragma once

#include "precomp.h"


class SCProg {
    SCMissionActors *actor;
    std::vector<PROG> prog;
    SCMission *mission;

    std::map<uint8_t, size_t> labels;

    SCProg(SCMissionActors *profile, std::vector<PROG> prog, SCMission *mission) {
        this->actor = profile;
        this->prog = prog;
        this->mission = mission;
    };
    void execute();
};