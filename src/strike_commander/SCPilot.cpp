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
    this->target_waypoint = {
        waypoint.x, waypoint.y, waypoint.z
    };
    Vector2D weapoint_direction = {waypoint.x - plane->x,
                                   waypoint.z - plane->z};
    float az = (atan2f((float)weapoint_direction.y, (float)weapoint_direction.x) * 180.0f / (float)M_PI);
    if (turning) {
        return;
    }
    turning = true;
    az -= 360.0f;
    az += 90.0f;
    if (az > 360.0f) {
        az -= 360.0f;
    }
    while (az < 0.0f) {
        az += 360.0f;
    }
    this->target_azimut = az*10.0f;
    target_climb = (int) waypoint.y;
    if (waypoint.y > plane->y || this->land) {
        
    } else {
        target_climb = (int) plane->y;
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
    if (this->plane->on_ground && this->target_climb == 0) {
        this->plane->SetThrottle(0);
        return;
    }
    if (this->plane->vz > this->target_speed) {
        this->plane->SetThrottle(100);
    } else {
        this->plane->SetThrottle(this->plane->GetThrottle() - 10);
    }


    float horizontal_distance = 1000.0f;
    float vertical_distance = this->target_climb - this->plane->y;
    float target_elevation = radToDegree(atan2(vertical_distance, horizontal_distance))*10.0f;
    
    
    float dt = 0.1f; // Time step, adjust as needed
    float control_signal = altitudeController.calculate(target_elevation, this->plane->pitch, dt);
    
    if (this->plane->pitch > target_elevation) {
        if (std::abs(target_elevation) > 1.0f) {
            this->plane->pitch -= 1.0f;
        } else {
            this->plane->pitch = target_elevation;
        } 
    } else if (this->plane->pitch < target_elevation) {
        if (std::abs(target_elevation) > 1.0f) {
            this->plane->pitch += 1.0f;
        } else {
            this->plane->pitch = target_elevation;
        }
    }

    
    float target_yaw = (float) (3600.0f - this->target_azimut);
    float target_roll = 0.0f;
    if (this->plane->yaw > target_yaw) {
        this->plane->roll = -300.0f;
        if (std::abs(this->plane->yaw-target_yaw) > 1.0f) {
            this->plane->yaw -= 1.0f;
        } else {
            this->plane->yaw = target_yaw;
        }
    } else if (this->plane->yaw < target_yaw) {
        this->plane->roll = 300.0f;
        if (std::abs(this->plane->yaw-target_yaw) > 1.0f) {
            this->plane->yaw += 1.0f;
        } else {
            this->plane->yaw = target_yaw;
        }
    } 
    
    if (std::abs(this->plane->yaw-target_yaw) <= 1.0f) {
        target_roll = 0.0f;
        this->plane->yaw = target_yaw;
        this->plane->roll = 0.0f;
        turning = false;
    }
    
}
