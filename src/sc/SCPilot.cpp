//
//  SCPilot.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

SCPilot::SCPilot() {
    target_speed = 0;
    target_climb = 0;
    target_azimut = 0;
}

SCPilot::~SCPilot() {}

void SCPilot::AutoPilot() {
    if (this->plane == nullptr) {
        return;
    }
    if (this->plane->vz > this->target_speed) {
        this->plane->SetThrottle(100);
    } else {
        this->plane->SetThrottle(this->plane->GetThrottle() - 10);
    }
    if (this->plane->y < this->target_climb) {
        if (this->plane->elevationf < 45) {
            this->plane->control_stick_y = this->plane->control_stick_y + 1;
        } else {
            this->plane->control_stick_y = 0;
        }
    } else {
        if (this->plane->elevationf > -45) {
            this->plane->control_stick_y = this->plane->control_stick_y - 1;
        } else {
            this->plane->control_stick_y = 0;
        }
    }

    if (this->plane->control_stick_y > 100) {
        this->plane->control_stick_y = 100;
    } else if (this->plane->control_stick_y < -100) {
        this->plane->control_stick_y = -100;
    }

    float azimut_diff = this->target_azimut - (360 - (this->plane->azimuthf / 10.0f));

    if (azimut_diff > 180.0f) {
        azimut_diff -= 360.0f;
    } else if (azimut_diff < -180.0f) {
        azimut_diff += 360.0f;
    }

    const float max_twist_angle = 80.0f;
    const float Kp = 3.0f;

    float target_twist_angle = Kp * azimut_diff;
    float current_twist = 360 - this->plane->twist / 10.0f;

    if (current_twist > 180.0f) {
        current_twist -= 360.0f;
    } else if (current_twist < -180.0f) {
        current_twist += 360.0f;
    }

    if (target_twist_angle > 180.0f) {
        target_twist_angle -= 360.0f;
    } else if (target_twist_angle < -180.0f) {
        target_twist_angle += 360.0f;
    }

    if (target_twist_angle > max_twist_angle) {
        target_twist_angle = max_twist_angle;
    } else if (target_twist_angle < -max_twist_angle) {
        target_twist_angle = -max_twist_angle;
    }

    if (azimut_diff > 0) {
        if (current_twist - target_twist_angle < 0) {
            this->plane->control_stick_x = 50;
        } else {
            this->plane->control_stick_x = 0;
        }
    } else {
        if (current_twist - target_twist_angle > 0) {
            this->plane->control_stick_x = -50;
        } else {
            this->plane->control_stick_x = 0;
        }
    }
}
