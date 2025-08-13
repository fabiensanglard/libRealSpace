#include "precomp.h"
#include "SCSimulatedObject.h"
#define GRAVITY 9.8f
#define DRAG_COEFFICIENT 0.010f  // Coefficient de traînée
#define CROSS_SECTIONAL_AREA 0.10f  // Section transversale (m^2)
#define LIFT_COEFFICIENT  50.0f
#define MAX_VELOCITY 2000.0f  // Vitesse maximale autorisée en m/s

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
    
    size_t cpt=this->smoke_positions.size();
    
    for (auto pos: this->smoke_positions) {
        float alpha = 0.6f * ((float) cpt / (1.0f*(float)this->smoke_positions.size()));
        Renderer.drawParticle(pos, alpha);
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
    float speed = velocity.Norm();
    int itemp;
    itemp = ((int)this->y) >> 10;
    if (itemp > 74) {
        itemp = 74;
    } else if (itemp < 0) {
        itemp = 0;
    }
    float drag_magnitude = 0.5f * ro[itemp] * powf(speed,2) *DRAG_COEFFICIENT * CROSS_SECTIONAL_AREA;
    return velocity * (-drag_magnitude / speed);
}
Vector3D SCSimulatedObject::calculate_lift(Vector3D velocity) {
    float speed = velocity.Norm()*tps*3.2808399f;
    int itemp;
    itemp = ((int)this->y) >> 10;
    if (itemp > 74) {
        itemp = 74;
    } else if (itemp < 0) {
        itemp = 0;
    }
    float lift_magnitude = 0.5f * (ro[itemp]*tps) * powf(speed,2) * LIFT_COEFFICIENT * CROSS_SECTIONAL_AREA;
    if (lift_magnitude> this->weight * GRAVITY) {
        lift_magnitude = this->weight * GRAVITY;
    }
    Vector3D lift_direction = {
        0.0, lift_magnitude, 0.0
    };
    return lift_direction;
}
std::tuple<Vector3D, Vector3D> SCSimulatedObject::ComputeTrajectory(int tps) {
    this->tps = tps;
    float deltaTime = 1.0f / (float) tps;
    float thrust = 10.0f;
    if (this->obj->dynn_miss != nullptr) {
        thrust = (float)this->obj->dynn_miss->velovity_m_per_sec*1000.0f;
    }
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
    Vector3D to_target = {
        0.0f,
        0.0f,
        0.0f
    };
    if (this->target != nullptr) {
        to_target.x = (float) this->target->object->position.x;
        to_target.y = (float) this->target->object->position.y;
        to_target.z = (float) this->target->object->position.z;
    }
    Vector3D gravity_force = {0.0, -this->weight * GRAVITY, 0.0};
    Vector3D drag_force = this->calculate_drag(velocity);
    Vector3D lift_force = this->calculate_lift(velocity);
    
    Vector3D error = (to_target - position);
    
    // Force de poussée ajustée en fonction de la direction de l'erreur
    Vector3D thrust_force = error * (thrust / error.Norm());
    Vector3D other_way;
    error.Normalize();
    other_way = (error * thrust);
    Vector3D total_force = gravity_force + drag_force + lift_force + thrust_force;
    Vector3D acceleration = total_force * (1.0f/ this->weight);
    velocity = (velocity+(acceleration*deltaTime)).limit(MAX_VELOCITY*deltaTime);
    position = position+velocity;
    this->run_iterations++;
    return { position, velocity };
}
bool SCSimulatedObject::CheckCollision(SCMissionActors *entity) { 
    BoudingBox *bb{nullptr};
    if (entity == shooter) {
        return false;
    }
    bb = entity->object->entity->GetBoudingBpx();
    if (bb != nullptr) {
        if (this->x >= entity->object->position.x + bb->min.x && this->x <= entity->object->position.x + bb->max.x &&
            this->y >= entity->object->position.y+bb->min.y && this->y   <= entity->object->position.y + bb->max.y &&
            this->z >= entity->object->position.z+bb->min.z && this->z <= entity->object->position.z + bb->max.z) {
            // Collision detected: mark both objects as not alive and update score.
            return true;
        }
    }
    return false;
}
void SCSimulatedObject::Simulate(int tps) {
    Vector3D position, velocity;
    std::tie(position, velocity) = this->ComputeTrajectory(tps);

    if (this->target != nullptr) {
        if (this->CheckCollision(this->target)) {
            this->alive = false;
            this->target->object->alive = false;
            if (this->target->object->entity->explos != nullptr) {
                SCExplosion *explosion = new SCExplosion(this->target->object->entity->explos->objct, position);
                this->mission->explosions.push_back(explosion);
            }
            
            this->shooter->score += 100;
            if (this->target->plane != nullptr) {
                this->shooter->plane_down += 1;
            } else {
                this->shooter->ground_down += 1;
            }
            return;
        } else {
            const float distanceThreshold = 50.0f;
            Vector3D targetPos = {
                static_cast<float>(this->target->object->position.x),
                static_cast<float>(this->target->object->position.y),
                static_cast<float>(this->target->object->position.z)
            };
            float distance = (targetPos - position).Norm();
            if (distance < distanceThreshold) {
                this->alive = false;
                this->target->object->alive = false;
                this->shooter->score += 100;
                if (this->target->object->entity->explos != nullptr) {
                    SCExplosion *explosion = new SCExplosion(this->target->object->entity->explos->objct, position);
                    this->mission->explosions.push_back(explosion);
                }
                if (this->target->plane != nullptr) {
                    this->shooter->plane_down += 1;
                } else {
                    this->shooter->ground_down += 1;
                }
                return;
            }
        }
    }
    this->smoke_positions.push_back(position);
    if (this->smoke_positions.size() > 20) {
        this->smoke_positions.erase(this->smoke_positions.begin());
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
    if (this->distance > this->obj->wdat->effective_range) {
        this->alive = false;
        this->mission->explosions.push_back(new SCExplosion(this->obj->explos->objct, position));
    }
    if (this->y < this->mission->area->getY(this->x, this->z)) {
        this->mission->explosions.push_back(new SCExplosion(this->obj->explos->objct, position));
        this->alive = false;
    }
}
std::tuple<Vector3D, Vector3D> GunSimulatedObject::ComputeTrajectory(int tps) {
    float deltaTime = 1.0f / static_cast<float>(tps);
    Vector3D position = { this->x, this->y, this->z };
    Vector3D velocity = { this->vx, this->vy, this->vz };

    // Calcul de la force de gravité (en utilisant la constante GRAVITY et le poids de l'objet)
    // 
    if (this->weight == 0) {
        this->weight = 1;
    }
    Vector3D gravity_force = { 0.0f, -this->weight * GRAVITY , 0.0f };

    // Calcul de la force de frottement (drag) avec une densité d'air standard (1.225 kg/m^3)
    float speed = velocity.Norm();
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
    this->smoke_positions.push_back(position);
    if (this->smoke_positions.size() > 20) {
        this->smoke_positions.erase(this->smoke_positions.begin());
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
    /*if (this->distance > this->obj->wdat->effective_range * 10.0f) {
        this->alive = false;
    }*/
    for (auto entity: this->mission->actors) {
        if (this->CheckCollision(entity)) {
            this->alive = false;
            entity->object->alive = false;
            if (entity->object->entity->explos != nullptr) {
                SCExplosion *explosion = new SCExplosion(entity->object->entity->explos->objct, position);
                this->mission->explosions.push_back(explosion);
            }
            this->shooter->score += 100;
            if (entity->plane != nullptr) {
                this->shooter->plane_down += 1;
            } else {
                this->shooter->ground_down += 1;
            }
            break;
        }
    }
    // Désactive l'objet s'il touche le sol
    if (this->y < this->mission->area->getY(this->x, this->z)) {
        this->alive = false;
        if (this->obj->explos != nullptr && this->obj->explos->objct != nullptr) {
            SCExplosion *explosion = new SCExplosion(this->obj->explos->objct, position);
            this->mission->explosions.push_back(explosion);
        }
    }
}

void GunSimulatedObject::Render() {
    if (this->obj->vertices.size() == 0) {
        Vector3D pos = {this->x, this->y, this->z};
        Vector3D end = {this->vx, this->vy, this->vz};
        Vector3D color = {1.0f, 1.0f, 0.0f};
        Renderer.drawLine(pos, end, color);
    } else {
        Vector3D pos = {this->x, this->y, this->z};
        Vector3D orient = {this->azimuthf, this->elevationf, 0.0f};
        Renderer.drawModel(this->obj, pos, orient);
    }
    
    size_t cpt=this->smoke_positions.size();
    for (auto pos: this->smoke_positions) {
        Renderer.drawParticle(pos, 0.6f * ((float) cpt / (1.0f*(float)this->smoke_positions.size())));
    }
}
