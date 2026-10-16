#pragma once
#include "precomp.h"

class SCMission;
class SCAIBrain;


class SCMissionActors {
public:
    bool talkative{true};
    int score{0};
    int plane_down{0};
    int ground_down{0};
    std::string actor_name;
    uint8_t actor_id{0};
    RSProf *profile{nullptr};
    std::vector<PROG> prog;
    std::vector<PROG> on_mission_start;
    std::vector<PROG> on_is_activated;
    std::vector<PROG> on_is_destroyed;
    std::vector<PROG> on_update;
    std::vector<PROG> override_progs;
    std::vector<SCSimulatedObject *> weapons_shooted;
    MISN_PART *object{nullptr};
    SCPlane *plane{nullptr};
    SCPilot *pilot{nullptr};
    SCAIBrain *brain{nullptr};
    SCMission *mission{nullptr};
    SCMissionActors *target{nullptr};
    SCMissionActors *attacker{nullptr};
    SCSimulatedObject *weapon_shooted_at_me{nullptr};
    prog_op current_objective;
    bool is_active{false};
    bool is_hidden{true};
    bool on_nav_map{false};   // bit 4 de +0x39 (opcodes 0xB6 / 0xB7) : trace sur la carte de navigation
    bool taken_off{false};
    bool is_destroyed{false};
    bool prog_executed{false};
    int health{0};
    int team_id{0};
    static constexpr int NO_TARGET = -1;
    int current_target{NO_TARGET};
    bool current_command_executed{false};
    prog_op current_command{prog_op::OP_NOOP};
    uint8_t current_command_arg;
    uint8_t current_command_arg2{0xFF};
    Vector3D follow_slot{0.0f, 0.0f, 0.0f};   // poste en formation (cote, avant, haut), entite+0x14A
    bool follow_slot_set{false};
    Vector3D aiming_vector{0.0f, 0.0f, 0.0f};
    std::vector<uint8_t> executed_opcodes;
    float wait_timer{0.0f};   // objet de mission +0x3A (opcode WAIT)
    virtual bool deactivate(uint8_t arg);
    virtual bool setMessage(uint8_t arg);
    virtual bool ifTargetInSameArea(uint8_t arg);
    virtual bool respondToRadioMessage(int message_id, SCMission *mission, SCMissionActors *sender=nullptr);
    virtual bool activateTarget(uint8_t arg);
    // Pose l'objectif courant (current_command/current_command_arg) sans
    // l'executer : le script PROG (SCProg) appelle uniquement ceci, c'est
    // SCAIBrain::executeGoalAction() qui execute reellement la commande persistee, a
    // la cadence GOAL/AIRefresh. Ne remet current_command_executed a false
    // que lors d'une VRAIE transition (commande ou argument different) —
    // sinon, comme le script repasse par cet appel a chaque frame tant que
    // l'objectif est en cours, on ecraserait en permanence le resultat
    // calcule par le dernier passage du GOAL loop.
    // Virtuel : le script de mission du joueur (SCMissionActorsPlayer) est
    // une liste continue d'objectifs executee UNE SEULE FOIS au chargement
    // (script d'initialisation, pas un etat re-evalue en continu comme pour
    // l'IA) — sa surcharge execute donc directement la commande ici, au
    // moment ou elle est posee.
    virtual bool setObjective(prog_op command, uint8_t arg);
    virtual int getDistanceToTarget(uint8_t arg);
    virtual int getDistanceToSpot(uint8_t arg);
    virtual void hasBeenHit(SCSimulatedObject *weapon, SCMissionActors *attacker);
    SCMissionActors();
    ~SCMissionActors();
private:
    
    AssetManager &Assets = AssetManager::getInstance();
    MessageBus &messageBus = MessageBus::getInstance();
    void onEvent(const EventMessage &event);
    void onGettingHit(const MissionEventActorHit &event);
    // cadence IA fixe (AIRefreshEvent) : ordre radio accepte puis SCAIBrain::tick (AIEntity_MasterTick_5ACC)
    void onAIRefresh(const AIRefreshEvent &event);
    MessageBus::SubscriptionId subscription_id{-1};

protected:
    virtual void onMissionUpdate(const MissionUpdateEvent &event);
};

class SCMissionActorsPlayer : public SCMissionActors {
public:
    bool takeOff(uint8_t arg);
    bool land(uint8_t arg);
    bool flyToWaypoint(uint8_t arg);
    bool flyToArea(uint8_t arg);
    bool setMessage(uint8_t arg) override;
    void hasBeenHit(SCSimulatedObject *weapon, SCMissionActors *attacker) override;
    bool setObjective(prog_op command, uint8_t arg) override;
};

class SCMissionActorsSwpn : public SCMissionActors {
    int frames{0};
    float fire_delay{0.0f};
    SCMissionActors *findTarget();
    void fireMissile();
    void fireGun();
    void updateFire();

public:
    void onMissionUpdate(const MissionUpdateEvent &event) override;
};

class SCMissionActorsStrikeBase : public SCMissionActors {
public:
    bool setMessage(uint8_t arg) override;
};