#include "precomp.h"
#include "SCSimulatedObject.h"
#define GRAVITY 9.8f
#define DRAG_COEFFICIENT 0.010f  // Coefficient de traînée
#define CROSS_SECTIONAL_AREA 0.10f  // Section transversale (m^2)
#define LIFT_COEFFICIENT  11.0f
#define MAX_VELOCITY 2000.0f  // Vitesse maximale autorisée en m/s
#define RO_TO_SI 515.379f

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

// Calcul de la force de frottement
Vector3D SCSimulatedObject::calculate_drag(Vector3D velocity) {
    float speed_per_tick = velocity.Length();
    if (speed_per_tick < EPSILON) return { 0.0f, 0.0f, 0.0f };

    float speed_mps = speed_per_tick * tps;

    float altitude_ft = this->y * 3.28084f;      // y est en mètres
    int itemp = (int)(altitude_ft / 1000.0f);
    if (itemp > 74) itemp = 74;
    else if (itemp < 0) itemp = 0;

    float drag_magnitude = 0.5f * ro[itemp] * RO_TO_SI * speed_mps * speed_mps * DRAG_COEFFICIENT * CROSS_SECTIONAL_AREA;
    return velocity * (-drag_magnitude / speed_per_tick);
}
Vector3D SCSimulatedObject::calculate_lift(Vector3D velocity) {
    float speed_per_tick = velocity.Length();
    if (speed_per_tick < EPSILON) {
        return { 0.0f, 0.0f, 0.0f };
    }
    if (this->obj->entity_type == EntityType::bomb) {
        return { 0.0f, 0.0f, 0.0f };
    }

    // Le lift compense exactement la gravité (indépendant de ro[] et de CL)
    float lift_magnitude = this->weight * GRAVITY;
    if (this->no_thrust) {
        lift_magnitude = lift_magnitude * 0.98f; // Réduire le lift si pas de poussée
    }
    return {0, lift_magnitude, 0};
}
std::tuple<Vector3D, Vector3D> SCSimulatedObject::ComputeTrajectory(int tps) {
    this->tps = tps;
    float deltaTime = 1.0f / (float)tps;

    float thrust = 0.0f;
    float mps = MAX_VELOCITY;  // Vitesse maximale par défaut
    if (this->obj->dynn_miss != nullptr) {
        mps = this->obj->dynn_miss->velovity_m_per_sec;
        thrust = (mps * mps * 0.5f * this->obj->weight_in_kg) / this->obj->wdat->effective_range;
    }
    if (this->no_thrust) {
        thrust = 0;
        this->no_gravity = false;
    }

    Vector3D position = { this->x, this->y, this->z };
    Vector3D velocity = { this->vx, this->vy, this->vz };

    Vector3D to_target = { 0.0f, 0.0f, 0.0f };
    if (this->target != nullptr) {
        to_target.x = (float)this->target->object->position.x;
        to_target.y = (float)this->target->object->position.y;
        to_target.z = (float)this->target->object->position.z;
    }

    // --- Forces de base ---
    Vector3D gravity_force = { 0.0f, -this->weight * GRAVITY, 0.0f };
    Vector3D drag_force    = this->calculate_drag(velocity);
    Vector3D lift_force    = this->calculate_lift(velocity);

    
    // --- Guidage aérodynamique ---
    Vector3D thrust_force  = { 0.0f, 0.0f, 0.0f };
    Vector3D steer_force   = { 0.0f, 0.0f, 0.0f };

    float speed = velocity.Length();
    float speed_mps = speed * tps;  // Convertir la vitesse par tick en m/s pour les calculs aérodynamiques
    Vector3D vel_dir = velocity;
    vel_dir.Normalize();

    float lift_y = lift_force.y;
    float gravity_y = this->weight * GRAVITY;
    //printf("[LIFT DEBUG] lift=%.2f < gravity=%.2f (deficit=%.2f) max_speed=%.1f, speed_mps=%.1f thrust=%.1f, y=%.1f\n", lift_y, gravity_y, gravity_y - lift_y, mps, speed_mps, thrust, this->y);
    if (this->guidance && this->target != nullptr && speed > 1.0f) {
        // Direction vers la cible
        Vector3D to_target_dir = (to_target - position);
        to_target_dir.Normalize();

        // Composante latérale : partie de (to_target_dir) perpendiculaire à la vitesse
        float dot = vel_dir.DotProduct(&to_target_dir);
        Vector3D lateral = to_target_dir - vel_dir * dot;
        float lateral_len = lateral.Length();

        if (lateral_len > 0.001f) {
            int itemp = ((int)this->y) >> 10;
            if (itemp > 74) {
                itemp = 74;
            } else if (itemp < 0) {
                itemp = 0;
            }

            // Force aérodynamique latérale ~ 0.5 * rho * v² * S_control
            // AERO_CONTROL_COEFF est l'efficacité des surfaces de contrôle
            const float AERO_CONTROL_COEFF = 5.0f;
            float steer_magnitude;
            if (this->obj->entity_type == EntityType::bomb) {
                // Guidage direct (ordinateur de bord) — indépendant de ro[]
                const float MAX_G_BOMB = 10.0f;
                steer_magnitude = this->weight * MAX_G_BOMB * GRAVITY;
            } else {
                // Force aérodynamique pour les missiles
                const float AERO_CONTROL_COEFF = 15.0f;
                float dyn_pressure = 0.5f * ro[itemp] * RO_TO_SI * speed_mps * speed_mps;
                steer_magnitude = dyn_pressure * AERO_CONTROL_COEFF * CROSS_SECTIONAL_AREA;
                const float MAX_G = 50.0f;
                float max_steer = this->weight * MAX_G * GRAVITY;
                if (steer_magnitude > max_steer) {
                    steer_magnitude = max_steer;
                }
            }

            // Limite en g (ex: 20g max)
            const float MAX_G = 50.0f;
            float max_steer = this->weight * MAX_G * GRAVITY;
            if (steer_magnitude > max_steer) steer_magnitude = max_steer;

            steer_force = lateral * (steer_magnitude / lateral_len);
        }

        // Poussée moteur : le long de la vitesse (forward), pas vers la cible
        thrust_force = vel_dir * thrust;

    } else {
        // Pas de guidage : poussée dans la direction de la vitesse
        thrust_force = vel_dir * thrust;
    }

    Vector3D total_force = gravity_force + drag_force + lift_force + thrust_force + steer_force;
    Vector3D acceleration = total_force * (1.0f / this->weight);
    velocity = (velocity + (acceleration * deltaTime)).limit(mps * deltaTime);
    position = position + velocity;

    this->run_iterations++;
    return { position, velocity };
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
    std::tie(position, velocity) = this->ComputeTrajectory(tps);
    if (this->target != nullptr) {
        this->target->weapon_shooted_at_me = this;
        if (this->target->object->alive == false) {
            this->target->weapon_shooted_at_me = nullptr;
            this->target = nullptr;
        }
    }
    if (this->target != nullptr && this->target->plane != nullptr) {
        RSEntity *weapon_entity = this->obj;
        switch (weapon_entity->wdat->weapon_category) {
        case 2:
            if (this->target->plane->chaff_timer > 0) {
                this->target->weapon_shooted_at_me = nullptr;
                this->target = nullptr;
            }
            break;
        case 1:
            if (this->target->plane->flare_timer > 0) {
                this->target->weapon_shooted_at_me = nullptr;
                this->target = nullptr;
            }
            break; 
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
    if (this->distance > this->obj->wdat->effective_range && this->obj->dynn_miss != nullptr) {
        this->no_thrust = true;
    }
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
