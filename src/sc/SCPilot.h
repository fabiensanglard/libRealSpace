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
public:

    int target_speed;
    int target_climb;
    int target_azimut;

    SCPlane* plane{nullptr};
    SCPilot();
    ~SCPilot();
    void SetTargetWaypoint(Vector3D waypoint);
    void AutoPilot();
};