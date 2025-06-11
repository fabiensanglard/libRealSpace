#pragma once

class SCJdynPlane : public SCPlane {
private:

    float pitch_input{0.0f};
    float roll_input{0.0f};
    float yaw_input{0.0f};

    Vector3D up {0.0f, 0.0f, 0.0f};
    Vector3D forward {0.0f, 0.0f, 0.0f};
    void updatePosition();
    void updateAcceleration();

public:
    SCJdynPlane();
    SCJdynPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
            float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
            RSArea *area, float x, float y, float z);
    ~SCJdynPlane();
    void Simulate();
    void Render();
};