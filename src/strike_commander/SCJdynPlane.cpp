#include "precomp.h"
const float GRAVITY = 9.81f; // m/s^2
const float AIR_DENSITY = 1.225f; // kg/m^3
const float DRAG_COEFFICIENT = 0.47f;
const float LIFT_COEFFICIENT = 0.5f; // Doit être ajusté en fonction de la forme de l'objet
const float WING_AREA = 1.0f; // m^2
SCJdynPlane::SCJdynPlane() {

}
SCJdynPlane::SCJdynPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
        float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
        RSArea *area, float x, float y, float z
    ): SCPlane(LmaxDEF, LminDEF, Fmax, Smax, ELEVF_CSTE, ROLLFF_CSTE, s, W, fuel_weight, Mthrust, b, ie_pi_AR, MIN_LIFT_SPEED, area, x, y, z) {
    
}
SCJdynPlane::~SCJdynPlane() {

}
void SCJdynPlane::Simulate() {

    uint32_t current_time = SDL_GetTicks();
    uint32_t elapsed_time = (current_time - this->last_time) / 1000;
    int newtps = 0;
    if (elapsed_time > 1) {
        uint32_t ticks = this->tick_counter - this->last_tick;
        newtps = ticks / elapsed_time;
        this->last_time = current_time;
        this->last_tick = this->tick_counter;
        if (newtps > this->tps / 2) {    
            this->tps = newtps;
        }
    }
    this->fps_knots = this->tps * (3600.0f / 6082.0f);
    float deltaTime = 1.0f / (float) this->tps;
    
    float pitch_input = (this->control_stick_y / 100.0f) * deltaTime;
    float roll_input = (-this->control_stick_x / 100.0f) * deltaTime;
    if (this->object->alive == false) {
        this->thrust = 0;
        this->vy -= +0.1f;
        if (this->vz > 5.0f) {
            this->vz = 5.0f;
        }
        
    }
    Matrix rottm;
    rottm.Identity();
    rottm.translateM(this->x, this->y, this->z);

    rottm.rotateM(this->yaw, 0, 1, 0);
    rottm.rotateM(this->pitch, 1, 0, 0);
    rottm.rotateM(this->roll, 0, 0, 1);

    rottm.rotateM(pitch_input, 1, 0, 0);
    rottm.rotateM(roll_input, 0, 0, 1);
    this->vz = this->vz - ((.01f / this->tps / this->tps * this->thrust * this->Mthrust) + (DRAG_COEFFICIENT * this->vz)) * deltaTime;
    this->vz = std::clamp(this->vz, -25.0f, 25.0f);
    rottm.translateM(this->vx/3.2808399f, this->vy/3.2808399f, this->vz/3.2808399f);
    this->on_ground = false;
    this->pitch = -asinf(rottm.v[2][1]);
    float temp = cosf(this->pitch);
    if (temp != 0.0) {
        float sincosas = rottm.v[2][0] / temp;
        if (sincosas > 1.0f) {
            sincosas = 1.0f;
        } else if (sincosas < -1.0f) {
            sincosas = -1;
        }
        this->yaw = asinf(sincosas);
        if (rottm.v[2][2] < 0.0f) {
            this->yaw = (float)M_PI - this->yaw;
        }
        if (this->yaw < 0.0f) {
            this->yaw += 2.0f*(float)M_PI;
        }
        if (this->yaw > 2.0f*(float)M_PI) {
            this->yaw -= 2.0f*(float)M_PI;
        }
        this->roll = asinf(rottm.v[0][1] / temp);
        if (rottm.v[1][1] < 0.0f) {
            /* if upside down	*/
            this->roll = (float)M_PI - this->roll;
        }
        if (this->roll < 0) {
            this->roll += 2.0f*(float)M_PI;
        }
    }

    this->x = rottm.v[3][0];
    this->y = rottm.v[3][1];
    this->z = rottm.v[3][2];
    
    // Retrieve dynamic parameters from jdynn data
    float mass = this->object->entity->weight_in_kg;
    float wingArea = 1.0f;
    float liftCoefficient = this->object->entity->jdyn->LIFT;
    float dragCoefficient = this->object->entity->jdyn->DRAG;

    // Calculate current speed (magnitude of velocity vector)
    float speed = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);

    // Compute dynamic pressure: q = 0.5 * air_density * speed^2
    float q = 0.5f * AIR_DENSITY * speed;

    // Compute forces (in Newtons)
    float F_drag = dragCoefficient * q * wingArea;
    float F_lift = liftCoefficient * q * wingArea;
    float F_thrust = this->thrust * this->Mthrust;
    float F_gravity = mass * GRAVITY;
    if (y <= this->area->getY(this->x, this->z)) {
        // If the plane is on the ground, set vertical velocity to zero
        //this->vy = 0.0f;
        F_gravity = 0.0f; // No gravity force when on ground
        this->on_ground = true;
    } else {
        this->on_ground = false;
    }
    // Compute net accelerations (in m/s^2) in the body frame:
    // Assume positive forward is along the aircraft's forward axis,
    // and positive upward is along its upwards axis.
    float acc_forward = (F_thrust - F_drag) / mass;
    float acc_upward = (F_lift - F_gravity) / mass;

    // Extract forward and up vectors from the rotation matrix.
    // (Assuming the third row is the forward vector and the second row is the up vector)
    struct Vector3 { float x, y, z; };
    Vector3 forward = { rottm.v[2][0], rottm.v[2][1], rottm.v[2][2] };
    Vector3 up = { rottm.v[1][0], rottm.v[1][1], rottm.v[1][2] };

    // Update velocity based on the net accelerations and the elapsed time.
    this->vx += (forward.x * acc_forward + up.x * acc_upward) * deltaTime;
    this->vy += (forward.y * acc_forward + up.y * acc_upward) * deltaTime;
    this->vz += (forward.z * acc_forward + up.z * acc_upward) * deltaTime;
    
    this->airspeed = -(int)(this->fps_knots * this->vz);
    this->azimuthf = (this->yaw * 180.0f / (float) M_PI) * 10.0f;
    this->elevationf = (this->pitch * 180.0f / (float) M_PI) * 10.0f;
    this->twist = (this->roll * 180.0f / (float) M_PI) * 10.0f;
    this->tick_counter++;
    if (this->object->alive == false) {
        this->smoke_positions.push_back({this->x, this->y, this->z});
        if (this->smoke_positions.size() > 100) {
            this->smoke_positions.erase(this->smoke_positions.begin());
        }
    }
}