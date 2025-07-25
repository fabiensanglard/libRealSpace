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
    float ELEVF_CSTE{0.0f};
    float ROLLFF_CSTE{0.0f};
    float LminDEF{0.0f};
    float LmaxDEF{0.0f};
    /* maximum flap deflection	*/
    float Fmax{0.0f};
    float Smax{0.0f};


    float fuel_rate{0.0f};

    float gefy{0.0f};
    float Splf{0.0f};
    float Spdf{0.0f};

    float fuel_weight{0.0f};
    
    float ipi_AR{0.0f};
    

    float ELEVF{0.0f};
    float ROLLF{0.0f};

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
    float fps_knots{0.0f};
    

    int nocrash;
    int IN_BOX(int llx, int urx, int llz, int urz);
    int report_card(int descent, float roll, int vx, int vz, int wheels);
    
    float fuel_consump(float f, float b);

    uint32_t last_time;
    uint32_t tick_counter;
    uint32_t last_tick;

    /* TRUE if the wheels are down	*/
    short wheels{0};
    /* flap and spoiler settings	*/
    int flaps{0};
    int spoilers{0};
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
    float Lmax{0.0f};
    float Lmin{0.0f};
    float groundlevel{0.0f};
    float s{0.0f};
    float b{0.0f};
    float W{0.0f};
    float Mthrust{0.0f};
    float inverse_mass{0.0f};
    float elevation_speedf{0.0f};
    float azimuth_speedf{0.0f};

    float lift_force {0.0f};
    float drag_force {0.0f};
    float gravity_drag_force {0.0f};
    float lift_drag_force {0.0f};
    float gravity_force {0.0f};

    int tps;
    /* plane velocity */
    float vx{0.0f};
    float vy{0.0f};
    float vz{0.0f};

        /* angle of attack for wing		*/
    float ae{0.0f};
    /* max and min coefficient of lift	*/
    float max_cl{0.0f};
    float min_cl{0.0f};
    /* wing angle tilt due to flaps		*/
    float tilt_factor{0.0f};

    /* last plane position	*/
    float last_px{0.0f};
    float last_py{0.0f};
    float last_pz{0.0f};

    float x{0.0f};
    float y{0.0f};
    float z{0.0f};

    /* plane acceleration	*/
    float ax{0.0f};
    float ay{0.0f};
    float az{0.0f};

    /* the effect of gravity realtive to tps */
    float gravity{0.0f};
    float drag{0.0f};
    float thrust_force{0.0f};
    
    /* coefficients of lift and drag	*/
    float uCl{0.0f};
    float Cl{0.0f};
    float Cd{0.0f};
    float Cdc{0.0f};
    float kl{0.0f};
    /* ground effect, ro/2*Vz*s		*/
    float qs{0.0f};
    /* TRUE if wing is stalling	*/
    short wing_stall;
    /* air density / 2.0, speed of sound	*/
    float ro2{0.0f};
    float sos{0.0f};

    int control_stick_x;
    int control_stick_y;

    float pitch_speed{0.0f};
    float yaw_speed{0.0f};
    float roll_speed{0.0f};

    short on_ground;

    int airspeed;
    float twist{0.0f};
    float azimuthf{0.0f};
    float elevationf{0.0f};

    float rollers{0.0f};
    float rudder{0.0f};
    float elevator{0.0f};
    RSArea *area;
    
    float lift{0.0f};
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