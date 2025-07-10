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
    enum TurnState {
        TURN_NONE,
        TURN_LEFT,
        TURN_RIGHT
    };
    TurnState turnState;
    float turnRate;
public:
    Vector3D target_waypoint{0.0f, 0.0f, 0.0f};
    bool turning{false};
    int target_speed{0};
    int target_climb{0};
    float target_azimut{0.0f};
    bool land{false};
    SCPlane* plane{nullptr};
    SCPilot();
    ~SCPilot();
    void SetTargetWaypoint(Vector3D waypoint);
    void AutoPilot();
};