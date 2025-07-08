#include "SCVectorPlane.h"

SCVectorPlane::SCVectorPlane() {}

SCVectorPlane::SCVectorPlane(
    float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
    float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED, RSArea *area, float x, float y,
    float z
) {
    this->acceleration = {0.0f, 0.0f, 0.0f};
    this->velocity = {0.0f, 0.0f, 0.0f};
    this->thrust_force = 0.0f;
    this->lift_force = 0.0f;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->vz = 0.0f;
    this->weaps_load.reserve(9);
    this->weaps_load.resize(9);
    this->alive = 0;
    this->status = 0;
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->roll = 0;
    this->roll_speed = 0;
    this->yaw = 0.0f;
    this->pitch = 0.0f;
    this->pitch_speed = 0.0f;
    this->yaw_speed = 0.0f;
    this->airspeed = 0;
    this->thrust = 0;
    this->rollers = 0.0f;
    this->rudder = 0.0f;
    this->elevator = 0.0f;
    this->object = nullptr;

    this->LmaxDEF = LmaxDEF;
    this->LminDEF = LminDEF;
    this->Fmax = Fmax;
    this->Smax = Smax;
    this->ELEVF_CSTE = ELEVF_CSTE;
    this->ROLLFF_CSTE = ROLLFF_CSTE;
    this->s = s;
    this->W = W;
    this->fuel_weight = fuel_weight;
    this->Mthrust = Mthrust;
    this->b = b;
    this->ie_pi_AR = ie_pi_AR;
    this->MIN_LIFT_SPEED = MIN_LIFT_SPEED;
    this->object = nullptr;
    this->area = area;
    this->tps = 30;
    this->last_time = SDL_GetTicks();
    this->tick_counter = 0;
    this->last_tick = 0;
    this->x = x;
    this->y = y;
    this->z = z;
    this->position = {x, y, z};
    this->ro2 = .5f * 1.522f;
    init();
}

void SCVectorPlane::updateAcceleration() {
    this->acceleration = {
        total_force.x / this->W,
        total_force.y / this->W,
        total_force.z / this->W
    };
}
void SCVectorPlane::updateVelocity() {
    float dt = 1.0f / (float)this->tps;
    
    Vector3D deltaV = this->acceleration * dt;
    Vector3D physicsVelocity = this->velocity + deltaV;
    
    
    if (this->thrust_force > 0.1f) {
    
        float dpitch = tenthOfDegreeToRad(this->pitch_speed);
        float droll = tenthOfDegreeToRad(this->roll_speed);
        float dyaw = tenthOfDegreeToRad(this->yaw_speed);
        
        
        if (dpitch != 0.0f) {
            rotateAroundAxis(physicsVelocity, right, dpitch);
        }

        if (dyaw != 0.0f) {
            rotateAroundAxis(physicsVelocity, up, dyaw);
        }

        if (droll != 0.0f) {
            rotateAroundAxis(physicsVelocity, forward, droll);
        }
    }
    this->velocity = physicsVelocity;
    // Calcul de la vitesse terminale en chute libre (seule la gravité et la traînée agissent)
    float rho = 1.522f; // densité de l'air (en kg/m³)
    float Cd = this->object->entity->drag / 65535.0f;
    float g = 9.81f;
    float v_terminal = sqrtf((2.0f * this->W * g) / (rho * Cd * this->s));

    // Si la vitesse verticale descendante dépasse la vitesse terminale, on la limite
    if (this->velocity.y < -v_terminal) {
        this->velocity.y = -v_terminal;
    }
    if (this->y <= this->area->getY(this->x, this->z)) {
        if (this->velocity.y < 0.0f) {
            this->velocity.y = 0.0f;
            this->acceleration.y = 0.0f;
        }
        this->y = this->area->getY(this->x, this->z);
        this->on_ground = true;
    } else {
        this->on_ground = false;
    }
}
void SCVectorPlane::updateForces() {

    Vector3D thrust = forward * this->thrust_force;
    Vector3D drag = forward * -this->drag_force;
    Vector3D lift = {0 , this->lift_force, 0};
    Vector3D gravity = {0, -this->gravity_force, 0};
    
    
    this->total_force = thrust + lift + gravity + drag;
}
void SCVectorPlane::computeLift() {
    // Vérifier si la vitesse est significative avant de calculer la portance
    float speed = this->velocity.Length();
    if (this->velocity.DotProduct(&forward) <= 0.0f) {
        this->lift_force = 0.0f;
        this->lift = 0.0f;
        this->ae = 0.0f;
        return;
    }
    
    // Calculer l'angle d'attaque réel entre la direction de l'avion et la vitesse
    Vector3D velocity_normalized = this->velocity;
    velocity_normalized.Normalize();
    
    // Angle entre le vecteur vitesse et l'axe forward de l'avion
    float dot_product = forward.DotProduct(&velocity_normalized);
    // Limiter le dot product entre -1 et 1 pour éviter les erreurs numériques
    dot_product = fmaxf(-1.0f, fminf(1.0f, dot_product));
    this->ae = acosf(dot_product);
    
    // Coefficient de portance basé sur l'angle d'attaque
    this->Cl = this->object->entity->jdyn->LIFT / 65535.0f + 0.1f * this->ae; // Modèle simplifié
    if (this->ae > 0.5f) {  // ~30 degrés
        this->Cl *= (1.0f - (this->ae - 0.5f) * 2.0f); // Décrochage progressif
    }
    
    // Limiter le coefficient de portance à des valeurs raisonnables
    this->Cl = fmaxf(0.0f, fminf(3.5f, this->Cl));
    
    // Calcul de la portance avec la vitesse réelle
    float vcarre = speed * speed;
    
    this->lift_force = 0.5f * 1.522f * vcarre * this->s * this->Cl;
    
    
    // Limiter la force de portance maximale à une valeur raisonnable
    // Par exemple, limiter à 3 fois le poids de l'avion
    float max_lift = this->W *9.81f * 1.0f;
    this->lift_force = fminf(this->lift_force, max_lift);
    this->lift = this->lift_force;
    if (this->thrust_force <= 0.1f) {
        // Si l'avion n'a pas de poussée, on ne peut pas générer de portance
        this->lift_force = 0.0f;
        this->lift = 0.0f;
        this->ae = 0.0f;
    }
}
void SCVectorPlane::computeDrag() {
    // Calculer la vitesse relative au lieu de la vitesse absolue
    float speed = this->velocity.Length();
    float vcarre = speed * speed;
    
    // Coefficient de traînée de base AUGMENTÉ
    float Cd = this->object->entity->drag/65535.0f;
    
    // Augmenter la traînée avec l'angle d'attaque
    float angle_of_attack = this->ae;
    Cd += 0.1f * angle_of_attack * angle_of_attack;
    
    
    this->drag_force = 0.5f * 1.522f * vcarre * this->s * Cd;
    
    this->drag = this->drag_force;
}
void SCVectorPlane::computeGravity() {
    float dt = 1.0f / (float)this->tps;
    this->gravity_force = this->W * 9.81f;
}
void SCVectorPlane::computeThrust() {
    this->thrust_force = .01f * this->thrust * this->Mthrust;
}
void SCVectorPlane::processInput() {
    float dt = 1.0f / (float)this->tps;
    if (this->airspeed < this->MIN_LIFT_SPEED && this->on_ground) {
        // Si l'avion est en dessous de la vitesse minimale de portance, on ne peut pas contrôler le pitch
        this->control_stick_y = 0;
    }
    this->pitch_speed = (this->control_stick_y ) * dt;
    this->roll_speed = (this->control_stick_x ) * dt;

    float induced_pitch_speed = 0.0f;
    if (this->lift_force < this->gravity_force * 0.95f && !this->on_ground) {
        Vector3D velocity_normalized = this->velocity;
        velocity_normalized.Normalize();
        float dot = forward.DotProduct(&velocity_normalized);
        dot = fmaxf(-1.0f, fminf(1.0f, dot));
        float angle = acosf(dot);
        if (angle > 0.05f) { // ~3 degrés
            induced_pitch_speed = -radToDegree(angle)*0.1f; // facteur à ajuster
        }
    }
    this->pitch_speed += induced_pitch_speed;

    // Convertir les vitesses angulaires en radians
    float dpitch = tenthOfDegreeToRad(this->pitch_speed);
    float droll  = tenthOfDegreeToRad(this->roll_speed);
    float dyaw   = tenthOfDegreeToRad(this->yaw_speed);

    if (dpitch != 0.0f) {
        rotateAroundAxis(forward, right, dpitch);
        rotateAroundAxis(up,      right, dpitch);
    }
    if (dyaw != 0.0f) {
        rotateAroundAxis(forward, up, dyaw);
        rotateAroundAxis(right,   up, dyaw);
    }
    if (droll != 0.0f) {
        rotateAroundAxis(right, forward, droll);
        rotateAroundAxis(up,    forward, droll);
    }

    forward.Normalize();
    right = forward.CrossProduct(&up);
    right.Normalize();
    up = right.CrossProduct(&forward);
    up.Normalize();
}
void SCVectorPlane::updatePosition() {
    
    float dt = 1.0f / (float)this->tps;
    Vector3D displacement = this->velocity * dt;
    this->position.Add(&displacement);
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;

    // --- Calcul des angles à partir des axes locaux ---
    // Pitch : arcsin(-forward.y)
    float new_pitch = asinf(forward.y);
    // Yaw : angle entre la projection de forward sur XZ et l'axe -Z
    float proj_length = sqrtf(forward.x * forward.x + forward.z * forward.z);
    float new_yaw = 0.0f;
    if (proj_length > 1e-6f) {
        new_yaw = atan2f(-forward.x, -forward.z);
    } else {
        new_yaw = 0.0f; // Avion à la verticale, yaw indéfini
    }
    float new_roll = atan2f(right.y, up.y);

    // Conversion en dixièmes de degrés
    this->pitch = radToDegree(new_pitch) * 10.0f;
    this->yaw   = radToDegree(new_yaw)   * 10.0f;
    this->roll  = radToDegree(new_roll)  * 10.0f;

}
void SCVectorPlane::rotateAroundAxis(Vector3D& v, const Vector3D& axis, float angle) {
    // Formule de Rodrigues
    Vector3D k = axis;
    k.Normalize();
    v = v * cosf(angle) + k.CrossProduct(&v) * sinf(angle) + k * (k.DotProduct(&v)) * (1 - cosf(angle));
}
void SCVectorPlane::updatePlaneStatus() {
    
    this->vx = this->velocity.x;
    this->vy = this->velocity.y;
    this->vz = this->velocity.z;
    this->ax = this->acceleration.x;
    this->ay = this->acceleration.y;
    this->az = this->acceleration.z;
    this->fps_knots = 1.944f;  // Facteur de conversion m/s vers nœuds
    this->airspeed = velocity.Length() * this->fps_knots;
}
void SCVectorPlane::Render() {
    //SCPlane::renderPlaneLined();
    // Forward: bleu
    Renderer.drawLine(
        this->position,
        {this->up.x * 10, this->up.y * 10, this->up.z * 10},
        {0, 0, 255}
    );

    Renderer.drawLine(
        this->position,
        {this->forward.x * 10, this->forward.y * 10, this->forward.z * 10},
        {255, 0, 0}
    );

    Renderer.drawLine(
        this->position,
        {this->right.x * 10, this->right.y * 10, this->right.z * 10},
        {0, 255, 0}
    );
    Vector3D total_force_normalized = {
        this->total_force.x,
        this->total_force.y,
        this->total_force.z
    };
    total_force_normalized.Normalize();
    Renderer.drawLine(
        this->position,
        {total_force_normalized.x * 10, total_force_normalized.y * 10, total_force_normalized.z * 10},
        {255, 255, 0}
    );
    Renderer.drawLine(
        this->position,
        this->velocity,
        {255, 0, 255}
    );
}