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
    this->fps_knots = this->tps * 1.944f;
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
    this->acceleration = this->acceleration * dt;
    this->velocity = this->velocity + this->acceleration;
    this->velocity = this->velocity * dt;
}
void SCVectorPlane::updateForces() {
    // 1. Calcul des forces dans le repère monde
    Vector3D thrust = forward * (thrust_force-drag_force);      // Poussée dans l'axe avant
    Vector3D lift   = up * lift_force;             // Portance dans l'axe haut de l'avion
    Vector3D gravity = {0, -this->gravity_force, 0};      // Gravité vers le bas du monde

    this->total_force = thrust + lift + gravity;
}
void SCVectorPlane::computeLift() {
    Vector3D velocity_dir = this->velocity;
    velocity_dir.Normalize();

    // Calcul de la projection de la vitesse sur le plan de portance (défini par right et forward)
    Vector3D lift_plane_normal = this->right.CrossProduct(&this->forward);
    lift_plane_normal.Normalize();

    // Composante de la vitesse dans le plan de portance
    Vector3D vel_in_lift_plane = velocity_dir - lift_plane_normal * velocity_dir.DotProduct(&lift_plane_normal);
    vel_in_lift_plane.Normalize();

    // L'angle d'attaque est l'angle entre le vecteur forward et la composante de la vitesse dans le plan de portance
    float dot = this->forward.DotProduct(&vel_in_lift_plane);
    dot = std::clamp(dot, -1.0f, 1.0f); // Sécurité numérique
    float angle_of_attack = acosf(dot);

    // Signe de l'angle d'attaque (positif si la vitesse vient du dessous de l'aile)
    float sign = this->up.DotProduct(&velocity_dir) < 0 ? 1.0f : -1.0f;
    angle_of_attack *= sign;

    // Stocker ou utiliser l'angle d'attaque selon les besoins
    this->ae = angle_of_attack;
    this->Cl = this->object->entity->jdyn->LIFT/65535.0f;
    this->lift_force = this->velocity.Length() * this->velocity.Length() * this->ae * this->Cl;
    this->lift = this->lift_force;
}
void SCVectorPlane::computeDrag() {
    this->drag_force = this->velocity.Length() *this->velocity.Length() * this->object->entity->jdyn->DRAG;
}
void SCVectorPlane::computeGravity() {
    this->gravity_force = this->W * 9.81f;
}
void SCVectorPlane::computeThrust() {
    this->thrust_force = .01f * this->thrust * this->Mthrust;
}
void SCVectorPlane::processInput() {
    float deltaTime = 1.0f / (float) this->tps;
    this->pitch_speed = (this->control_stick_y ) * deltaTime;
    this->roll_speed = (this->control_stick_x ) * deltaTime;

    float dt = 1.0f / (float)this->tps;

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
    // Mise à jour de la position selon l'axe avant (forward)
    
    //velocity.Scale(dt);
    //velocity = velocity * dt;
    if (this->y <= this->area->getY(this->x, this->z)) {
        this->velocity.y = 0.0f;
        this->acceleration.y = 0.0f;
        this->y = this->area->getY(this->x, this->z);
    }
    this->position.Add(&this->velocity);
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
    this->vz = this->velocity.z;
    this->vx = this->velocity.x;
    this->vy = this->velocity.y;
    this->ax = this->acceleration.x;
    this->ay = this->acceleration.y;
    this->az = this->acceleration.z;
    
    this->airspeed = this->velocity.Length() * this->fps_knots;
    //vy = this->velocity.y;
    //this->velocity = this->forward * vz;
}
void SCVectorPlane::Render() {
    SCPlane::renderPlaneLined();
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