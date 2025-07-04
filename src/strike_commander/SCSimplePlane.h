#pragma once

class SCSimplePlane : public SCPlane {
protected:
    void updateAcceleration();
    void updateVelocity();
    void updateForces();
    void computeLift();
    void computeDrag();
    void computeGravity();
    void computeThrust();
    void processInput();
    void updatePosition();
    void updatePlaneStatus();
public:
    SCSimplePlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
            float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
            RSArea *area, float x, float y, float z);
    SCSimplePlane();
    void Render();
};
