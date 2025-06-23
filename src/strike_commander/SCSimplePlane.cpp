#include "precomp.h"
const float DRAG_COEFFICIENT = 0.47f;
SCSimplePlane::SCSimplePlane() : SCPlane() {

}
SCSimplePlane::SCSimplePlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
        float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
        RSArea *area, float x, float y, float z
    ): SCPlane(LmaxDEF, LminDEF, Fmax, Smax, ELEVF_CSTE, ROLLFF_CSTE, s, W, fuel_weight, Mthrust, b, ie_pi_AR, MIN_LIFT_SPEED, area, x, y, z) {
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
    this->ro2 = .5f * ro[0];
    init();
}
void SCSimplePlane::updateAcceleration() {
    

    this->ax = 0.0f;
    this->ay = 0.0f;
    this->az = 0.0f;
}
void SCSimplePlane::updateVelocity(){
    float deltaTime = 1.0f / (float) this->tps;
    this->vz = this->vz - ((.01f / this->tps / this->tps * this->thrust * this->Mthrust) + (DRAG_COEFFICIENT * this->vz)) * deltaTime;
    this->vz = std::clamp(this->vz, -25.0f, 25.0f);
    this->vx = 0.0f;
    this->vy = 0.0f;
}
void SCSimplePlane::updateForces() {
    
    this->acceleration.x = 0.0f;
    this->acceleration.y = 0.0f;
    this->acceleration.z = 0.0f;
    
}
void SCSimplePlane::computeLift() {
    this->lift_force = 0.0f; // Placeholder for lift force calculation
}
void SCSimplePlane::computeDrag() {
    this->drag_force = 0.0f; // Placeholder for drag force calculation
}
void SCSimplePlane::computeGravity() {
    this->gravity_force = 0.0f; // Placeholder for gravity force calculation
}
void SCSimplePlane::computeThrust() {

}
void SCSimplePlane::processInput() {
    float deltaTime = 1.0f / (float) this->tps;
    this->pitch_speed = (this->control_stick_y ) * deltaTime;
    this->roll_speed = (-this->control_stick_x ) * deltaTime;
    if (this->wheels && this->y > this->area->getY(this->x, this->z)) {
        this->wheels = 0;
    }
}