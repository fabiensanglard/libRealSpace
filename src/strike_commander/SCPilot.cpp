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
    // Constantes PID optimisées pour les intervalles de manche spécifiques
    PIDController altitudeController(1.0f, 0.1f, 0.05f);
    PIDController rollController(0.05f, 0.01f, 0.02f);
    PIDController headingController(0.02f, 0.005f, 0.01f);

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
        return;
    }
    if (this->plane == nullptr) {
        return;
    }
    if (this->plane->on_ground && this->target_climb == 0) {
        this->plane->SetThrottle(0);
        return;
    }
    
    // Contrôle de la vitesse
    if (this->plane->vz > this->target_speed) {
        this->plane->SetThrottle(100);
    } else {
        this->plane->SetThrottle(this->plane->GetThrottle() - 10);
    }

    // Calcul de l'altitude cible
    float horizontal_distance = 1000.0f;
    float vertical_distance = this->target_climb - this->plane->y;
    float target_elevation = radToDegree(atan2(vertical_distance, horizontal_distance))*10.0f;
    
    // Calcul de l'écart de cap
    float target_yaw = (float)(3600.0f - this->target_azimut);
    float yaw_difference = this->plane->yaw - target_yaw;
        
    // Normaliser la différence entre -1800 et 1800 (entre -180° et 180°)
    while (yaw_difference > 1800.0f) yaw_difference -= 3600.0f;
    while (yaw_difference < -1800.0f) yaw_difference += 3600.0f;
        
    float dt = 0.1f; // Pas de temps

    // Déterminer la direction du virage et la magnitude du virage
    float turn_magnitude = std::abs(yaw_difference) / 1800.0f; // Normaliser entre 0 et 1
    
    if (yaw_difference > 10.0f) {
        turnState = TURN_LEFT;
    } else if (yaw_difference < -10.0f) {
        turnState = TURN_RIGHT;
    } else {
        turnState = TURN_NONE;
        if (std::abs(yaw_difference) <= 1.0f) {
            turning = false;
        }
    }
    
    // Ajustement du manche basé sur l'état de virage et l'altitude
    switch (turnState) {
        case TURN_LEFT:
            // Pour tourner à gauche, nous voulons un roll de 270° (2700)
            {
                float roll_error = 0;
                // Si le roll actuel est entre 0 et 180, prendre le chemin le plus court vers 270
                if (this->plane->roll >= 0 && this->plane->roll <= 1800) {
                    roll_error = 2700.0f - this->plane->roll;
                } 
                // Si le roll est entre 180 et 360, il peut être plus court de "dérouler"
                else {
                    roll_error = -(this->plane->roll - 2700.0f);
                    // Assurer que nous prenons le chemin le plus court
                    if (roll_error < -1800.0f) roll_error += 3600.0f;
                }
                
                // Utiliser PID pour déterminer la position du manche en x
                // Multiplie par un facteur proportionnel à l'amplitude du virage nécessaire
                float stick_adjustment = -rollController.calculate(0, roll_error, dt);
                
                // Application plus progressive pour les grands changements
                if (std::abs(roll_error) > 900.0f) {
                    this->plane->control_stick_x = stick_adjustment * 0.8f;  // 80% de la puissance pour grands changements
                } else {
                    this->plane->control_stick_x = stick_adjustment;
                }
                
                // Limiter les valeurs du manche à l'intervalle -160 à 160
                this->plane->control_stick_x = (std::max)(-160, (std::min)(160, this->plane->control_stick_x));
                
                // Une fois proche de l'inclinaison cible, ajuster le manche en y pour maintenir l'altitude
                if (std::abs(roll_error) < 200.0f) {
                    // En virage, nous avons besoin de plus de portance, donc tirez légèrement sur le manche
                    // Pull-up proportionnel à l'intensité du virage (max -20)
                    this->plane->control_stick_y = -20.0f * turn_magnitude;
                }
            }
            break;
                
        case TURN_RIGHT:
            // Pour tourner à droite, nous voulons un roll de 90° (900)
            {
                float roll_error = 0;
                // Calculer l'erreur pour atteindre 900
                if (this->plane->roll >= 1800 && this->plane->roll <= 3600) {
                    roll_error = -(this->plane->roll - 900.0f);
                    if (roll_error < -1800.0f) roll_error += 3600.0f;
                } else {
                    roll_error = 900.0f - this->plane->roll;
                }
                
                // Utiliser PID pour déterminer la position du manche en x
                float stick_adjustment = rollController.calculate(0, roll_error, dt);
                
                // Application plus progressive pour les grands changements
                if (std::abs(roll_error) > 900.0f) {
                    this->plane->control_stick_x = stick_adjustment * 0.8f;  // 80% de la puissance pour grands changements
                } else {
                    this->plane->control_stick_x = stick_adjustment;
                }
                
                // Limiter les valeurs du manche à l'intervalle -160 à 160
                this->plane->control_stick_x = (std::max)(-160, (std::min)(160, this->plane->control_stick_x));
                
                // Une fois incliné, ajuster le manche en y
                if (std::abs(roll_error) < 200.0f) {
                    // Pull-up proportionnel à l'intensité du virage (max -20)
                    this->plane->control_stick_y = -20.0f * turn_magnitude;
                }
            }
            break;
                
        case TURN_NONE:
            // Stabiliser l'avion à l'horizontale (roll = 0)
            {
                float roll_error = 0;
                if (this->plane->roll > 1800.0f) {
                    roll_error = -(this->plane->roll - 3600.0f);
                } else {
                    roll_error = -this->plane->roll;
                }
                
                this->plane->control_stick_x = rollController.calculate(0, roll_error, dt);
                this->plane->control_stick_x = (std::max)(-160, (std::min)(160, this->plane->control_stick_x));
            }
            break;
    }
    
    // Contrôle d'altitude
    float pitch_error = target_elevation - this->plane->pitch;
    float pitch_control = altitudeController.calculate(0, pitch_error, dt);
    
    // Ne pas écraser les ajustements de pitch pour les virages si on est en train de tourner
    if (turnState == TURN_NONE || std::abs(pitch_error) > 100.0f) {
        // Ajustement plus doux pour le manche Y si déjà en ajustement de virage
        if (turnState != TURN_NONE) {
            pitch_control *= 0.5f; // Réduire l'effet pour ne pas perturber le virage
        }
        
        this->plane->control_stick_y -= pitch_control; // Inverser car -y = monter, +y = descendre
        this->plane->control_stick_y = (std::max)(-100, (std::min)(100, this->plane->control_stick_y));
    }
    
    // Vérifier si on a atteint le cap cible
    if (std::abs(yaw_difference) <= 1.0f) {
        turning = false;
    }
}
