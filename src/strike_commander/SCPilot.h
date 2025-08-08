//
//  SCPilot.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "precomp.h"
class PIDController {
public:
    PIDController() : kp(0), ki(0), kd(0), prev_error(0), integral(0) {};
    PIDController(float kp, float ki, float kd)
        : kp(kp), ki(ki), kd(kd), prev_error(0), integral(0) {};
    float calculate(float setpoint, float measured_value, float dt) {
        float error = setpoint - measured_value;
        integral += error * dt;
        float derivative = (error - prev_error) / dt;
        prev_error = error;
        return kp * error + ki * integral + kd * derivative;
    };

private:
    float kp, ki, kd;
    float prev_error;
    float integral;
};

class SCPilot {
private:
    bool alive{true};
    enum TurnState {
        TURN_NONE,
        TURN_LEFT,
        TURN_RIGHT
    };
    
    float turnRate;
    
    
public:
    Vector3D target_waypoint{0.0f, 0.0f, 0.0f};
    bool turning{false};
    int target_speed{0};
    int target_climb{0};
    float target_azimut{0.0f};
    float old_target_azimut{0.0f};
    float targetRoll{0.0f};
    bool land{false};
    TurnState turnState;
    SCPlane* plane{nullptr};
    SCPilot();
    ~SCPilot();
    void SetTargetWaypoint(Vector3D waypoint);
    void DirectAutoPilot();
    void AutoPilot();
};