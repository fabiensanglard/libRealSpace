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
    this->elevation_speedf = radToDegree(this->pitch_input);
    this->roll_input = (-this->control_stick_x / 100.0f) * deltaTime;
    this->roll_speed = (int) radToDegree(this->roll_input);
    float rudder = 0.0f;

    float temp = rudder * this->vz - (2.0f) * this->vx;
    int itemp = (int) temp;
    itemp -= (int) this->yaw_input;
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
        //Renderer.drawModelWithChilds(this->object->entity, LOD_LEVEL_MAX, pos, orientation, wheel_index, thrust, weapons);
        
        BoudingBox *bb = this->object->entity->GetBoudingBpx();
        Vector3D position = {this->x, this->y, this->z};          
        orientation = {
            this->azimuthf/10.0f,
            this->elevationf/10.0f,
            -this->twist/10.0f
        };
        Renderer.drawLine({this->x, this->y, this->z}, {this->vx*10.0f, this->vy*10.0f, this->vz*10.0f}, {1.0f, 1.0f, 0.0f},orientation);
        Renderer.drawLine({this->x, this->y, this->z}, {this->ax*100.0f, this->ay*100.0f, this->az*100.0f}, {1.0f, 0.0f, 1.0f},orientation);
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
        Renderer.drawLine(pos, up, {0.0f, 1.0f, 0.0f});
        
        Renderer.drawLine(pos, forward, {1.0f, 0.0f, 1.0f});
        
    }
    
    //Renderer.drawLine({this->x, this->y, this->z}, forward, {1.0f, 0.0f, 0.0f});
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
    this->up.Scale(5.0f);
    
    this->forward.x = -cosf(this->pitch) * sinf(this->yaw);
    this->forward.y = sinf(this->pitch);
    this->forward.z = -cosf(this->pitch) * cosf(this->yaw);
    this->forward.Normalize();
    
    this->forward.Scale(5.0f);
    

}
void SCJdynPlane::updateAcceleration() {
    float deltaTime = 1.0f / (float) this->tps;
    this->gravity = GRAVITY;
    float mass = this->object->entity->weight_in_kg * GRAVITY;
    this->inverse_mass = 1.0f /mass;
    
    

    this->thrust_force = 0.01f * thrust * Mthrust;

    this->lift = std::fabs(this->vz) * this->object->entity->jdyn->LIFT * this->object->entity->jdyn->CS3_qqch_lift;
    if (lift > mass) {
        lift = mass;
    }

    this->az = -this->vy*0.6;
    this->ay = this->vz*0.6;
    this->ax = 0.0f;

    this->az -= this->thrust_force;


    this->ay = this->ay;

    this->az = this->az / mass;
    this->ax = this->ax / mass;
    this->ay = this->ay / mass;

    this->thrust_force = this->az;

    this->gravity = GRAVITY * deltaTime;
    this->ax -= this->ptw.v[0][1]*GRAVITY*deltaTime;
    this->ay -= this->ptw.v[1][1]*GRAVITY*deltaTime;
    this->az -= this->ptw.v[2][1]*GRAVITY*deltaTime;

    /*this->ax = 0.0f;
    this->ay = 0.0f;
    this->az = 0.0f;

    this->az = -this->thrust_force / mass;*/


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