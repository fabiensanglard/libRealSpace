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
    float ground_y = this->plane->area->getY(waypoint.x, waypoint.z);
    float ground_level = this->plane->area->getY(this->plane->x, this->plane->z);
    if (this->plane->y < ground_y + 500) {
        waypoint.y = ground_y + 500;
    }
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
    if (!this->plane->on_ground && this->plane->GetWheel()) {
        this->plane->SetWheel();
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
        turning = true;
    } else if (yaw_difference < -10.0f) {
        turnState = TURN_RIGHT;
        turning = true;
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
    turnRate = fabs(yaw_difference) / 450.0f;
    if (turnRate < 1.0f) {
        turnRate = 1.0f; // Assurer un minimum de virage
    }
    targetRoll = 0.0f;
    if (turnState == TURN_LEFT) {
        // Pour les virages à gauche, incliner vers 2700 (aile gauche vers le bas)
        targetRoll = 2700.0f-(900.0f-(900.0f/turnRate)); 
    } else if (turnState == TURN_RIGHT) {
        // Pour les virages à droite, incliner vers 900 (aile droite vers le bas)
        targetRoll = 900.0f-(900.0f-(900.0f/turnRate));
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
                turning = false;
            }
            break;
    }    
}

void SCPilot::DirectAutoPilot() {
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
    
    // Gestion de la poussée et du déplacement au sol
    if (this->plane->on_ground && this->target_climb == 0) {
        this->plane->SetThrottle(0);
        return;
    }
    if (this->plane->vz > this->target_speed) {
        this->plane->SetThrottle(100);
    } else {
        this->plane->SetThrottle(this->plane->GetThrottle() - 10);
    }

    float delta_time = 1.0f / this->plane->tps;
    
    // === CONTRÔLE DU TANGAGE (PITCH) ===
    // Calcul de l'élévation cible basée sur l'altitude désirée
    float horizontal_distance = 1000.0f;
    float vertical_distance = this->target_climb - this->plane->y;
    float target_elevation = radToDegree(atan2(vertical_distance, horizontal_distance)) * 10.0f;
    
    // Conversion en valeur control_stick_y (-100 à 100)
    // Pendant les virages, maintenir un pitch légèrement positif
    int pitch_control;
    if (turnState != TURN_NONE && this->plane->y < this->target_climb - 100) {
        // Maintenir un angle de montée léger pendant les virages pour compenser la perte d'altitude
        pitch_control = 20; // Valeur modérée positive pour un angle de montée doux
    } else {
        // Sinon, viser l'altitude cible normalement
        pitch_control = (int)(target_elevation / 4.5f); // 450 / 4.5 = 100 (pleine échelle)
    }
    
    // Limiter aux valeurs autorisées
    this->plane->control_stick_y = std::clamp(pitch_control, -100, 100);
    
    // === CONTRÔLE DU ROULIS (ROLL) ET DU CAP (YAW) ===
    float target_yaw = (float)(3600.0f - this->target_azimut);
    float yaw_difference = this->plane->yaw - target_yaw;
    
    // Normaliser la différence entre -1800 et 1800 (entre -180° et 180°)
    while (yaw_difference > 1800.0f) yaw_difference -= 3600.0f;
    while (yaw_difference < -1800.0f) yaw_difference += 3600.0f;
        
    // Déterminer la direction du virage (comme dans le code original)
    if (yaw_difference > 10.0f) {
        turnState = TURN_LEFT;
    } else if (yaw_difference < -10.0f) {
        turnState = TURN_RIGHT;
    } else {
        turnState = TURN_NONE;
        // On est presque aligné, on remet le stick au centre
        this->plane->control_stick_x = 0;
        
        if (std::abs(yaw_difference) <= 1.0f) {
            //turning = false;
        }
    }

    // Taux de virage basé sur l'écart (comme dans le code original)
    turnRate = (std::min)(5.0f, std::abs(yaw_difference) / 100.0f);
    if (turnRate < 0.5f) turnRate = 0.5f;
    
    // Calculer la valeur control_stick_x en fonction de la direction et du taux de virage
    int roll_control = 0;
    
    switch (turnState) {
        case TURN_LEFT:
            // Pour les virages à gauche, valeur négative
            // Plus l'écart est grand, plus le stick est poussé à gauche
            roll_control = -(int)(turnRate * 32.0f); // 5 * 32 = 160 (valeur max)
            
            // Ajuster selon l'état actuel du roulis (comme dans le code original)
            // Si l'avion est déjà suffisamment incliné (roll entre 2500 et 2900),
            // réduire l'entrée pour stabiliser l'inclinaison
            if (this->plane->roll > 2500.0f && this->plane->roll < 2900.0f) {
                roll_control = roll_control / 2; // Réduire la commande pour stabiliser
            }
            break;
            
        case TURN_RIGHT:
            // Pour les virages à droite, valeur positive
            roll_control = (int)(turnRate * 32.0f);
            
            // Ajuster selon l'état actuel du roulis
            if (this->plane->roll > 700.0f && this->plane->roll < 1100.0f) {
                roll_control = roll_control / 2; // Réduire la commande pour stabiliser
            }
            break;
            
        case TURN_NONE:
            // Pour redresser l'avion, utiliser une valeur qui dépend de l'angle actuel
            if (this->plane->roll > 0 && this->plane->roll < 1800.0f) {
                // Si incliné à droite, pousser légèrement à gauche
                roll_control = -20;
            } else if (this->plane->roll > 1800.0f && this->plane->roll < 3600.0f) {
                // Si incliné à gauche, pousser légèrement à droite
                roll_control = 20;
            }
            
            // Si presque à plat, ne rien faire
            if (this->plane->roll < 100.0f || this->plane->roll > 3500.0f) {
                roll_control = 0;
            }
            break;
    }
    
    // Limiter aux valeurs autorisées
    this->plane->control_stick_x = std::clamp(roll_control, -160, 160);
}