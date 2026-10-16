#include "precomp.h"
#include "SCSimulatedObject.h"
#define GRAVITY 9.8f
#define DRAG_COEFFICIENT 0.010f  // Coefficient de traînée
#define CROSS_SECTIONAL_AREA 0.10f  // Section transversale (m^2)

SCSimulatedObject::SCSimulatedObject() {             

    this->elevationf = 0.0f;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->vz = 0.0f;

    this->azimuthf = 0.0f;
    this->elevationf = 0.0f;
    
    this->smoke_positions.clear();
    this->smoke_positions.reserve(20);

}
SCSimulatedObject::~SCSimulatedObject() {

}

void SCSimulatedObject::Render() {

    Vector3D position = { this->x, this->y, this->z };
    Vector3D orientaton = { this->azimuthf, this->elevationf, 0.0f };
    
    Renderer.drawModel(this->obj, position, orientaton);
    
    if (this->obj->dynn_miss != nullptr && this->no_thrust == false) {
        int cpt=0;
        int nb_smoke = (int) this->smoke_positions.size();
        int cpt_smoke = 0;
        for (auto pos: this->smoke_positions) {
            float alpha = (nb_smoke - cpt_smoke) / ((float) nb_smoke);
            cpt_smoke++;
            if (cpt > this->SmokeSet.missile_smoke_textures.size()-1) {
                cpt = 0;
            }
            Renderer.drawBillboard(pos, this->SmokeSet.missile_smoke_textures[cpt], 10, alpha);
        }
    }
}

void SCSimulatedObject::GetPosition(Vector3D *position) {
    position->x = this->x;
    position->y = this->y;
    position->z = this->z;
}
// Fonction pour convertir les coordonnées polaires en coordonnées cartésiennes
Vector3D polarToCartesian(float speed, float phi, float theta) {
    Vector3D v;
    v.x = speed * sinf(theta) * cosf(phi);
    v.y = speed * sinf(theta) * sinf(phi);
    v.z = speed * cosf(theta);
    return v;
}
#define EPSILON 1e-10 // Tolérance pour les très petits nombres
// Fonction pour convertir les coordonnées cartésiennes en coordonnées polaires
void cartesianToPolar(Vector3D v, float *phi, float *theta) {
    *phi = atan2f(v.x, v.z);
    *theta = acosf(v.y / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z));
}

// Calcul de la norme d'un vecteur

std::tuple<Vector3D, Vector3D> SCSimulatedObject::ComputeTrajectory(int tps) {
    this->tps = tps;
    float deltaTime = 1.0f / (float)tps;
    Vector3D position = {this->x, this->y, this->z};
    Vector3D velocity = {this->vx * tps, this->vy * tps, this->vz * tps};
    Vector3D world_up = {0.0f, 1.0f, 0.0f};

    auto *miss = this->obj->dynn_miss;
    float phi = 0.0f;
    float theta = 0.0f;
    if (this->run_iterations == 0) {
        cartesianToPolar(velocity, &phi, &theta);
        this->azimuthf = phi - (float)M_PI_2;
        this->elevationf = (float)M_PI_2 - theta;
        this->missile_roll = 0.0f;
        if (this->shooter->plane == nullptr) {
            velocity = {0.0f, 0.0f, 0.0f};
        }
    }
    float heading = this->azimuthf + (float)M_PI_2;
    Vector3D nose = {cosf(this->elevationf) * sinf(heading), sinf(this->elevationf),
                     cosf(this->elevationf) * cosf(heading)};
    Vector3D level_right = nose.CrossProduct(&world_up);
    level_right.Normalize();
    Vector3D level_up = level_right.CrossProduct(&nose);
    Vector3D up = level_up * cosf(this->missile_roll) + level_right * sinf(this->missile_roll);
    Vector3D right = nose.CrossProduct(&up);

    bool has_target = this->target != nullptr || this->decoy_target != nullptr;
    if (has_target && this->guidance) {
        Vector3D aim;
        Vector3D aim_velocity;
        if (this->target != nullptr) {
            aim = this->target->object->position;
            if (this->target->plane != nullptr) {
                aim_velocity = this->target->plane->worldVelocity();
            }
        } else {
            aim = {this->decoy_target->x, this->decoy_target->y, this->decoy_target->z};
            aim_velocity = Vector3D(this->decoy_target->vx, this->decoy_target->vy, this->decoy_target->vz) * (float)tps;
        }
        Vector3D delta = aim - position;
        float dist = delta.Length();
        float speed = velocity.Length();
        float lead = 1.0f;
        if (speed != 0.0f) {
            lead = dist / speed;
        }
        if (lead > 1.0f) {
            lead = 1.0f;
        }
        delta = delta + aim_velocity * lead;

        float roll = atan2f(delta.DotProduct(&right), delta.DotProduct(&up));
        up = up * cosf(roll) + right * sinf(roll);
        right = nose.CrossProduct(&up);

        float forward = delta.DotProduct(&nose);
        float normal = delta.DotProduct(&up);
        float pitch = (float)M_PI_2;
        if (forward != 0.0f) {
            pitch = atanf(normal / forward);
        } else if (normal < 0.0f) {
            pitch = (float)-M_PI_2;
        }
        if (forward < 0.0f) {
            pitch = (float)M_PI - std::fabs(pitch);
        }
        float max_pitch = miss->turn_degre_per_sec * deltaTime * (float)M_PI / 180.0f;
        if (pitch > max_pitch) {
            pitch = max_pitch;
        }
        Vector3D old_nose = nose;
        nose = nose * cosf(pitch) + up * sinf(pitch);
        up = up * cosf(pitch) - old_nose * sinf(pitch);
        nose.Normalize();
        right = nose.CrossProduct(&up);
        up = right.CrossProduct(&nose);
        up.Normalize();
        right.Normalize();
    }

    Vector3D cruise = right * miss->cruise_lateral + nose * miss->velovity_m_per_sec + up * miss->cruise_normal;
    if (!this->no_thrust) {
        if (velocity.Length() < miss->velovity_m_per_sec) {
            float along = velocity.DotProduct(&nose) + miss->acceleration * deltaTime;
            velocity = nose * along;
        } else {
            velocity = cruise;
            this->no_thrust = true;
        }
    } else if (has_target) {
        velocity = cruise;
    } else {
        velocity.y -= GRAVITY * deltaTime;
        nose = velocity;
        nose.Normalize();
        right = nose.CrossProduct(&up);
        up = right.CrossProduct(&nose);
        up.Normalize();
    }

    cartesianToPolar(nose, &phi, &theta);
    this->azimuthf = phi - (float)M_PI_2;
    this->elevationf = (float)M_PI_2 - theta;
    level_right = nose.CrossProduct(&world_up);
    level_right.Normalize();
    level_up = level_right.CrossProduct(&nose);
    this->missile_roll = atan2f(up.DotProduct(&level_right), up.DotProduct(&level_up));

    if (!this->is_simulated && this->run_iterations % tps == 0) {
        printf("MISSILE %s t=%.1fs speed=%.1f %s target=%s\n", this->obj->name.c_str(),
               (float)this->run_iterations / (float)tps, velocity.Length(), this->no_thrust ? "cruise" : "boost",
               this->target != nullptr ? this->target->actor_name.c_str() : (this->decoy_target != nullptr ? "decoy" : "none"));
    }
    this->run_iterations++;
    Vector3D step = velocity * deltaTime;
    position = position + step;
    return {position, step};
}

bool SCSimulatedObject::CheckCollision(SCMissionActors *entity) { 
    BoudingBox *bb{nullptr};
    Vector3D position = { this->x, this->y, this->z };
    Vector3D targetPos = {
        static_cast<float>(entity->object->position.x),
        static_cast<float>(entity->object->position.y),
        static_cast<float>(entity->object->position.z)
    };
    const float distanceThreshold = 50.0f; // Seuil de distance pour la collision
    if (entity->team_id == this->shooter->team_id) {
        return false;
    }
    if (entity == shooter) {
        return false;
    }
    if (entity->is_destroyed) {
        return false;
    }
    if (entity->object->alive == false) {
        return false;
    }
    bb = entity->object->entity->GetBoudingBpx();
    if (bb != nullptr) {
        if (this->x >= targetPos.x + bb->min.x && this->x <= targetPos.x + bb->max.x &&
            this->y >= targetPos.y+bb->min.y && this->y   <= targetPos.y + bb->max.y &&
            this->z >= targetPos.z+bb->min.z && this->z <= targetPos.z + bb->max.z) {
            // Collision detected: mark both objects as not alive and update score.
            return true;
        }
    }
    float distance = (targetPos - position).Length();
    if (distance < distanceThreshold) {
        return true;
    }
    return false;
}
void SCSimulatedObject::Simulate(int tps) {
    Vector3D position, velocity;
    if (this->obj->dynn_miss != nullptr) {
        Vector3D current = {this->x, this->y, this->z};
        if ((float)(this->run_iterations + 1) / (float)tps >= (float)this->obj->missile_flight_time) {
            if (!this->is_simulated) {
                this->mission->explosions.push_back(new SCExplosion(this->obj->explos->objct, current));
                if (this->mission->sound.sounds.size() > 0) {
                    MemSound *sound;
                    sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_1];
                    Mixer.playSoundVoc(sound->data, sound->size);
                }
            }
            this->alive = false;
            if (this->target != nullptr) {
                this->target->weapon_shooted_at_me = nullptr;
            }
            return;
        }
        bool target_active = this->target != nullptr && (this->target->is_active || this->target == this->mission->player);
        if (this->target != nullptr && (!target_active || !this->target->object->alive)) {
            this->target->weapon_shooted_at_me = nullptr;
            this->target = nullptr;
        }
        if (this->decoy_target != nullptr && !this->decoy_target->alive) {
            this->decoy_target = nullptr;
        }
        if (!this->is_simulated && this->obj->wdat->weapon_aspec != 0 && (this->run_iterations + 1) % 5 == 0) {
            float heading = this->azimuthf + (float)M_PI_2;
            Vector3D nose = {cosf(this->elevationf) * sinf(heading), sinf(this->elevationf),
                             cosf(this->elevationf) * cosf(heading)};
            Vector3D own_velocity = Vector3D(this->vx, this->vy, this->vz) * (float)tps;
            SCSimulatedObject *decoy = nullptr;
            SCMissionActors *chosen = SCPlane::SeekerChoose(this->obj, this->target, this->decoy_target, this->mission,
                                                            current, nose, own_velocity, &decoy);
            if (this->target != nullptr && this->target != chosen) {
                this->target->weapon_shooted_at_me = nullptr;
            }
            this->target = chosen;
            this->decoy_target = decoy;
            this->guidance = this->target != nullptr || this->decoy_target != nullptr;
        }
        if (this->decoy_target != nullptr && !this->is_simulated) {
            Vector3D decoy_position = {this->decoy_target->x, this->decoy_target->y, this->decoy_target->z};
            if ((decoy_position - current).Length() < (float)this->obj->missile_fuse_radius) {
                this->mission->explosions.push_back(new SCExplosion(this->obj->explos->objct, current));
                if (this->mission->sound.sounds.size() > 0) {
                    MemSound *sound;
                    sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_1];
                    Mixer.playSoundVoc(sound->data, sound->size);
                }
                this->alive = false;
                return;
            }
        }
        if (this->target != nullptr && !this->is_simulated) {
            Vector3D target_position = {this->target->object->position.x, this->target->object->position.y,
                                        this->target->object->position.z};
            if ((target_position - current).Length() < (float)this->obj->missile_fuse_radius) {
                auto hit_event = std::make_unique<MissionEventActorHit>();
                hit_event->attacker = this->shooter;
                hit_event->target = this->target;
                hit_event->weapon = this;
                hit_event->mission = this->mission;
                MessageBus::getInstance().publish(std::move(hit_event));
                return;
            }
        }
    }
    std::tie(position, velocity) = this->ComputeTrajectory(tps);
    if (this->target != nullptr) {
        this->target->weapon_shooted_at_me = this;
        if (this->target->object->alive == false) {
            this->target->weapon_shooted_at_me = nullptr;
            this->target = nullptr;
        }
    }
    if (this->target != nullptr && !this->is_simulated) {
        if (this->CheckCollision(this->target)) {
            auto hit_event = std::make_unique<MissionEventActorHit>();
            hit_event->attacker = this->shooter;
            hit_event->target = this->target;
            hit_event->weapon = this;
            hit_event->mission = this->mission;
            MessageBus::getInstance().publish(std::move(hit_event));
            return;
        }
    } else {
        for (auto entity: this->mission->actors) {
            if (this->CheckCollision(entity)) {
                entity->hasBeenHit(this, this->shooter);
                this->alive = false;
                if (this->target != nullptr) {
                    this->target->weapon_shooted_at_me = nullptr;
                }
                if (entity->object->alive == true) {
                    this->mission->explosions.push_back(new SCExplosion(this->obj->explos->objct, position));
                    if (this->mission->sound.sounds.size() > 0) {
                        MemSound *sound;
                        sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_1];
                        Mixer.playSoundVoc(sound->data, sound->size);
                    }
                }
                break;
            }
        }
    }
    float dist;
    Vector3D pos = {
        this->x,
        this->y,
        this->z
    };
    if (this->shooter->plane != nullptr) {
        dist = (this->shooter->plane->position - pos).Length();
    } else {
        dist = (this->shooter->object->position - pos).Length();
    }
    
    if (dist > 30 && this->no_thrust == false) {
        this->smoke_positions.insert(this->smoke_positions.begin(), position);
    }
    if (this->smoke_positions.size() > 20) {
        this->smoke_positions.pop_back();
    }
    
    
    float azimut = 0.0f;
    float elevation = 0.0f;
    if (this->obj->dynn_miss == nullptr) {
        cartesianToPolar(velocity, &azimut, &elevation);
        this->azimuthf = (float)(azimut - M_PI_2);
        this->elevationf = (float)(M_PI_2-elevation);
    }
    this->vx = velocity.x;
    this->vy = velocity.y;
    this->vz = velocity.z;
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;

    Vector3D length_vect = { this->x - this->last_px, this->y - this->last_py, this->z - this->last_pz };
    this->distance += length_vect.Length();
    if (this->y < this->mission->area->getY(this->x, this->z)) {
        if (!this->is_simulated) {
            this->mission->explosions.push_back(new SCExplosion(this->obj->explos->objct, position));
            if (this->mission->sound.sounds.size() > 0) {
                MemSound *sound;
                sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_1];
                Mixer.playSoundVoc(sound->data, sound->size);
            }
            for (auto entity: this->mission->actors) {
                if (this->CheckCollision(entity)) {
                    entity->hasBeenHit(this, this->shooter);
                }
            }
        }
        this->alive = false;
        if (this->target != nullptr) {
            this->target->weapon_shooted_at_me = nullptr;
        }
    }
}
std::tuple<Vector3D, Vector3D> GunSimulatedObject::ComputeTrajectory(int tps) {
    float deltaTime = 1.0f / static_cast<float>(tps);
    Vector3D position = { this->x, this->y, this->z };
    Vector3D velocity = { this->vx, this->vy, this->vz };

    // Calcul de la force de gravité (en utilisant la constante GRAVITY et le poids de l'objet)
    // 
    if (this->weight == 0) {
        this->weight = 0.2f;
    }
    Vector3D gravity_force = { 0.0f, -this->weight * GRAVITY , 0.0f };

    // Calcul de la force de frottement (drag) avec une densité d'air standard (1.225 kg/m^3)
    float speed = velocity.Length();
    Vector3D drag_force = { 0.0f, 0.0f, 0.0f };
    if (speed > EPSILON) {
        float air_density = 1.225f;
        float drag_magnitude = 0.005f * air_density * speed * speed * DRAG_COEFFICIENT * CROSS_SECTIONAL_AREA;
        drag_force = velocity * (-drag_magnitude / speed);
    }

    // Calcul de l'accélération à partir des forces appliquées (a = F/m)
    
    Vector3D acceleration = (gravity_force + drag_force) * (1.0f / this->weight);

    // Mise à jour de la vitesse et de la position en tenant compte du temps écoulé
    velocity = velocity + acceleration * deltaTime;
    position = position + velocity * deltaTime;
    this->run_iterations++;
    return { position, velocity };
}
std::tuple<Vector3D, Vector3D> GunSimulatedObject::ComputeTrajectoryUntilGround(int tps) {

    Vector3D position{0,0,0};
    Vector3D velocity{0,0,0};
    Vector3D oldpos{0,0,0};
    std::tie(position, velocity) = this->ComputeTrajectory(tps);
    int cpt_iteration = 0;
    while (position.y > this->mission->area->getY(position.x, position.z) == true && cpt_iteration<100000) {
        oldpos = position;
        std::tie(position, velocity) = this->ComputeTrajectory(tps);
        this->x = position.x;
        this->y = position.y;
        this->z = position.z;
        this->vx = velocity.x;
        this->vy = velocity.y;
        this->vz = velocity.z;
        if (oldpos.x == position.x && oldpos.y == position.y && oldpos.z == position.z && cpt_iteration>1000) {
            printf("should not happen\n");
            break;
        }
        cpt_iteration++;
    }
    return { position, velocity };
}
void GunSimulatedObject::Simulate(int tps) {
    
    // Actualisation des attributs de l'objet
    Vector3D position;
    Vector3D velocity;
    std::tie(position, velocity) = this->ComputeTrajectory(tps);
    float azimut = 0.0f;
    float elevation = 0.0f;
    cartesianToPolar(velocity, &azimut, &elevation);
    this->azimuthf = (float)(azimut - M_PI_2);
    this->elevationf = (float)(M_PI_2-elevation);
    this->vx = velocity.x;
    this->vy = velocity.y;
    this->vz = velocity.z;
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;

    Vector3D length_vect = { this->x - this->last_px, this->y - this->last_py, this->z - this->last_pz };
    this->distance += length_vect.Length();
    if (this->distance > this->obj->wdat->effective_range * 10.0f && this->obj->entity_type == EntityType::tracer) {
        this->alive = false;
    }
    if (!this->is_simulated) {
        for (auto entity: this->mission->actors) {
            if (this->CheckCollision(entity)) {
                entity->hasBeenHit(this, this->shooter);
                this->alive = false;
                break;
            }
        }
        if (this->y < this->mission->area->getY(this->x, this->z)) {
            this->alive = false;
            if (this->obj->wdat->radius > 5 && this->obj->entity_type == EntityType::bomb) {
                for (auto entity : this->mission->actors) {
                    float distance = entity->object->position.Distance(&position);
                    if (!entity->object->alive) {
                        continue;
                    }
                    if (distance < this->obj->wdat->radius) {
                        entity->hasBeenHit(this, this->shooter);
                    }
                }
            }
            if (this->obj->explos != nullptr && this->obj->explos->objct != nullptr) {
                SCExplosion *explosion = new SCExplosion(this->obj->explos->objct, position);
                this->mission->explosions.push_back(explosion);
                if (this->obj->entity_type != EntityType::tracer) {
                    if (this->mission->sound.sounds.size() > 0) {
                        MemSound *sound = this->mission->sound.sounds[SoundEffectIds::EXPLOSION_3];
                        Mixer.playSoundVoc(sound->data, sound->size);
                    }
                }
            }
        }
    }
}

void GunSimulatedObject::Render() {
    if (this->obj->vertices.size() == 0) {
        Vector3D pos = {this->x, this->y, this->z};
        Vector3D dir =  {this->vx, this->vy, this->vz};
        dir.Normalize();
        Vector3D color = {1.0f, 0.85f, 0.3f}; // jaune-orangé typique d'un traceur
        Renderer.drawTracer(pos, dir, 20.0f /*longueur monde*/, 3.0f /*largeur en px*/, color, 0.9f);
    } else {
        Vector3D pos = {this->x, this->y, this->z};
        Vector3D orient = {this->azimuthf, this->elevationf, 0.0f};
        Renderer.drawModel(this->obj, pos, orient);
    }
}

void DumbSimulatedObject::Simulate(int tps) {
    Vector3D gravity_force = { 0.0f, -this->weight * GRAVITY, 0.0f };
    Vector3D acceleration = gravity_force * (1.0f / this->weight);
    Vector3D position = {
        this->x,
        this->y,
        this->z
    };
    Vector3D velocity= {
        this->vx,
        this->vy,
        this->vz
    };
    this->tps = tps;
    float deltaTime = 1.0f / (float)tps;
    velocity = (velocity + (acceleration * deltaTime));
    position = position + velocity;
    this->vx = velocity.x;
    this->vy = velocity.y;
    this->vz = velocity.z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;
    this->distance -= deltaTime;
    if (this->distance < 0) {
        this->alive = false;
    }
}

void DumbSimulatedObject::Render() {
    this->fps++;
    if (this->fps > 12) {
        this->fps = 0;
        this->current_frame = (this->current_frame + 1) % this->obj->animations.size();
    }
    Renderer.drawBillboard(
        {this->x, this->y, this->z},
        this->obj->animations[this->current_frame],
        1.0f
    );
}

void AgravSimulatedObject::Simulate(int tps) {
    this->tps = tps;
    float deltaTime = 1.0f / (float)tps;
    if (!this->airdens.isLoaded()) {
        TreEntry *airdens_entry = AssetManager::getInstance().GetEntryByName("..\\..\\DATA\\AIRDENS.TBL");
        if (airdens_entry != nullptr) {
            this->airdens.initFromRam(airdens_entry->data, airdens_entry->size);
        }
    }
    Vector3D velocity = {this->vx * tps, this->vy * tps, this->vz * tps};
    float speed = velocity.Length();
    Vector3D acceleration = {0.0f, -GRAVITY, 0.0f};
    if (speed > 0.0f) {
        float dynamic_pressure = 0.5f * this->airdens.densityAt(this->y) * speed * speed;
        Vector3D direction = velocity;
        direction.Normalize();
        acceleration = acceleration - direction * (this->obj->drag_coefficient * dynamic_pressure / this->obj->weight_in_kg);
    }
    velocity = velocity + acceleration * deltaTime;
    Vector3D step = velocity * deltaTime;
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x += step.x;
    this->y += step.y;
    this->z += step.z;
    this->vx = step.x;
    this->vy = step.y;
    this->vz = step.z;
    this->lifetime_left -= deltaTime;
    if (this->lifetime_left <= 0.0f) {
        this->alive = false;
    }
}

int AgravSimulatedObject::seekerSignature(int aspec) {
    if (this->obj->radar_signature == nullptr) {
        return 0;
    }
    if (aspec == 3) {
        return this->obj->radar_signature->unknown3;
    }
    if (aspec == 4) {
        return this->obj->radar_signature->unknown2;
    }
    return (int)(this->obj->radar_signature->unknown1 * this->lifetime_left / this->obj->decoy_lifetime);
}

std::tuple<Vector3D, Vector3D> GuidedBombSimulatedObject::ComputeTrajectory(int tps) {
    this->tps = tps;
    float deltaTime = 1.0f / (float)tps;
    Vector3D position = {this->x, this->y, this->z};
    Vector3D velocity = {this->vx * tps, this->vy * tps, this->vz * tps};
    Vector3D right;
    if (this->run_iterations == 0) {
        this->nose = velocity;
        this->nose.Normalize();
        this->up = {0.0f, 1.0f, 0.0f};
        right = this->nose.CrossProduct(&this->up);
        right.Normalize();
        this->up = right.CrossProduct(&this->nose);
        if (!this->airdens.isLoaded()) {
            TreEntry *airdens_entry = AssetManager::getInstance().GetEntryByName("..\\..\\DATA\\AIRDENS.TBL");
            if (airdens_entry != nullptr) {
                this->airdens.initFromRam(airdens_entry->data, airdens_entry->size);
            }
        }
    }

    if (this->target != nullptr && this->guidance) {
        if (!this->launch_speed_set) {
            this->guided_speed = velocity.Length();
            this->launch_speed_set = true;
        }
        Vector3D delta = {this->target->object->position.x - position.x,
                          this->target->object->position.y - position.y,
                          this->target->object->position.z - position.z};
        float dist = delta.Length();
        float speed = velocity.Length();
        float lead = 1.0f;
        if (speed != 0.0f) {
            lead = dist / speed;
        }
        if (lead > 1.0f) {
            lead = 1.0f;
        }
        if (this->target->plane != nullptr) {
            delta = delta + this->target->plane->worldVelocity() * lead;
        }

        right = this->nose.CrossProduct(&this->up);
        float roll = atan2f(delta.DotProduct(&right), delta.DotProduct(&this->up));
        this->up = this->up * cosf(roll) + right * sinf(roll);

        float forward = delta.DotProduct(&this->nose);
        float normal = delta.DotProduct(&this->up);
        float pitch = atanf(normal / forward);
        if (forward < 0.0f) {
            pitch = (float)M_PI - std::fabs(pitch);
        }
        float max_pitch = this->obj->bomb_turn_rate * deltaTime * (float)M_PI / 180.0f;
        if (pitch > max_pitch) {
            pitch = max_pitch;
        }
        Vector3D old_nose = this->nose;
        this->nose = this->nose * cosf(pitch) + this->up * sinf(pitch);
        this->up = this->up * cosf(pitch) - old_nose * sinf(pitch);
        this->nose.Normalize();
        right = this->nose.CrossProduct(&this->up);
        this->up = right.CrossProduct(&this->nose);
        this->up.Normalize();

        velocity = this->nose * this->guided_speed;
    } else {
        float speed = velocity.Length();
        Vector3D acceleration = {0.0f, -GRAVITY, 0.0f};
        if (speed > 0.0f) {
            float dynamic_pressure = 0.5f * this->airdens.densityAt(position.y) * speed * speed;
            Vector3D direction = velocity;
            direction.Normalize();
            acceleration = acceleration - direction * (this->obj->drag_coefficient * dynamic_pressure / this->obj->weight_in_kg);
        }
        velocity = velocity + acceleration * deltaTime;
        this->nose = velocity;
        this->nose.Normalize();
        right = this->nose.CrossProduct(&this->up);
        this->up = right.CrossProduct(&this->nose);
        this->up.Normalize();
    }

    this->run_iterations++;
    Vector3D step = velocity * deltaTime;
    position = position + step;
    return {position, step};
}

void GuidedBombSimulatedObject::Simulate(int tps) {
    if ((float)this->run_iterations / (float)tps >= (float)this->obj->bomb_arming_delay) {
        SCSimulatedObject::Simulate(tps);
        return;
    }
    Vector3D position;
    Vector3D velocity;
    std::tie(position, velocity) = this->ComputeTrajectory(tps);
    float azimut = 0.0f;
    float elevation = 0.0f;
    cartesianToPolar(velocity, &azimut, &elevation);
    this->azimuthf = (float)(azimut - M_PI_2);
    this->elevationf = (float)(M_PI_2 - elevation);
    this->vx = velocity.x;
    this->vy = velocity.y;
    this->vz = velocity.z;
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;
}

EjectionSeatSimulatedObject::EjectionSeatSimulatedObject() {
    AssetManager &Assets = AssetManager::getInstance();
    std::string path = "..\\..\\DATA\\OBJECTS\\EJECSEAT.IFF";
    TreEntry *smk = Assets.GetEntryByName(path.c_str());
    this->ejection_seat_obj = new RSEntity();
    this->ejection_seat_obj->InitFromRAM(smk->data, smk->size, "EJECSEAT.IFF");
    smk = Assets.GetEntryByName("..\\..\\DATA\\OBJECTS\\EJGUY.IFF");
    this->pilot_obj = new RSEntity();
    this->pilot_obj->InitFromRAM(smk->data, smk->size, "EJGUY.IFF");
    this->weight = 150.0f;
}

EjectionSeatSimulatedObject::~EjectionSeatSimulatedObject() {
    if (this->ejection_seat_obj != nullptr) {
        delete this->ejection_seat_obj;
        this->ejection_seat_obj = nullptr;
    }
}

void EjectionSeatSimulatedObject::Simulate(int tps) {
    this->tps = tps;
    Vector3D position = {
        this->x,
        this->y,
        this->z
    };
    Vector3D velocity = {
        this->vx,
        this->vy,
        this->vz
    };
    Vector3D gravity_force = { 0.0f, -this->weight * GRAVITY, 0.0f };
    Vector3D ejection_force = { 0.0f, this->ejection_seat_velocity * this->weight, 0.0f };
    float deltaTime = 1.0f / (float)tps;
    this->ejection_seat_velocity -= 10.0f*deltaTime; // Décélération progressive de la poussée
    if (this->ejection_seat_velocity < 0.0f) {
        this->ejection_seat_velocity = 0.0f;
    }
    Vector3D total_force = gravity_force + ejection_force;
    Vector3D acceleration = total_force * (1.0f / this->weight);
    velocity = (velocity + (acceleration * deltaTime));
    position = position + velocity * deltaTime;
    this->vx = velocity.x;
    this->vy = velocity.y;
    this->vz = velocity.z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;
    if (this->y < this->mission->area->getY(this->x, this->z)) {
        this->alive = false;
    }
}
void EjectionSeatSimulatedObject::Render() {
    Vector3D position = { this->x, this->y, this->z };
    Vector3D orientation = { 0.0f, -1.5f, 0.0f };
    Renderer.drawModel(this->ejection_seat_obj, position, orientation);
    if (this->vy > 0.0f) {
        Renderer.drawModel(this->pilot_obj, position, orientation);
    }
}

EjectedPilotSimulatedObject::EjectedPilotSimulatedObject() {
    AssetManager &Assets = AssetManager::getInstance();
    std::string path = "..\\..\\DATA\\OBJECTS\\CHUTE.IFF";
    TreEntry *smk = Assets.GetEntryByName(path.c_str());
    this->ejected_pilot_obj = new RSEntity();
    this->ejected_pilot_obj->InitFromRAM(smk->data, smk->size, "EJECSEAT.IFF");
    this->weight = 20.0f;
}

EjectedPilotSimulatedObject::~EjectedPilotSimulatedObject() {
    if (this->ejected_pilot_obj != nullptr) {
        delete this->ejected_pilot_obj;
        this->ejected_pilot_obj = nullptr;
    }
}

void EjectedPilotSimulatedObject::Simulate(int tps) {
    this->tps = tps;
    Vector3D position = {
        this->x,
        this->y,
        this->z
    };
    Vector3D velocity = {
        this->vx,
        this->vy,
        this->vz
    };
    Vector3D gravity_force = { 0.0f, -this->weight * GRAVITY*0.1f, 0.0f };
    
    float deltaTime = 1.0f / (float)tps;
    
    Vector3D total_force = gravity_force;
    Vector3D acceleration = total_force * (1.0f / this->weight);
    velocity = (velocity + (acceleration * deltaTime));
    position = position + velocity * deltaTime;
    this->vx = velocity.x;
    this->vy = velocity.y;
    this->vz = velocity.z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;
    if (this->y < this->mission->area->getY(this->x, this->z)) {
        this->alive = false;
    }
}

void EjectedPilotSimulatedObject::Render() {
    Vector3D position = { this->x, this->y, this->z };
    Vector3D orientation = { 0.0f, -1.5f, 0.0f };
    Renderer.drawModel(this->ejected_pilot_obj, position, orientation);
}
