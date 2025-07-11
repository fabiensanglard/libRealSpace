//
//  SCPilot.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include <algorithm>
#include "precomp.h"

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
    Vector2D weapoint_direction = {
        waypoint.x - plane->x,
        waypoint.z - plane->z
    };
    float az = (atan2f((float)weapoint_direction.y, (float)weapoint_direction.x) * 180.0f / (float)M_PI);
    if (turning) {
        return;
    }
    //turning = true;
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

    
    float target_yaw = (float)(3600.0f - this->target_azimut);
    float yaw_difference = this->plane->yaw - target_yaw;
    float delta_time = 1.0f / this->plane->tps;

    float roll_rate = (this->plane->object->entity->jdyn->ROLL_RATE / 60.0f) *10.0f * delta_time;
    
    // Normaliser la différence entre -1800 et 1800 (entre -180° et 180°)
    while (yaw_difference > 1800.0f) yaw_difference -= 3600.0f;
    while (yaw_difference < -1800.0f) yaw_difference += 3600.0f;
        
    // Déterminer la direction du virage
    if (yaw_difference > 10.0f) {
        turnState = TURN_LEFT;
    } else if (yaw_difference < -10.0f) {
        turnState = TURN_RIGHT;
    } else {
        turnState = TURN_NONE;
        // On est presque aligné, on fait un ajustement final précis
        if (std::abs(yaw_difference) <= 1.0f) {
            this->plane->yaw = target_yaw;
            turning = false;
        } else {
            this->plane->yaw += (yaw_difference > 0) ? -1.0f : 1.0f;
        }
    }

    // Taux de virage basé sur l'écart (plus rapide pour grand angle)
    turnRate = (std::min)(5.0f, std::abs(yaw_difference) / 100.0f);
    if (turnRate < 0.5f) turnRate = 0.5f;
    // Calculate target roll angle proportional to turn rate
    targetRoll = 0.0f;
    if (turnState == TURN_LEFT) {
        // Pour les virages à gauche, incliner vers 2700 (aile gauche vers le bas)
        targetRoll = 2700.0f * (turnRate / 5.0f);
        // Assurer une valeur minimale pour l'inclinaison
        if (targetRoll < 900.0f) targetRoll = 900.0f;
    } else if (turnState == TURN_RIGHT) {
        // Pour les virages à droite, incliner vers 900 (aile droite vers le bas)
        targetRoll = 900.0f * (turnRate / 5.0f);
        // Assurer une valeur minimale pour l'inclinaison
        if (targetRoll < 300.0f) targetRoll = 300.0f;
    }
    // Gérer l'inclinaison et le virage
    switch (turnState) {
        case TURN_LEFT:
            // Incliner progressivement à gauche vers la valeur targetRoll
            if (this->plane->roll < targetRoll && this->plane->roll > 899.0f) {
                this->plane->roll += (std::min)(roll_rate, targetRoll - this->plane->roll);
            } else if (this->plane->roll <= 899.0f) {
                this->plane->roll = 3600.0f - roll_rate;
            } else if (this->plane->roll > targetRoll) {
                this->plane->roll -= roll_rate;
            }
                
            // Une fois suffisamment incliné, effectuer le virage
            // Utiliser targetRoll au lieu de 2700.0f
            if (std::abs(this->plane->roll - targetRoll) < 200.0f) {
                this->plane->yaw -= turnRate;
            }
            break;
                
        case TURN_RIGHT:
            // Incliner progressivement à droite vers la valeur targetRoll
            if (this->plane->roll > targetRoll && this->plane->roll < 2700.0f) {
                this->plane->roll -= (std::min)(roll_rate, this->plane->roll - targetRoll);
            } else if (this->plane->roll >= 2700.0f) {
                this->plane->roll = (int) (this->plane->roll + roll_rate) % 3600;
            } else if (this->plane->roll < targetRoll) {
                this->plane->roll += roll_rate;
            }
                
            // Une fois suffisamment incliné, effectuer le virage
            // Utiliser targetRoll au lieu de 900.0f
            if (std::abs(this->plane->roll - targetRoll) < 200.0f) {
                this->plane->yaw += turnRate;
            }
            break;
                
        case TURN_NONE:
            // Redresser l'avion quand on ne tourne pas (vers 0 ou 3600)
            if (this->plane->roll > roll_rate && this->plane->roll < 1800.0f) {
                this->plane->roll -= roll_rate;
            } else if (this->plane->roll > 1800.0f && this->plane->roll < 3600.0f - roll_rate) {
                this->plane->roll += roll_rate;
                if (this->plane->roll >= 3600.0f) {
                    this->plane->roll = 0.0f;
                }
            } else {
                this->plane->roll = 0.0f;
            }
            break;
    }

    if (std::abs(this->plane->yaw-target_yaw) <= 1.0f) {
        this->plane->yaw = target_yaw;
        turning = false;
    }
    
}
