#pragma once
#include "precomp.h"

class SCMission;

class MissionUpdateEvent: public EventMessage {
public:
    int32_t tick{0};
    float delta_time{0.0f};
    std::string message;
    std::string objective;
    uint8_t area_id{0};
    uint8_t mission_update_id{0};
    SCMission *mission{nullptr};
};
// Publie par SCMission a une cadence fixe (~25 Hz, la cadence de decision
// d'origine, cf. analysis/AI_SYSTEM.md §6) plutot qu'a chaque frame reelle.
// Chaque SCMissionActors qui possede un profil GOAL declenche sur reception
// sa propre boucle GOAL locale (voir SCMissionActors::onAIRefresh).
class AIRefreshEvent: public EventMessage {
public:
    SCMission *mission{nullptr};
};
class MusicUpdateEvent: public EventMessage {
public:
    SCMission *mission{nullptr};
};
class MissionEventActorDestroyed: public EventMessage {
public:
    SCMissionActors *actor{nullptr};
    SCMission *mission{nullptr};
};
class MissionEventActorHit: public EventMessage {
public:
    SCMissionActors *attacker{nullptr};
    SCMissionActors *target{nullptr};
    SCSimulatedObject *weapon{nullptr};
    SCMission *mission{nullptr};
};
class MissionEventSceneActivated: public EventMessage {
public:
    MISN_SCEN *scene{nullptr};
    SCMission *mission{nullptr};
};
class PlaneControlEvent: public EventMessage {
public:
    SCPlane *plane{nullptr};
    float throttle{0.0f};
    float control_stick_x{0.0f};
    float control_stick_y{0.0f};
    float rudder{0.0f};
    int flaps{0};
    int spoilers{0};
    int wheel{0};
    bool normalized_stick{false};
};
class PlaneFireEvent: public EventMessage {
public:
    SCPlane *plane{nullptr};
    int hardpoint{0};
    SCMissionActors *target{nullptr};
    SCMission *mission{nullptr};
};
class PlaneWreckEvent: public EventMessage {
public:
    SCPlane *plane{nullptr};
};
class PlaneAttitudeEvent: public EventMessage {
public:
    SCPlane *plane{nullptr};
    float yaw{0.0f};
    float pitch{0.0f};
    float roll{0.0f};
};
class PlaneKinematicEvent: public EventMessage {
public:
    SCPlane *plane{nullptr};
    bool engaged{false};
    Vector3D velocity{0.0f, 0.0f, 0.0f};
    float yaw{0.0f};
    float pitch{0.0f};
    float roll{0.0f};
    bool set_position{false};
    Vector3D position{0.0f, 0.0f, 0.0f};
};