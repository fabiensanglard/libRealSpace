#include "precomp.h"
#include <limits>
#include "SCMission.h"
#include "SCAIBrain.h"
#include "../engine/gametimer.h"

SCMission::SCMission() {
    this->camera_director = new SCCameraDirector();
    this->last_time = SDL_GetTicks();
    this->last_tick = 0;
    this->tick_counter = 0;
    this->tps = 0;
}
SCMission::SCMission(std::string mission_name, std::unordered_map<std::string, RSEntity *> *objCache) {
    this->camera_director = new SCCameraDirector();
    this->mission_name = mission_name;
    this->obj_cache = objCache;
    this->last_time = SDL_GetTicks();
    this->last_tick = 0;
    this->tick_counter = 0;
    this->tps = 0;
    this->loadMission();
    subscription_id = this->messageBus.subscribeEvent(std::bind(&SCMission::onEvent, this, std::placeholders::_1));
}
SCMission::~SCMission() {
    if (subscription_id != -1) {
        this->messageBus.unsubscribe(subscription_id);
    }
    this->cleanup();
}
void SCMission::cleanup() {
    if (this->camera_director != nullptr) {
        delete this->camera_director;
        this->camera_director = nullptr;
    }
    if (this->mission != nullptr) {
        delete this->mission;
        this->mission = nullptr;
    }
    if (this->area != nullptr) {
        delete this->area;
        this->area = nullptr;
    }
    
    for (auto actor : this->actors) {
        delete actor;
    }
    this->actors.clear();
    this->actors.shrink_to_fit();
    for (auto wp : this->waypoints) {
        delete wp;
    }
    this->waypoints.clear();
    this->waypoints.shrink_to_fit();
    this->messageBus.reset();
}
// Cockpit_ReadControlsFrame_8F720 : avions du camp ennemi (+0x50 == 0xFF) -> PilotProfile_RescaleSkillByDifficulty_12FC9,
// FL, AG, AA du fichier decales a droite de word_7235F (0 = intacts, 1 = moitie, 2 = quart)
void SCMission::applyDifficulty() {
    int shift = (std::clamp)(this->difficulty, 0, 2);
    for (auto actor : this->actors) {
        if (actor->plane == nullptr || actor->profile == nullptr || actor->team_id != 255) {
            continue;
        }
        AI_ATTR &file = actor->profile->ai.atrb_file;
        actor->profile->ai.atrb.FL = file.FL >> shift;
        actor->profile->ai.atrb.AG = file.AG >> shift;
        actor->profile->ai.atrb.AA = file.AA >> shift;
    }
}
RSProf *SCMission::LoadProfile(std::string name) {
    RSProf *profile = new RSProf();
    std::string filename = Assets.intel_root_path+ name + ".IFF";
    TreEntry *profile_tre = Assets.GetEntryByName(filename);
    if (profile_tre != nullptr) {
        profile->InitFromRAM(profile_tre->data, profile_tre->size);
    } else {
        printf("Unable to load profile %s\n", name.c_str());
        return nullptr;
    }
    return profile;
}
void SCMission::onEvent(const EventMessage &event) {
    if (dynamic_cast<const MusicUpdateEvent *>(&event) != nullptr) {
        this->onMusicUpdate();
        return;
    }
    auto destroyed_event = dynamic_cast<const MissionEventActorDestroyed *>(&event);
    if (destroyed_event != nullptr) {
        this->onActorDestroyed(destroyed_event->actor);
    }
}
void SCMission::loadMission() {
    
    this->difficulty = Config::getInstance().getInt("Gameplay", "difficulty", 0);
    std::string miss_file_name = Assets.mission_root_path + this->mission_name; 
    std::transform(miss_file_name.begin(), miss_file_name.end(), miss_file_name.begin(), ::toupper);
    TreEntry *mission_tre = Assets.GetEntryByName(miss_file_name.c_str());
    this->mission = new RSMission();
    this->mission->InitFromRAM(mission_tre->data, mission_tre->size);

    TreEntry *intel_tre = Assets.GetEntryByName(Assets.intel_root_path + "INTEL.IFF");
    if (intel_tre != nullptr) {
        this->intel.InitFromRAM(intel_tre->data, intel_tre->size);
    }


    std::string area_filename = Assets.mission_root_path+this->mission->mission_data.world_filename + ".IFF";
    std::transform(area_filename.begin(), area_filename.end(), area_filename.begin(), ::toupper);   
    this->world = new RSWorld();
    TreEntry *treEntry = NULL;
    treEntry = Assets.GetEntryByName(area_filename.c_str());
    if (treEntry != NULL) {
        this->world->InitFromRAM(treEntry->data, treEntry->size);
    }
    if (this->world->has_light) {
        Renderer.setObjectLight(this->world->light_ambient_weight, this->world->light_weight,
                                this->world->light_direction[0], this->world->light_direction[1],
                                this->world->light_direction[2]);
    }
    std::string area_fn = this->world->tera+".PAK";
    std::transform(area_fn.begin(), area_fn.end(), area_fn.begin(), ::toupper);
    this->area = new RSArea();
    this->area->InitFromPAKFileName(area_fn.c_str());

    Renderer.InvalidateAABBCache();           // ou InvalidateAABBCache(oldArea);
    Renderer.PrecomputeAABBs(this->area, 0, 1);  // optionnel: pré-calc LOD 0-1

    for (auto &area_entity: this->area->objects) {
        area_entity.entity = LoadEntity(area_entity.name);
    }
    for (auto scene : this->mission->mission_data.scenes) {
        this->scenes.push_back(new SCMissionScene(this, scene));
    }
    int cpt_actor=0;
    for (auto part : mission->mission_data.parts) {
        int search_id = 0;
        if (part->entity == nullptr) {
            part->entity = LoadEntity(part->member_name);
        }
        for (auto cast : mission->mission_data.casting) {
            if (part->id == search_id) {
                SCMissionActors *actor = new SCMissionActors();
                if (cast->actor == "PLAYER") {
                    actor = new SCMissionActorsPlayer();
                    if (part->area_id != 255 && part->unknown2 == 0) {
                        Vector3D correction = this->mission->mission_data.areas[part->area_id]->position;
                        part->position.x += correction.x;
                        part->position.y += correction.y;
                        part->position.z += correction.z;
                    }
                } else if (cast->actor == "STRIBASE") {
                    actor = new SCMissionActorsStrikeBase();
                } else if (part->entity != nullptr && part->entity->entity_type == EntityType::swpn) {
                    actor = new SCMissionActorsSwpn();
                }
                if (cast->actor == "TEAM0") {
                    cast->actor = GameState.wingman;
                }
                actor->team_id = part->unknown_bytes[2];
                actor->actor_name = cast->actor;
                actor->actor_id = part->id;
                actor->object = part;
                actor->health = part->entity->health;
                actor->profile = this->LoadProfile(cast->actor);
                actor->mission = this;
                if (actor->object->on_is_activated != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_is_activated]) {
                        actor->on_is_activated.push_back(op);
                    }
                }
                if (actor->object->on_is_destroyed != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_is_destroyed]) {
                        actor->on_is_destroyed.push_back(op);
                    }
                }
                if (actor->object->on_missions_init != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_missions_init]) {
                        actor->on_mission_start.push_back(op);
                    }
                }
                if (actor->object->on_mission_update != 255) {
                    for (auto op: *this->mission->mission_data.prog[actor->object->on_mission_update]) {
                        actor->on_update.push_back(op);
                    }
                }
                
                if (actor->profile != nullptr && actor->profile->ai.isAI) {
                    actor->brain = new SCAIBrain(actor);
                    if (actor->profile->ai.goal.size() > 0) {
                        actor->pilot = new SCPilot();
                        actor->pilot->actor = actor;
                        BoudingBox *bb = actor->object->entity->GetBoudingBpx();
                        
                        actor->plane = new SCJetpPlane(
                            actor->object->entity->jdyn->max_g,
                            -(float) actor->object->entity->jdyn->max_g/2.0f,
                            40.0f,
                            60.0f,
                            (float) actor->object->entity->jdyn->pitch_rate_limit_dps/2.0f,
                            (float) actor->object->entity->jdyn->rate_limit_dps/2.0f,
                            actor->object->entity->wing_area,
                            (float) actor->object->entity->weight_in_kg,
                            (float) actor->object->entity->jdyn->fuel_capacity,
                            (float) actor->object->entity->thrust_in_newton,
                            sqrtf(actor->object->entity->wing_area / ((float)M_PI * 0.83f * actor->object->entity->jdyn->induced_drag_k)),
                            actor->object->entity->jdyn->induced_drag_k,
                            (int) actor->object->entity->jdyn->stall_speed_ms,
                            this->area,
                            part->position.x,
                            part->position.y,
                            part->position.z
                        );
                        actor->plane->yaw = (360 - part->azymuth) * 10.0f;
                        for (auto &sys: actor->object->entity->sysm) {
                            for (auto &subsys: sys.second) {
                                actor->plane->system_health[sys.first][subsys.first] = subsys.second;
                            }
                        }
                        part->weapon_load.shrink_to_fit();
                        if (part->weapon_load.size() > 0) {
                            std::string weapon_path_name = Assets.object_root_path + part->weapon_load + ".IFF";
                            std::transform(weapon_path_name.begin(), weapon_path_name.end(), weapon_path_name.begin(), ::toupper);
                            TreEntry *weapon_entry = Assets.GetEntryByName(weapon_path_name.c_str());
                            part->entity->weaps.clear();
                            part->entity->weaps.shrink_to_fit();
                            part->entity->parseREAL_OBJT_JETP(weapon_entry->data, weapon_entry->size);   
                        }
                        actor->plane->object = part;
                        actor->plane->InitLoadout();
                        actor->plane->pilot = actor;
                        if (abs(this->area->getY(part->position.x, part->position.z)-part->position.y) <= 10 ) {
                            part->position.y = this->area->getY(part->position.x, part->position.z);
                            actor->plane->on_ground = true;
                        } else {
                            actor->plane->on_ground = false;
                        }
                        if (part->position.y < this->area->getY(part->position.x, part->position.z)) {
                            actor->plane->on_ground = true;
                        }
                        if (!actor->plane->on_ground) {
                            actor->plane->SetThrottle(100);
                            actor->pilot->target_climb = (int) (part->position.y);
                            actor->plane->vz = -20;
                            float spawn_yaw = tenthOfDegreeToRad(actor->plane->yaw);
                            float spawn_speed = (float) actor->object->entity->jdyn->ai_speed_cruise;
                            actor->plane->velocity = Vector3D(-sinf(spawn_yaw) * spawn_speed, 0.0f, -cosf(spawn_yaw) * spawn_speed);
                            actor->pilot->target_azimut = actor->plane->azimuthf / 10.0f;
                            actor->pilot->target_speed = -20;
                        } else {
                            actor->plane->SetThrottle(0);
                            actor->pilot->target_climb = 0;
                            actor->plane->vz = 0;
                            actor->pilot->target_azimut = actor->plane->azimuthf / 10.0f;
                            actor->pilot->target_speed = 0;
                        }
                        actor->pilot->plane = actor->plane;
                    }
                    this->actors.push_back(actor);
                } else if (actor->profile != nullptr && cast->actor == "PLAYER") {
                    /*actor->plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f,
                                                23000.0f, 32.0f, .93f, 120, this->area, part->position.x,
                                                part->position.y, part->position.z);*/
                    BoudingBox *bb = actor->object->entity->GetBoudingBpx();
                    actor->plane = new SCJetpPlane(
                        actor->object->entity->jdyn->max_g,
                        -actor->object->entity->jdyn->max_g/2.0f,
                        40.0f,
                        60.0f,
                        (float) actor->object->entity->jdyn->pitch_rate_limit_dps/2.0f,
                        (float) actor->object->entity->jdyn->rate_limit_dps/2.0f,
                        actor->object->entity->wing_area,
                        (float) actor->object->entity->weight_in_kg,
                        (float) actor->object->entity->jdyn->fuel_capacity,
                        (float) actor->object->entity->thrust_in_newton,
                        sqrtf(actor->object->entity->wing_area / ((float)M_PI * 0.83f * actor->object->entity->jdyn->induced_drag_k)),
                        actor->object->entity->jdyn->induced_drag_k,
                        (int) actor->object->entity->jdyn->stall_speed_ms,
                        this->area,
                        part->position.x,
                        part->position.y,
                        part->position.z
                    );
                    for (auto &sys: actor->object->entity->sysm) {
                        for (auto &subsys: sys.second) {
                            actor->plane->system_health[sys.first][subsys.first] = subsys.second;
                        }
                    }
                    actor->plane->yaw = (360 - part->azymuth) * 10.0f;
                    actor->plane->simple_simulation = false;
                    actor->plane->yaw = (360 - part->azymuth) * (float) M_PI / 180.0f;
                    actor->plane->object = part;
                    actor->plane->pilot = actor;
                    this->actors.push_back(actor);
                    this->player = actor;
                    // MissionScenario_LoadMainRecord_A8331 : bits 4 et 5 de +0x39 poses sur l'objet du joueur
                    actor->on_nav_map = true;
                } else {
                    this->actors.push_back(actor);
                }
                cpt_actor++;
                break;
            }
            search_id++;
        }
    }
    for (auto area_actor: this->area->objects) {
        SCMissionActors *actor = new SCMissionActors();
        if (area_actor.entity != nullptr && area_actor.entity->entity_type == EntityType::swpn) {
            actor = new SCMissionActorsSwpn();
        }
        MISN_PART *part = new MISN_PART();
        part->id = cpt_actor++;
        part->member_name = area_actor.name;
        part->member_name_destroyed = area_actor.destroyedName;
        part->entity = area_actor.entity;
        part->position = area_actor.position;
        actor->actor_name = area_actor.name;
        actor->health = area_actor.entity->health;
        actor->plane = nullptr;
        actor->pilot = nullptr;
        actor->actor_id = part->id;
        actor->object = part;
        actor->profile = nullptr;
        actor->is_active = true;
        actor->is_hidden = false;
        actor->team_id = 255; // by default set it to enemy
        if (area_actor.entity == nullptr) {
            continue;
        }
        actor->mission = this;
        if (actor->object->entity->entity_type == EntityType::rnwy) {
            for (auto runway: this->area->objectOverlay) {
                Vector3D pos = actor->object->position;
                
                // Vérifier si la position de l'objet est à l'intérieur de la piste
                if (pos.x >= runway.lx && pos.x <= runway.hx && 
                    pos.z >= runway.ly && pos.z <= runway.hy) {
                    
                    // Calculer les dimensions de la piste
                    float width = std::abs(runway.lx - runway.hx); 
                    float length = std::abs(runway.ly - runway.hy);
                    
                    // Calculer l'orientation (angle) de la piste
                    float angle = std::atan2(runway.ly - runway.hy, 
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
        this->actors.push_back(actor);
    }
    for (auto member: this->mission->mission_data.team) {
        int id = this->mission->mission_data.parts[member]->id;
        for (auto actor: this->actors) {
            if (actor->actor_id == id) {
                this->friendlies.push_back(actor);
            }
        }
    }
    for (auto enemis : this->actors) {
        if (std::find(this->friendlies.begin(), this->friendlies.end(), enemis) == this->friendlies.end()) {
            this->enemies.push_back(enemis);
        }
    }
    this->applyDifficulty();
    if (this->player->on_is_activated.size() > 0) {
        SCProg *p = new SCProg(this->player, this->player->on_is_activated, this, this->player->object->on_is_activated);
        p->execute();
    }
    if (this->player->on_mission_start.size() > 0 && this->player->prog_executed == false) {
        SCProg *p = new SCProg(this->player, this->player->on_mission_start, this, this->player->object->on_missions_init);
        p->execute();
        this->player->prog_executed = true;
    }
    for (auto spot: this->mission->mission_data.spots) {
        spot->origin = {spot->position.x, spot->position.y, spot->position.z};
        if (spot->area_id != -1) {
            AREA *ar = this->mission->mission_data.areas[spot->area_id];
            spot->position += ar->position;
        }
    }

    if (this->player != nullptr && this->player->plane != nullptr) {
        this->camera_director->init(this->world, this->player);
    }
    this->Mixer.switchBank(2);
    this->musicRequest(this->mission->mission_data.tune, "TUNE de la mission");
}
// ajout a la liste des objets du monde (59C3h) : missile air-air (WDAT target_domain 1) -> byte_6E4C6
void SCMission::onWeaponSpawned(SCSimulatedObject *weapon) {
    if (weapon->obj->wdat->target_domain == 1) {
        this->aa_missile_launched = true;
    }
}
// retrait de la liste des objets du monde : les references SetReference sur l'arme se vident (+0x281)
void SCMission::onWeaponRemoved(SCSimulatedObject *weapon) {
    for (auto actor : this->actors) {
        if (actor->brain != nullptr && actor->brain->missile_threat == weapon) {
            actor->brain->missile_threat = nullptr;
        }
        if (actor->plane != nullptr) {
            for (auto missile : actor->plane->weaps_object) {
                if (missile->decoy_target == weapon) {
                    missile->decoy_target = nullptr;
                }
            }
        }
        for (auto missile : actor->weapons_shooted) {
            if (missile->decoy_target == weapon) {
                missile->decoy_target = nullptr;
            }
        }
    }
}
RSEntity * SCMission::LoadEntity(std::string name) {
    std::string tmpname = Assets.object_root_path + name + ".IFF";
    RSEntity *objct = new RSEntity();
    TreEntry *entry = Assets.GetEntryByName((char *)tmpname.c_str());
    if (entry != nullptr) {
        objct->InitFromRAM(entry->data, entry->size, tmpname);
        return objct;
    }
    return nullptr;
}
void SCMission::update() {
    
    this->messageBus.processEvents();
    this->tps = 1.0f/GameTimer::getInstance().getDeltaTime();
    if (this->mission_ended) {
        return;
    }
    this->tick_counter++;
    uint8_t area_id = this->getAreaID({this->player->plane->x, this->player->plane->y, this->player->plane->z});
    if (area_id != this->current_area_id) {
        this->current_area_id = area_id;
    }
    MissionUpdateEvent mission_update_event;
    mission_update_event.delta_time = GameTimer::getInstance().getDeltaTime();
    mission_update_event.tick = this->tick_counter;
    mission_update_event.area_id = area_id;
    mission_update_event.mission = this;
    this->messageBus.publish(std::make_unique<MissionUpdateEvent>(mission_update_event));

    // Cadence IA fixe (~25 fps d'origine) : eviter un rattrapage explosif
    // apres un hoquet en plafonnant l'accumulateur avant de vider le retard.
    this->ai_refresh_accumulator += mission_update_event.delta_time;
    if (this->ai_refresh_accumulator > 1.0f) {
        this->ai_refresh_accumulator = 1.0f;
    }
    while (this->ai_refresh_accumulator >= AI_REFRESH_INTERVAL) {
        this->ai_refresh_accumulator -= AI_REFRESH_INTERVAL;
        // RadioFlags_ShiftHistory
        this->player_tail_threat = this->player_tail_seen;
        this->player_tail_seen = nullptr;
        this->aa_missile_launched_last = this->aa_missile_launched;
        this->aa_missile_launched = false;
        AIRefreshEvent ai_refresh_event;
        ai_refresh_event.mission = this;
        this->messageBus.publish(std::make_unique<AIRefreshEvent>(ai_refresh_event));
    }

    this->music_accumulator += mission_update_event.delta_time;
    if (this->music_accumulator > 1.0f) {
        this->music_accumulator = 1.0f;
    }
    while (this->music_accumulator >= MUSIC_UPDATE_INTERVAL) {
        this->music_accumulator -= MUSIC_UPDATE_INTERVAL;
        MusicUpdateEvent music_update_event;
        music_update_event.mission = this;
        this->messageBus.publish(std::make_unique<MusicUpdateEvent>(music_update_event));
    }
    this->musicPollEvents();
    SCPlane *player_plane = this->player->plane;
    if (player_plane->ejected && !this->music_eject_requested) {
        this->music_eject_requested = true;
        this->musicRequest(10, "ejection du joueur");
    }
    if ((player_plane->crached || !this->player->object->alive) && !player_plane->ejected &&
        !this->music_shot_down_requested) {
        this->music_shot_down_requested = true;
        this->musicRequest(11, "avion du joueur detruit");
    }
    if (player_plane->landed && !this->music_landing_requested) {
        this->music_landing_requested = true;
        this->musicRequest(20, "atterrissage");
    }

    for (auto scene: this->mission->mission_data.scenes) {
        if (scene->area_id == area_id - 1 || scene->area_id == -1) {
            if (scene->is_active == 1) {
                MissionEventSceneActivated scene_activated_event;
                scene_activated_event.scene = scene;
                scene_activated_event.mission = this;
                this->messageBus.publish(std::make_unique<MissionEventSceneActivated>(scene_activated_event));
            }
        }        
    }
    this->in_combat = false;
    
    if (this->player->plane->landed) {
        this->mission_ended = true;
    }
}


void SCMission::musicRequest(int tune, const char *reason) {
    printf("MUSIC request %d (%s) current=%u requested=%u\n", tune, reason, this->Mixer.getMusicID(),
           this->Mixer.getRequestedMusicID());
    this->Mixer.playMusic((uint32_t)tune);
}

void SCMission::musicPollEvents() {
    SCMusicEvent music_event;
    while (this->Mixer.pollMusicEvent(music_event)) {
        printf("MUSIC event type=%d track=%d from=%d link=%d measure=%d\n", (int)music_event.type, music_event.track,
               music_event.fromTrack, music_event.link, music_event.measure);
        bool punctuation = music_event.track >= 16 && music_event.track <= 18;
        if (music_event.type == SCMusicEvent::TRACK_STARTED && punctuation && this->music_resume_tune >= 0) {
            this->musicRequest(this->music_resume_tune, "reprise apres la ponctuation");
        }
        if (music_event.type == SCMusicEvent::TRACK_FINISHED && this->music_resume_tune >= 0) {
            this->musicRequest(this->music_resume_tune, "piste terminee, reprise");
        }
    }
}

uint8_t SCMission::musicScanEnemies(SCMissionActors *excluded) {
    uint8_t flags = 0;
    Vector3D player_position = this->player->plane->position;
    for (auto actor : this->actors) {
        if (actor == excluded || actor == this->player || !actor->is_active || actor->is_destroyed) {
            continue;
        }
        if (actor->object == nullptr || actor->object->entity == nullptr || !actor->object->alive ||
            actor->team_id != 255) {
            continue;
        }
        Vector3D position = actor->plane != nullptr ? actor->plane->position : actor->object->position;
        if ((position - player_position).Length() >= 18520.0f) {
            continue;
        }
        EntityType type = actor->object->entity->entity_type;
        if (actor->plane != nullptr && type == EntityType::jet && !actor->plane->ejected) {
            flags |= 1;
        }
        if (type == EntityType::swpn || type == EntityType::ground || type == EntityType::xmit) {
            flags |= 2;
        }
    }
    return flags;
}

float SCMission::playerDamage() {
    float current = 0.0f;
    float initial = 0.0f;
    for (auto &system : this->player->object->entity->sysm) {
        auto health_system = this->player->plane->system_health.find(system.first);
        for (auto &sub_system : system.second) {
            initial += (float)sub_system.second;
            if (health_system != this->player->plane->system_health.end() &&
                health_system->second.find(sub_system.first) != health_system->second.end()) {
                current += (float)health_system->second[sub_system.first];
            } else {
                current += (float)sub_system.second;
            }
        }
    }
    return initial > 0.0f ? 1.0f - current / initial : 0.0f;
}

bool SCMission::enemyOnPlayerSix() {
    SCPlane *player_plane = this->player->plane;
    for (auto actor : this->actors) {
        if (actor == this->player || actor->plane == nullptr || !actor->is_active || actor->is_destroyed ||
            actor->team_id != 255 || actor->plane->ejected) {
            continue;
        }
        Vector3D to_player = player_plane->position - actor->plane->position;
        if (to_player.Length() >= 3500.0f) {
            continue;
        }
        Vector3D from_player = actor->plane->position - player_plane->position;
        bool nose_on_player = actor->plane->forward.AngleBetween(to_player) < 45.0f;
        bool behind_player = player_plane->forward.DotProduct(&from_player) < 0.0f;
        if (nose_on_player && behind_player) {
            return true;
        }
    }
    return false;
}

bool SCMission::musicFallback() {
    if (this->mission_over && this->mission_won) {
        this->musicRequest(12, "fin de combat, mission terminee et gagnee");
        return true;
    }
    if (this->music_combat_happened && !this->music_end_played) {
        this->music_end_played = true;
        this->musicRequest(13, "fin de combat");
        return true;
    }
    return false;
}

void SCMission::onMusicUpdate() {
    SCPlane *player_plane = this->player->plane;
    if (player_plane->ejected || player_plane->crached || !this->player->object->alive) {
        return;
    }
    if (this->Mixer.getMusicID() == 20 || this->Mixer.getRequestedMusicID() != this->Mixer.getMusicID()) {
        return;
    }
    uint8_t flags = this->musicScanEnemies(nullptr);
    this->music_enemy_flags = flags;
    if (flags == 0) {
        this->musicFallback();
        return;
    }
    this->music_combat_happened = true;
    int tune = 19;
    const char *reason = "menaces au sol seules";
    float damage = this->playerDamage();
    if (this->player->weapon_shooted_at_me != nullptr || this->player_tail_threat != nullptr) {
        tune = 9;
        reason = "missile ou avion sur le joueur";
    } else if (this->enemyOnPlayerSix()) {
        tune = 5;
        reason = "ennemi dans les six heures";
    } else if (damage >= 0.75f) {
        tune = 7;
        reason = "degats >= 75 %";
    } else if (damage >= 0.35f) {
        tune = 6;
        reason = "degats >= 35 %";
    } else if (flags & 1) {
        tune = 4;
        reason = "avion ennemi proche";
    }
    if ((uint32_t)tune != this->Mixer.getMusicID()) {
        this->musicRequest(tune, reason);
    }
}

void SCMission::onActorDestroyed(SCMissionActors *actor) {
    SCPlane *player_plane = this->player->plane;
    if (actor == this->player || player_plane->crached || !this->player->object->alive) {
        return;
    }
    if (actor->plane != nullptr && actor->plane->ejected) {
        return;
    }
    if (actor->object == nullptr || actor->object->entity == nullptr) {
        return;
    }
    EntityType type = actor->object->entity->entity_type;
    if (type != EntityType::jet && type != EntityType::swpn && type != EntityType::ground && type != EntityType::ornt &&
        type != EntityType::xmit) {
        return;
    }
    printf("MUSIC destroyed %s#%d team=%d by=%s\n", actor->actor_name.c_str(), actor->actor_id, actor->team_id,
           actor->last_hit_by != nullptr ? actor->last_hit_by->actor_name.c_str() : "none");
    if (actor->team_id == 255) {
        uint8_t before = this->music_enemy_flags;
        uint8_t after = this->musicScanEnemies(actor);
        this->music_enemy_flags = after;
        bool changed = false;
        if ((before & 1) && !(after & 1)) {
            this->musicRequest(19, "dernier avion ennemi abattu");
            changed = true;
        } else if (before != 0 && after == 0) {
            changed = this->musicFallback();
        }
        if (changed || actor->last_hit_by != this->player) {
            return;
        }
        int punctuation = 18;
        if (type == EntityType::jet) {
            punctuation = 16;
        } else if (type == EntityType::swpn || type == EntityType::ground) {
            punctuation = 17;
        }
        int current = (int)this->Mixer.getMusicID();
        this->music_resume_tune = current;
        if (current == 5) {
            this->music_resume_tune = 4;
        } else if (current == 8) {
            this->music_resume_tune = (this->music_enemy_flags & 1) ? 4 : 19;
        } else if (current == 21) {
            this->music_resume_tune = 19;
        }
        this->musicRequest(punctuation, "victoire du joueur");
        return;
    }
    if (actor->team_id == this->player->team_id) {
        this->musicRequest(14, "objet de l'equipe du joueur detruit");
    } else {
        this->musicRequest(15, "objet allie detruit");
    }
}

void SCMission::executeProg(std::vector<PROG> *prog) {
    
}
uint8_t SCMission::getAreaID(Vector3D position) {
    uint8_t area_id = 255;
    float smallest_area_width = (std::numeric_limits<float>::max)();
    
    for (auto ar: this->mission->mission_data.areas) {
        if (ar->position.x - ar->AreaWidth / 2 <= position.x && ar->position.x + ar->AreaWidth / 2 >= position.x) {
            if (ar->position.z - ar->AreaWidth / 2 <= position.z && ar->position.z + ar->AreaWidth / 2 >= position.z) {
                if (area_id == 255 || ar->AreaWidth <= smallest_area_width) {
                    smallest_area_width = ar->AreaWidth;
                    area_id = ar->id;
                }
            }
        }
    }
    
    return area_id;
}