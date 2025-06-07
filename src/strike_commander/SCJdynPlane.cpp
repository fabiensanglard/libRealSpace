#include "precomp.h"
const float GRAVITY = 9.81f; // m/s^2
const float AIR_DENSITY = 1.225f; // kg/m^3
const float DRAG_COEFFICIENT = 0.47f;
const float MAX_LIFT_COEFFICIENT = 2.0f; // Coefficient de portance maximum
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
    
    this->pitch_input = (this->control_stick_y / 100.0f) * deltaTime;
    this->elevation_speedf = radToDegree(this->pitch);
    this->roll_input = (-this->control_stick_x / 100.0f) * deltaTime;
    this->roll_speed = radToDegree(this->roll);
    float rudder = 0.0f;

    float temp = rudder * this->vz - (2.0f) * this->vx;
    int itemp = (int) temp;
    itemp -= this->yaw_input;
    int TDELAY = tps/4;
    if (itemp!=0) {
        if (itemp >= TDELAY || itemp <= -TDELAY)
            itemp /= TDELAY;
        else
            itemp = itemp>0 ? 1 : -1;
    }
	
    this->yaw_input += itemp;
    this->azimuth_speedf = this->yaw_input;
    this->updatePosition();
    this->updateAcceleration();

    this->airspeed = -(int)(this->fps_knots * this->vz);
    this->azimuthf = (this->yaw * 180.0f / (float) M_PI) * 10.0f;
    this->elevationf = (this->pitch * 180.0f / (float) M_PI) * 10.0f;
    this->twist = (this->roll * 180.0f / (float) M_PI) * 10.0f;
    this->tick_counter++;
}
void SCJdynPlane::Render() {
    if (this->object != nullptr) {
        
        Vector3D pos = {
            this->x, this->y, this->z
        };
        Vector3D orientation = {
            tenthOfDegreeToRad(this->azimuthf + 900),
            tenthOfDegreeToRad(this->elevationf),
            -tenthOfDegreeToRad(this->twist)
        };
        std::vector<std::tuple<Vector3D, RSEntity*>> weapons;
        for (auto weaps:this->weaps_load) {
            float decy=0.5f;
            if (weaps == nullptr) {
                continue;  
            }
            if (weaps->hpts_type == 0) {
                continue;
            }
            if (weaps->hpts_type == 4) {
                decy = 0.0f;
            }
            Vector3D position = weaps->position;
            std::vector<Vector3D> path = {
                {0, -2*decy, 0},
                {decy, -decy, 0},
                {-decy,-decy,0},
                {0, -2*decy, -2*decy},
                {decy, -decy, -2*decy},
                {-decy,-decy,-2*decy}
            };

            for (int i = 0; i < weaps->nb_weap; i++) {
                Vector3D weap_pos = {position.z/250+path[i].z, position.y/250 + path[i].y, -position.x/250+path[i].x};
                std::tuple<Vector3D, RSEntity*> weapon = std::make_tuple(weap_pos, weaps->objct);
                weapons.push_back(weapon);
                position.y -= 0.5f;
            }
        }
        Renderer.drawModelWithChilds(this->object->entity, LOD_LEVEL_MAX, pos, orientation, wheel_index, thrust, weapons);
        Renderer.drawLine({this->x, this->y, this->z}, {-this->vx, this->vy, -this->vz}, {1.0f, 1.0f, 0.0f});
        Renderer.drawLine({this->x, this->y, this->z}, {this->ax*100.0f, this->ay*100.0f, this->az*100.0f}, {1.0f, 0.0f, 1.0f});
        BoudingBox *bb = this->object->entity->GetBoudingBpx();
        Vector3D position = {this->x, this->y, this->z};          
        orientation = {
            this->azimuthf/10.0f + 90,
            this->elevationf/10.0f,
            -this->twist/10.0f
        };
        for (auto vertex: this->object->entity->vertices) {
            if (vertex.x == bb->min.x) {
                Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
            }
            if (vertex.x == bb->max.x) {
                Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
            }
            if (vertex.z == bb->min.z) {
                Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
            }
            if (vertex.z == bb->max.z) {
                Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
            }
        }
    }
}
void SCJdynPlane::updatePosition() {
    this->ptw.Identity();
    this->ptw.translateM(this->x, this->y, this->z);

    this->ptw.rotateM(this->yaw, 0, 1, 0);
    this->ptw.rotateM(this->pitch, 1, 0, 0);
    this->ptw.rotateM(this->roll, 0, 0, 1);

    this->ptw.translateM(this->vx, this->vy, this->vz);

    if (yaw_input != 0.0f) {
        this->ptw.rotateM(tenthOfDegreeToRad(yaw_input), 0, 1, 0);    
    }
    if (pitch_input != 0.0f) {
        this->ptw.rotateM(pitch_input, 1, 0, 0);    
    }
    if (roll_input != 0.0f) {
        this->ptw.rotateM(roll_input, 0, 0, 1);    
    }
    

    this->x = this->ptw.v[3][0];
    this->y = this->ptw.v[3][1];
    this->z = this->ptw.v[3][2];

    this->pitch = -asinf(this->ptw.v[2][1]);
    float cos_pitch = cosf(this->pitch);
    if (cos_pitch != 0.0) {
        float sincosas = this->ptw.v[2][0] / cos_pitch;
        if (sincosas > 1.0f) {
            sincosas = 1.0f;
        } else if (sincosas < -1.0f) {
            sincosas = -1;
        }

        this->yaw = asinf(sincosas);
        if (this->ptw.v[2][2] < 0.0f) {
            this->yaw = (float)M_PI - this->yaw;
        }
        if (this->yaw < 0.0f) {
            this->yaw += 2.0f*(float)M_PI;
        }
        if (this->yaw > 2.0f*(float)M_PI) {
            this->yaw -= 2.0f*(float)M_PI;
        }
        this->roll = asinf(this->ptw.v[0][1] / cos_pitch);
        if (this->ptw.v[1][1] < 0.0f) {
            /* if upside down	*/
            this->roll = (float)M_PI - this->roll;
        }
        if (this->roll < 0) {
            this->roll += 2.0f*(float)M_PI;
        }
    }
    
    this->incremental.Identity();

    if (roll_input != 0.0f) {
        this->incremental.rotateM(-roll_input, 0, 0, 1);
    }
    if (pitch_input != 0.0f) {
        this->incremental.rotateM(-pitch_input, 1, 0, 0);
    }
    if (yaw_input != 0.0f) {
        this->incremental.rotateM(-tenthOfDegreeToRad(yaw_input), 0, 1, 0);
    }
    
    this->incremental.translateM(this->vx, this->vy, this->vz);

    this->vx = this->incremental.v[3][0];
    this->vy = this->incremental.v[3][1];
    this->vz = this->incremental.v[3][2];

}
void SCJdynPlane::updateAcceleration() {
    float deltaTime = 1.0f / (float) this->tps;

    float mass = this->object->entity->weight_in_kg * GRAVITY;

    this->gravity = mass;
    

    this->thrust_force = thrust * Mthrust;
    this->lift = this->vz*this->vz * this->object->entity->jdyn->LIFT;
    if (lift > mass) {
        lift = mass;
    }

    this->az = this->vy * lift;
    this->ay = -this->vz * lift;
    this->ax = 0.0f;

    this->az -= this->thrust_force;


    this->ay = this->ay + lift;

    this->az = this->az / mass;
    this->ax = this->ax / mass;
    this->ay = this->ay / mass;


    this->ax -= this->ptw.v[0][1]*GRAVITY;
    this->ay -= this->ptw.v[1][1]*GRAVITY;
    this->az -= this->ptw.v[2][1]*GRAVITY;

    this->ax *= deltaTime;
    this->ay *= deltaTime;
    this->az *= deltaTime;

    this->vz += this->az;
    this->vx += this->ax; 
    this->vy += this->ay;

    this->vz = std::clamp(this->vz, -9.0f, 9.0f);
    this->vx = std::clamp(this->vx, -9.0f, 9.0f);
    this->vy = std::clamp(this->vy, -9.0f, 9.0f);


    float groundlevel = this->area->getY(this->x, this->z);

    if (this->y < groundlevel) {
        this->vy = 0.0f;
        this->y = groundlevel;
        this->on_ground = true;
    }
}