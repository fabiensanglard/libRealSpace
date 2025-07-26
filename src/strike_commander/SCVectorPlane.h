#pragma once
#include "precomp.h"

class SCVectorPlane: public SCPlane {
protected:
    Vector3D forward{0.0f,0.0f,-1.0f};
    Vector3D up{0.0f,1.0f,0.0f};
    Vector3D right{1.0f,0.0f,0.0f};
    Vector3D total_force{0.0f,0.0f,0.0f};

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
    void rotateAroundAxis(Vector3D& v, const Vector3D& axis, float angle);

public:
    SCVectorPlane();
    SCVectorPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s, float W,
            float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
            RSArea *area, float x, float y, float z);
    void Render();

};