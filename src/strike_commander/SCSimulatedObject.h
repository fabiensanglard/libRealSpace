#pragma once
#include "precomp.h"
#include "../realspace/RSAirdens.h"

class SCMissionActors;
class SCMission;
class SCSimulatedObject {

protected:

    float last_px{0.0f};
    float last_py{0.0f};
    float last_pz{0.0f};

    int tps{30};
    std::vector<Vector3D> smoke_positions;
    int run_iterations{0};
    SCRenderer &Renderer = SCRenderer::getInstance();
    RSMixer &Mixer = RSMixer::getInstance();
    SCSmokeSet &SmokeSet = SCSmokeSet::getInstance();
    float missile_roll{0.0f};
public:
    bool is_simulated{false};
    bool guidance{true};
    bool no_gravity{true};
    bool no_thrust{false};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float vz{0.0f};

    float azimuthf{0.0f};
    float elevationf{0.0f};

    float weight{1.0f};
    float distance{0.0f};
    bool alive{true};
    RSEntity *obj;
    SCMissionActors *target{nullptr};
    SCSimulatedObject *decoy_target{nullptr};
    SCMissionActors *shooter{nullptr};
    SCMission *mission{nullptr};
    SCSimulatedObject();
    ~SCSimulatedObject();

    void GetPosition(Vector3D *position);
    virtual void Simulate(int tps);
    virtual void Render();
    virtual std::tuple<Vector3D, Vector3D> ComputeTrajectory(int tps);
    bool CheckCollision(SCMissionActors *entity);
};

class GunSimulatedObject : public SCSimulatedObject {

public:
    void Simulate(int tps);
    void Render();
    std::tuple<Vector3D, Vector3D> ComputeTrajectory(int tps);
    std::tuple<Vector3D, Vector3D> ComputeTrajectoryUntilGround(int tps);
};

class DumbSimulatedObject : public SCSimulatedObject {
    int fps{0};
    int current_frame{0};
public:
    void Simulate(int tps);
    void Render();
};

class AgravSimulatedObject : public DumbSimulatedObject {
    RSAirdens airdens;

public:
    float lifetime_left{0.0f};
    void Simulate(int tps);
    int seekerSignature(int aspec);
};

class GuidedBombSimulatedObject : public SCSimulatedObject {
    Vector3D nose{0.0f, 0.0f, -1.0f};
    Vector3D up{0.0f, 1.0f, 0.0f};
    bool launch_speed_set{false};
    float guided_speed{0.0f};
    RSAirdens airdens;

public:
    void Simulate(int tps);
    std::tuple<Vector3D, Vector3D> ComputeTrajectory(int tps);
};

class EjectionSeatSimulatedObject : public SCSimulatedObject {
    RSEntity *ejection_seat_obj{nullptr};
    RSEntity *pilot_obj{nullptr};
    float ejection_seat_velocity{50.0f};
public:
    EjectionSeatSimulatedObject();
    ~EjectionSeatSimulatedObject();
    void Simulate(int tps);
    void Render();
};

class EjectedPilotSimulatedObject : public SCSimulatedObject {
    RSEntity *ejected_pilot_obj{nullptr};
public:
    EjectedPilotSimulatedObject();
    ~EjectedPilotSimulatedObject();
    void Simulate(int tps);
    void Render();
};