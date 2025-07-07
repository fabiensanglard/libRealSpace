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
    
    // NOUVEAU: Appliquer les rotations au vecteur vitesse
    // Facteur de suivi de rotation - 1.0 = suivi complet, 0.0 = pas de suivi
    float velocity_follow_factor = 1.0f;
    
    if (velocity_follow_factor > 0.0f && physicsVelocity.Length() > 0.1f) {
        // Convertir les vitesses angulaires en radians
        float dpitch = tenthOfDegreeToRad(this->pitch_speed);
        float droll = tenthOfDegreeToRad(this->roll_speed);
        float dyaw = tenthOfDegreeToRad(this->yaw_speed);
        

        // Rotation autour de l'axe "right" (pitch)
        if (dpitch != 0.0f) {
            rotateAroundAxis(physicsVelocity, right, dpitch);
        }
        // Rotation autour de l'axe "up" (yaw)
        if (dyaw != 0.0f) {
            rotateAroundAxis(physicsVelocity, up, dyaw);
        }
        // Rotation autour de l'axe "forward" (roll)
        if (droll != 0.0f) {
            rotateAroundAxis(physicsVelocity,    forward, droll);
        }
    }
    this->velocity = physicsVelocity;
    if (this->y <= this->area->getY(this->x, this->z)) {
        if (this->velocity.y < 0.0f) {
            this->velocity.y = 0.0f;
            this->acceleration.y = 0.0f;
        }
        this->y = this->area->getY(this->x, this->z);
    }
}
void SCVectorPlane::updateForces() {
    // 1. Calcul des forces dans le repère monde
    
    // Traînée dans la direction opposée à la vitesse
    Vector3D velocity_direction = this->velocity;
    velocity_direction.Normalize();
    Vector3D drag = velocity_direction * (-this->drag_force);
    
    // Poussée dans l'axe avant
    Vector3D thrust = forward * this->thrust_force;
    
    // Portance dans l'axe haut de l'avion
    Vector3D lift = up * lift_force;
    if (up.y < 0) {
        lift.Negate();
    }
    
    // Gravité vers le bas du monde
    Vector3D gravity = {0, -this->gravity_force, 0};
    
    // Force latérale due à l'inclinaison (roll) de l'avion
    Vector3D lateral_force = {0, 0, 0};
    if (this->roll != 0) {
        float roll_rad = tenthOfDegreeToRad(this->roll);
        float lateral_component = this->lift_force * sinf(roll_rad);
        lateral_force = right * lateral_component;
    }
    
    this->total_force = thrust + lift + gravity + drag + lateral_force;
}
void SCVectorPlane::computeLift() {
    // Vérifier si la vitesse est significative avant de calculer la portance
    float speed = this->velocity.Length();
    if (speed < 0.1f) {
        // Vitesse trop faible pour générer une portance significative
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
    this->Cl = 0.5f + 0.1f * this->ae; // Modèle simplifié
    if (this->ae > 0.5f) {  // ~30 degrés
        this->Cl *= (1.0f - (this->ae - 0.5f) * 2.0f); // Décrochage progressif
    }
    
    // Limiter le coefficient de portance à des valeurs raisonnables
    this->Cl = fmaxf(0.0f, fminf(3.5f, this->Cl));
    
    // Calcul de la portance avec la vitesse réelle
    float vcarre = speed * speed;
    
    this->lift_force = 0.5f * 1.522f * vcarre * this->s * this->Cl;
    
    // Augmenter artificiellement la portance quand l'avion pointe vers le haut, mais avec limite
    float upFactor = this->forward.DotProduct(&Vector3D(0,1,0));
    if (upFactor > 0) {
        // Limiter le facteur d'augmentation à 1.5x maximum
        this->lift_force *= (1.0f + fminf(upFactor, 1.0f) * 0.5f);
    }
    
    // Limiter la force de portance maximale à une valeur raisonnable
    // Par exemple, limiter à 3 fois le poids de l'avion
    float max_lift = this->W *9.81f * 1.0f;
    this->lift_force = fminf(this->lift_force, max_lift);
    
    this->lift = this->lift_force;
}
void SCVectorPlane::computeDrag() {
    // Calculer la vitesse relative au lieu de la vitesse absolue
    float speed = this->velocity.Length();
    float vcarre = speed * speed;
    
    // Coefficient de traînée de base AUGMENTÉ
    float Cd = 0.05f;  // Valeur augmentée de 0.02 à 0.05
    
    // Augmenter la traînée avec l'angle d'attaque
    float angle_of_attack = this->ae;
    Cd += 0.1f * angle_of_attack * angle_of_attack;
    
    // Augmenter la traînée avec le roll
    float roll_rad = fabsf(tenthOfDegreeToRad(this->roll));
    Cd += 0.05f * roll_rad;
    
    // Ajouter une traînée SUPPLÉMENTAIRE quand les moteurs sont coupés
    if (this->thrust_force < 0.1f * this->Mthrust) {
        Cd += 0.1f;  // Traînée additionnelle IMPORTANTE quand moteurs au ralenti
    }
    
    this->drag_force = 0.5f * 1.522f * vcarre * this->s * Cd;
    
    // Garantir une traînée minimale à toute vitesse
    float min_drag = 0.05f * this->W;
    this->drag_force = fmaxf(this->drag_force, min_drag);
    
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
    this->pitch_speed = (this->control_stick_y ) * dt;
    this->roll_speed = (this->control_stick_x ) * dt;

    

    // Convertir les vitesses angulaires en radians
    float dpitch = tenthOfDegreeToRad(this->pitch_speed);
    float droll  = tenthOfDegreeToRad(this->roll_speed);
    float dyaw   = tenthOfDegreeToRad(this->yaw_speed);

    // Rotation autour de l'axe "right" (pitch)
    if (dpitch != 0.0f) {
        rotateAroundAxis(forward, right, dpitch);
        rotateAroundAxis(up,      right, dpitch);
    }
    // Rotation autour de l'axe "up" (yaw)
    if (dyaw != 0.0f) {
        rotateAroundAxis(forward, up, dyaw);
        rotateAroundAxis(right,   up, dyaw);
    }
    // Rotation autour de l'axe "forward" (roll)
    if (droll != 0.0f) {
        rotateAroundAxis(right, forward, droll);
        rotateAroundAxis(up,    forward, droll);
    }

    // Renormaliser pour garder l'orthogonalité
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
    // Roll : atan2(right.y, up.y)
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