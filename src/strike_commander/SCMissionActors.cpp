#include "precomp.h"
#include "SCMissionActors.h"

bool SCMissionActors::execute() {
    return true;
}
/**
 * SCMissionActors::takeOff
 *
 * Called when the "take off" mission objective is triggered.
 *
 * If the actor has already taken off, this function does nothing and
 * returns true.
 *
 * Otherwise, this function sets the actor's current objective to
 * OP_SET_OBJ_TAKE_OFF and sets the pilot's target climb to 300 units above
 * the current height.  If the actor is close enough to the target height,
 * the actor is marked as having taken off and the function returns true.
 *
 * @param arg Unused argument.
 *
 * @return True if the actor has taken off, false otherwise.
 */
bool SCMissionActors::takeOff(uint8_t arg) {
    if (taken_off) {
        return true;
    }
    this->current_objective = OP_SET_OBJ_TAKE_OFF;
    if (this->pilot->target_climb == 0) {
        this->pilot->target_speed = -15;
        this->pilot->target_climb = (int) (this->plane->y + 300.0f);
        this->pilot->target_azimut = this->plane->yaw;
    }
    if (std::abs(this->plane->y-this->pilot->target_climb) < 10.0f) {
        this->taken_off = true;
    }
    return this->taken_off;
}
bool SCMissionActors::land(uint8_t arg) {
    this->current_objective = OP_SET_OBJ_LAND;
    auto it = std::find(this->mission->friendlies.begin(), this->mission->friendlies.end(), this);
    if (it != this->mission->friendlies.end()) {
        this->mission->friendlies.erase(it);
    }
    if (arg < this->mission->mission->mission_data.spots.size()) {
        SPOT *wp = this->mission->mission->mission_data.spots[arg];
        this->pilot->SetTargetWaypoint(wp->position);
        this->pilot->target_speed = -10;
        Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
        Vector3D diff = wp->position - position;
        float dist = diff.Length();
        const float landing_dist = 3000.0f;
        if (dist < landing_dist) {
            this->pilot->turning = false;
            this->pilot->land = true;
            this->pilot->target_speed = (int) (-10.0f * (dist/landing_dist));
        }
        if (dist < 2000.0f) {
            return true;
        }
    }
    return false;
}
bool SCMissionActors::flyToWaypoint(uint8_t arg) {
    this->current_objective = OP_SET_OBJ_FLY_TO_WP;
    if (arg < this->mission->mission->mission_data.spots.size()) {
        SPOT *wp = this->mission->mission->mission_data.spots[arg];
        if (this->pilot != nullptr) {
            this->pilot->SetTargetWaypoint(wp->position);
            this->pilot->target_speed = -10;
            Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
            Vector3D diff = wp->position - position;
            float dist = diff.Length();
            if (dist < 3000.0f) {
                return true;
            }
        }
    }
    
    return false;
}
bool SCMissionActors::flyToArea(uint8_t arg) {
    this->current_objective = OP_SET_OBJ_FLY_TO_AREA;
    if (arg < this->mission->mission->mission_data.areas.size()) {
        AREA *area = this->mission->mission->mission_data.areas[arg];
        this->pilot->SetTargetWaypoint(area->position);
        this->pilot->target_speed = -10;
        Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
        Vector3D diff = area->position - position;
        float dist = diff.Length();
        if (dist < 3000.0f) {
            return true;
        }
    }
    return false;
}
/**
 * SCMissionActors::destroyTarget
 *
 * Called when the "destroy target" mission objective is triggered.
 *
 * This function sets the actor's current objective to
 * OP_SET_OBJ_DESTROY_TARGET and sets the pilot's target waypoint to the
 * position of the target actor.  If the target actor is not on the ground,
 * the pilot's target climb is set to the target actor's current height and
 * the target speed is set to either -60 or the target actor's current
 * vertical speed, depending on how far away the actor is.
 *
 * If the target actor is already destroyed, this function returns true.
 *
 * @param arg The ID of the target actor to destroy.
 *
 * @return True if the target actor is already destroyed, false otherwise.
 */
bool SCMissionActors::destroyTarget(uint8_t arg) {
    Vector3D wp;
    this->current_objective = OP_SET_OBJ_DESTROY_TARGET;
    this->current_target = arg;
    Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            if (actor->plane == nullptr) {
                return true;
            }
            if (!actor->plane->object->alive) {
                return true;
            }
            wp.x = actor->plane->x;
            wp.y = actor->plane->y;
            wp.z = actor->plane->z;
            wp = wp + actor->attack_pos_offset;
            if (target_position.Length() == 0.0f) {
                target_position = wp;
                target_position_update = 2000;
            }
            Vector3D target_position_diff = target_position - wp;
            if (target_position_diff.Length() > 0.0f && target_position_update == 0) {
                target_position = wp;
                target_position_update = 2000;
            } else if (target_position_diff.Length() > 0.0f) {
                target_position_update -= 1;
            }
            this->pilot->SetTargetWaypoint(wp);
            Vector3D diff = wp - position;
            float dist = diff.Length();
            if (!actor->plane->on_ground) {
                this->pilot->target_climb = (int) wp.y;
                if (dist > 1000.0f) {
                    this->pilot->target_speed = -60;
                } else if (dist > 300.0f) {
                    this->pilot->target_speed = (int) actor->plane->vz;
                    if (this->plane->weaps_object.size() < 40) {
                        this->plane->Shoot(0, actor, this->mission);    
                    }
                }
            }
            return false;
        }
    }
    return false;
}
/**
 * SCMissionActors::defendTarget
 *
 * Called when the "defend target" mission objective is triggered.
 *
 * This function sets the actor's current objective to OP_SET_OBJ_DEFEND_TARGET.
 * It first checks if there is an existing goal to destroy a target. If so,
 * it attempts to destroy that target and resets the goal if successful.
 *
 * If no current goal exists, the function determines the area in which the actor is located
 * and checks for any enemies within the same area. If an enemy is found, it sets the goal
 * to destroy that enemy and attempts to do so.
 *
 * @param arg Unused argument.
 *
 * @return True if no action is taken or if the current goal is successfully completed,
 *         false otherwise.
 */

bool SCMissionActors::defendTarget(uint8_t arg) {
    this->current_objective = OP_SET_OBJ_DEFEND_TARGET;
    if (this->profile->ai.goal[0] != 0) {
        bool ret = this->destroyTarget(this->profile->ai.goal[0]);
        if (ret) {
            this->profile->ai.goal[0] = 0;
        }
        return ret;
    }
    Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
    uint8_t area_id = this->mission->getAreaID(position);
    for (auto actor: this->mission->enemies) {
        if (actor->plane != nullptr) {
            uint8_t actor_area_id = this->mission->getAreaID({actor->plane->x, actor->plane->y, actor->plane->z});
            if (actor_area_id == area_id) {
                this->profile->ai.goal[0] = actor->actor_id;
                bool ret = this->destroyTarget(actor->actor_id);
                if (ret) {
                    this->profile->ai.goal[0] = 0;
                }
                return ret;
            }
        }
    }
    return true;
}
/**
 * @brief Deactivates an actor in the mission based on the provided actor ID.
 *
 * This function iterates through the list of actors in the mission and sets the
 * `is_active` flag to `false` for the actor whose `actor_id` matches the provided argument.
 *
 * @param arg The ID of the actor to deactivate.
 * @return true if an actor with the specified ID was found and deactivated, false otherwise.
 */
bool SCMissionActors::deactivate(uint8_t arg) {
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            actor->is_active = false;
            return true;
        }
    }
    return false;
}
/**
 * @brief Sets a message for the mission actor.
 * 
 * This function sets a message for the mission actor based on the provided argument.
 * 
 * @param arg The index of the message to be set.
 * @return true Always returns true.
 */
bool SCMissionActors::setMessage(uint8_t arg) {   
    if (arg >= this->profile->radi.msgs.size()) {
        return true;
    }
    std::string *message = new std::string();
    *message = this->profile->radi.info.callsign + ": " + this->profile->radi.msgs[arg];
    this->mission->radio_messages.push_back(message);
    printf("Message  %s\n", message->c_str()); 
    return true;
}
/**
 * @brief Sets the current objective to follow an ally and adjusts the pilot's target waypoint, speed, and climb.
 * 
 * This function iterates through the mission's actors to find the actor with the specified ID.
 * If the actor is found, it sets the waypoint to the actor's position plus the formation position offset,
 * and adjusts the pilot's target waypoint, speed, and climb based on the distance to the waypoint and whether
 * the actor's plane is on the ground.
 * 
 * @param arg The ID of the ally actor to follow.
 * @return true if the ally actor with the specified ID is found and the objective is set, false otherwise.
 */
bool SCMissionActors::followAlly(uint8_t arg) {
    Vector3D wp;
    this->current_objective = OP_SET_OBJ_FOLLOW_ALLY;
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            wp.x = actor->plane->x;
            wp.y = actor->plane->y;
            wp.z = actor->plane->z;
            wp = wp + actor->formation_pos_offset;
            this->pilot->SetTargetWaypoint(wp);
            Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
            Vector3D diff = wp - position;
            float dist = diff.Length();
            if (!actor->plane->on_ground) {
                this->pilot->target_climb = (int) wp.y;
                if (dist > 1000.0f) {
                    this->pilot->target_speed = -60;
                } else if (dist < 100.0f) {
                    this->pilot->target_speed = (int) actor->plane->vz;
                    this->pilot->turning = false;
                }
            }
            
            return true;
        }
    }
    return false;
}
/**
 * SCMissionActors::ifTargetInSameArea
 *
 * Returns true if the actor with the given ID is in the same area as
 * the current actor, false otherwise.
 *
 * @param arg The ID of the target actor to check.
 *
 * @return True if the target actor is in the same area as the current
 * actor, false otherwise.
 */
bool SCMissionActors::ifTargetInSameArea(uint8_t arg) {
    Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
    Uint8 area_id = this->mission->getAreaID(position);
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            if (actor->is_active == true && !actor->is_destroyed) {
                return true;
            }
            /*if (actor->plane == nullptr) {
                return (area_id == actor->object->area_id);
            }
            Uint8 target_area_id = this->mission->getAreaID({actor->plane->x, actor->plane->y, actor->plane->z});
            if (area_id == target_area_id) {
                return true;
            }*/
        }
    }
    return false;
}
/**
 * @brief Activates the target actor with the specified ID.
 *
 * This function iterates through the list of actors in the mission and sets the
 * `is_active` flag to true for the actor whose `actor_id` matches the provided argument.
 *
 * @param arg The ID of the actor to be activated.
 * @return true if the actor with the specified ID was found and activated, false otherwise.
 */
bool SCMissionActors::activateTarget(uint8_t arg) {
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg && actor->is_active == false && actor->is_destroyed == false) {
            actor->is_active = true;
            
            Vector3D correction = {
                this->mission->player->plane->x,
                this->mission->player->plane->y,
                this->mission->player->plane->z
            };
            actor->object->position += correction;
            if (actor->plane != nullptr) {
                actor->plane->on_ground = false;
                actor->plane->x = actor->object->position.x;
                actor->plane->y = actor->object->position.y;
                actor->plane->z = actor->object->position.z;
            }
            
            if (actor->on_is_activated.size() > 0) {
                SCProg *p = new SCProg(actor, actor->on_is_activated, this->mission);
                p->execute();
            }
            return true;
        }
    }
    return false;
}

int SCMissionActors::getDistanceToTarget(uint8_t arg) {
    Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
    Vector3D diff = this->mission->actors[arg]->plane->position - position;
    return (int) diff.Length()/1000;
}

int SCMissionActors::getDistanceToSpot(uint8_t arg) { 
    Vector3D position = this->mission->mission->mission_data.spots[arg]->position;
    Vector3D plane_pos = {this->plane->x, this->plane->y, this->plane->z};
    Vector3D diff = plane_pos - position;
    return (int) diff.Length()/1000;
}
void SCMissionActors::shootWeapon(SCMissionActors *target) {
    if (this->object->entity->entity_type != EntityType::swpn) {
        return;
    }
    if (this->object->entity->swpn_data == nullptr) {
        return;
    }
    if (this->object->entity->swpn_data->weapon_entity == nullptr) {
        return;
    }
    if (this->weapons_shooted.size()> 20) {
        return; // Too many weapons already shot
    }
    if (this->object->entity->swpn_data->max_simultaneous_shots > 0 && this->weapons_shooted.size() >= this->object->entity->swpn_data->max_simultaneous_shots) {
        return;
    }
    if (this->object->entity->swpn_data->weapons_round <= 0) {
        return; // No ammo left
    }
    RSEntity *weapon_entity = this->object->entity->swpn_data->weapon_entity;
    Vector3D direction = target->object->position - this->object->position;
    if (direction.Length() > this->object->entity->swpn_data->effective_range) {
        return; // No direction to shoot
    }
    direction.Normalize();

    SCSimulatedObject *weapon = nullptr;
    switch (weapon_entity->wdat->weapon_category) {
        case 2: // Missiles
            weapon = new SCSimulatedObject();
            weapon->obj = weapon_entity;
            weapon->x = this->object->position.x;
            weapon->y = this->object->position.y;
            weapon->z = this->object->position.z;

            
            weapon->vx = direction.x * weapon_entity->dynn_miss->velovity_m_per_sec / 100.0f;
            weapon->vy = direction.y * weapon_entity->dynn_miss->velovity_m_per_sec / 100.0f;
            weapon->vz = direction.z * weapon_entity->dynn_miss->velovity_m_per_sec / 100.0f;
            break;
        case 0: // Guns
            weapon = new GunSimulatedObject();
            weapon->obj = weapon_entity;
            weapon->x = this->object->position.x;
            weapon->y = this->object->position.y;
            weapon->z = this->object->position.z;

            
            weapon->vx = direction.x *  100.0f;
            weapon->vy = direction.y *  100.0f;
            weapon->vz = direction.z *  100.0f;
            break;
        default:
            return;
    }
    weapon->mission = this->mission;
    weapon->shooter = this;
    this->object->entity->swpn_data->weapons_round--;
    this->weapons_shooted.push_back(weapon);
}
/**
 * SCMissionActorsPlayer::takeOff
 *
 * Sets the current objective to a take-off objective with the given
 * argument as the target spot ID.
 *
 * @param arg The ID of the target spot to take off from.
 *
 * @return True if the objective was set successfully, false otherwise.
 */
bool SCMissionActorsPlayer::takeOff(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("take off");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
/**
 * SCMissionActorsPlayer::land
 *
 * Sets the current objective to a land objective with the given
 * argument as the target spot ID.
 *
 * @param arg The ID of the target spot to land on.
 *
 * @return True if the objective was set successfully, false otherwise.
 */
bool SCMissionActorsPlayer::land(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("landing");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
/**
 * Sets the current objective to a fly-to-waypoint objective with the given
 * argument as the target waypoint ID.
 *
 * @param arg The ID of the target waypoint to fly to.
 *
 * @return True if the objective was set successfully, false otherwise.
 */
bool SCMissionActorsPlayer::flyToWaypoint(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    if (arg >= this->mission->mission->mission_data.spots.size()) {
        return false; // Invalid waypoint ID
    }
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Fly to\nWay Point");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
/**
 * SCMissionActorsPlayer::flyToArea
 *
 * Sets the current objective to a fly-to-waypoint objective with the given
 * argument as the target waypoint ID.
 *
 * @param arg The ID of the target waypoint to fly to.
 *
 * @return True if the objective was set successfully, false otherwise.
 */
bool SCMissionActorsPlayer::flyToArea(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Fly to\nWay Area");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
/**
 * Sets the current objective to a destroy-target objective with the given
 * argument as the target object ID.
 *
 * @param arg The ID of the target object to destroy.
 *
 * @return True if the objective was set successfully, false otherwise.
 */
bool SCMissionActorsPlayer::destroyTarget(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Destroy\nTarget");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
/**
 * SCMissionActorsPlayer::defendTarget
 *
 * Sets the current objective to a defend-target objective with the given
 * argument as the target object ID.
 *
 * @param arg The ID of the target object to defend.
 *
 * @return True if the objective was set successfully, false otherwise.
 */
bool SCMissionActorsPlayer::defendTarget(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Defend\nAlly");
    this->mission->waypoints.push_back(waypoint);
    return true;
}

/**
 * SCMissionActorsPlayer::setMessage
 *
 * Sets the current objective to display a message with the given
 * argument as the message ID.
 *
 * @param arg The ID of the message to display.
 *
 * @return True if the objective was set successfully, false otherwise.
 */
bool SCMissionActorsPlayer::setMessage(uint8_t arg) {
    if (arg >= this->profile->radi.msgs.size()) {
        return true;
    }
    std::transform(this->mission->mission->mission_data.messages[arg]->begin(), this->mission->mission->mission_data.messages[arg]->end(), this->mission->mission->mission_data.messages[arg]->begin(), ::tolower);
    if (this->mission->waypoints.size() > 0) {
        this->mission->waypoints.back()->message = this->mission->mission->mission_data.messages[arg];
    }
    return true;
}

bool SCMissionActorsStrikeBase::setMessage(uint8_t arg) {
    this->mission->radio_messages.push_back(&this->profile->radi.msgs[arg]);
    printf("Message  %s\n", this->profile->radi.msgs[arg].c_str()); 
    return true;
}
/*
@TODO
Fly to#Precise Way
Defend#Point
Follow#Leader
*/