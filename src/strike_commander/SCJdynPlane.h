#pragma once
int mrandom(int maxr);
void gl_sincos(float a, float *b, float *c);
class SCJdynPlane : public SCPlane {
private:

    float pitch_input{0.0f};
    float roll_input{0.0f};
    float yaw_input{0.0f};

    Vector3D up {0.0f, 0.0f, 0.0f};
    Vector3D forward {0.0f, 0.0f, 0.0f};

    Vector3D thrust_vector {0.0f, 0.0f, 0.0f};
    Vector3D lift_vector {0.0f, 0.0f, 0.0f};
    Vector3D gravity_vector {0.0f, 0.0f, 0.0f};
    Vector3D drag_vector {0.0f, 0.0f, 0.0f};


    Vector3D acceleration {0.0f, 0.0f, 0.0f};
    Vector3D velocity {0.0f, 0.0f, 0.0f};
    
    float lift_force {0.0f};
    float drag_force {0.0f};
    float gravity_drag_force {0.0f};
    float lift_drag_force {0.0f};
    float gravity_force {0.0f};

    float airspeed_in_ms {0.0f};
    void updatePosition();
    void updateAcceleration();
    void applyPhysicsForces();

    void computeLift();
    void computeDrag();
    void computeGravity();
    void computeThrust();
    void processInput();
public:
    SCJdynPlane();
    SCJdynPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
            float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
            RSArea *area, float x, float y, float z);
    ~SCJdynPlane();
    void Simulate();
    void Render();
};