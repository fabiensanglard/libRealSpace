#pragma once
#include "precomp.h"

class SCSimulatedObject {

    float last_px{0.0f};
    float last_py{0.0f};
    float last_pz{0.0f};

    Vector3D SCSimulatedObject::calculate_drag(Vector3D velocity);
    Vector3D SCSimulatedObject::calculate_lift(Vector3D velocity);
    int tps{30};
public:
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float vz{0.0f};

    float azimuthf{0.0f};
    float elevationf{0.0f};

    float weight{1.0f};

    bool alive{true};
    RSEntity *obj;
    MISN_PART *target{nullptr};
    SCSimulatedObject();
    ~SCSimulatedObject();
    void Simulate(int tps);
    void SimulateWithVector(int tps);
    void GetPosition(Vector3D *position);
    void Render();
};