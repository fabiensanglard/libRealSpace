//
//  SCPilot.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "precomp.h"
class PIDController {
public:
    PIDController() : kp(0), ki(0), kd(0), i_limit(0), prev_error(0), integral(0) {};
    PIDController(float kp, float ki, float kd, float i_limit = 0.0f)
        : kp(kp), ki(ki), kd(kd), i_limit(i_limit), prev_error(0), integral(0) {};
    float update(float error, float dt) {
        if (dt <= 0.0f) {
            dt = 1.0f / 30.0f;
        }
        this->integral += error * dt;
        if (this->i_limit > 0.0f) {
            if (this->integral > this->i_limit) {
                this->integral = this->i_limit;
            }
            if (this->integral < -this->i_limit) {
                this->integral = -this->i_limit;
            }
        }
        float derivative = (error - this->prev_error) / dt;
        this->prev_error = error;
        return this->kp * error + this->ki * this->integral + this->kd * derivative;
    };
    void reset() {
        this->prev_error = 0.0f;
        this->integral = 0.0f;
    };

private:
    float kp;
    float ki;
    float kd;
    float i_limit;
    float prev_error;
    float integral;
};

class SCPilot {
    friend class DebugStrike;
private:
    bool alive{true};
    enum TurnState {
        TURN_NONE,
        TURN_LEFT,
        TURN_RIGHT
    };

    float turnRate;
    float calculateMaxTurnRate(float airspeed, float maxG);

    // Boucle laterale en cascade : cap -> inclinaison, inclinaison -> taux de roulis, taux -> manche
    PIDController heading_pid{1.0f, 0.0f, 0.0f, 0.0f};
    PIDController bank_pid{0.6f, 0.0f, 0.02f, 0.0f};
    PIDController roll_rate_pid{0.5f, 0.0f, 0.01f, 0.0f};
    // Boucle verticale en cascade : altitude -> vario, vario -> assiette, assiette -> manche
    PIDController vspeed_pid{1.5f, 0.0f, 0.05f, 0.0f};
    PIDController pitch_pid{1.2f, 0.0f, 0.08f, 0.0f};

    float getDeltaTime();
    float maxBankForG(float maxG);
    void controlThrottle();

    float control_stick_x{0.0f};
    float control_stick_y{0.0f};
    float throttle{0.0f};
    int flap{0};
    int gear{1};
    int spoilers{0};
    int autopilot_state{-1};
    Vector3D autopilot_point{0.0f, 0.0f, 0.0f};
    Vector3D autopilot_velocity{0.0f, 0.0f, 0.0f};
    Vector3D autopilot_world_velocity{0.0f, 0.0f, 0.0f};
    bool autopilot_reached{false};
    float autopilot_hspeed{0.0f};
    void runAutopilot(float dt);
    void publishControls(bool normalized = false);
    enum GuidanceMode {
        GUIDANCE_NONE,
        GUIDANCE_DIRECTION,
        GUIDANCE_PITCH,
        GUIDANCE_MANUAL
    };
    int manual_throttle{-1};
    bool ground_ops{false};
    float ground_pitch_stick{0.0f};
    int ground_throttle{0};
    GuidanceMode guidance_mode{GUIDANCE_NONE};
    Vector3D guidance_direction{0.0f, 0.0f, 0.0f};
    float guidance_pitch{0.0f};
    float guidance_deadzone{0.0f};
    float roll_stick{0.0f};
    float pitch_stick{0.0f};
    int guidance_log_counter{0};
    float guidance_log_h{0.0f};
    float guidance_log_v{0.0f};
    float guidance_log_r{0.0f};
    float guidance_log_t{0.0f};     // consigne d'inclinaison de la branche petits ecarts (banc de DebugStrike)
    float guidance_log_s{0.0f};     // manche de tangage de la meme branche
    void runGuidance(float dt);
    float bankAngle();
    float nosePitch();
    bool guidanceSolution(Vector3D direction, float dt);
    bool combatDecision(float h, float v, float r, float dt);
    bool bankError(float error, float deadzone, float dt);
    bool rollToAngle(float bank, float deadzone, float dt);
    bool pitchToAngle(float pitch, float deadzone, float dt);
    float rollStickFromError(float error, float dt);
    float maxRollRate(float dt);
    float clampPitch(float stick);
public:
    Vector3D target_waypoint{0.0f, 0.0f, 0.0f};   // lecture exterieure seulement (DebugStrike)
    Vector3D steer_waypoint{0.0f, 0.0f, 0.0f};
    bool has_waypoint{false};
    bool turning{false};
    int target_speed{0};
    float target_speed_ms{0.0f};
    int target_climb{0};
    float target_azimut{0.0f};
    float old_target_azimut{0.0f};
    float targetRoll{0.0f};
    bool land{false};
    TurnState turnState;
    SCPlane* plane{nullptr};
    SCMissionActors *actor{nullptr};
    SCPilot();
    ~SCPilot();
    void SetTargetWaypoint(Vector3D waypoint);
    void SetAttitudeError(float heading_error_deg, float pitch_error_deg, float deadband_deg);
    void SetGuidanceDirection(Vector3D direction);
    void SetPitchCommand(float pitch_deg, float deadzone_deg);
    void BeginManual();
    bool CmdRollTo(float bank_deg, float deadzone_deg);
    bool CmdPitchTo(float pitch_deg, float deadzone_deg);
    bool CmdGuidance(Vector3D direction);
    static void AttitudeFromAxes(Vector3D nose, Vector3D span, float &yaw, float &pitch, float &roll);
    void CmdPitchStick(float stick16);
    void CmdRollStick(float stick16);
    void CmdThrottle(int notch);
    void BeginGroundOps();
    void EndGroundOps();
    void CmdGearUp();
    bool GroundOpsActive() const { return ground_ops; }
    void CmdGroundControls(float pitch_stick16, int throttle_notch, int flaps, int gear, int spoilers);
    void CmdKinematic(bool engaged, Vector3D velocity, Vector3D heading_dir, float pitch_deg);
    void CmdPlaceAt(Vector3D position, Vector3D heading_dir, float pitch_deg);
    static float YawOf(Vector3D heading_dir);
    float BankAngle();
    float NosePitch();
    float BearingToRef(Vector3D direction);
    float RollStickValue() { return roll_stick; }
    void ClearGuidance();
    bool attitude_mode{false};
    float attitude_heading_error{0.0f};
    float attitude_pitch_error{0.0f};
    float attitude_deadband{0.0f};
    void FlyTo();
    void Fire(uint16_t weapon_mask, SCMissionActors *target);
    void engageAutopilot(Vector3D target_point, Vector3D desired_velocity);
    void setAutopilotTarget(Vector3D target_point, Vector3D desired_velocity);
    void disengageAutopilot();
    bool autopilotActive() const { return autopilot_state >= 0; }
    bool autopilotReached() const { return autopilot_reached; }
};