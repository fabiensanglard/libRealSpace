#include "precomp.h"
#include "SCMissionScene.h"

void SCMissionScene::onEvent(const EventMessage &event) {
    if (const MissionUpdateEvent *mission_update_event = dynamic_cast<const MissionUpdateEvent*>(&event)) {
        this->onMissionUpdate(*mission_update_event);
        return;
    }
    if (const MissionEventSceneActivated *scene_activated_event = dynamic_cast<const MissionEventSceneActivated*>(&event)) {
        this->onSceneActivated(*scene_activated_event);
        return;
    }
}

void SCMissionScene::onMissionUpdate(const MissionUpdateEvent &event) {
    if (event.area_id == scene->area_id + 1 || scene->area_id == -1) {
        if (scene->is_active == 0) {
            if (scene->on_mission_update != -1) {
                if (scene->on_mission_update < this->mission->mission->mission_data.prog.size()) {
                    std::vector<PROG> prog;
                    for (auto prg: *this->mission->mission->mission_data.prog[scene->on_mission_update]) {
                        prog.push_back(prg);
                    }
                    SCProg *p = new SCProg(this->mission->player, prog, this->mission, scene->on_mission_update);
                    p->scene_script = true;
                    p->execute();
                    delete p;
                    prog.clear();
                    prog.shrink_to_fit();
                }
            }
            if (scene->on_leaving != -1) {
                if (scene->on_leaving < this->mission->mission->mission_data.prog.size()) {
                    std::vector<PROG> prog;
                    for (auto prg: *this->mission->mission->mission_data.prog[scene->on_leaving]) {
                        prog.push_back(prg);
                    }
                    SCProg *p = new SCProg(this->mission->player, prog, this->mission, scene->on_leaving);
                    p->scene_script = true;
                    p->execute();
                    delete p;
                    prog.clear();
                    prog.shrink_to_fit();
                }
            }
        }
    }
}

void SCMissionScene::onSceneActivated(const MissionEventSceneActivated &event) {
    MISN_SCEN *scene = event.scene;
    if (scene != this->scene) {
        return;
    }
    uint8_t area_id = event.mission->current_area_id;
    RSMission *mission = event.mission->mission;
    SCMission *sc_mission = event.mission;
    for (auto cast: scene->cast) {
        int i=0;
        for (auto part: mission->mission_data.parts) {
            if (i == cast) {
                for (auto actor: sc_mission->actors) {
                    if (actor->actor_name == "PLAYER") {
                        continue;
                    }
                    if (actor->actor_id == part->id && actor->is_active == false) {
                        actor->is_active = true;
                        actor->is_hidden = false;
                        if (scene->area_id != -1) {
                            Vector3D correction;
                            if (actor->object->unknown2 == 0) {
                                correction = sc_mission->mission->mission_data.areas[scene->area_id]->position;
                            } else if (actor->object->unknown2 == 1) {
                                correction = {
                                    sc_mission->player->plane->x,
                                    sc_mission->player->plane->y,
                                    sc_mission->player->plane->z
                                };
                            }
                            if (actor->object->area_id != 255) {
                                actor->object->position += correction;
                            }
                            float ground_y = sc_mission->area->getY(actor->object->position.x, actor->object->position.z);
                            
                            if (actor->plane != nullptr) {
                                actor->plane->on_ground = false;
                                actor->plane->x = actor->object->position.x;
                                actor->plane->y = actor->object->position.y;
                                actor->plane->z = actor->object->position.z;
                                if (abs(ground_y - actor->object->position.y) <= 10 ) {
                                    actor->object->position.y = ground_y;
                                    actor->plane->on_ground = true;
                                } else {
                                    actor->plane->on_ground = false;
                                }
                                if (actor->object->position.y < ground_y) {
                                    actor->plane->position.y += ground_y;
                                }
                            } else if (actor->object->position.y < ground_y) {
                                actor->object->position.y = ground_y;
                            }
                        }
                        
                        // Shared_TriggerExprInstruction_5247D : minuteur WAIT (+0x3A) remis a zero a l'activation
                        actor->wait_timer = 0.0f;
                        if (actor->on_is_activated.size() > 0) {
                            SCProg *p = new SCProg(actor, actor->on_is_activated, sc_mission, actor->object->on_is_activated);
                            p->execute();
                            delete p;
                        }
                        if (actor->object->entity->entity_type == EntityType::rnwy) {
                            for (auto runway: sc_mission->area->objectOverlay) {
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
                        break;
                    }
                }
                break;
            }
            i++;
        }
    }
    if (scene->on_is_activated != -1) {
        if (scene->on_is_activated < sc_mission->mission->mission_data.prog.size() && scene->has_been_activated == 0) {
            std::vector<PROG> prog;
            for (auto prg: *sc_mission->mission->mission_data.prog[scene->on_is_activated]) {
                prog.push_back(prg);
            }
            SCProg *p = new SCProg(sc_mission->player, prog, sc_mission, scene->on_is_activated);
            p->scene_script = true;
            p->execute();
            scene->has_been_activated = 1;
            delete p;
            prog.clear();
            prog.shrink_to_fit();
        }
    }
    scene->is_active = 0;
}

SCMissionScene::SCMissionScene(SCMission *mission, MISN_SCEN *scene) {
    this->mission = mission;
    this->scene = scene;
    subscription_id = MessageBus::getInstance().subscribeEvent(std::bind(&SCMissionScene::onEvent, this, std::placeholders::_1));
}

SCMissionScene::~SCMissionScene() {
    if (subscription_id != -1) {
        MessageBus::getInstance().unsubscribe(subscription_id);
    }
}
