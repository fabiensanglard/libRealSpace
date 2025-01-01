//
//  SCPilot.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include <algorithm>
#include "precomp.h"
class PIDController {
public:
    PIDController(float kp, float ki, float kd)
        : kp(kp), ki(ki), kd(kd), prev_error(0), integral(0) {}

    float calculate(float setpoint, float measured_value, float dt) {
        float error = setpoint - measured_value;
        integral += error * dt;
        float derivative = (error - prev_error) / dt;
        prev_error = error;
        return kp * error + ki * integral + kd * derivative;
    }

private:
    float kp, ki, kd;
    float prev_error;
    float integral;
};

SCPilot::SCPilot() {
    target_speed = 0;
    target_climb = 0;
    target_azimut = 0;
}

SCPilot::~SCPilot() {}

void SCPilot::SetTargetWaypoint(Vector3D waypoint) {

    float azimuth = atan2(waypoint.z - plane->z, waypoint.x - plane->x);
    target_azimut = azimuth * 180.0f / (float) M_PI;
    if (target_azimut < 0) {
        target_azimut += 360.0f;
    } else if (target_azimut > 360.0f) {
        target_azimut -= 360.0f;
    }
    if (waypoint.y > plane->y) {
        target_climb = waypoint.y;
    } else {
        target_climb = plane->y;
    }
}

void SCPilot::AutoPilot() {
    PIDController altitudeController(1.0f, 0.1f, 0.05f); // Adjust these values as needed
    PIDController azimuthController(1.0f, 0.1f, 0.05f); // Adjust these values as needed

    if (!this->alive) {
        return;
    }
    if (!this->plane->object->alive) {
        this->plane->Mthrust = 0;
        this->plane->s = 0.001f;
        this->plane->b = 0.001f;
        this->target_speed = 0;
        this->target_climb = 0;
        this->target_azimut = 0;
        this->plane->SetSpoilers();
        this->plane->vz /= 1.5f;
        this->plane->vy *=1.05f;
        this->plane->SetThrottle(0);
        this->alive = false;
    }
    if (this->plane == nullptr) {
        return;
    }
    if (this->plane->vz > this->target_speed) {
        this->plane->SetThrottle(100);
    } else {
        this->plane->SetThrottle(this->plane->GetThrottle() - 10);
    }


    float horizontal_distance = 1000.0f;
    float vertical_distance = this->target_climb - this->plane->y;
    float target_elevation_rad = atan2(vertical_distance, horizontal_distance);

    
    float dt = 0.1f; // Time step, adjust as needed
    float control_signal = altitudeController.calculate(target_elevation_rad, this->plane->pitch, dt);
    
    this->plane->pitch = target_elevation_rad;
    const float max_twist_angle = 80.0f;
    const float Kp = 3.0f;

    float azimut_diff = this->target_azimut - (360 - (this->plane->azimuthf / 10.0f));

    if (azimut_diff > 180.0f) {
        azimut_diff -= 360.0f;
    } else if (azimut_diff < -180.0f) {
        azimut_diff += 360.0f;
    }

    float target_yaw = this->target_azimut * M_PI / 180.0f;
    this->plane->yaw = target_yaw;

}
