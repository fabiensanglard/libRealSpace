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
        this->weaps_load.reserve(9);
    this->weaps_load.resize(9);
    this->planeid = 0;
    this->version = 0;
    this->cmd = 0;
    this->type = 0;
    this->alive = 0;
    this->myname[0] = '\0';
    this->status = 0;
    this->won = 0;
    this->lost = 0;
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->twist = 0;
    this->roll_speed = 0;
    this->azimuthf = 0.0f;
    this->elevationf = 0.0f;
    this->elevation_speedf = 0.0f;
    this->azimuth_speedf = 0.0f;
    this->airspeed = 0;
    this->thrust = 0;
    this->mtype = 0;
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
    this->obj = obj;
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
SCJdynPlane::~SCJdynPlane() {

}
void SCJdynPlane::Simulate() {
    int itemp;
    float temp;
    float elevtemp = 0.0f;

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
    

    float groundlevel = this->area->getY(this->x, this->z);
    int DELAY = tps/4;
    float DELAYF = tps/4.0f;

    if (this->status > MEXPLODE) {
        
        this->processInput();

        this->updatePosition();
        
        /****************************************************************/
        /*	check altitude for too high, and landing/takeoff            */
        /****************************************************************/
        /* flame out		*/
        if (this->y > 50000.0)
            this->thrust = 0;
        else if (this->y > groundlevel + 4.0) {
            /* not on ground	*/
            if (!this->takeoff) {
                this->takeoff = true;
                this->landed = false;
            }
            if (this->on_ground) {
                this->Cdp /= 3.0;
                this->min_throttle = 0;
            }
            this->on_ground = FALSE;
        } else if ((int) this->y <= groundlevel+2) {
            /* check for on the ground */
            if (this->object->alive == 0) {
                this->status = MEXPLODE;
            }
            
            if (this->isOnRunWay())
                /* and not on ground before */
                if (!this->on_ground) {
                    int rating;
                    /* increase drag	*/
                    this->Cdp *= 3.0;
                    /* allow reverse engines*/
                    this->min_throttle = -this->max_throttle;
                    rating = report_card(-this->climbspeed, this->twist, (int)(this->vx * this->tps),
                                            (int)(-this->vz * this->fps_knots), this->wheels);
                    /* oops - you crashed	*/
                    if (this->nocrash) {
                        if (rating == -1) {
                            /* set to exploding	*/
                            this->status = MEXPLODE;
                            /* increment count	*/
                            this->lost++;
                        } else {
                            this->fuel += rating << 7;
                            if (this->fuel > (100 << 7))
                                this->fuel = 100 << 7;
                            this->max_throttle = 100;
                        }
                    }
                } else {
                    if (this->nocrash == 0) {
                        this->status = MEXPLODE;
                    }
                }
            //this->ptw.v[3][1] = this->y = groundlevel;
            this->on_ground = TRUE;
            if (this->airspeed < 30 && this->thrust < 20) {
                this->thrust = 0;
                this->vx = 0.0f;
                this->vy = 0.0f;
                this->vz = 0.0f;
                this->airspeed = 0;
                if (this->takeoff) {
                    this->landed = true;
                }
            }
            if (this->status > MEXPLODE) {
                /* kill negative elevation */
                if (this->elevationf < 0) {
                    this->elevationf = 0;
                }
                /* kill any twist	*/
                if (this->twist != 0) {
                    this->twist = 0;
                }
            }
        }
    } /* end not crashing	*/

    /****************************************************************
    /*	compute new velocities, accelerations
    /****************************************************************/

    /* out of gas	*/
    if (this->fuel <= 0) {
        this->thrust = 0;
        this->max_throttle = 0;
        this->min_throttle = 0;
    }

    if (this->status > MEXPLODE) {
        this->computeLift();

        /* engine thrust		*/
        this->computeThrust();

        this->computeDrag();

        this->updateAcceleration();

        this->airspeed = -(int)(this->fps_knots * this->vz);
        this->climbspeed = (short)(this->tps * (this->y - this->last_py));

        this->vx += this->vx_add;
        this->vy += this->vy_add;
        this->vz += this->vz_add;
        if (this->thrust > 0) {
            itemp = this->thrust;
        } else {
            itemp = -this->thrust;
        }
        if (this->tick_counter % (100 * TPS) == 1) {
            this->fuel_rate = fuel_consump(this->Mthrust, this->W);
            this->fuel -= (int)(itemp * this->fuel_rate);
            this->inverse_mass = G_ACC / (this->W + this->fuel / 12800.0f * this->fuel_weight);
        }
        if (this->wheels) {
            this->wheel_anim --;
            if (this->wheel_anim == 0) {
                this->wheel_index ++;
                if (this->wheel_index > 5) {
                    this->wheel_index = 5;
                }
                this->wheel_anim = 10;
            }
        } else {
            if (this->wheel_index>=1) {
                this->wheel_anim --;
                if (this->wheel_anim == 0) {
                    this->wheel_index --;
                    if (this->wheel_index < 1) {
                        this->wheel_index = 0;
                    }
                    this->wheel_anim = 10;
                }
            } else {
                this->wheel_index = 0;
            }
            
        }
    }
    for (auto sim_obj: this->weaps_object) {
        sim_obj->Simulate(this->tps);
    }
    // remove dead objects
    this->weaps_object.erase(std::remove_if(this->weaps_object.begin(), this->weaps_object.end(), [](SCSimulatedObject *obj) {
        return obj->alive == false;
    }), this->weaps_object.end());
    this->object->entity->position.x = this->x;
    this->object->entity->position.y = this->y;
    this->object->entity->position.z = this->z;
    if (this->object->alive == false) {
        this->smoke_positions.push_back({this->x, this->y, this->z});
        if (this->smoke_positions.size() > 100) {
            this->smoke_positions.erase(this->smoke_positions.begin());
        }
    }
    this->tick_counter++;
}
void SCJdynPlane::Render() {
    if (this->object != nullptr) {
        
        Vector3D pos = {
            this->x, this->y, this->z
        };
        Vector3D orientation = {
            this->azimuthf/10.0f + 90,
            this->elevationf/10.0f,
            -this->twist/10.0f
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
    float temp{0.0f};

    if (this->tick_counter % 100 == 0) {
        // rebuild old ptw
        // to keep it normalized

        this->ptw.Identity();
        this->ptw.translateM(this->x, this->y, this->z);

        this->ptw.rotateM(tenthOfDegreeToRad(this->azimuthf), 0, 1, 0);
        this->ptw.rotateM(tenthOfDegreeToRad(this->elevationf), 1, 0, 0);
        this->ptw.rotateM(tenthOfDegreeToRad(this->twist), 0, 0, 1);
    }
    this->ptw.translateM(this->vx/3.2808399f, this->vy/3.2808399f, this->vz/3.2808399f);
    if (round(this->azimuth_speedf) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->azimuth_speedf)), 0, 1, 0);
    if (round(this->elevation_speedf) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->elevation_speedf)), 1, 0, 0);
    if (round(this->roll_speed) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad((float)this->roll_speed), 0, 0, 1);

    /* analyze new ptw	*/
    temp = 0.0f;
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

    this->vx = this->incremental.v[3][0];
    this->vy = this->incremental.v[3][1];
    this->vz = this->incremental.v[3][2];
}
void SCJdynPlane::processInput() {
    int itemp {0};
    float temp {0.0f};
    float elevtemp{0.0f};
    int DELAY = tps/4;
    float DELAYF = tps/4.0f;

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
    if (this->wing_stall > 0) {
        itemp >>= this->wing_stall;
        itemp += mrandom(this->wing_stall << 3);
    }
    this->roll_speed += itemp;
    this->elevator = -1.0f * (this->ELEVF * ((this->control_stick_y + 8) >> 4));
    itemp = (int)(this->elevator * this->vz + this->vy - this->elevation_speedf);
    elevtemp = this->elevator * this->vz + this->vy - this->elevation_speedf;
    if (itemp != 0) {
        if (itemp >= DELAY || itemp <= -DELAY) {
            itemp /= DELAY;
        } else {
            itemp = itemp > 0 ? 1 : -1;
        }
    }
    if (this->wing_stall > 0) {
        itemp >>= this->wing_stall;
        elevtemp = elevtemp / powf(2, this->wing_stall);
        itemp += mrandom(this->wing_stall * 2);
        elevtemp += mrandom(this->wing_stall * 2);
    }
    this->elevation_speedf += itemp;
    float aztemp;
    temp = this->rudder * this->vz - (2.0f) * this->vx;
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

    aztemp = temp;
    /* itemp is now desired-actual		*/
    itemp -= (int)this->azimuth_speedf;
    aztemp -= this->azimuth_speedf;
    if (itemp != 0) {
        if (itemp >= DELAY || itemp <= -DELAY) {
            itemp /= DELAY;
        } else {
            itemp = itemp > 0 ? 1 : -1;
        }
    }
    
    this->azimuth_speedf += itemp;

    if (this->on_ground) {
        /* dont allow negative pitch unless positive elevation	*/
        if (this->elevation_speedf < 0) {
            if (this->elevationf <= 0) {
                this->elevation_speedf = 0;
            }
        } else {
            /* mimic gravitational torque	*/
            elevtemp = -(this->vz * this->tps + this->MIN_LIFT_SPEED) / 4.0f;
            if (elevtemp < 0 && this->elevationf <= 0) {
                elevtemp = 0.0f;
            }
            if (this->elevation_speedf > elevtemp) {
                this->elevation_speedf = elevtemp;
            }
        }
        this->roll_speed = 0;
    }
}
void SCJdynPlane::applyPhysicsForces() {
    
}
void SCJdynPlane::computeLift() {
    int itemp {0};
    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;

    this->max_cl = 1.5f + this->flaps / 62.5f;
    this->min_cl = this->flaps / 62.5f - 1.5f;
    this->tilt_factor = .005f * this->flaps + .017f;

    this->Spdf = .0025f * this->spoilers;
    this->Splf = 1.0f - .005f * this->spoilers;
    
    if (this->y > this->gefy) {
        // ground effect factor
        this->kl = 1.0f;
    } else {
        this->kl = (this->y / this->b);
        if (this->kl > .225f) {
            this->kl = .484f * this->kl + .661f;
        } else {
            this->kl = 2.533f * this->kl + .20f;
        }
    }

    /* compute new accelerations, lift: only if vz is negative	*/
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
        itemp = ((int)this->y) >> 10;
        if (itemp > 74) {
            itemp = 74;
        } else if (itemp < 0) {
            itemp = 0;
        }
        this->ro2 = .5f * ro[itemp];
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

    this->lift = this->Cl * this->qs;
    this->g_limit = FALSE;
    if (this->spoilers > 0) {
        this->lift *= this->Splf;
    }
relift:
    this->ay = this->vz * this->lift;
    this->az = -this->vy * this->lift;
    /* save for wing loading meter	*/
    this->lift = this->ay * this->inverse_mass;
    if (this->lift > this->Lmax) {
        this->lift = .99f * this->Lmax / this->inverse_mass / this->vz;
        this->g_limit = TRUE;
        goto relift;
    } else if (this->lift < this->Lmin) {
        this->lift = .99f * this->Lmin / this->inverse_mass / this->vz;
        this->g_limit = TRUE;
        goto relift;
    }
}

void SCJdynPlane::computeDrag() {
    /* drag - needs to be broken into y/z components		*/
    this->Cd = this->Cdp + this->kl * this->uCl * this->uCl * this->ie_pi_AR + this->Cdc;
    this->zdrag = this->Cd * this->qs;
    this->ydrag = this->vy * this->zdrag;
    this->zdrag = this->vz * this->zdrag;
    /* if vz is positive	*/
    if (this->val) {
        this->ay -= this->ydrag;
        this->az -= this->zdrag;
    } else {
        this->ay += this->ydrag;
        this->az += this->zdrag;
    }
}
void SCJdynPlane::computeGravity() {
    this->gravity = G_ACC / this->tps / this->tps;
}
void SCJdynPlane::computeThrust() {
    this->thrust_force = .01f / this->tps / this->tps * this->thrust * this->Mthrust;
    this->az = this->az - this->thrust_force ;
}
void SCJdynPlane::updateAcceleration() {
    float temp{0.0f};
    /* now convert forces to accelerations (A=F/M)	*/
    this->ax = 0.0f;
    this->ay *= this->inverse_mass;
    this->az *= this->inverse_mass;

    /* add in gravity which is an acceleration	*/
    /**/
    this->ax -= this->ptw.v[0][1] * this->gravity;
    this->ay -= this->ptw.v[1][1] * this->gravity;
    this->az -= this->ptw.v[2][1] * this->gravity;
    /**/
    this->vx += this->ax;
    this->vz += this->az;
    if (this->on_ground && this->status > MEXPLODE) {
        temp = 0.0f;
        float mcos;
        this->vx = 0.0;
        gl_sincos(this->elevationf, &temp, &mcos);
        temp = this->vz * temp / mcos;
        if (this->vy + this->ay < temp) {
            this->ay = temp - this->vy;
        }
    }
    this->vy += this->ay;
}