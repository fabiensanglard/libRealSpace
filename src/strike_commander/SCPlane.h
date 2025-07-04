//
//  SCPlane.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 31/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#pragma once
#ifndef __libRealSpace__SCPlane__
#define __libRealSpace__SCPlane__

#define NAME_LENGTH 8
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static float ro[75] = {
    .0023081f,                                                  /* 1000 feet	*/
    .0022409f,  .0021752f,  .0021110f,  .0020482f,              /* 5000 feet	*/
    .0019869f,  .0019270f,  .0018685f,  .0018113f,  .0017556f,  /* 10000 feet	*/
    .0017011f,  .0016480f,  .0015961f,  .0015455f,  .0014962f,  /* 15000 feet	*/
    .0014480f,  .0014011f,  .0013553f,  .0013107f,  .0012673f,  /* 20000 feet	*/
    .0012249f,  .0011836f,  .0011435f,  .0011043f,  .0010663f,  /* 25000 feet	*/
    .0010292f,  .00099311f, .00095801f, .00092387f, .00089068f, /* 30000 feet	*/
    .00085841f, .00082704f, .00079656f, .00076696f, .00073820f, /* 35000 feet	*/
    .00071028f, .00067800f, .00064629f, .00061608f, .00058727f, /* 40000 feet	*/
    .00055982f, .00053365f, .00050871f, .00048493f, .00046227f, /* 45000 feet	*/
    .00044067f, .00042008f, .00040045f, .00038175f, .00036391f, /* 50000 feet	*/
    .00034692f, .00033072f, .00031527f, .00030055f, .00028652f, /* 55000 feet	*/
    .00027314f, .00026039f, .00024824f, .00023665f, .00022561f, /* 60000 feet	*/
    .00021508f, .00020505f, .00019548f, .00018336f, .00017767f, /* 65000 feet	*/
    .00016938f, .00016148f, .00015395f, .00014678f, .00013993f, /* 70000 feet	*/
    .00013341f, .00012719f, .00012126f, .00011561f, .00011022f, /* 75000 feet	*/
};

#define MEXPLODE 0
#define TPS 20
#define MAX_TURNRATE (600 / TPS)
#define DELAY_FACTOR 4
#define DDELAY TPS / 4
#define DDELAYF TPS / 4.0f

#define G_ACC 32.17f

#define COORD_SCALE 1.0f

class SCMissionActors;
class SCMission;

struct SCWeaponLoadoutHardPoint {
    RSEntity *objct;
    int nb_weap;
    int hpts_type;
    Point2D hud_pos;
    Vector3D position;
    std::string name;
};

class SCPlane {

protected:
    short alive;
    int thrust;
    float ELEVF_CSTE;
    float ROLLFF_CSTE;
    float LminDEF;
    float LmaxDEF;
    /* maximum flap deflection	*/
    float Fmax;
    float Smax;


    /* fuel consumption rate	*/
    float fuel_rate;
    /* maximum height for ground effect	*/
    float gefy;

    /* spoiler factors on lift and drag	*/
    float Splf;
    float Spdf;

    /* mach #, crest critical #, ratio	*/
    

    /* weight of fuel			*/
    float fuel_weight;
    /* 1.0 / mass of plane			*/
    
    /* plane design parameters	*/
    
    
    
    float ipi_AR;
    

    float ELEVF;
    float ROLLF;

    /* TRUE if wing g-limit is hit	*/
    short g_limit;
    /* fuel (0 - 12800)		*/
    int fuel;
    /* upper limit on engines	*/
    short max_throttle;
    /* lower limit on engines	*/
    short min_throttle;

    /* minimum lift-up speed fps	*/
    short MIN_LIFT_SPEED;
    short climbspeed;

    /* fps to knots conversion factor */
    float fps_knots;
    RSArea *area;

    int nocrash;
    int IN_BOX(int llx, int urx, int llz, int urz);
    int report_card(int descent, float roll, int vx, int vz, int wheels);
    
    float fuel_consump(float f, float b);

    uint32_t last_time;
    uint32_t tick_counter;
    uint32_t last_tick;

    /* TRUE if the wheels are down	*/
    short wheels;
    /* flap and spoiler settings	*/
    int flaps;
    int spoilers;
    int wheel_index{0};
    int wheel_anim{10};

    SCSmokeSet *smoke_set{nullptr};
    std::vector<Vector3D> smoke_positions;
    

    Vector3D thrust_vector {0.0f, 0.0f, 0.0f};
    Vector3D lift_vector {0.0f, 0.0f, 0.0f};
    Vector3D gravity_vector {0.0f, 0.0f, 0.0f};
    Vector3D drag_vector {0.0f, 0.0f, 0.0f};


    Vector3D acceleration {0.0f, 0.0f, 0.0f};
    


    float airspeed_in_ms {0.0f};
    virtual void updatePosition();
    virtual void updateAcceleration();
    virtual void updateVelocity();
    virtual void updateForces();
    virtual void updateSpeedOfSound();
    virtual void checkStatus();
    virtual void computeLift();
    virtual void computeDrag();
    virtual void computeGravity();
    virtual void computeThrust();
    virtual void processInput();
    virtual void updatePlaneStatus();
public:
    unsigned int status;
    float mach{0.0f};
    float mcc{0.0f};
    float mratio{0.0f};
    float ie_pi_AR{0.0f};
    float g_load{0.0f};
    float Cdp{0.0f};
    float Lmax;
    float Lmin;
    float groundlevel{0.0f};
    float s;
    float b;
    float W;
    float Mthrust;
    float inverse_mass;
    float elevation_speedf;
    float azimuth_speedf;

    float lift_force {0.0f};
    float drag_force {0.0f};
    float gravity_drag_force {0.0f};
    float lift_drag_force {0.0f};
    float gravity_force {0.0f};

    int tps;
    /* plane velocity */
    float vx;
    float vy;
    float vz;

        /* angle of attack for wing		*/
    float ae;
    /* max and min coefficient of lift	*/
    float max_cl;
    float min_cl;
    /* wing angle tilt due to flaps		*/
    float tilt_factor;

    /* last plane position	*/
    float last_px{0.0f};
    float last_py{0.0f};
    float last_pz{0.0f};

    float x;
    float y;
    float z;

    /* plane acceleration	*/
    float ax;
    float ay;
    float az;

    /* the effect of gravity realtive to tps */
    float gravity;
    float drag;
    float thrust_force;
    
    /* coefficients of lift and drag	*/
    float uCl;
    float Cl;
    float Cd;
    float Cdc;
    float kl;
    /* ground effect, ro/2*Vz*s		*/
    float qs;
    /* TRUE if wing is stalling	*/
    short wing_stall;
    /* air density / 2.0, speed of sound	*/
    float ro2;
    float sos;

    int control_stick_x;
    int control_stick_y;

    float pitch_speed{0.0f};
    float yaw_speed{0.0f};
    float roll_speed{0.0f};

    short on_ground;

    int airspeed;
    float twist;
    float azimuthf;
    float elevationf;

    float rollers;
    float rudder;
    float elevator;

    float lift;
    std::vector <SCWeaponLoadoutHardPoint *> weaps_load;
    std::vector <SCSimulatedObject *> weaps_object;
    int selected_weapon{0};
    MISN_PART *object;
    SCMissionActors *pilot;
    bool takeoff{false};
    bool landed{false};
    float pitch{0.0f};
    float roll{0.0f};
    float yaw{0.0f};
    Vector3D velocity {0.0f, 0.0f, 0.0f};
    Vector3D position {0.0f, 0.0f, 0.0f};
    Vector3D forward = {0, 0, -1};
    bool simple_simulation{true};
    /* my ptw matrix, temp matrix	*/
    Matrix ptw;
    Matrix incremental;
    bool disable_azimuth{false};
    SCPlane();
    SCPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
            float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
            RSArea *area, float x, float y, float z);
    ~SCPlane();
    virtual void init();
    int isOnRunWay();
    void SetThrottle(int throttle);
    int GetThrottle();
    void SetFlaps();
    void SetSpoilers();
    void SetWheel();
    short GetWheel();
    int GetFlaps();
    int GetSpoilers();

    void SetControlStick(int x, int y);
    virtual void Simulate();
    virtual void getPosition(Point3D *position);
    virtual void Render();
    virtual void RenderSimulatedObject();
    virtual void RenderSmoke(); 
    virtual void Shoot(int weapon_hard_point_id, SCMissionActors *target, SCMission *mission);
    virtual void InitLoadout();
    void renderPlaneLined();
};

#endif