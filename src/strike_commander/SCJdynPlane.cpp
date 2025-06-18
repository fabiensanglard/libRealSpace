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
    this->acceleration = {0.0f, 0.0f, 0.0f};
    this->velocity = {0.0f, 0.0f, 0.0f};
    this->thrust_force = 0.0f;
    this->lift_force = 0.0f;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->vz = 0.0f;
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
    this->computeGravity();
    this->fps_knots = this->tps * (3600.0f / 6082.0f);
    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;

    this->max_cl = 1.5f + this->flaps / 62.5f;
    this->min_cl = this->flaps / 62.5f - 1.5f;
    this->tilt_factor = .005f * this->flaps + .017f;

    this->Spdf = .0025f * this->spoilers;
    this->Splf = 1.0f - .005f * this->spoilers;
    float deltaTime = 1.0f / (float) this->tps;
    
    float rudder = 0.0f;
    int TDELAY = tps/4;
    int DELAY = tps/4;
    float DELAYF = tps/4.0f;

    int itemp = 0;
    /* tenths of degrees per tick	*/
    this->rollers = (this->ROLLF * ((this->control_stick_x + 8) >> 4));
    /* delta */
    itemp = (int)(this->rollers * this->vz - this->roll_speed);
    if (itemp != 0) {
        if (itemp >= DELAY || itemp <= -DELAY) {
            itemp /= DELAY;
        } else {
            itemp = itemp > 0 ? 1 : -1;
        }
    }
    
    this->roll_speed += itemp;

    this->elevator = -1.0f * (this->ELEVF * ((this->control_stick_y + 8) >> 4));
    itemp = (int)(this->elevator * this->vz + this->vy - this->elevation_speedf);
    float elevtemp = this->elevator * this->vz + this->vy - this->elevation_speedf;
    if (itemp != 0) {
        if (itemp >= DELAY || itemp <= -DELAY) {
            itemp /= DELAY;
        } else {
            itemp = itemp > 0 ? 1 : -1;
        }
    }
    this->elevation_speedf += itemp;
    float temp = this->rudder * this->vz - (2.0f) * this->vx;
    if (this->on_ground) {
        itemp = (int)(16.0f * temp);
        if (itemp < -MAX_TURNRATE || itemp > MAX_TURNRATE) {
            /* clip turn rate	*/
            if (itemp < 0) {
                itemp = -MAX_TURNRATE;
            } else {
                itemp = MAX_TURNRATE;
            }
            /* decrease with velocity */
            if (fabs(this->vz) > 10.0f / this->tps) {
                /* skid effect */
                temp = 0.4f * this->tps * (this->rudder * this->vz - .75f);
                if (temp < -MAX_TURNRATE || temp > MAX_TURNRATE) {
                    temp = (float)itemp;
                }
                itemp -= (int)temp;
            }
        }
        temp = (float)itemp;
    } else {
        /* itemp is desired azimuth speed	*/
        itemp = (int)temp;
    }

    temp = rudder * this->vz - (2.0f) * this->vx;
    itemp = (int) temp;
    itemp -= (int) this->yaw_input;
    
    if (itemp!=0) {
        if (itemp >= TDELAY || itemp <= -TDELAY)
            itemp /= TDELAY;
        else
            itemp = itemp>0 ? 1 : -1;
    }
	
    this->azimuth_speedf += itemp;

    this->updatePosition();
    this->computeGravity();
    this->computeThrust();
    this->computeLift();
    this->computeDrag();
    this->updateAcceleration();

    this->airspeed = -(int)(this->fps_knots * this->vz);
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
        //Renderer.drawModelWithChilds(this->object->entity, LOD_LEVEL_MAX, pos, orientation, wheel_index, thrust, weapons);
        
        BoudingBox *bb = this->object->entity->GetBoudingBpx();
        Vector3D position = {this->x, this->y, this->z};          
        orientation = {
            this->azimuthf/10.0f,
            this->elevationf/10.0f,
            -this->twist/10.0f
        };
        orientation.x += 90.0f;
        int vert_id = 0;
        int wing_id_left = 0;
        std::vector<int> wing_ids;
        for (auto vertex: this->object->entity->vertices) {
            if (vertex.x == bb->min.x) {
                Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
            }
            if (vertex.x == bb->max.x) {
                Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
            }
            if (vertex.z == bb->min.z) {
                wing_ids.push_back(vert_id);
                Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
            }
            if (vertex.z == bb->max.z) {
                wing_ids.push_back(vert_id);
                Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
            }
            vert_id++;
        }
        
        std::vector<int> wing_tr_id;
        std::vector<Point2Df> wing_surface_points;
        for (int i=0; i < this->object->entity->lods[LOD_LEVEL_MAX].numTriangles; i++) {
            int triangle_id = this->object->entity->lods[LOD_LEVEL_MAX].triangleIDs[i];
            Triangle triangle = this->object->entity->triangles[triangle_id];
            
            for (auto id : triangle.ids) {
                if (std::find(wing_ids.begin(), wing_ids.end(), id) != wing_ids.end()) {
                    Vector3D v0,v1,v2;
                    v0 = this->object->entity->vertices[triangle.ids[0]];
                    v1 = this->object->entity->vertices[triangle.ids[1]];
                    v2 = this->object->entity->vertices[triangle.ids[2]];

                    Point2Df p0 = {v0.x, v0.z};
                    if (std::find(wing_surface_points.begin(), wing_surface_points.end(), p0) == wing_surface_points.end()) {
                        wing_surface_points.push_back(p0);
                    }

                    Point2Df p1 = {v1.x, v1.z};
                    if (std::find(wing_surface_points.begin(), wing_surface_points.end(), p1) == wing_surface_points.end()) {
                        wing_surface_points.push_back(p1);
                    }
                    Point2Df p2 = {v2.x, v2.z};
                    if (std::find(wing_surface_points.begin(), wing_surface_points.end(), p2) == wing_surface_points.end()) {
                        wing_surface_points.push_back(p2);
                    }
                    if (std::find(wing_tr_id.begin(), wing_tr_id.end(), triangle_id) == wing_tr_id.end()) {
                        wing_tr_id.push_back(triangle_id);
                    }
                    v0  = this->object->entity->vertices[triangle.ids[0]];
                    v1  = this->object->entity->vertices[triangle.ids[1]];
                    v2  = this->object->entity->vertices[triangle.ids[2]];

                    Renderer.drawPoint(v0, {0.0f,1.0f,1.0f}, position, orientation);
                    Renderer.drawPoint(v1, {0.0f,1.0f,1.0f}, position, orientation);
                    Renderer.drawPoint(v2, {0.0f,1.0f,1.0f}, position, orientation);
                }
            }
        }

        for (int i=0; i < this->object->entity->lods[LOD_LEVEL_MAX].numTriangles; i++) {
            int triangle_id = this->object->entity->lods[LOD_LEVEL_MAX].triangleIDs[i];
            Vector3D v0,v1,v2;
            v0  = this->object->entity->vertices[this->object->entity->triangles[triangle_id].ids[0]];
            v1  = this->object->entity->vertices[this->object->entity->triangles[triangle_id].ids[1]];
            v2  = this->object->entity->vertices[this->object->entity->triangles[triangle_id].ids[2]];
            if (std::find(wing_tr_id.begin(), wing_tr_id.end(), triangle_id) != wing_tr_id.end()) {
                Renderer.drawLine(v0,v1,{1.0f,0.0f,0.0f},orientation, position);
                Renderer.drawLine(v1,v2,{1.0f,0.0f,0.0f},orientation, position);
                Renderer.drawLine(v2,v0,{1.0f,0.0f,0.0f},orientation, position);
            } else {
                Renderer.drawLine(v0,v1,{0.5f,0.5f,0.5f},orientation, position);
                Renderer.drawLine(v1,v2,{0.5f,0.5f,0.5f},orientation, position);
                Renderer.drawLine(v2,v0,{0.5f,0.5f,0.5f},orientation, position);
            }
        }
        if (!wing_surface_points.empty()) {
            float area = 0.0f;
            
            // Compute the convex hull of wing_surface_points using the monotone chain algorithm.
            auto cross = [](const Point2Df &O, const Point2Df &A, const Point2Df &B) -> float {
                return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
            };

            // Tri des points par coordonnées (x, puis y)
            std::sort(wing_surface_points.begin(), wing_surface_points.end(), [](const Point2Df &a, const Point2Df &b) {
                return (a.x == b.x) ? (a.y < b.y) : (a.x < b.x);
            });

            std::vector<Point2Df> hull;
            // Construction de la chaîne inférieure
            for (const auto &p : wing_surface_points) {
                while (hull.size() >= 2 && cross(hull[hull.size()-2], hull.back(), p) <= 0)
                    hull.pop_back();
                hull.push_back(p);
            }
            // Construction de la chaîne supérieure
            for (int i = (int)wing_surface_points.size() - 2, t = hull.size() + 1; i >= 0; i--) {
                while (hull.size() >= t && cross(hull[hull.size()-2], hull.back(), wing_surface_points[i]) <= 0)
                    hull.pop_back();
                hull.push_back(wing_surface_points[i]);
            }
            // Retirer le dernier point car il est identique au premier
            if (!hull.empty())
                hull.pop_back();
            
            
            wing_surface_points = hull;
            size_t n = wing_surface_points.size();
            // Calcul de la surface selon la formule du polygone (formule de shoelace)
            for (size_t i = 0; i < n; i++) {
                Vector3D sp1 = {wing_surface_points[i].x, 0.0f , wing_surface_points[i].y};
                Vector3D sp2 = {wing_surface_points[(i + 1) % n].x, 0.0f , wing_surface_points[(i + 1) % n].y};
                Renderer.drawLine(sp1,sp2,{1.0f,1.0f,0.0f},orientation, position);
            }
            for (size_t i = 0; i < n; i++) {
                size_t next = (i + 1) % n;
                area += wing_surface_points[i].x * wing_surface_points[next].y - wing_surface_points[i].y * wing_surface_points[next].x;
            }
            area = std::fabs(area) / 2.0f;
        }
        Vector3D scaled_forward{forward.x,forward.y,forward.z};
        Vector3D scaled_up{up.x,up.y,up.z};
        scaled_up.Scale(5.0f);
        scaled_forward.Scale(5.0f);
        Renderer.drawLine(pos, scaled_up, {1.0f, 0.0f, 0.0f});
        Renderer.drawLine(pos, scaled_forward, {1.0f, 0.0f, 1.0f});
        
        Vector3D ptw_down = {
            -this->ptw.v[0][1],
            -this->ptw.v[1][1],
            -this->ptw.v[2][1]
        };
        ptw_down.Normalize();
        Vector3D ptw_forward = {
            this->ptw.v[0][2],
            this->ptw.v[1][2],
            this->ptw.v[2][2]
        };
        ptw_forward.Normalize();
        ptw_down.Scale(5.0f);
        ptw_forward.Scale(5.0f);
        Renderer.drawLine(pos, ptw_down, {0.0f, 1.0f, 1.0f});
        Renderer.drawLine(pos, {this->vx*10.0f, this->vy*10.0f, this->vz*10.0f}, {1.0f, 1.0f, 0.0f});
        Renderer.drawLine(pos, ptw_forward, {1.0f, 1.0f, 0.0f});
    }
    
    //Renderer.drawLine({this->x, this->y, this->z}, forward, {1.0f, 0.0f, 0.0f});
}
void SCJdynPlane::updatePosition() {
    this->ptw.Identity();
    this->ptw.translateM(this->x, this->y, this->z);

    this->ptw.rotateM(tenthOfDegreeToRad(this->azimuthf), 0, 1, 0);
    this->ptw.rotateM(tenthOfDegreeToRad(this->elevationf), 1, 0, 0);
    this->ptw.rotateM(tenthOfDegreeToRad(this->twist), 0, 0, 1);

    this->ptw.translateM(this->vx/3.2808399f, this->vy/3.2808399f, this->vz/3.2808399f);
    if (round(this->azimuth_speedf) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->azimuth_speedf)), 0, 1, 0);
    if (round(this->elevation_speedf) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->elevation_speedf)), 1, 0, 0);
    if (round(this->roll_speed) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad((float)this->roll_speed), 0, 0, 1);

    /* analyze new ptw	*/
    float temp = 0.0f;
    this->elevationf = (-asinf(this->ptw.v[2][1]) * 180.0f / (float)M_PI) * 10;

    float ascos = 0.0f;

    temp = cosf(tenthOfDegreeToRad(this->elevationf));

    if (temp != 0.0) {

        float sincosas = this->ptw.v[2][0] / temp;

        if (sincosas > 1) {
            sincosas = 1.0f;
        } else if (sincosas < -1) {
            sincosas = -1;
        }
        this->azimuthf = asinf(sincosas) / (float)M_PI * 1800.0f;
        if (this->ptw.v[2][2] < 0.0) {
            /* if heading into z	*/
            this->azimuthf = 1800 - this->azimuthf;
        }
        if (this->azimuthf < 0) {
            /* then adjust		*/
            this->azimuthf += 3600;
        }

        this->twist = (asinf(this->ptw.v[0][1] / temp) / (float)M_PI) * 1800.0f;
        if (this->ptw.v[1][1] < 0.0) {
            /* if upside down	*/
            this->twist = 1800.0f - this->twist;
        }
        if (this->twist < 0) {
            this->twist += 3600.0f;
        }
    }
    /* save last position	*/
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x = this->ptw.v[3][0];
    this->y = this->ptw.v[3][1];
    this->z = this->ptw.v[3][2];

    /****************************************************************
    /*	perform incremental rotations on velocities
    /****************************************************************/

    this->incremental.Identity();
    if (this->roll_speed)
        this->incremental.rotateM(tenthOfDegreeToRad((float)-this->roll_speed), 0, 0, 1);
    if (this->elevation_speedf)
        this->incremental.rotateM(tenthOfDegreeToRad(-this->elevation_speedf), 1, 0, 0);
    if (this->azimuth_speedf)
        this->incremental.rotateM(tenthOfDegreeToRad(-this->azimuth_speedf), 0, 1, 0);
    this->incremental.translateM(this->vx, this->vy, this->vz);

    /*this->vx = this->incremental.v[3][0];
    this->vy = this->incremental.v[3][1];
    this->vz = this->incremental.v[3][2];*/


    /*this->yaw = tenthOfDegreeToRad(this->azimuthf / 10.0f);
    this->pitch = tenthOfDegreeToRad(this->elevationf / 10.0f);
    this->roll = tenthOfDegreeToRad(this->twist / 10.0f);
    // Calculer le vecteur UP à partir des angles roll, pitch et yaw
    float cosYaw   = cosf(this->yaw);
    float sinYaw   = sinf(this->yaw);
    float cosPitch = cosf(this->pitch);
    float sinPitch = sinf(this->pitch);
    float cosRoll  = cosf(this->roll);
    float sinRoll  = sinf(this->roll);

    
    this->up.x = -cosYaw * sinRoll + sinYaw * cosRoll * sinPitch;
    this->up.y = cosRoll * cosPitch;
    this->up.z = sinYaw * sinRoll + cosYaw * cosRoll * sinPitch;

    this->up.Normalize();
    // Calcul du vecteur FORWARD à partir des angles roll, pitch et yaw
    this->forward.x = -sinYaw * cosPitch;
    this->forward.y = sinPitch;
    this->forward.z = -cosYaw * cosPitch;

    this->forward.Normalize();
    */
    
}
void SCJdynPlane::applyPhysicsForces() {
    float deltaTime = 1.0f / static_cast<float>(this->tps);
    float mass = this->object->entity->weight_in_kg;
    
    this->gravity_force = mass * GRAVITY;
    this->gravity = this->gravity_force;
    this->thrust_force = 0.01f * thrust * Mthrust;
    
    
    this->lift_vector = {0.0f, 1.0f, 0.0f};
    if (airspeed_in_ms > 0.1f) { // Éviter division par zéro
        // Calcul de la portance: L = 0.5 * ρ * V² * S * CL
        float dynamicPressure = 0.5f * AIR_DENSITY * airspeed_in_ms * airspeed_in_ms;
        
        // Coefficient de portance basé sur l'angle d'attaque
        float liftCoeff = LIFT_COEFFICIENT * sinf(2.0f * this->pitch);
        liftCoeff = std::clamp(liftCoeff, 0.0f, MAX_LIFT_COEFFICIENT);
        
        this->lift_force = dynamicPressure * this->object->entity->wing_area * liftCoeff;
        this->lift = this->lift_force;
        this->drag_force = dynamicPressure * this->object->entity->wing_area * DRAG_COEFFICIENT;
        this->drag = this->drag_force;
    }
}
void SCJdynPlane::computeLift() {
    /* compute new accelerations, lift: only if vz is negative	*/
    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;

    this->max_cl = 1.5f + this->flaps / 62.5f;
    this->min_cl = this->flaps / 62.5f - 1.5f;
    this->tilt_factor = .005f * this->flaps + .017f;

    this->Spdf = .0025f * this->spoilers;
    this->Splf = 1.0f - .005f * this->spoilers;
    this->kl = 1.0f;
    this->val = (this->vz >= 0.0);
    if (!this->val) {
        this->ae = this->vy / this->vz + this->tilt_factor;
        this->Cl = this->uCl = this->ae / (.17f + this->kl * this->ipi_AR);
        /* check for positive stall	*/
        if (this->Cl > this->max_cl) {
            this->Cl = 3.0f * this->max_cl - 2.0f * this->Cl;
            this->wing_stall = 1;
            if (this->Cl < 0.0f) {
                this->wing_stall += 1 - (int)(this->Cl / this->max_cl);
                this->Cl = 0.0f;
            }
            if (this->uCl > 5.0f) {
                this->uCl = 5.0f;
            }
        } else if (this->Cl < this->min_cl) {
            /* check for negative stall	*/
            this->Cl = 3.0f * this->min_cl - 2.0f * this->Cl;
            this->wing_stall = 1;
            if (this->Cl > 0.0f) {
                this->wing_stall += 1 - (int)(this->Cl / this->min_cl);
                this->Cl = 0.0f;
            }
            if (this->uCl < -5.0f) {
                this->uCl = -5.0f;
            }
        } else {
            this->wing_stall = FALSE;
        }
    } else {
        this->Cl = this->uCl = 0.0f;
        this->wing_stall = this->on_ground ? 0 : (int)this->vz;
        this->ae = 0.0f;
    }
    if (this->wing_stall > 64) {
        this->wing_stall = 64;
    }
    if ((this->tick_counter & 1) == 0) {
        /* only do on even ticks	*/
        /* compute speed of sound	*/
        if (this->y <= 36000.0f) {
            this->sos = -1116.0f / this->tps + (1116.0f - 968.0f) / this->tps / 36000.0f * this->y;
        } else {
            this->sos = -968.0f / this->tps;
        }
        int ro2_index = ((int)this->y) >> 10;
        if (ro2_index > 74) {
            ro2_index = 74;
        } else if (ro2_index < 0) {
            ro2_index = 0;
        }
        this->ro2 = .5f * ro[ro2_index];
        if (this->Cl < .2) {
            this->mcc = .7166666f + .1666667f * this->Cl;
        } else {
            this->mcc = .7833333f - .1666667f * this->Cl;
        }
        /* and current mach number	*/
        this->mach = this->vz / this->sos;
        this->mratio = this->mach / this->mcc;
        if (this->mratio < 1.034f) {
            this->Cdc = 0.0f;
        } else {
            this->Cdc = .082f * this->mratio - 0.084788f;
            if (this->Cdc > .03f)
                this->Cdc = .03f;
        }
        if (this->spoilers > 0.0f) {
            this->Cdc += this->Spdf;
        }
    }

    /* assume V approx = vz	*/
    this->qs = this->ro2 * this->vz * this->s;

    float tmp_lift = this->Cl * this->qs;
    this->g_limit = FALSE;
    if (this->spoilers > 0) {
        tmp_lift *= this->Splf;
    }
    relift:
    this->lift_force = this->vz * tmp_lift; 
    this->lift_drag_force = -this->vy * tmp_lift;
    tmp_lift = this->lift_force * this->inverse_mass;
    /*while (tmp_lift > this->Lmax || tmp_lift < this->Lmin) {
        if (tmp_lift > this->Lmax) {
            tmp_lift = .99f * this->Lmax / this->inverse_mass / this->vz;
        } else {
            tmp_lift = .99f * this->Lmin / this->inverse_mass / this->vz;
        }
        this->g_limit = TRUE;
        this->lift_force = this->vz * tmp_lift; 
        this->lift_drag_force = -this->vy * tmp_lift;
    }*/
    tmp_lift = this->lift_force * this->inverse_mass;
    if (tmp_lift > this->Lmax) {
        tmp_lift = .99f * this->Lmax / this->inverse_mass / this->vz;
        this->g_limit = TRUE;
        goto relift;
    } else if (tmp_lift < this->Lmin) {
        tmp_lift = .99f * this->Lmin / this->inverse_mass / this->vz;
        this->g_limit = TRUE;
        goto relift;
    }
    this->lift = this->lift_force;
}

void SCJdynPlane::computeDrag() {
    this->Cd = this->Cdp + this->kl * this->uCl * this->uCl * this->ie_pi_AR + this->Cdc;
    this->gravity_drag_force = this->vy * this->zdrag;
    this->drag_force = this->Cd * this->qs * this->vz;
    this->drag = this->drag_force;
}
void SCJdynPlane::computeGravity() {
    this->gravity = G_ACC / this->tps / this->tps;
    this->gravity_force = this->object->entity->weight_in_kg * this->gravity;
}
void SCJdynPlane::computeThrust() {
    this->thrust_force = .01f / this->tps / this->tps * this->thrust * this->Mthrust;
}
void SCJdynPlane::updateAcceleration() {
    this->ax = 0.0f;
    this->ay = this->lift_force;
    this->az = this->lift_drag_force;

    this->az = this->az - this->thrust_force;
    if (this->vz < 0.0f) {
        this->az += this->drag_force;
        this->ay += this->gravity_drag_force;
    } else {
        this->az -= this->drag_force;
        this->ay -= this->gravity_drag_force;
    }

    
    this->ay *= this->inverse_mass;
    this->az *= this->inverse_mass;

    this->ax -= this->ptw.v[0][1] * this->gravity;
    this->ay -= this->ptw.v[1][1] * this->gravity;
    this->az -= this->ptw.v[2][1] * this->gravity;

    this->vx += this->ax;
    this->vz += this->az;
    this->vy += this->ay;

    if (this->y <= this->area->getY(this->x, this->z)) {
        /* on ground	*/
        this->on_ground = TRUE;
        if (this->ay < 0.0f) {
            this->vy = 0.0f;
        }
        this->y = this->area->getY(this->x, this->z);
    }
}