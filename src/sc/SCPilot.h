//
//  SCPilot.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "precomp.h"

class SCPilot {
private:
    bool alive{true};
    bool turning{false};
public:

    int target_speed{0};
    int target_climb{0};
    float target_azimut{0.0f};
    int current_objective;
    SCPlane* plane{nullptr};
    SCPilot();
    ~SCPilot();
    void SetTargetWaypoint(Vector3D waypoint);
    void AutoPilot();
};