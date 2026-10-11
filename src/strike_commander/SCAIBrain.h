#pragma once
#include "precomp.h"

class SCMissionActors;
class SCSimulatedObject;

enum ReactionLevel : uint8_t {
    REACT_NONE = 0,
    REACT_ENGAGED = 1,
    REACT_MISSILE = 2,
    REACT_COLLISION = 3,
    REACT_GROUND_AVOID = 4,
    REACT_STALL_RECOVERY = 5
};

struct CombatContext {
    SCMissionActors *target{nullptr};
    bool aircraft{true};
    float my_speed{0.0f};
    float cruise{0.0f};
    float min_speed{0.0f};
    float ias{0.0f};
    Vector3D D{0.0f, 0.0f, 0.0f};
    Vector3D future_rel{0.0f, 0.0f, 0.0f};
    float dist{0.0f};
    float nose_angle{0.0f};
    float future_angle{0.0f};
    float target_vel_angle{0.0f};
    float aspect{0.0f};
    float target_speed{0.0f};
    float side{0.0f};
    bool behind{false};
    bool head_on{false};
};

struct ThreatScore {
    int a{0};
    int b{0};
    int aptitude{0};
};

// entite+0x174..0x179 : horloge de ciblage, masque M selon FL, fenetres lente et rapide
struct RetargetClock {
    float clock{0.0f};
    uint8_t mask{3};
    bool slow_fired{false};
    bool fast_fired{false};
    // AIEntity_MasterTick_5ACC : avance, fenetres rearmees a leur fermeture
    void advance(float dt) {
        clock += dt;
        int t = (int) floorf(clock);
        if (slow_fired && (t & mask) != 0) {
            slow_fired = false;
        }
        if (fast_fired && (t & (mask >> 1)) != 0) {
            fast_fired = false;
        }
    }
    // AI_RetargetWindowSlow_A288
    bool slowWindow() {
        if (slow_fired || ((int) floorf(clock) & mask) != 0) {
            return false;
        }
        slow_fired = true;
        return true;
    }
    // AI_RetargetWindowFast_A2BD
    bool fastWindow() {
        if (fast_fired || ((int) floorf(clock) & (mask >> 1)) != 0) {
            return false;
        }
        fast_fired = true;
        return true;
    }
};

struct ManeuverState {
    int id{0};
    SCMissionActors *target{nullptr};
    uint8_t level{0};
    float timer{0.0f};
    int phase{0};
    int phase_hint{0};
    int segments{0};
    int side{0};
    uint8_t bits{0};
    Vector3D point{0.0f, 0.0f, 0.0f};
    Vector3D segment{0.0f, 0.0f, 0.0f};
    float segment_timer{0.0f};
    float start_heading{0.0f};
    int uses[32]{};
    float pursuit_aspect{0.0f};     // word_72097, angle (D, vitesse de la cible) memorise par ID7
    const char *end_reason{""};     // raison de la derniere fin de manoeuvre (banc de DebugStrike)
};

// entite+0x10F/+0x111/+0x139/+0x13D/+0x141 et bloc de commandes du noeud ID21
struct NavigationState {
    SCMissionActors *reference{nullptr};
    Vector3D center{0.0f, 1000.0f, 0.0f};
    float radius{30000.0f};
    float altitude{2000.0f};
    float speed{250.0f};
    Vector3D point{0.0f, 0.0f, 0.0f};
    Vector3D velocity{0.0f, 0.0f, 0.0f};
    float timer{0.0f};
    bool reached{false};
};

// noeud ID19 (GroundAttack_*)
struct GroundAttackState {
    SCMissionActors *target{nullptr};
    int phase{0};
    RSEntity *weapon{nullptr};
    SCSimulatedObject *released{nullptr};
    Vector3D last_position{0.0f, 0.0f, 0.0f};
    int last_tick{-2};
    Vector3D autopilot_target{0.0f, 0.0f, 0.0f};
    std::vector<SCSimulatedObject *> known_objects;
    int release_wait{0};
    float phase3_time{0.0f};
};

enum GroundOp { GROUND_OP_NONE, GROUND_OP_TAKEOFF, GROUND_OP_LANDING };

// decollage (TOFF) et atterrissage (LAND)
struct GroundOpsState {
    GroundOp kind{GROUND_OP_NONE};
    int phase{0};
    float time{0.0f};
    float stick{0.0f};
    Vector3D axis{0.0f, 0.0f, 1.0f};
    Vector3D origin{0.0f, 0.0f, 0.0f};
    Vector3D landing_target{0.0f, 0.0f, 0.0f};
    Vector3D landing_dir{0.0f, 0.0f, 0.0f};
    float landing_speed{0.0f};
    float landing_duration{0.0f};
    float landing_pitch{0.0f};
    int landing_counter{0};
    bool landing_leveled{false};
    bool landing_done{false};
};

// FOLLOW_ALLY : leader, etat de l'ailier +0x149, historique du leader
struct FormationState {
    SCMissionActors *leader{nullptr};
    uint8_t leader_arg{0xFF};
    uint8_t leader_state{0};
    bool active{false};
    bool history_ready{false};
    Vector3D nose[32];
    Vector3D span[32];
    int index{0};
    float clock{0.0f};
};

// moral, discipline, fuite
struct MoodState {
    bool disciplined{true};
    float discipline_timer{0.0f};
    float timer{0.0f};
    bool mutiny{false};
    bool fleeing{false};
    Vector3D destination{0.0f, 0.0f, 0.0f};
    Vector3D home{0.0f, 0.0f, 0.0f};
    bool home_set{false};
    int enemies_alive{0};
    int own_losses{0};
    bool enemies_active{false};
};

class SCAIBrain {
    friend class DebugStrike;
public:
    static constexpr float TICK_DURATION = 1.0f / 25.0f;
    SCAIBrain(SCMissionActors *owner);
    void tick();
    void updateTimers();
    void topLevelThink();
    bool runGoalSelectors();

    SCMissionActors *air_target{nullptr};
    SCMissionActors *ground_target{nullptr};
    SCSimulatedObject *missile_threat{nullptr};
    uint16_t weapon_mask{0};
    bool ground_attack_active{false};
    uint8_t reaction_level{REACT_NONE};
    bool just_hit{false};
    SCMissionActors *last_attacker{nullptr};
    bool objective_locked{false};
    int morale{2};
    int fire_solution_quality{0};

    bool acquireBestThreat(bool allow_new_target);
    void onObjectiveSet(bool assigned);
    char decision[480]{};           // chemin de decision du dernier tick (trace de DebugStrike)
    char tournament_scores[160]{};  // dernier tournoi : identifiant:note des candidats retenus
    void decide(const char *format, ...);
    std::string behaviorLabel();

private:
    SCMissionActors *owner{nullptr};
    RetargetClock retarget;
    ManeuverState maneuver;
    NavigationState nav;
    GroundAttackState ground;
    GroundOpsState ground_ops;
    FormationState formation;
    MoodState mood;
    int last_air_candidates{-1};
    int last_ground_candidates{-1};
    int last_missile_candidates{-1};
    int debug_ticks{0};
    int last_weapon_mask{-1};
    int burst_remaining{0};                      // entite+0x280
    uint16_t last_fired_weapon{0};               // entite+0x10D
    int last_fire_second{0};                     // entite+0x109
    SCMissionActors *station_tracked{nullptr};   // point d'emport engage +0x0D : objet suivi
    bool selector_ran{false};                    // bit 2 de +0x28B
    bool escort_leader_free{false};              // bit 1 de +0x28B : pose par l'ailier, leader non engage
    SCMissionActors *engage_target{nullptr};     // entite+0x285
    SCMissionActors *announced_target{nullptr};
    bool skillCheck(uint8_t stat, int modifier);
    uint16_t loadedWeaponMask();
    uint16_t selectWeaponMask();
    int computeFireSolutionQuality();
    RSEntity *loadedMissile(uint16_t mask);
    bool testMissileLock(RSEntity *missile);
    bool reactionThreshold(int quality);
    bool weaponRecoveryBusy();
    bool behaviorSelector();
    void updateGroundAttack(SCMissionActors *target);
    bool combatStep(bool ground_allowed);
    bool destroyTargetOrder(uint8_t arg);
    bool takeoffOrder();
    bool landingOrder(uint8_t approach_spot, uint8_t touchdown_spot);
    bool defendTargetOrder(uint8_t arg);
    bool followAllyOrder(uint8_t arg);
    CombatContext ctx;
    bool threat_alerted{false};
    SCSimulatedObject *complained_missile{nullptr};
    float floorAltitude();
    bool tooSlow();
    bool tooLow();
    int decisionWeight();
    void buildCombatContext(SCMissionActors *target);
    int scoreManeuver(int id, SCMissionActors *target);
    bool runTournament();
    void applyManeuver(int id, SCMissionActors *target, uint8_t level);
    void endManeuver(bool finished);
    void onBehaviorEnded(bool finished);
    bool tickManeuver();
    bool tickSegment();
    void maneuverSpeed(float wanted);
    void speedThrottle(float wanted);
    bool ejectDecision(int mode);
    bool runReflexes();
    bool scanCollisionThreats();
    bool collisionCourse(SCMissionActors *other);
    float referenceSpeed();
    void interceptSpeed(SCMissionActors *target, float threshold);
    bool gunSnap(SCMissionActors *target);
    bool interceptDispatcher(SCMissionActors *target);
    void incomingThreatWarning();
    int computeMorale();
    bool canHoldOrder();
    bool moraleReaction();
    void leaveFight();
    SCMissionActors *playerActor();
    SCMissionActors *leaderActor();
    bool engageAttackerReaction();
    void abandonNavigation();
    void wander();
    void resetGroundAttack(bool finished);
    RSEntity *selectGroundWeapon();
    Vector3D predictBombImpact(RSEntity *bomb);
    bool guidedWeaponLock(RSEntity *weapon, SCMissionActors *target);
    float headingDelta(Vector3D direction);
    void computeAttitudeError(Vector3D direction, float &heading_error, float &pitch_error);
    int missileDistanceBand();
    bool reactToMissile();
    void missileCountermeasures();
    ThreatScore scoreAirCandidate(SCMissionActors *candidate, Vector3D delta);
    ThreatScore scoreGroundCandidate(SCMissionActors *candidate, Vector3D delta);
    ThreatScore scoreMissile(SCSimulatedObject *missile);
    bool executeGoalAction();
    bool tryWanderRandom();
    Vector3D runwayAxis(Vector3D direction);
    void endGroundOp();
    SCMissionActors *followLeader();
    bool followAllyExec(SCMissionActors *leader);
    int escortQueryLeader(SCMissionActors *leader, SCMissionActors *&engage);
    bool formationGuidance(SCMissionActors *leader);
    Vector3D formationSlot(SCMissionActors *leader);
    void followWaypoints(SCMissionActors *leader);
    int last_finished_behavior{0};
    void applyNavigation(Vector3D point, Vector3D velocity, float duration);
    void tickNavigation();
    enum BehaviorKind : uint8_t { BEHAVIOR_MANEUVER, BEHAVIOR_NAVIGATION, BEHAVIOR_GROUND_ATTACK, BEHAVIOR_TAKEOFF, BEHAVIOR_LANDING };
    std::vector<BehaviorKind> behaviors;               // entite+0x0D
    void pushBehavior(BehaviorKind kind);
    void endBehavior(BehaviorKind kind, bool finished);
    void abandonBehavior();
    bool isRunning(BehaviorKind kind);
    bool behaviorRunning();
    void tickBehavior();
    bool returnToBase(Vector3D point, Vector3D velocity);
    bool navSolutionToPoint();
    bool flyToWaypointOrder(uint8_t point_spot, uint8_t velocity_spot);
    bool defendAreaOrder(uint8_t arg);
    bool defendExec();
    int opposingCampAlive();
};
