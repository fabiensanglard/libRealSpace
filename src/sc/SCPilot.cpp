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
    Vector2D weapoint_direction = {waypoint.x - plane->x,
                                   waypoint.z - plane->z};
    float az = (atan2f((float)weapoint_direction.y, (float)weapoint_direction.x) * 180.0f / (float)M_PI);
    
    az -= 360.0f;
    az += 90.0f;
    if (az > 360.0f) {
        az -= 360.0f;
    }
    while (az < 0.0f) {
        az += 360.0f;
    }
    this->target_azimut = az;
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
    
    if (this->plane->pitch > target_elevation_rad) {
        if (std::abs(target_elevation_rad) > 0.01f) {
            this->plane->pitch -= 0.01f;
        } else {
            this->plane->pitch = target_elevation_rad;
        } 
    } else if (this->plane->pitch < target_elevation_rad) {
        if (std::abs(target_elevation_rad) > 0.01f) {
            this->plane->pitch += 0.01f;
        } else {
            this->plane->pitch = target_elevation_rad;
        }
    }

    float target_yaw = (float) ((360.0f - this->target_azimut) * M_PI / 180.0f);
    float target_roll = 0.0f;
    if (this->plane->yaw > target_yaw) {
        this->plane->roll = 5.78f;
        if (std::abs(this->plane->yaw-target_yaw) > 0.01f) {
            this->plane->yaw -= 0.01f;
        } else {
            this->plane->yaw = target_yaw;
        }
    } else if (this->plane->yaw < target_yaw) {
        this->plane->roll = 0.5f;
        if (std::abs(this->plane->yaw-target_yaw) > 0.01f) {
            this->plane->yaw += 0.01f;
        } else {
            this->plane->yaw = target_yaw;
        }
    } 
    
    if (this->plane->yaw == target_yaw) {
        target_roll = 0.0f;
       
        this->plane->roll = 0.0f;
        
    }
    
}
