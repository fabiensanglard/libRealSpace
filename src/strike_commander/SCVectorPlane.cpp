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
    this->acceleration = this->acceleration * dt * dt;
    this->velocity = this->velocity + this->acceleration;
}
void SCVectorPlane::updateForces() {
    // 1. Calcul des forces dans le repère monde
    Vector3D thrust = forward * (thrust_force-drag_force);      // Poussée dans l'axe avant
    Vector3D lift   = up * lift_force;             // Portance dans l'axe haut de l'avion
    Vector3D gravity = {0, -this->gravity_force, 0};      // Gravité vers le bas du monde

    
    this->total_force = thrust;
}
void SCVectorPlane::computeLift() {
    
    this->ae = tenthOfDegreeToRad(this->pitch);
    this->Cl = this->object->entity->jdyn->LIFT/65535.0f;
    float vcarre = this->velocity.Length() * this->velocity.Length();
    this->lift_force = 0.5 * 1.522f * vcarre * this->s * this->ae * this->Cl;
    this->lift = this->lift_force;
}
void SCVectorPlane::computeDrag() {
    float vcarre = this->velocity.Length() * this->velocity.Length();
    this->drag_force = 0.5 * 1.522f * vcarre * this->s * 0.45f;
}
void SCVectorPlane::computeGravity() {
    float dt = 1.0f / (float)this->tps;
    this->gravity_force = this->W * (9.81f * dt);
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
    float dt = 1.0f / (float)this->tps;

    if (this->y <= this->area->getY(this->x, this->z)) {
        if (this->velocity.y < 0.0f) {
            this->velocity.y = 0.0f;
            this->acceleration.y = 0.0f;
        }
        this->y = this->area->getY(this->x, this->z);
    }
    // 2. Arcade : pousse la vitesse vers le forward, mais sans l'imposer
    float arcade_align = 0.8f; // 0 = pas d'effet arcade, 1 = effet arcade pur
    float speed = this->velocity.Length();
    Vector3D target_velocity = this->forward * speed;
    this->velocity = this->velocity * (1.0f - arcade_align) + target_velocity * arcade_align;

    // 3. Mets à jour les autres variables
    this->vx = this->velocity.x;
    this->vy = this->velocity.y;
    this->vz = this->velocity.z;
    this->ax = this->acceleration.x;
    this->ay = this->acceleration.y;
    this->az = this->acceleration.z;
    this->airspeed = sqrtf(this->velocity.x * this->velocity.x + this->velocity.z * this->velocity.z) * this->fps_knots;
}
void SCVectorPlane::Render() {
    //SCPlane::renderPlaneLined();
    // Forward: bleu
    Renderer.drawLine(
        this->position,
        {this->up.x * 10, this->up.y * 10, this->up.z * 10},
        {0, 0, 255}
    );

    /*Renderer.drawLine(
        this->position,
        {this->forward.x * 10, this->forward.y * 10, this->forward.z * 10},
        {255, 0, 0}
    );*/

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