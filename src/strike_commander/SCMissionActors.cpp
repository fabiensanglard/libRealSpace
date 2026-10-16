#include "precomp.h"
#include "SCAIBrain.h"
#include "SCMissionActors.h"
#include "../engine/gametimer.h"
#include <cstdlib>
#include <ctime>

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
    if (!this->talkative) {
        return false;
    }
    for (auto message: this->profile->radi.msgs) {
        if (message.first == arg) {
            std::string *message = new std::string();
            *message = this->profile->radi.info.callsign + ": " + this->profile->radi.msgs[arg];
            RadioMessages *msg = new RadioMessages();
            msg->message = *message;
            
            if (this->mission->sound.inGameVoices.size() > 0) {
                
                if (this->mission->sound.inGameVoices.find(this->profile->radi.spch) != this->mission->sound.inGameVoices.end()) {
                    if (this->mission->sound.inGameVoices[this->profile->radi.spch]->messages.find(arg) != this->mission->sound.inGameVoices[this->profile->radi.spch]->messages.end()) {
                        MemSound *message_sound = this->mission->sound.inGameVoices[this->profile->radi.spch]->messages[arg];
                        msg->sound = message_sound;
                    }
                }
                
                
            }
            this->mission->radio_messages.push_back(msg);
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
    Vector3D position;
    if (this->plane == nullptr) {
        position = {this->object->position.x, this->object->position.y, this->object->position.z};
    } else {
        position = {this->plane->x, this->plane->y, this->plane->z};
    }
    Uint8 area_id = this->mission->getAreaID(position);
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            if (actor->is_destroyed) {
                return false;
            }
            if (actor->is_active == false) {
                return false;
            }
            if (actor->is_active == true && !actor->is_destroyed) {
                return true;
            }
            if (actor->plane == nullptr) {
                return (area_id == actor->object->area_id);
            }
            Uint8 target_area_id = this->mission->getAreaID({actor->plane->x, actor->plane->y, actor->plane->z});
            if (area_id == target_area_id) {
                return true;
            }
        }
    }
    return false;
}
bool SCMissionActors::respondToRadioMessage(int message_id, SCMission *mission, SCMissionActors *sender) {
    int cpt = 1;
    float health_remaing = this->health / (float) this->object->entity->health * 100.0f;
    bool obeying = true;
    for (auto ask: this->profile->radi.opts) {
        if (cpt == message_id) {
            switch (ask) {
                case 'd':
                {
                    // request status 2 - 5 
                    
                    if (health_remaing > 75.0f) {
                        this->setMessage(2); // All is well
                    } else if (health_remaing > 50.0f) {
                        this->setMessage(3); // Minor damage
                    } else if (health_remaing > 25.0f) {
                        this->setMessage(4); // Major damage
                    } else {
                        this->setMessage(5); // Critical damage
                    }
                }
                break;
                case 'e':
                    // request take off
                break;
                case 'f':
                    // request land 
                break;
                case 'g':
                {
                    uint8_t area_to_defend = this->mission->getAreaID({this->plane->x, this->plane->y, this->plane->z});
                    this->override_progs.clear();
                    this->override_progs.push_back({prog_op::OP_SET_OBJ_DEFEND_AREA, area_to_defend});
                    this->setMessage(1);
                }
                break;
                case 'h':
                    // build formation
                    this->override_progs.clear();
                    this->setMessage(1);
                break;
                case 'i':
                    // attack my target
                    if (sender != nullptr && sender->target != nullptr) {
                        if (this->current_command == prog_op::OP_SET_OBJ_DEFEND_TARGET || this->current_command == prog_op::OP_SET_OBJ_DESTROY_TARGET) {
                            obeying = std::rand() % 16 < this->profile->ai.atrb.LY;
                        }
                        if (obeying) {
                            this->override_progs.clear();
                            this->override_progs.push_back({prog_op::OP_SET_OBJ_DESTROY_TARGET, sender->target->actor_id});
                            this->setMessage(1);
                        } else {
                            this->setMessage(0);
                        }
                    }
                break;
                case 'j':
                    if (this->current_command == prog_op::OP_SET_OBJ_DEFEND_TARGET || this->current_command == prog_op::OP_SET_OBJ_DESTROY_TARGET || health_remaing > 75.0f) {
                        obeying = std::rand() % 16 < this->profile->ai.atrb.LY;
                    }
                    if (health_remaing < 30.0f) {
                        obeying = true;
                    }
                    if (obeying) {
                        this->override_progs.clear();
                        this->override_progs.push_back({prog_op::OP_SET_OBJ_LAND, 0});
                        this->setMessage(1);
                    } else {
                        this->setMessage(0);
                    }
                    
                break;
                case 'k':
                    if (sender != nullptr) {
                        this->override_progs.clear();
                        this->override_progs.push_back({prog_op::OP_SET_OBJ_DEFEND_TARGET, sender->actor_id});
                        this->setMessage(1);
                    }
                break;
                case 'l':
                    // maintain radio silence
                    this->talkative = false;
                break;
                case 'm':
                    // break radio silence
                    this->talkative = true;
                    this->setMessage(1);
                break;
                default:
                break;
            }
        }
        cpt++;
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
            actor->is_hidden = false;
            Vector3D correction = {0.0f, 0.0f, 0.0f};
            if (actor->object->area_id != 255 && actor->object->unknown2 == 0) {
                bool spot_found = false;
                for (auto spot: this->mission->mission->mission_data.spots) {
                    if (spot->area_id == actor->object->area_id) {
                        correction = spot->position;
                        spot_found = true;
                        break;
                    }
                }
                if (!spot_found) {
                    correction = this->mission->mission->mission_data.areas[actor->object->area_id]->position;
                }
            } else {
                correction = {
                    this->mission->player->plane->x,
                    this->mission->player->plane->y,
                    this->mission->player->plane->z
                };
            }
            if ((actor->object->area_id != 255) || (actor->object->area_id == 255 && actor->object->unknown2 == 1)) {
                actor->object->position += correction;
            }
            float ground_y = this->mission->area->getY(actor->object->position.x, actor->object->position.z);
            if (actor->object->position.y < ground_y) {
                actor->object->position.y = ground_y;
            } else {
                if (actor->plane != nullptr) {
                    actor->pilot->target_climb = (int) (actor->object->position.y);
                    actor->pilot->target_speed = -20;
                    actor->plane->vz = -20;
                }
            }
            if (actor->plane != nullptr) {
                if (actor->object->position.y <= ground_y) {
                    actor->object->position.y = ground_y+10.0f;
                    actor->plane->on_ground = true;
                } else {
                    actor->plane->on_ground = false;
                }
                actor->plane->x = actor->object->position.x;
                actor->plane->y = actor->object->position.y;
                actor->plane->z = actor->object->position.z;
            }
            if (actor->plane == nullptr) {
                if (actor->object->position.y > ground_y) {
                    actor->object->position.y = ground_y+2.0f;
                } else if (actor->object->position.y < ground_y) {
                    actor->object->position.y = ground_y+2.0f;
                }
            }
            actor->wait_timer = 0.0f;
            if (actor->on_is_activated.size() > 0) {
                SCProg *p = new SCProg(actor, actor->on_is_activated, this->mission, actor->object->on_is_activated);
                p->execute();
            }
            if (actor->object->entity->entity_type == EntityType::rnwy) {
                for (auto runway: this->mission->area->objectOverlay) {
                    Vector3D pos = actor->object->position;
                    
                    // Vérifier si la position de l'objet est à l'intérieur de la piste
                    if (pos.x >= runway.lx && pos.x <= runway.hx && 
                        pos.z >= runway.ly && pos.z <= runway.hy) {
                        
                        // Calculer les dimensions de la piste
                        float width = (float)std::abs(runway.lx - runway.hx); 
                        float length = (float)std::abs(runway.ly - runway.hy);
                        
                        // Calculer l'orientation (angle) de la piste
                        float angle = (float)std::atan2(runway.ly - runway.hy, 
                                                runway.lx - runway.hx);
                        
                        // Recalculer la bounding box
                        actor->object->entity->bb.min.x = -width / 2.0f;
                        actor->object->entity->bb.max.x = width / 2.0f;
                        actor->object->entity->bb.min.y = -5;
                        actor->object->entity->bb.max.y = 5;
                        actor->object->entity->bb.min.z = -length / 2.0f;
                        actor->object->entity->bb.max.z = length / 2.0f;
                        
                        // Appliquer l'orientation à l'objet
                        actor->object->azymuth = angle * 180.0f / M_PI;
                        
                        break;
                    }
                }
            }
            return true;
        }
    }
    return false;
}
/**
 * SCMissionActors::setObjective
 *
 * Pose l'objectif courant sans l'executer — voir la note dans
 * SCMissionActors.h. Ne reinitialise current_command_executed que lors
 * d'une vraie transition (commande ou argument different de l'actuel) :
 * le script PROG repasse par cet appel a chaque frame tant que l'objectif
 * reste le meme, et ne doit donc pas ecraser le resultat calcule par le
 * dernier passage du GOAL loop (sinon OP_GOTO_IF_CURRENT_COMMAND_IN_PROGRESS
 * ne verrait jamais l'objectif comme termine).
 */
bool SCMissionActors::setObjective(prog_op command, uint8_t arg) {
    if (this->brain != nullptr && this->brain->objective_locked) {
        this->brain->onObjectiveSet(false);
        return true;
    }
    if (command == OP_SET_OBJ_FLY_TO_AREA) {
        return false;
    }
    if (this->current_command != command || this->current_command_arg != arg) {
        this->current_command_executed = false;
    }
    this->current_command = command;
    this->current_command_arg = arg;
    if ((command == OP_SET_OBJ_DESTROY_TARGET || command == OP_SET_OBJ_DEFEND_TARGET) && (this->target == nullptr || this->target->actor_id != arg)) {
        for (auto actor : this->mission->actors) {
            if (actor->actor_id == arg && !actor->is_destroyed) {
                this->target = actor;
                break;
            }
        }
    }
    if (this->brain != nullptr) {
        this->brain->onObjectiveSet(true);
    }
    return !this->current_command_executed;
}

int SCMissionActors::getDistanceToTarget(uint8_t arg) {
    Vector3D position;
    if (this->plane == nullptr) {
        position = {this->object->position.x, this->object->position.y, this->object->position.z};
    } else {
        position = {this->plane->x, this->plane->y, this->plane->z};
    }
    Vector3D diff;
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            if (actor->plane == nullptr) {
                diff = actor->object->position - position;
            } else {
                Vector3D target_pos = {actor->plane->x, actor->plane->y, actor->plane->z};
                diff = target_pos - position;
            }
            break;
        }
    }
    return (int) diff.Length()/1000;
}

int SCMissionActors::getDistanceToSpot(uint8_t arg) { 
    Vector3D position = this->mission->mission->mission_data.spots[arg]->position;
    Vector3D plane_pos = {this->plane->x, this->plane->y, this->plane->z};
    Vector3D diff = plane_pos - position;
    return (int) diff.Length()/1000;
}
void SCMissionActors::hasBeenHit(SCSimulatedObject *weapon, SCMissionActors *attacker) {
    this->last_hit_by = attacker;
    if (this->brain != nullptr) {
        this->brain->just_hit = true;
        this->brain->last_attacker = attacker;
    }
    int damage = weapon->obj->wdat->damage;
    if (this->plane != nullptr) {
        int nb_systems = this->plane->system_health.size();
        int system_to_hit = std::rand() % nb_systems;
        std::string system_name;
        int i = 0;
        for (auto system: this->plane->system_health) {
            if (i == system_to_hit) {
                system_name = system.first;
                break;
            }
            i++;
        }
        int sub_system_to_hit = std::rand() % this->plane->system_health[system_name].size();
        i = 0;
        for (auto &sub_system: this->plane->system_health[system_name]) {
            if (i == sub_system_to_hit) {
                sub_system.second = sub_system.second > damage ? sub_system.second - damage : 0;
                break;
            }
            i++;
        }
    }
    this->health -= damage;
    if (this->object->alive == false) {
        return;
    }
    if (this->health <= 0 && this->object->alive) {
        if (this->profile != nullptr && this->profile->radi.msgs.size() > 0) {
            std::srand(std::time(0));
            int r = std::rand() % 16;
            if (r<=this->profile->ai.atrb.VB) {
                int message = std::rand() % 4;
                this->setMessage(23 + message); // Bail out messages
            }
        }
        this->object->alive = false;
        if (this->object->entity->explos != nullptr) {
            SCExplosion *explosion = new SCExplosion(this->object->entity->explos->objct, this->object->position);
            this->mission->explosions.push_back(explosion);
            if (this->mission->sound.sounds.size() > 0) {   
                MemSound *sound;
                if (weapon->obj->entity_type == EntityType::tracer) {
                    sound = this->mission->sound.sounds[SoundEffectIds::GUN_IMPACT_1];
                } else {
                    sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_1];
                }
                RSMixer::getInstance().playSoundVoc(sound->data, sound->size);
            }
        }
        attacker->score += 100;
        if (this->plane != nullptr) {
            attacker->plane_down += 1;
        } else {
            attacker->ground_down += 1;
        }
    }
    
}
SCMissionActors::SCMissionActors() {
    subscription_id = MessageBus::getInstance().subscribeEvent(std::bind(&SCMissionActors::onEvent, this, std::placeholders::_1));
}
SCMissionActors::~SCMissionActors() {
    delete this->brain;
    if (subscription_id != -1) {
        MessageBus::getInstance().unsubscribe(subscription_id);
    }
}
void SCMissionActors::onEvent(const EventMessage &event) {
    if (auto eventData = dynamic_cast<const MissionEventActorHit*>(&event)) {
        this->onGettingHit(*eventData);
        return;
    }
    if (auto eventData = dynamic_cast<const MissionUpdateEvent*>(&event)) {
        this->onMissionUpdate(*eventData);
        return;
    }
    if (auto eventData = dynamic_cast<const AIRefreshEvent*>(&event)) {
        this->onAIRefresh(*eventData);
        return;
    }
}
void SCMissionActors::onGettingHit(const MissionEventActorHit &event) {
    if (event.attacker != nullptr && event.target != nullptr && event.target == this) {
        event.weapon->alive = false;
        this->hasBeenHit(event.weapon, event.attacker);
        
        event.target->weapon_shooted_at_me = nullptr;
        if (event.target->object->alive == true) {
            this->mission->explosions.push_back(new SCExplosion(event.weapon->obj->explos->objct, event.weapon->obj->position));
            if (this->mission->sound.sounds.size() > 0) {
                RSMixer &Mixer = RSMixer::getInstance();
                MemSound *sound;
                sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_1];
                Mixer.playSoundVoc(sound->data, sound->size);
            }
        }
    }
}
void SCMissionActors::onMissionUpdate(const MissionUpdateEvent &event) {
    /*if (this->is_active == false) {
        return;
    }*/
    if (this->is_destroyed == true && this->is_active == false) {
        return;
    }
    if (this->object == nullptr) {
        return;
    }
    SCMissionActors *ai_actor = this;
    SCMission *mission = event.mission;

    if (ai_actor->object->alive == false && ai_actor->is_destroyed == false) {
        ai_actor->is_destroyed = true;
        MissionEventActorDestroyed destroyed_event;
        destroyed_event.actor = ai_actor;
        destroyed_event.mission = mission;
        this->messageBus.publish(std::make_unique<MissionEventActorDestroyed>(destroyed_event));
        if (ai_actor->on_is_destroyed.size() > 0 && ai_actor->plane == nullptr) {
            ai_actor->is_active = false;
            SCProg *p = new SCProg(ai_actor, ai_actor->on_is_destroyed, mission, ai_actor->object->on_is_destroyed);
            p->execute();
            delete p;
        }
        if (ai_actor->object->member_name_destroyed != "") {
            RSEntity *entity = mission->LoadEntity(ai_actor->object->member_name_destroyed);
            if (entity != nullptr && ai_actor->object->entity->jdyn == nullptr) {
                ai_actor->object->entity = entity;
            }
        }
        if (ai_actor->object->entity->destroyed_object != nullptr && ai_actor->plane == nullptr) {
            if (ai_actor->object->entity->jdyn == nullptr) {
                ai_actor->object->entity = ai_actor->object->entity->destroyed_object;
            }
        }
        if (ai_actor->target != nullptr) {
            if (ai_actor->target->attacker == ai_actor) {
                ai_actor->target->attacker = nullptr;
            }
        }
    }
    for (auto weapon: ai_actor->weapons_shooted) {
        if (weapon) {
            weapon->Simulate(mission->tps);
        }
    }

    for (auto it = ai_actor->weapons_shooted.begin(); it != ai_actor->weapons_shooted.end(); ) {
        auto weapon = *it;
        if (weapon->alive == false) {
            it = ai_actor->weapons_shooted.erase(it);
            mission->onWeaponRemoved(weapon);
            delete weapon;
            weapon = nullptr;
        } else {
            ++it;
        }
    }
    
    if (ai_actor->profile == nullptr) {
        return;
    }
    if (ai_actor->profile->radi.info.callsign == "Strike Base") {
        SCProg *p = new SCProg(ai_actor, ai_actor->on_update, mission, ai_actor->object->on_mission_update);
        p->execute();
        delete p;
        return;
    }
    if (ai_actor->is_active == false) {
        return;
    }

    // tant qu'un ordre radio est en cours (onAIRefresh), le script de mission ne reprend pas la main
    if (ai_actor->on_update.size() > 0 && ai_actor->is_destroyed == false && ai_actor->override_progs.size() == 0) {
        mission->in_combat = ai_actor->target != nullptr && ai_actor->target == mission->player;
        SCProg *p = new SCProg(ai_actor, ai_actor->on_update, mission, ai_actor->object->on_mission_update);
        p->execute();
        delete p;
    }

    // l'avion du joueur est simule par SCStrike, y compris sous cerveau IA de test (DebugStrike)
    if (ai_actor->pilot == nullptr || ai_actor == mission->player) {
        return;
    }
    if (ai_actor->plane == nullptr) {
        return;
    }
    
    ai_actor->plane->Simulate();
    // Update attack position offset based on plane's yaw to keep it behind the plane
    

    ai_actor->pilot->FlyTo();
    
    Vector3D npos;
    ai_actor->plane->getPosition(&npos);
    
    ai_actor->object->position.x = npos.x;
    ai_actor->object->position.z = npos.z;
    ai_actor->object->position.y = npos.y;
    ai_actor->object->azymuth = 360 - (uint16_t)(ai_actor->plane->azimuthf / 10.0f);
    ai_actor->object->roll = (uint16_t)(ai_actor->plane->twist / 10.0f);
    ai_actor->object->pitch = (uint16_t)(ai_actor->plane->elevationf / 10.0f);
    if (ai_actor->is_destroyed == true && ai_actor->object->position.y < mission->area->getY(ai_actor->object->position.x, ai_actor->object->position.z)) {
        ai_actor->object->alive = false;
        ai_actor->is_active = false;
        if (ai_actor->on_is_destroyed.size() > 0 && ai_actor->plane != nullptr) {
            SCProg *p = new SCProg(ai_actor, ai_actor->on_is_destroyed, mission, ai_actor->object->on_is_destroyed);
            p->execute();
            delete p;
        }
        mission->explosions.push_back(new SCExplosion(ai_actor->object->entity->explos->objct, ai_actor->object->position));
        if (mission->sound.sounds.size() > 0) {
            RSMixer &Mixer = RSMixer::getInstance();
            MemSound *sound = mission->sound.sounds[SoundEffectIds::EXPLOSION_4];
            Mixer.playSoundVoc(sound->data, sound->size, 4, 0);
        }
        if (ai_actor->object->entity->destroyed_object != nullptr && ai_actor->plane == nullptr) {
            ai_actor->object->entity = ai_actor->object->entity->destroyed_object;
        }
    }
}
/**
 * SCMissionActors::onAIRefresh
 *
 * Point d'entree de la decision IA (equivalent AIEntity_MasterTick /
 * AI_TopLevelThink, cf. analysis/AI_SYSTEM.md §4/§6), declenche par
 * SCMission a cadence fixe ~25fps (AIRefreshEvent) plutot qu'a chaque
 * frame reelle du port. Ne s'applique qu'aux acteurs actifs porteurs d'un
 * profil GOAL complet (plane + pilot deja assignes, cf. SCMission.cpp).
 */
void SCMissionActors::onAIRefresh(const AIRefreshEvent &event) {
    if (!this->is_active || this->is_destroyed || this == this->mission->player) {
        return;
    }
    if (this->profile == nullptr || !this->profile->ai.isAI) {
        return;
    }
    if (this->profile->ai.goal.empty()) {
        return;
    }
    if (this->plane == nullptr || this->pilot == nullptr) {
        return;
    }
    // ordre radio accepte : pose l'objectif comme le script (Goal_SetObjective_A307)
    if (!this->override_progs.empty()) {
        SCProg *p = new SCProg(this, this->override_progs, this->mission, 255);
        p->execute();
        delete p;
        if (this->current_command_executed) {
            this->override_progs.clear();
            this->override_progs.shrink_to_fit();
        }
    }
    this->brain->tick();
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
    if (arg >= this->mission->mission->mission_data.messages.size()) {
        return true;
    }
    std::transform(this->mission->mission->mission_data.messages[arg]->begin(), this->mission->mission->mission_data.messages[arg]->end(), this->mission->mission->mission_data.messages[arg]->begin(), ::tolower);
    if (this->mission->waypoints.size() > 0) {
        this->mission->waypoints.back()->message = this->mission->mission->mission_data.messages[arg];
    }
    return true;
}

void SCMissionActorsPlayer::hasBeenHit(SCSimulatedObject *weapon, SCMissionActors *attacker) {
    if (this->object->alive == false) {
        return;
    }
    int damage = weapon->obj->wdat->damage;
    if (this->plane != nullptr) {
        int nb_systems = this->plane->system_health.size();
        int system_to_hit = std::rand() % nb_systems;
        std::string system_name;
        int i = 0;
        for (auto system: this->plane->system_health) {
            if (i == system_to_hit) {
                system_name = system.first;
                break;
            }
            i++;
        }
        int sub_system_to_hit = std::rand() % this->plane->system_health[system_name].size();
        i = 0;
        for (auto &sub_system: this->plane->system_health[system_name]) {
            if (i == sub_system_to_hit) {
                sub_system.second = sub_system.second > damage ? sub_system.second - damage : 0;
                break;
            }
            i++;
        }
    }
    this->health -= damage;
    
    if (this->health <= 0 && this->object->alive) {
        this->object->alive = false;
        if (this->object->entity->explos != nullptr) {
            SCExplosion *explosion = new SCExplosion(this->object->entity->explos->objct, this->object->position);
            this->mission->explosions.push_back(explosion);
            if (this->mission->sound.sounds.size() > 0) {   
                MemSound *sound;
                if (weapon->obj->entity_type == EntityType::tracer) {
                    sound = this->mission->sound.sounds[SoundEffectIds::GUN_IMPACT_1];
                } else {
                    sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_1];
                }
                RSMixer::getInstance().playSoundVoc(sound->data, sound->size);
            }
        }
        attacker->score += 100;
        if (this->plane != nullptr) {
            attacker->plane_down += 1;
        } else {
            attacker->ground_down += 1;
        }
    }
}
/**
 * SCMissionActorsPlayer::setObjective
 *
 * Le script de mission du joueur est une liste continue d'objectifs
 * executee UNE SEULE FOIS au chargement de la mission (script
 * d'initialisation) — pas un etat re-evalue en continu comme pour l'IA
 * (GOAL/AIRefresh, cf. SCMissionActors::executeGoalAction). Chaque objectif
 * doit donc etre execute immediatement ici, au moment ou il est pose, pour
 * creer le SCMissionWaypoint correspondant (takeOff/land/flyToWaypoint/...
 * sont surcharges cote joueur pour ca, pas pour piloter).
 */
bool SCMissionActorsPlayer::setObjective(prog_op command, uint8_t arg) {
    switch (command) {
        case OP_SET_OBJ_TAKE_OFF:
            this->current_command_executed = this->takeOff(arg);
        break;
        case OP_SET_OBJ_LAND:
            this->current_command_executed = this->land(arg);
        break;
        case OP_SET_OBJ_FLY_TO_WP:
            this->current_command_executed = this->flyToWaypoint(arg);
        break;
        case OP_SET_OBJ_FLY_TO_AREA:
            this->current_command_executed = this->flyToArea(arg);
        break;
        default:
        break;
    }
    this->current_command = command;
    this->current_command_arg = arg;
    return !this->current_command_executed;
}

bool SCMissionActorsStrikeBase::setMessage(uint8_t arg) {
    RadioMessages *msg = new RadioMessages();
    msg->message = this->profile->radi.msgs[arg];
    if (this->mission->sound.inGameVoices.size() > 0) {
        if (this->mission->sound.inGameVoices.find(this->profile->radi.spch) == this->mission->sound.inGameVoices.end()) {
            printf("No voice found for %d\n", this->profile->radi.spch);
        }
        if (this->mission->sound.inGameVoices[this->profile->radi.spch]->messages.find(arg) == this->mission->sound.inGameVoices[this->profile->radi.spch]->messages.end()) {
            printf("No message found for %d\n", arg);
        }
        MemSound *message_sound = this->mission->sound.inGameVoices[this->profile->radi.spch]->messages[arg];
        msg->sound = message_sound;
    }
    this->mission->radio_messages.push_back(msg);
    if (arg == 20) {
        this->mission->mission_over = true;
        this->mission->mission_won = true;
    } else if (arg == 21) {
        this->mission->mission_over = true;
        this->mission->mission_won = false;
    } else if (arg == 22) {
        this->mission->mission_over = true;
        this->mission->mission_won = false;
    }
    return true;
}
/*
@TODO
Fly to#Precise Way
Defend#Point
Follow#Leader
*/
SCMissionActors *SCMissionActorsSwpn::findTarget() {
    SCMissionActors *nearest = nullptr;
    float nearest_distance = (float)this->object->entity->swpn_data->detection_range;
    for (auto actor : this->mission->actors) {
        bool active = actor->is_active || actor == this->mission->player;
        if (actor->plane == nullptr || !active || actor->is_destroyed || !actor->object->alive) {
            continue;
        }
        if (actor->object->entity == nullptr || actor->object->entity->target_type != 1) {
            continue;
        }
        if (actor->team_id == this->team_id || actor->plane->ejected) {
            continue;
        }
        float distance = (actor->plane->position - this->object->position).Length();
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            nearest = actor;
        }
    }
    return nearest;
}

void SCMissionActorsSwpn::fireMissile() {
    RSEntity *weapon_entity = this->object->entity->swpn_data->weapon_entity;
    Vector3D target_position = this->target->plane->position;
    float ground = this->mission->area->getY(target_position.x, target_position.z);
    if (target_position.y - ground <= 305.0f) {
        printf("SWPN %s#%d missile refused %s height=%.0f\n", this->actor_name.c_str(), this->actor_id,
               this->target->actor_name.c_str(), target_position.y - ground);
        return;
    }
    Vector3D start = this->object->position;
    start.y += 2.0f * (float)this->object->entity->swpn_data->launch_offset;
    Vector3D direction = target_position - start;
    direction.Normalize();
    SCSimulatedObject *missile = new SCSimulatedObject();
    missile->obj = weapon_entity;
    missile->target = this->target;
    missile->shooter = this;
    missile->mission = this->mission;
    missile->guidance = true;
    missile->x = start.x;
    missile->y = start.y;
    missile->z = start.z;
    missile->vx = direction.x;
    missile->vy = direction.y;
    missile->vz = direction.z;
    this->weapons_shooted.push_back(missile);
    this->mission->onWeaponSpawned(missile);
    printf("SWPN %s#%d fire missile at %s ammo=%d max_speed=%.1f accel=%.1f turn=%.1f flight_time=%d fuse=%d\n",
           this->actor_name.c_str(), this->actor_id, this->target->actor_name.c_str(),
           this->object->entity->swpn_data->weapons_round - 1, weapon_entity->dynn_miss->velovity_m_per_sec,
           weapon_entity->dynn_miss->acceleration, weapon_entity->dynn_miss->turn_degre_per_sec,
           weapon_entity->missile_flight_time, weapon_entity->missile_fuse_radius);
    this->object->entity->swpn_data->weapons_round--;
    this->fire_delay = this->object->entity->swpn_data->fire_interval;
}

void SCMissionActorsSwpn::fireGun() {
    auto *data = this->object->entity->swpn_data;
    RSEntity *weapon_entity = data->weapon_entity;
    auto *tracer = weapon_entity->tracer_info;
    float dt = GameTimer::getInstance().getDeltaTime();
    Vector3D delta = this->target->plane->position - this->object->position;
    float dist = delta.Length();
    float flight_time = dist / tracer->muzzle_speed;
    Vector3D lead = this->target->plane->worldVelocity() * (dt * flight_time);

    int spread = (int)data->dispersion * (std::rand() % 32768) / 32768 + (int)delta.y / 100;
    float offset = dist * (float)spread / 200.0f;
    if (spread & 1) {
        offset = -offset;
    }
    Vector3D wing = {this->target->plane->ptw.v[0][0], this->target->plane->ptw.v[0][1],
                     this->target->plane->ptw.v[0][2]};
    Vector3D direction = delta + lead + wing * offset;
    direction.Normalize();

    float speed = tracer->muzzle_speed;
    int jitter = (int)(tracer->speed_jitter * 256.0f);
    speed += (float)(2 * jitter * (std::rand() % 32768) / 32768 - jitter) / 256.0f;
    Vector3D velocity = direction * speed;
    Vector3D position = this->object->position + direction * (float)data->launch_offset;
    position = position + velocity * (tracer->shooter_speed_part * dt);

    GunSimulatedObject *shell = new GunSimulatedObject();
    shell->obj = weapon_entity;
    shell->shooter = this;
    shell->mission = this->mission;
    shell->x = position.x;
    shell->y = position.y;
    shell->z = position.z;
    shell->vx = velocity.x;
    shell->vy = velocity.y;
    shell->vz = velocity.z;
    this->weapons_shooted.push_back(shell);
    this->mission->onWeaponSpawned(shell);
    printf("SWPN %s#%d fire gun at %s speed=%.0f ammo=%d\n", this->actor_name.c_str(), this->actor_id,
           this->target->actor_name.c_str(), speed, data->weapons_round - 1);
    data->weapons_round--;
    this->fire_delay = data->fire_interval;
}

void SCMissionActorsSwpn::onMissionUpdate(const MissionUpdateEvent &event) {
    if (!this->is_destroyed && this->is_active) {
        this->updateFire();
    }
    SCMissionActors::onMissionUpdate(event);
}

void SCMissionActorsSwpn::updateFire() {
    auto *data = this->object->entity->swpn_data;
    if (data->weapons_round <= 0 || data->weapon_entity == nullptr) {
        this->target = nullptr;
        return;
    }
    this->frames++;
    if (this->fire_delay > 0.0f) {
        this->fire_delay -= GameTimer::getInstance().getDeltaTime();
        return;
    }
    if (this->frames % 10 == 0) {
        SCMissionActors *previous = this->target;
        this->target = this->findTarget();
        if (this->target != previous) {
            printf("SWPN %s#%d target=%s ammo=%d detection=%d range=%d interval=%.2f launcher=%d weapon=%s\n",
                   this->actor_name.c_str(), this->actor_id,
                   this->target != nullptr ? this->target->actor_name.c_str() : "none", data->weapons_round,
                   data->detection_range, data->effective_range, data->fire_interval, data->missile_launcher,
                   data->weapon_name.c_str());
        }
    }
    if (this->target == nullptr) {
        return;
    }
    Vector3D to_target = this->target->plane->position - this->object->position;
    this->aiming_vector = to_target;
    float distance = to_target.Length();
    to_target.Normalize();
    // cos²(élévation) <= cos 15° : cible à au moins 10,6° au-dessus de l'horizon
    float horizontal_part = to_target.x * to_target.x + to_target.z * to_target.z;
    const float max_horizontal_part = cosf(15.0f * (float)M_PI / 180.0f);
    if (distance >= (float)data->effective_range || horizontal_part > max_horizontal_part) {
        printf("SWPN %s#%d drop %s distance=%.0f range=%d elevation=%.1f\n", this->actor_name.c_str(),
               this->actor_id, this->target->actor_name.c_str(), distance, data->effective_range,
               asinf(to_target.y) * 180.0f / (float)M_PI);
        this->target = nullptr;
        return;
    }
    if (data->missile_launcher != 0) {
        this->fireMissile();
    } else {
        this->fireGun();
    }
}
