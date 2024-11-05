#pragma once
#include "precomp.h"

class SCSimulatedObject {
    
    
    float ax{0.0f};
    float ay{0.0f};
    float az{0.0f};
    float gravity{0.0f};

    Matrix ptw;
    Matrix incremental;
    int32_t tick_counter{0};

    float last_px{0.0f};
    float last_py{0.0f};
    float last_pz{0.0f};

    float roll_speed{0.0f};
    float elevation_speedf{0.0f};
    float azimuth_speedf{0.0f};

    /* coefficients of lift and drag	*/
    float uCl{0.0f};
    float Cl{0.0f};
    float Cd{0.0f};
    float Cdc{0.0f};
    float kl{0.0f};
    /* ground effect, ro/2*Vz*s		*/
    float qs{0.0f};

    /* air density / 2.0, speed of sound	*/
    float ro2{0.0f};
    float sos{0.0f};

    /* angle of attack for wing		*/
    float ae{0.0f};


     /* max and min coefficient of lift	*/
    float max_cl{1.5f};
    float min_cl{-1.5f};
    /* wing angle tilt due to flaps		*/
    float tilt_factor{.017f};

    float LminDEF{0.0f};
    float LmaxDEF{0.0f};

    /* plane design parameters	*/
    float s{0.0f};

    float Mthrust{0.0f};
    float b{0.0f};
    float Cdp{0.0f};
    float ipi_AR{0.0f};
    float ie_pi_AR{0.0f};
    float Lmax{0.0f};
    float Lmin{0.0f};
    float ELEVF{0.0f};
    float ROLLF{0.0f};
    float last_zdrag{0.0f};
    float mach{0.0f};
    float mratio{0.0f};
    float mcc{0.0f};
    int wing_stall{0};
    short on_ground{0};
    float lift{0.0f};

public:
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float vz{0.0f};

    float azimuthf{0.0f};
    float elevationf{0.0f};
    float twist{0.0f};

    float weight{1.0f};
    float ydrag{0.0f};
    float zdrag{0.0f};

    RSEntity *obj;

    SCSimulatedObject();
    ~SCSimulatedObject();
    void Simulate(int tps);
    void GetPosition(Vector3D *position);
    void Render();
};