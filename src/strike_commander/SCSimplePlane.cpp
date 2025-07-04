#include "precomp.h"
const float DRAG_COEFFICIENT = 0.47f;
const float LIFT_COEFFICIENT = 0.83f;
const float GRAVITY = 9.80f; // m/s^2
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
    this->ro2 = .5f * 1.522f;
    init();
    this->fps_knots = this->tps * 1.944f;
}
void SCSimplePlane::updatePlaneStatus() {
    this->fps_knots = this->tps * 1.944f;
    this->airspeed = -(int)(this->fps_knots * this->vz);
    this->climbspeed = (short)(this->tps * (this->y - this->last_py));
    //this->g_load = (this->lift_force*) / this->gravity;
    this->ax = this->acceleration.x;
    this->ay = this->acceleration.y;
    this->az = this->acceleration.z;
}
void SCSimplePlane::updatePosition() {
    float temp{0.0f};
    
    this->ptw.Identity();
    this->ptw.translateM(this->x, this->y, this->z);

    this->ptw.rotateM(tenthOfDegreeToRad(this->yaw), 0, 1, 0);
    this->ptw.rotateM(tenthOfDegreeToRad(this->pitch), 1, 0, 0);
    this->ptw.rotateM(tenthOfDegreeToRad(this->roll), 0, 0, 1);
    
    this->ptw.translateM(this->vx, this->vy, this->vz);
    if (round(this->yaw_speed) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->yaw_speed)), 0, 1, 0);
    if (round(this->pitch_speed) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->pitch_speed)), 1, 0, 0);
    if (round(this->roll_speed) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad((float)this->roll_speed), 0, 0, 1);

    temp = 0.0f;
    this->pitch = (-asinf(this->ptw.v[2][1]) * 180.0f / (float)M_PI) * 10;

    float ascos = 0.0f;

    temp = cosf(tenthOfDegreeToRad(this->pitch));

    if (temp != 0.0) {

        float sincosas = this->ptw.v[2][0] / temp;

        if (sincosas > 1) {
            sincosas = 1.0f;
        } else if (sincosas < -1) {
            sincosas = -1;
        }
        this->yaw = asinf(sincosas) / (float)M_PI * 1800.0f;
        if (this->ptw.v[2][2] < 0.0) {
            /* if heading into z	*/
            this->yaw = 1800 - this->yaw;
        }
        if (this->yaw < 0) {
            /* then adjust		*/
            this->yaw += 3600;
        }

        this->roll = (asinf(this->ptw.v[0][1] / temp) / (float)M_PI) * 1800.0f;
        if (this->ptw.v[1][1] < 0.0) {
            /* if upside down	*/
            this->roll = 1800.0f - this->roll;
        }
        if (this->roll < 0) {
            this->roll += 3600.0f;
        }
    }
    /* save last position	*/
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x = this->ptw.v[3][0];
    this->y = this->ptw.v[3][1];
    this->z = this->ptw.v[3][2];

    this->groundlevel = this->area->getY(this->x, this->z);
    /****************************************************************
    /*	perform incremental rotations on velocities
    /****************************************************************/

    this->incremental.Identity();
    if (this->roll_speed)
        this->incremental.rotateM(tenthOfDegreeToRad((float)-this->roll_speed), 0, 0, 1);
    if (this->pitch_speed)
        this->incremental.rotateM(tenthOfDegreeToRad(-this->pitch_speed), 1, 0, 0);
    if (this->yaw_speed)
        this->incremental.rotateM(tenthOfDegreeToRad(-this->yaw_speed), 0, 1, 0);
    this->incremental.translateM(this->vx, this->vy, this->vz);

    this->vx = this->incremental.v[3][0];
    this->vy = this->incremental.v[3][1];
    this->vz = this->incremental.v[3][2];

}
void SCSimplePlane::updateAcceleration() {
    
    if (this->acceleration.x != 0.0f) {
        this->acceleration.x /= this->inverse_mass;
    }
    this->acceleration.y /= this->W+this->fuel;
    this->acceleration.z /= this->W+this->fuel;

    this->acceleration.x -= this->ptw.v[0][1] * this->gravity;
    this->acceleration.y -= this->ptw.v[1][1] * this->gravity;
    this->acceleration.z -= this->ptw.v[2][1] * this->gravity;
}
void SCSimplePlane::updateVelocity(){
    float temp{0.0f};
    this->vz += this->acceleration.z;
    this->vx += this->acceleration.x;
    if (this->on_ground && this->status > MEXPLODE) {
        temp = 0.0f;
        float mcos;
        this->vx = 0.0;
        gl_sincos(this->pitch, &temp, &mcos);
        temp = this->vz * temp / mcos;
        if (this->vy + this->acceleration.y < temp) {
            this->acceleration.y = temp - this->vy;
        }
    }
    this->vy += this->acceleration.y;
}
void SCSimplePlane::updateForces() {
    
    int vz_sign = (this->vz < 0.0f) ? 1 : -1;
    this->acceleration.x = 0.0f;
    this->acceleration.y = this->lift_force + (vz_sign * this->gravity_drag_force);
    this->acceleration.z = this->lift_drag_force - this->thrust_force + (vz_sign * this->drag_force);
    
}
void SCSimplePlane::computeLift() {
    // Calcul de l'angle d'attaque en radians
    if (this->vz >= 0.0f) {
        return;
    }
    this->ae = (this->vy / this->vz) + 0.17f;
    this->Cl = LIFT_COEFFICIENT * this->ae;
    this->ro2 = 0.5f * 1.522f; // Coefficient de portance, ajusté pour l'exemple
    float areo_lift = this->s * this->vz * this->ro2 * this->Cl;
    this->lift_force = areo_lift * this->vz;
    this->lift_drag_force = areo_lift * -this->vy;
    this->lift = this->lift_force;
    this->g_load = this->lift_force / this->gravity_force;
}
void SCSimplePlane::computeDrag() {
    this->drag_force = DRAG_COEFFICIENT * this->vz * this->vz; // Placeholder for drag force calculation
    this->drag = this->drag_force;
    this->gravity_drag_force = this->vy * this->vy * DRAG_COEFFICIENT ;
}
void SCSimplePlane::computeGravity() {
    this->gravity_force = 0.0f; // Placeholder for gravity force calculation
    this->gravity = GRAVITY / this->tps / this->tps;
    this->gravity_force = this->gravity * (this->W + (this->fuel));
}
void SCSimplePlane::computeThrust() {
    this->thrust_force = .01f / this->tps / this->tps * this->thrust * this->Mthrust;
}
void SCSimplePlane::processInput() {
    float deltaTime = 1.0f / (float) this->tps;
    this->pitch_speed = (this->control_stick_y ) * deltaTime;
    this->roll_speed = (-this->control_stick_x ) * deltaTime;
    if (this->airspeed < this->MIN_LIFT_SPEED && this->on_ground) {
        this->pitch_speed = 0.0f;
        this->roll_speed = 0.0f;
        this->pitch = 0.0f;
        this->roll = 0.0f;
    } 
    if (this->wheels && this->y > this->area->getY(this->x, this->z)) {
        this->wheels = 0;
    }
    this->elevation_speedf = this->pitch_speed;
    this->azimuth_speedf = this->yaw_speed;
}
