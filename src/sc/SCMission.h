#pragma once
#include "precomp.h"


struct SCMissionActors {
    std::string actor_name;
    uint8_t actor_id;
    RSProf *profile;
    MISN_PART *object;
    SCPlane *plane;
    SCPilot *pilot;
};

class SCMission {
    std::string mission_name;
    RSArea *area{nullptr};
    RSMission *mission{nullptr};
};