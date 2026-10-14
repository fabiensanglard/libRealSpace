#include "precomp.h"
#include "SCAIBrain.h"
#include <cstdarg>

// Goal_MoraleReaction_878F : vitesse voulue de la fuite, (250, 100, 0) en axes asm
static const Vector3D FLEE_VELOCITY(250.0f, 0.0f, -100.0f);

SCAIBrain::SCAIBrain(SCMissionActors *owner) {
    this->owner = owner;
    // AIEntity_CreateByType_12B4E : horloge decalee de 0x19 (24.8) par entite creee
    owner->mission->ai_clock_stagger += 25.0f / 256.0f;
    retarget.clock = owner->mission->ai_clock_stagger;
    // PilotProfile_LoadNUMSCompanionFile_73FB4 : masque +0x179 selon FL
    int flying = owner->profile->ai.atrb.FL;
    retarget.mask = flying < 4 ? 15 : flying < 11 ? 7 : 3;
}

// AIEntity_MasterTick_5ACC
void SCAIBrain::decide(const char *format, ...) {
    size_t length = strlen(decision);
    if (length > 0 && length + 3 < sizeof(decision)) {
        strcat(decision, " > ");
        length += 3;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(decision + length, sizeof(decision) - length, format, args);
    va_end(args);
}
std::string SCAIBrain::behaviorLabel() {
    if (behaviors.empty()) {
        return "aucun";
    }
    switch (behaviors.back()) {
        case BEHAVIOR_MANEUVER:
            return "manoeuvre ID " + std::to_string(maneuver.id);
        case BEHAVIOR_NAVIGATION:
            return "navigation ID 21";
        case BEHAVIOR_GROUND_ATTACK:
            return "attaque au sol";
        case BEHAVIOR_TAKEOFF:
            return "decollage";
        case BEHAVIOR_LANDING:
            return "atterrissage";
    }
    return "?";
}
void SCAIBrain::tick() {
    decision[0] = '\0';
    owner->pilot->ClearGuidance();
    selector_ran = false;
    this->updateTimers();
    if (!owner->plane->ejected) {
        this->topLevelThink();
    } else if (formation.active) {
        // pilote ejecte (flags_75 bit 5) en mode formation (bit 3 de +0x28B)
        this->decide("pilote ejecte : formation");
        this->followAllyExec(this->followLeader());
    } else {
        // AI_TriggerBehaviorUpdate : pilote ejecte, le comportement en cours est abandonne
        this->decide("pilote ejecte : abandon");
        this->abandonBehavior();
    }
    just_hit = false;
    escort_leader_free = false;
}

void SCAIBrain::updateTimers() {
    if (!mood.home_set) {
        mood.home = owner->plane->position;
        mood.home_set = true;
        if (!owner->follow_slot_set) {
            // PilotProfile_LoadNUMSCompanionFile_73FB4 : entite+0x14A = NUMS (RSIntel range {x, z, y})
            Vector3D nums = owner->mission->intel.formation_offset;
            owner->follow_slot = Vector3D(nums.x, nums.z, nums.y);
        }
    }
    debug_ticks++;
    // horloge +0x175, fenetres de ciblage rearmees a leur fermeture
    retarget.advance(TICK_DURATION);
    if (mood.mutiny) {
        air_target = this->playerActor();
    }
    morale = this->computeMorale();
    mood.discipline_timer -= TICK_DURATION;
    if (mood.discipline_timer <= 0.0f) {
        static const int adjust[4] = {7, 4, -3, -5};
        mood.disciplined = owner->profile->ai.atrb.LY + adjust[morale - 2] > 7;
        mood.discipline_timer = 3.0f;
    }
    mood.timer -= TICK_DURATION;
    ground_attack_active = false;
}

// AI_TopLevelThink (AI_TICK_CALL_GRAPH.md, "Ordre complet d'un tick")
void SCAIBrain::topLevelThink() {
    bool airborne = !owner->plane->on_ground;
    // 1. missile air-air tire au cycle precedent (byte_6E4D7) et FL >= 12
    if (owner->mission->aa_missile_launched_last && owner->profile->ai.atrb.FL >= 12) {
        this->acquireBestThreat(false);
    }
    // 2. alerte de menace, sautee en decollage/atterrissage et au sol
    bool ground_order = owner->current_command == OP_SET_OBJ_TAKE_OFF || owner->current_command == OP_SET_OBJ_LAND;
    if (airborne && !ground_order) {
        this->incomingThreatWarning();
    }
    // 3. reflexes : decrochage (niveau <= 5), sol (<= 4), collision entre avions (<= 3) ; si l'un reagit, fin
    if (airborne) {
        bool reacted = !this->behaviorRunning() && this->runReflexes();
        if (reacted) {
            this->decide("reflexe : manoeuvre ID %d", maneuver.id);
        }
        if (!reacted && reaction_level <= REACT_COLLISION) {
            reacted = this->scanCollisionThreats();
            if (reacted) {
                this->decide("evitement de collision");
            }
        }
        if (reacted) {
            if (missile_threat != nullptr) {
                this->missileCountermeasures();
            }
            return;
        }
    }
    // 4. reactions prioritaires : esquive de missile (+0x281 pose, aucun comportement, niveau 2), entree en combat contre un attaquant
    if (missile_threat != nullptr) {
        if (!this->behaviorRunning() && reaction_level == REACT_MISSILE) {
            if (this->reactToMissile()) {
                this->decide("esquive de missile");
                return;
            }
        } else {
            this->missileCountermeasures();
        }
    }
    if (airborne && reaction_level <= REACT_ENGAGED && this->engageAttackerReaction()) {
        this->decide("reaction a l'attaquant %s", air_target != nullptr ? air_target->actor_name.c_str() : "?");
        return;
    }
    // 5. pendant une reaction, le comportement en cours garde la main
    if (this->behaviorRunning() && reaction_level != REACT_NONE) {
        this->decide("reaction %d : %s en cours", reaction_level, this->behaviorLabel().c_str());
        this->tickBehavior();
        return;
    }
    // 6. GOAL
    this->runGoalSelectors();
    // loc_850B : rafale de canon en cours, selecteur pas passe ce tick
    if (burst_remaining > 0 && last_fired_weapon == 0x800 && !selector_ran) {
        burst_remaining--;
        owner->pilot->Fire(0x800, air_target);
    }
}

bool SCAIBrain::engageAttackerReaction() {
    // AI_EngageAttackerReaction_E246 (1) : touche, ou fenetre rapide
    if (just_hit || retarget.fastWindow()) {
        this->acquireBestThreat(false);
    }
    bool was_engaged = reaction_level == REACT_ENGAGED;
    if (reaction_level == REACT_ENGAGED) {
        reaction_level = REACT_NONE;
    }
    if (air_target == nullptr || air_target->plane == nullptr || reaction_level != REACT_NONE) {
        return false;
    }
    Vector3D to_target = air_target->plane->position - owner->plane->position;
    float nose_vs_target_nose = owner->plane->forward.AngleBetween(air_target->plane->forward);
    float nose_vs_direction = owner->plane->forward.AngleBetween(to_target);
    bool on_my_six = nose_vs_direction > 150.0f && nose_vs_target_nose < 30.0f && to_target.Length() < (float) owner->mission->intel.range_close;
    if (!just_hit && !on_my_six) {
        return false;
    }
    reaction_level = REACT_ENGAGED;
    if (!was_engaged) printf("AI %s#%d engage attacker=%s hit=%d six=%d command=%d formation.leader_state=%d\n", owner->actor_name.c_str(), owner->actor_id, air_target->actor_name.c_str(), just_hit ? 1 : 0, on_my_six ? 1 : 0, owner->current_command, formation.leader_state);
    if (owner->current_command == OP_SET_OBJ_FOLLOW_ALLY) {
        if (formation.leader_state != 0) {
            return false;
        }
        bool leader_is_player = false;
        for (auto actor : owner->mission->actors) {
            if (actor->actor_id == owner->current_command_arg) {
                leader_is_player = actor->actor_name == "PLAYER";
                break;
            }
        }
        bool aims_at_me = air_target->brain != nullptr && air_target->brain->air_target == owner;
        if (!leader_is_player && !aims_at_me) {
            return false;
        }
        formation.leader_state = 3;
        if (owner->target != air_target) {
            printf("AI %s#%d switching engage airtarget target from %s#%d to %s#%d\n", owner->actor_name.c_str(), owner->actor_id,
                   owner->target ? owner->target->actor_name.c_str() : "none", owner->target ? owner->target->actor_id : -1,
                   air_target->actor_name.c_str(), air_target->actor_id);
        }
        owner->target = air_target;
        engage_target = air_target;
        if (leader_is_player) {
            owner->setMessage(0x12);
        }
        return false;
    }
    // (4b) chasseur (JINF +0x52 >= 9) : abandon d'une navigation ID21 en cours, combat, renvoie 1
    if (owner->object->entity->combat_class >= 9) {
        if (!behaviors.empty() && behaviors.back() == BEHAVIOR_NAVIGATION) {
            this->abandonNavigation();
        }
        this->combatStep(false);
        return true;
    }
    // (4c) rien en cours, sans ordre ou ordre de vol : errance, ordre verrouille
    if (!this->behaviorRunning() && (owner->current_command == OP_NOOP || owner->current_command == OP_SET_OBJ_FLY_TO_WP)) {
        nav.reached = true;
        objective_locked = true;
        this->wander();
    }
    return false;
}

bool SCAIBrain::followAllyOrder(uint8_t arg) {
    // Goal_SelectTransition
    bool formed = this->followAllyExec(this->followLeader());
    SCMissionActors *leader = formation.leader;
    if (!formed && leader == nullptr) {
        owner->current_command_executed = true;
        return false;
    }
    owner->current_command_executed = false;
    if (formation.leader_state == 2) {
        // Goal_SelectTransition, etat 2 : comportement en cours, sinon ID21 vers le point pose par Goal_MoraleReaction_878F
        objective_locked = true;
        if (this->behaviorRunning()) {
            this->tickBehavior();
        } else {
            this->applyNavigation(mood.destination, Vector3D(250.0f, 0.0f, 0.0f), 2.0f);
        }
        return true;
    }
    if (formation.leader_state == 3) {
        SCMissionActors *engaged = owner->target;
        if (engaged == nullptr || engaged->is_destroyed || (engaged->plane != nullptr && engaged->plane->object->alive == 0)) {
            formation.leader_state = 0;
            owner->target = nullptr;
            owner->current_target = SCMissionActors::NO_TARGET;
            printf("followAllyOrder l202 : AI %s#%d switching current target from %d to %d\n", owner->actor_name.c_str(), owner->actor_id, owner->current_target, SCMissionActors::NO_TARGET);
        }
    }
    if (formation.leader_state == 0) {
        objective_locked = false;
        SCMissionActors *threat = owner->mission->player_tail_threat;
        if (threat != nullptr && leader == this->playerActor() && mood.disciplined) {
            this->endManeuver(false);
            formation.leader_state = 3;
            if (owner->target != threat) {
                printf("AI %s#%d switching threat engage target from %s#%d to %s#%d\n", owner->actor_name.c_str(), owner->actor_id,
                       owner->target ? owner->target->actor_name.c_str() : "none", owner->target ? owner->target->actor_id : -1,
                       threat->actor_name.c_str(), threat->actor_id);
            }
            owner->target = threat;
            air_target = threat;
            engage_target = threat;
            objective_locked = true;
            owner->setMessage(0x10);
        } else if (!formation.active) {
            if (maneuver.id == 0 && reaction_level == REACT_MISSILE) {
                this->combatStep(false);
            } else {
                this->followWaypoints(leader);
            }
        }
    }
    if (formation.leader_state == 3) {
        // Goal_SelectTransition etat 3 (loc_BC7E) : ordre verrouille
        objective_locked = true;
        this->combatStep(false);
    } else if (formation.leader_state == 1) {
        objective_locked = true;
        this->combatStep(!mood.mutiny);
    }
    return true;
}

SCMissionActors *SCAIBrain::playerActor() {
    for (auto actor : owner->mission->actors) {
        if (actor->actor_name == "PLAYER") {
            return actor;
        }
    }
    return nullptr;
}

SCMissionActors *SCAIBrain::leaderActor() {
    if (owner->current_command != OP_SET_OBJ_FOLLOW_ALLY) {
        return nullptr;
    }
    for (auto actor : owner->mission->actors) {
        if (actor->actor_id == owner->current_command_arg) {
            return actor;
        }
    }
    return nullptr;
}

bool SCAIBrain::canHoldOrder() {
    uint16_t loaded = this->loadedWeaponMask();
    if (owner->current_command == OP_SET_OBJ_DESTROY_TARGET) {
        SCMissionActors *target = owner->target;
        if (target == nullptr) {
            return true;
        }
        return (loaded & (target->plane == nullptr ? 0xFC : 0xF03)) != 0;
    }
    if (owner->current_command == OP_SET_OBJ_DEFEND_TARGET) {
        return (loaded & 0xF03) != 0;
    }
    return true;
}

int SCAIBrain::computeMorale() {
    int score = 100;
    bool damaged = false;
    for (auto &system : owner->object->entity->sysm) {
        auto health_system = owner->plane->system_health.find(system.first);
        if (health_system == owner->plane->system_health.end()) {
            continue;
        }
        for (auto &sub_system : system.second) {
            auto health = health_system->second.find(sub_system.first);
            if (health != health_system->second.end() && health->second < sub_system.second) {
                damaged = true;
            }
        }
    }
    if (damaged) {
        score -= 100;
    }
    float capacity = (float) owner->object->entity->jdyn->fuel_capacity;
    if (capacity > 0.0f) {
        score += (int) (51.0f * (float) owner->plane->GetFuel() / capacity) - 50;
    }
    if (!this->canHoldOrder()) {
        score -= 50;
    }
    mood.enemies_alive = 0;
    mood.own_losses = 0;
    mood.enemies_active = false;
    float radar_range = (float) owner->mission->intel.range_far;
    for (auto actor : owner->mission->actors) {
        if (actor->plane == nullptr) {
            continue;
        }
        if (actor->team_id == owner->team_id) {
            if (actor->is_destroyed) {
                mood.own_losses++;
            }
        } else if (!actor->is_destroyed && actor->is_active) {
            mood.enemies_alive++;
            if ((actor->plane->position - owner->plane->position).Length() < radar_range) {
                mood.enemies_active = true;
            }
        }
    }
    if (mood.enemies_alive > 0) {
        score += -8 * mood.enemies_alive - 32 * mood.own_losses;
    }
    if (reaction_level != REACT_NONE) {
        score -= 50;
    }
    int confidence = owner->profile->ai.atrb.CN;
    score += confidence < 3 ? 0 : confidence < 6 ? 15 : confidence < 12 ? 30 : confidence < 15 ? 50 : 75;
    if (mood.enemies_alive > 0 && score >= 80) {
        score = 79;
    }
    if (confidence > 9 && score < 25) {
        score = 25;
    }
    if (confidence <= 0) {
        score = 0;
    }
    return score < 25 ? 5 : score < 50 ? 4 : score < 80 ? 3 : 2;
}

void SCAIBrain::leaveFight() {
    owner->setMessage(8);
    formation.leader_state = 2;
    objective_locked = true;
    mood.destination = mood.home;
    mood.destination.y += 1000.0f;
    // Goal_MoraleReaction_878F (B2) : abandon, ID21 2 s, vitesse voulue (250, 0, 0)
    this->abandonBehavior();
    this->applyNavigation(mood.destination, Vector3D(250.0f, 0.0f, 0.0f), 2.0f);
    printf("AI %s#%d morale=%d leaves the fight\n", owner->actor_name.c_str(), owner->actor_id, morale);
}

bool SCAIBrain::moraleReaction() {
    if (mood.timer > 0.0f) {
        return false;
    }
    mood.timer = 5.0f;
    SCMissionActors *player = this->playerActor();
    bool player_side = player != nullptr && owner->team_id == player->team_id;
    SCMissionActors *leader = this->leaderActor();
    bool leader_is_player = leader != nullptr && leader == player;
    bool following = owner->current_command == OP_SET_OBJ_FOLLOW_ALLY;
    if (morale >= 4) {
        if (!player_side) {
            if (mood.fleeing) {
                return false;
            }
            owner->setMessage(8);
            mood.fleeing = true;
            objective_locked = true;
            mood.destination = mood.home;
            mood.destination.y += 1000.0f;
            // Goal_MoraleReaction_878F (A) : abandon, ID21 2 s, vitesse voulue (250, 100, 0), puis ordre 0xA5
            this->abandonBehavior();
            this->applyNavigation(mood.destination, FLEE_VELOCITY, 2.0f);
            printf("AI %s#%d morale=%d flees\n", owner->actor_name.c_str(), owner->actor_id, morale);
            return true;
        }
        if (leader_is_player && !mood.disciplined && following) {
            if (last_attacker != nullptr && last_attacker == player && !mood.enemies_active) {
                mood.mutiny = true;
                air_target = player;
                formation.leader_state = 1;
                objective_locked = true;
                owner->setMessage(0x20);
                printf("AI %s#%d morale=%d turns on the player\n", owner->actor_name.c_str(), owner->actor_id, morale);
                this->combatStep(false);
                return true;
            }
            if (formation.leader_state != 2) {
                this->leaveFight();
                return true;
            }
            return false;
        }
        if (leader_is_player && (reaction_level == REACT_ENGAGED || reaction_level == REACT_MISSILE)) {
            owner->setMessage(6);
        }
        return false;
    }
    if (player_side && leader_is_player && !mood.disciplined && formation.leader_state != 1 && formation.leader_state != 2 && following && mood.enemies_active) {
        owner->setMessage(0x12);
        formation.leader_state = 1;
        objective_locked = true;
        printf("AI %s#%d morale=%d engages on its own\n", owner->actor_name.c_str(), owner->actor_id, morale);
        return true;
    }
    return false;
}

bool SCAIBrain::combatStep(bool ground_allowed) {
    // AI_BehaviorStateMachine_WeightedOptionSelector_9D05 (1) : esquive de missile
    if (this->reactToMissile()) {
        this->decide("esquive de missile");
        return true;
    }
    // (2) ciblage
    if (air_target != nullptr) {
        if (air_target->plane != nullptr && air_target->plane->ejected && !this->acquireBestThreat(ground_allowed)) {
            return false;
        }
    } else if (missile_threat != nullptr || (ground_target != nullptr && ground_allowed)) {
        if (retarget.slowWindow() && !this->acquireBestThreat(ground_allowed)) {
            return false;
        }
    } else if (!this->acquireBestThreat(ground_allowed)) {
        this->decide("aucune cible");
        return false;
    }
    // (3) sans cible aerienne : esquive si menace missile, sinon attaque au sol si autorisee, sinon le GOAL suivant prend la main
    if (air_target == nullptr) {
        if (missile_threat != nullptr) {
            bool evaded = this->reactToMissile();
            this->decide(evaded ? "esquive de missile" : "missile retenu, pas d'esquive");
            return evaded;
        }
        if (ground_allowed && ground_target != nullptr) {
            this->decide("attaque au sol de %s", ground_target->actor_name.c_str());
            this->updateGroundAttack(ground_target);
            return ground_attack_active;
        }
        this->decide("pas de cible aerienne");
        return false;
    }
    // la cible aerienne (+0x287) ne remplace pas la cible de mission (+0x137)
    owner->current_target = air_target->actor_id;
    air_target->attacker = owner;
    // (4) AI_BehaviorSelector agit : le comportement en cours est abandonne
    if (reaction_level <= REACT_ENGAGED && this->behaviorSelector()) {
        this->decide("tir et poursuite sur %s", air_target->actor_name.c_str());
        this->abandonBehavior();
        return true;
    }
    // (5) comportement en cours, (6) tournoi
    if (this->behaviorRunning()) {
        this->decide("%s en cours contre %s", this->behaviorLabel().c_str(), air_target->actor_name.c_str());
        this->tickBehavior();
        return true;
    }
    bool won = this->runTournament();
    if (won) {
        this->decide("tournoi : ID %d contre %s (%s)", maneuver.id, air_target->actor_name.c_str(), tournament_scores);
    } else {
        this->decide("tournoi sans gagnant (%s)", tournament_scores);
    }
    return won;
}

void SCAIBrain::abandonNavigation() {
    if (this->isRunning(BEHAVIOR_NAVIGATION)) {
        owner->pilot->disengageAutopilot();
        this->endBehavior(BEHAVIOR_NAVIGATION, false);
    }
}

// MVRS_ID21_ApplyAutopilotNav_11AC4 : bloc de commandes (point, vitesse voulue), minuteur = scalaire du contexte
void SCAIBrain::applyNavigation(Vector3D point, Vector3D velocity, float duration) {
    nav.point = point;
    nav.velocity = velocity;
    nav.timer = duration;
    nav.reached = false;
    owner->pilot->target_waypoint = point;
    if (owner->pilot->autopilotActive()) {
        owner->pilot->setAutopilotTarget(point, velocity);
    }
    this->pushBehavior(BEHAVIOR_NAVIGATION);
    this->tickNavigation();
}

// MVRS_ID21_TickAutopilotNav_11B16
void SCAIBrain::tickNavigation() {
    nav.timer -= TICK_DURATION;
    owner->pilot->target_waypoint = nav.point;
    if (!owner->pilot->autopilotActive()) {
        if (std::fabs(owner->pilot->NosePitch()) < 15.0f) {
            owner->pilot->engageAutopilot(nav.point, nav.velocity);
        } else {
            owner->pilot->SetPitchCommand(0.0f, 5.0f);
        }
    }
    nav.reached = owner->pilot->autopilotReached();
    if (nav.timer < 0.0f || nav.reached) {
        // JDYN+0x68 = 0xFF puis Behavior_PopFinished_75612
        owner->pilot->disengageAutopilot();
        this->endBehavior(BEHAVIOR_NAVIGATION, true);
    }
}

// Behavior_PushRunning_756A4
void SCAIBrain::pushBehavior(BehaviorKind kind) {
    if (behaviors.empty() || behaviors.back() != kind) {
        behaviors.push_back(kind);
    }
}

// Behavior_PopFinished_75612 (fin normale : le precedent reprend) / NotifiableRef_DetachTarget_75661 (abandon : pile videe)
void SCAIBrain::endBehavior(BehaviorKind kind, bool finished) {
    auto it = std::find(behaviors.begin(), behaviors.end(), kind);
    if (it == behaviors.end()) {
        return;
    }
    static const int ids[] = {0, 21, 19, 0x11, 0x12};
    if (kind != BEHAVIOR_MANEUVER) {
        last_finished_behavior = ids[kind];
    }
    if (finished) {
        behaviors.erase(it);
    } else {
        behaviors.clear();
    }
    if (!finished || behaviors.empty()) {
        this->onBehaviorEnded(finished);
    }
}

void SCAIBrain::abandonBehavior() {
    if (behaviors.empty()) {
        return;
    }
    switch (behaviors.back()) {
        case BEHAVIOR_MANEUVER:
            this->endManeuver(false);
        break;
        case BEHAVIOR_NAVIGATION:
            this->abandonNavigation();
        break;
        case BEHAVIOR_GROUND_ATTACK:
            this->resetGroundAttack(false);
        break;
        default:
            this->endBehavior(behaviors.back(), false);
        break;
    }
}

bool SCAIBrain::isRunning(BehaviorKind kind) {
    return std::find(behaviors.begin(), behaviors.end(), kind) != behaviors.end();
}

// entite+0x0D non nul : un comportement est en cours
bool SCAIBrain::behaviorRunning() {
    return !behaviors.empty();
}

// methode +0xC du comportement en cours (sommet de la pile)
void SCAIBrain::tickBehavior() {
    switch (behaviors.back()) {
        case BEHAVIOR_MANEUVER:
            this->tickManeuver();
        break;
        case BEHAVIOR_NAVIGATION:
            this->tickNavigation();
        break;
        case BEHAVIOR_GROUND_ATTACK:
            this->updateGroundAttack(ground.target);
        break;
        case BEHAVIOR_TAKEOFF:
            this->takeoffOrder();
        break;
        case BEHAVIOR_LANDING:
            this->landingOrder(owner->current_command_arg, owner->current_command_arg2);
        break;
    }
}

// AI_NavSolutionToPoint : vers +0x10F (objet) ou +0x111, a +0x13D au-dessus du terrain SOUS MOI
bool SCAIBrain::navSolutionToPoint() {
    if (this->behaviorRunning()) {
        return false;
    }
    SCPlane *plane = owner->plane;
    Vector3D point = nav.center;
    if (nav.reference != nullptr) {
        point = nav.reference->plane != nullptr ? nav.reference->plane->position : nav.reference->object->position;
    }
    point.y = plane->groundlevel + nav.altitude;
    Vector3D delta = point - plane->position;
    if (nav.radius >= delta.Length()) {
        this->decide("dans le rayon (d %.0f <= %.0f)", delta.Length(), nav.radius);
        return false;
    }
    this->decide("retour vers %s (d %.0f > %.0f)", nav.reference != nullptr ? nav.reference->actor_name.c_str() : "le centre", delta.Length(), nav.radius);
    delta.Normalize();
    this->applyNavigation(point, delta * nav.speed, 2.0f);
    return true;
}

// Goal_WanderRandom (gestionnaire GOAL 3)
void SCAIBrain::wander() {
    if (this->behaviorRunning()) {
        this->decide("errance : %s en cours", this->behaviorLabel().c_str());
        this->tickBehavior();
        return;
    }
    if (!nav.reached && last_finished_behavior == 21) {
        this->decide("errance : reprise du segment de navigation");
        this->applyNavigation(nav.point, nav.velocity, 30.0f);
        return;
    }
    // CRT_Rand % 20000 - 10000 sur les deux axes horizontaux (asm c0 = x, c1 = -z)
    float rx = (float) (std::rand() % 20000 - 10000);
    float ry = (float) (std::rand() % 20000 - 10000);
    Vector3D direction(rx, 0.0f, -ry);
    direction.Normalize();
    SCPlane *plane = owner->plane;
    float climb = plane->groundlevel + nav.altitude - plane->position.y;
    climb = (std::clamp)(climb, -1000.0f, 1000.0f);
    Vector3D point = plane->position + direction * 30000.0f + Vector3D(0.0f, climb, 0.0f);
    this->decide("errance : nouvelle direction (cap %.0f)", direction.Azimuth());
    this->applyNavigation(point, direction * nav.speed, 30.0f);
}

// Goal_IsComplete 0xA8/0xA9 : camp 1 -> word_706A7 - word_706A9, sinon word_706A3 - word_706A5
int SCAIBrain::opposingCampAlive() {
    int camp = owner->team_id == 1 ? 0xFF : 1;
    int alive = 0;
    for (auto actor : owner->mission->actors) {
        if (actor->team_id == camp && !actor->is_destroyed && (actor->is_active || actor->actor_name == "PLAYER")) {
            alive++;
        }
    }
    return alive;
}

// Goal_ExecuteAction_A8AC, cas 0xA8/0xA9 (loc_A9DB)
bool SCAIBrain::defendExec() {
    if (this->navSolutionToPoint()) {
        return true;
    }
    if (this->combatStep(false)) {
        return true;
    }
    this->wander();
    return true;
}

bool SCAIBrain::defendTargetOrder(uint8_t arg) {
    SCMissionActors *defended = nullptr;
    for (auto actor : owner->mission->actors) {
        if (actor->actor_id == arg) {
            defended = actor;
            break;
        }
    }
    // Goal_IsComplete 0xA8 : actif tant que la reference +0x137 existe
    if (defended == nullptr || defended->is_destroyed || (defended->plane != nullptr && defended->plane->object->alive == 0)) {
        owner->current_command_executed = true;
        objective_locked = false;
        return false;
    }
    owner->current_command_executed = false;
    owner->current_objective = OP_SET_OBJ_DEFEND_TARGET;
    // Goal_SetObjective_A307 0xA8 : +0x10F = +0x137 = objet defendu
    nav.reference = defended;
    return this->defendExec();
}

bool SCAIBrain::defendAreaOrder(uint8_t arg) {
    // Goal_SetObjective_A307 0xA9 : +0x10F = nul, +0x111 = spot[param1]
    std::vector<SPOT *> &spots = owner->mission->mission->mission_data.spots;
    nav.reference = nullptr;
    nav.center = arg < spots.size() ? spots[arg]->position : Vector3D(0.0f, 0.0f, 0.0f);
    owner->current_objective = OP_SET_OBJ_DEFEND_AREA;
    // Goal_IsComplete 0xA9 : actif seulement si le camp adverse n'a plus d'unite vivante
    if (this->opposingCampAlive() != 0) {
        owner->current_command_executed = true;
        objective_locked = false;
        return false;
    }
    owner->current_command_executed = false;
    return this->defendExec();
}

bool SCAIBrain::flyToWaypointOrder(uint8_t point_spot, uint8_t velocity_spot) {
    // Goal_SetObjective_A307 0xA5 : +0x11F = spot[param1], +0x12B = spot[param2] (absent -> (0, 0, 0))
    std::vector<SPOT *> &spots = owner->mission->mission->mission_data.spots;
    Vector3D point = point_spot < spots.size() ? spots[point_spot]->position : Vector3D(0.0f, 0.0f, 0.0f);
    Vector3D velocity = velocity_spot < spots.size() ? spots[velocity_spot]->position : Vector3D(0.0f, 0.0f, 0.0f);
    owner->current_objective = OP_SET_OBJ_FLY_TO_WP;
    return this->returnToBase(point, velocity);
}

// Goal_IsComplete 0xA4/0xA5 (distance horizontale <= 500 m) puis Goal_ReturnToBase
bool SCAIBrain::returnToBase(Vector3D point, Vector3D velocity) {
    Vector3D delta = point - owner->plane->position;
    delta.y = 0.0f;
    if (delta.Length() <= 500.0f) {
        owner->current_command_executed = true;
        objective_locked = false;
        return false;
    }
    owner->current_command_executed = false;
    if (this->behaviorRunning()) {
        this->tickBehavior();
        return true;
    }
    this->applyNavigation(point, velocity, 2.0f);
    return true;
}

bool SCAIBrain::destroyTargetOrder(uint8_t arg) {
    owner->current_command_executed = false;
    if (owner->is_destroyed || owner->plane == nullptr || owner->plane->object->alive == 0) {
        owner->current_command_executed = true;
        return false;
    }
    // +0x137 est pose par SCMissionActors::setObjective (Goal_SetObjective_A307) ; nul -> Goal_IsComplete 0xA7 : termine
    SCMissionActors *target = owner->target;
    if (target == nullptr) {
        owner->current_command_executed = true;
        objective_locked = false;
        return false;
    }
    bool is_new_target = announced_target != target;
    if (is_new_target) {
        announced_target = target;
        target->attacker = owner;
        if (std::rand() % 16 >= owner->profile->ai.atrb.VB) {
            owner->setMessage(target->plane == nullptr ? 11 : 12);
        }
    }
    if (target->is_destroyed || (target->plane != nullptr && target->plane->object->alive == 0)) {
        if (owner->current_target != SCMissionActors::NO_TARGET) {
            printf("destroytargetorder l721 : AI %s#%d switching current target from %d to %d\n", owner->actor_name.c_str(), owner->actor_id, owner->current_target, SCMissionActors::NO_TARGET);
            owner->target = nullptr;
        }
        owner->current_target = SCMissionActors::NO_TARGET;
        target->attacker = nullptr;
        owner->target = nullptr;
        if (!is_new_target && std::rand() % 16 >= owner->profile->ai.atrb.VB) {
            owner->setMessage(13);
        }
        owner->current_command_executed = true;
        return false;
    }
    owner->current_objective = OP_SET_OBJ_DESTROY_TARGET;
    // Goal_ExecuteAction_A8AC 0xA7 : un comportement en cours passe avant tout
    if (this->behaviorRunning()) {
        this->decide("%s en cours", this->behaviorLabel().c_str());
        this->tickBehavior();
        return true;
    }
    if (target->plane == nullptr) {
        // Goal_ExecuteAction_A8AC 0xA7 : noeud d'attaque au sol, ou GOAL suivant sans arme sol
        this->updateGroundAttack(target);
        return ground_attack_active;
    }
    // Goal_ExecuteAction_A8AC 0xA7 : renvoie 1 (Goal_IsComplete), quel que soit le resultat du combat
    this->combatStep(false);
    return true;
}

bool SCAIBrain::acquireBestThreat(bool allow_new_target) {
    int air_candidates = 0;
    int ground_candidates = 0;
    int missile_candidates = 0;
    int missile_threshold = 0;
    std::string air_names;
    int best_score = -5000;
    SCMissionActors *best_candidate = nullptr;
    SCSimulatedObject *best_missile = nullptr;
    if (reaction_level == REACT_MISSILE) {
        reaction_level = REACT_NONE;
    }
    if (air_target != nullptr && air_target->is_destroyed) {
        air_target = nullptr;
    }
    if (ground_target != nullptr && ground_target->is_destroyed) {
        ground_target = nullptr;
    }
    uint16_t weapon_mask = this->loadedWeaponMask();
    int weight_a = owner->profile->ai.atrb.AR - owner->profile->ai.atrb.FL + 16;
    int weight_b = owner->profile->ai.atrb.FL - owner->profile->ai.atrb.AR + 16;
    for (auto actor : owner->mission->actors) {
        if (actor == owner || actor->is_destroyed) {
            continue;
        }
        if (!actor->is_active && actor != owner->mission->player) {
            continue;
        }
        if (actor->team_id == owner->team_id) {
            continue;
        }
        if (actor->object == nullptr || actor->object->entity == nullptr) {
            continue;
        }
        Vector3D candidate_position = actor->object->position;
        if (actor->plane != nullptr) {
            candidate_position = actor->plane->position;
        }
        Vector3D delta = candidate_position - owner->plane->position;
        char geometry[192];
        if (actor->object->entity->entity_type == EntityType::jet) {
            air_candidates++;
            // loc_3623 : avion candidat seulement si ma classe >= 6 et son pilote non ejecte
            if (actor->plane != nullptr && owner->object->entity->combat_class >= 6 && !actor->plane->ejected) {
                ThreatScore score = this->scoreAirCandidate(actor, delta);
                bool accepted = this->skillCheck(owner->profile->ai.atrb.FL, score.aptitude);
                int total = weight_a * score.a + weight_b * score.b;
                if (accepted && total > best_score) {
                    best_score = total;
                    best_candidate = actor;
                    best_missile = nullptr;
                }
                snprintf(geometry, sizeof(geometry), " d=%.0f ahead=%.0f aims=%.0f heading=%.0f A=%d B=%d si=%d score=%d %s", delta.Length(), owner->plane->forward.AngleBetween(delta), actor->plane->forward.AngleBetween(-delta), owner->plane->forward.AngleBetween(actor->plane->forward), score.a, score.b, score.aptitude, total, accepted ? "ok" : "skip");
            } else {
                snprintf(geometry, sizeof(geometry), " d=%.0f ahead=%.0f", delta.Length(), owner->plane->forward.AngleBetween(delta));
            }
            air_names += " " + actor->actor_name + "#" + std::to_string(actor->actor_id) + "(" + std::to_string(actor->team_id) + geometry + ")";
        } else if (allow_new_target && actor->object->entity->target_type == 2 && (weapon_mask & 0x83C) != 0) {
            ground_candidates++;
            ThreatScore score = this->scoreGroundCandidate(actor, delta);
            bool accepted = this->skillCheck(owner->profile->ai.atrb.FL, score.aptitude);
            int total = weight_a * score.a + weight_b * score.b;
            if (accepted && total > best_score) {
                best_score = total;
                best_candidate = actor;
                best_missile = nullptr;
            }
        }
    }
    SCSimulatedObject *missile = owner->weapon_shooted_at_me;
    if (missile != nullptr && missile->alive && missile->target == owner && missile->obj->entity_type == EntityType::missiles && missile->obj->wdat->target_domain == 1) {
        missile_candidates++;
        ThreatScore score = this->scoreMissile(missile);
        missile_threshold = owner->profile->ai.atrb.FL + score.aptitude;
        bool accepted = this->skillCheck(owner->profile->ai.atrb.FL, score.aptitude);
        int total = weight_a * score.a + weight_b * score.b;
        if (accepted) {
            missile_threat = missile;
            if (total > best_score) {
                best_score = total;
                best_candidate = nullptr;
                best_missile = missile;
            }
        }
    }
    if (best_missile != nullptr) {
        missile_threat = best_missile;
        air_target = nullptr;
        ground_target = nullptr;
        reaction_level = REACT_MISSILE;
    } else if (best_candidate != nullptr) {
        if (best_candidate->object->entity->target_type == 2) {
            ground_target = best_candidate;
            air_target = nullptr;
        } else {
            air_target = best_candidate;
            ground_target = nullptr;
        }
    }
    if (debug_ticks % 100 == 0 || air_candidates != last_air_candidates || ground_candidates != last_ground_candidates || missile_candidates != last_missile_candidates) {
        const char *target_name = "none";
        if (air_target != nullptr) {
            target_name = air_target->actor_name.c_str();
        } else if (ground_target != nullptr) {
            target_name = ground_target->actor_name.c_str();
        }
        printf("AI %s#%d(team %d) command=%d current_target=%d candidates: air=%d ground=%d missile=%d target=%s reaction=%d\n  air:%s\n", owner->actor_name.c_str(), owner->actor_id, owner->team_id, (int) owner->current_command, owner->current_target, air_candidates, ground_candidates, missile_candidates, target_name, (int) reaction_level, air_names.c_str());
        last_air_candidates = air_candidates;
        last_ground_candidates = ground_candidates;
        last_missile_candidates = missile_candidates;
    }
    if (best_missile != nullptr) {
        this->decide("ciblage : missile gagnant (%d avions, %d missile)", air_candidates, missile_candidates);
    } else if (best_candidate != nullptr) {
        if (missile_candidates == 0) {
            this->decide("ciblage : %s (%d avions, 0 missile)", best_candidate->actor_name.c_str(), air_candidates);
        } else {
            this->decide("ciblage : %s (%d avions, %d missile, %s, seuil %d)", best_candidate->actor_name.c_str(), air_candidates, missile_candidates,
                         missile_threat != nullptr ? "missile vu non gagnant" : "missile refuse au jet", missile_threshold);
        }
    } else {
        this->decide("ciblage : rien (%d avions, %d missile, seuil %d)", air_candidates, missile_candidates, missile_threshold);
    }
    return best_candidate != nullptr || best_missile != nullptr;
}

bool SCAIBrain::skillCheck(uint8_t stat, int modifier) {
    int roll = (std::rand() & 15) + 1;
    return roll <= stat + modifier;
}

uint16_t SCAIBrain::loadedWeaponMask() {
    uint16_t mask = 0;
    for (auto weap : owner->plane->weaps_load) {
        if (weap == nullptr || weap->nb_weap <= 0) {
            continue;
        }
        int weapon_id = weap->objct->wdat->weapon_id;
        if (weapon_id >= 1) {
            mask |= (1 << (weapon_id - 1));
        }
    }
    return mask;
}

ThreatScore SCAIBrain::scoreAirCandidate(SCMissionActors *candidate, Vector3D delta) {
    ThreatScore score;
    float distance = delta.Length();
    uint16_t mask = this->loadedWeaponMask();
    bool has_short_missile = (mask & 0x1) != 0;
    bool has_ir_missile = (mask & 0x3) != 0;
    bool has_long_missile = (mask & 0x700) != 0;
    int ahead = (int) owner->plane->forward.AngleBetween(delta);
    int aims = (int) candidate->plane->forward.AngleBetween(-delta);
    int heading = (int) owner->plane->forward.AngleBetween(candidate->plane->forward);

    if (ahead > 135) {
        if (!escort_leader_free) {
            score.aptitude -= 4;
        }
        score.a -= 4;
        score.b += (aims < 45) ? 8 : 2;
    } else if (ahead > 90) {
        if (!escort_leader_free) {
            score.aptitude -= 1;
        }
        score.a -= 2;
        if (aims < 60) {
            score.b += 5;
        }
    } else if (ahead > 30) {
        if (aims < 60) {
            score.b += 4;
        }
        score.aptitude += 2;
        score.a += 2;
    } else {
        if (aims < 60) {
            score.b += 4;
        }
        score.aptitude += 4;
        score.a += 4;
        if (heading < 30) {
            score.a += 4;
        }
    }

    // cible nettement sous moi : dot(D, mon axe haut) < -0,70 (0FFFFFF4Ch)
    Vector3D down_check = delta;
    down_check.Normalize();
    Vector3D own_up(owner->plane->ptw.v[1][0], owner->plane->ptw.v[1][1], owner->plane->ptw.v[1][2]);
    if (down_check.DotProduct(&own_up) < -180.0f / 256.0f && !escort_leader_free) {
        score.aptitude -= 4;
    }
    if (heading > 80 && heading < 100) {
        score.a -= 5;
    } else if (heading > 60 && heading < 120) {
        score.a -= 3;
    }

    RSIntel &intel = owner->mission->intel;
    if (distance > intel.range_far) {
        score.aptitude -= 3;
        score.a -= 5;
    } else if (distance > intel.range_medium && has_long_missile) {
        score.aptitude -= 1;
        score.a += 3;
    } else if (has_ir_missile) {
        if (distance > intel.range_long) {
            score.a -= 2;
            score.aptitude -= 1;
        } else if (distance < intel.range_close) {
            score.a -= 1;
            score.b += 4;
        }
        if (!has_short_missile) {
            if (heading > 150) {
                score.a -= 3;
            } else if (heading > 60) {
                score.a -= 1;
            }
        }
    }

    if (candidate == this->air_target) {
        score.a += 3;
        score.aptitude += 5;
    }
    if (candidate == owner->target) {
        score.a += 2;
        score.aptitude += 4;
    }
    // entite+0x289 : le dernier qui m'a touche
    if (candidate == last_attacker) {
        score.aptitude += 3;
        score.b += 5;
    }
    if (candidate->target == owner) {
        score.aptitude += 2;
        score.b += 1;
    }
    // loc_3CCF : classe du candidat c, la mienne m, reference byte_72038 = 6 (PilotProfile_LoadNUMSCompanionFile_73FB4)
    static const int CLASS_REFERENCE = 6;
    int candidate_class = candidate->object->entity->combat_class;
    int own_class = owner->object->entity->combat_class;
    if (candidate_class <= CLASS_REFERENCE) {
        score.b = 0;
    } else {
        int bonus = candidate_class - 2 <= own_class ? candidate_class - CLASS_REFERENCE : own_class - candidate_class;
        score.a += bonus;
        score.aptitude += bonus;
    }
    if (escort_leader_free) {
        score.aptitude += 4;
    }
    // entite+0x285 : cible designee
    if (candidate == engage_target) {
        score.a += 10;
        score.aptitude += 5;
    }
    return score;
}
// Goal_ExecuteAction_A8AC (gestionnaire GOAL 2)
bool SCAIBrain::executeGoalAction() {
    if (mood.fleeing) {
        this->decide("fuite");
        return this->returnToBase(mood.destination, FLEE_VELOCITY);
    }
    auto name = prog_op_names.find(owner->current_command);
    this->decide("ordre %s %d", name != prog_op_names.end() ? name->second.c_str() : "?", owner->current_command_arg);
    switch (owner->current_command) {
        case OP_SET_OBJ_TAKE_OFF:
            owner->current_command_executed = this->takeoffOrder();
        break;
        case OP_SET_OBJ_LAND:
            owner->current_command_executed = this->landingOrder(owner->current_command_arg, owner->current_command_arg2);
        break;
        case OP_SET_OBJ_FLY_TO_WP:
            return this->flyToWaypointOrder(owner->current_command_arg, owner->current_command_arg2);
        case OP_SET_OBJ_FOLLOW_ALLY:
            return this->followAllyOrder(owner->current_command_arg);
        case OP_SET_OBJ_DESTROY_TARGET:
            return this->destroyTargetOrder(owner->current_command_arg);
        case OP_SET_OBJ_DEFEND_TARGET:
            return this->defendTargetOrder(owner->current_command_arg);
        case OP_SET_OBJ_DEFEND_AREA:
            return this->defendAreaOrder(owner->current_command_arg);
        case OP_SET_OBJ_BE:
            // Goal_ExecuteAction_A8AC 0xBF (loc_A9B7) : AI_NavSolutionToPoint, sinon Goal_WanderRandom ; toujours en cours
            owner->current_command_executed = false;
            if (!this->navSolutionToPoint()) {
                this->wander();
            }
            return true;
        default:
            return false;
    }
    return true;
}
bool SCAIBrain::tryWanderRandom() {
    // Goal_WanderRandom renvoie toujours 1
    this->wander();
    return true;
}
// AI_TopLevelThink, etape 6 : au sol Goal_ExecuteAction_A8AC, en vol les gestionnaires GOAL du profil (2, 3, 4, 5)
bool SCAIBrain::runGoalSelectors() {
    if (owner->plane->on_ground) {
        this->decide("au sol");
        return this->executeGoalAction();
    }
    for (uint8_t rawSelector : owner->profile->ai.goal) {
        size_t mark = strlen(decision);
        bool acted = false;
        switch ((GoalSelector) rawSelector) {
            case GOAL_EMPTY:
                continue;
            case GOAL_EXECUTE_ACTION:
                this->decide("GOAL %d action", rawSelector);
                acted = this->executeGoalAction();
                break;
            case GOAL_WANDER_RANDOM:
                this->decide("GOAL %d errance", rawSelector);
                acted = this->tryWanderRandom();
                break;
            case GOAL_BEHAVIOR_STATE_MACHINE:
                this->decide("GOAL %d combat", rawSelector);
                acted = this->combatStep(false);
                break;
            case GOAL_ACTIVE_WINGMAN:
                this->decide("GOAL %d moral", rawSelector);
                acted = this->moraleReaction();
                break;
            default:
                continue;
        }
        if (acted) {
            return true;
        }
        // un gestionnaire qui n'agit pas laisse une seule mention, le suivant prend la main
        decision[mark] = '\0';
        this->decide("GOAL %d sans effet", rawSelector);
    }
    return false;
}

ThreatScore SCAIBrain::scoreGroundCandidate(SCMissionActors *candidate, Vector3D delta) {
    ThreatScore score;
    float distance = delta.Length();
    int ahead = (int) owner->plane->forward.AngleBetween(delta);
    RSIntel &intel = owner->mission->intel;
    RSEntity *entity = candidate->object->entity;

    if (candidate == owner->target) {
        score.aptitude += 3;
        score.a += 6;
    }
    if (entity->entity_type == EntityType::swpn && entity->swpn_data != nullptr && entity->swpn_data->weapons_round > 0) {
        float range = (float) entity->swpn_data->effective_range;
        if (range > 0.0f && distance <= range) {
            score.b = (int) (10.0f * (1.0f - distance / range)) + 5;
        }
    }
    if (distance > intel.range_ground) {
        score.aptitude -= 4;
    } else if (ahead < 45) {
        score.aptitude += 5;
        score.a += 6;
    } else if (ahead < 90) {
        score.aptitude += 3;
        score.a += 3;
    }
    if (escort_leader_free) {
        score.aptitude += 4;
    }
    if (candidate == engage_target) {
        score.a += 10;
        score.aptitude += 5;
    }
    return score;
}

ThreatScore SCAIBrain::scoreMissile(SCSimulatedObject *missile) {
    ThreatScore score;
    Vector3D missile_position = {missile->x, missile->y, missile->z};
    Vector3D missile_forward = {missile->vx, missile->vy, missile->vz};
    Vector3D delta = missile_position - owner->plane->position;
    float distance = delta.Length();
    int ahead = (int) owner->plane->forward.AngleBetween(delta);
    int aims = (int) missile_forward.AngleBetween(-delta);
    RSIntel &intel = owner->mission->intel;
    int trigger_happy = owner->profile->ai.atrb.FL;

    if (aims <= 90 && distance < intel.range_far) {
        bool long_range = (missile->obj->wdat->weapon_id >= 1) && ((1 << (missile->obj->wdat->weapon_id - 1)) & 0x700) != 0;
        if (long_range || distance <= intel.range_long) {
            float range = long_range ? (float) intel.range_far : (float) intel.range_long;
            score.b += (int) (16.0f * (1.0f - distance / range)) + 24;
            score.aptitude += (trigger_happy * trigger_happy) / 16 - 8;
            // le tireur est ma cible aerienne ou ma cible au sol
            if (missile->shooter != nullptr && (missile->shooter == air_target || missile->shooter == ground_target)) {
                score.aptitude += 4;
            }
            if (ahead > 135 && !escort_leader_free) {
                score.aptitude -= 4;
            }
            Vector3D down_check = delta;
            down_check.Normalize();
            Vector3D own_up(owner->plane->ptw.v[1][0], owner->plane->ptw.v[1][1], owner->plane->ptw.v[1][2]);
            if (down_check.DotProduct(&own_up) < -180.0f / 256.0f && !escort_leader_free) {
                score.aptitude -= 4;
            }
        }
    }
    if (escort_leader_free) {
        score.aptitude += 4;
    }
    return score;
}

uint16_t SCAIBrain::selectWeaponMask() {
    if (air_target == nullptr || air_target->plane == nullptr) {
        return 0;
    }
    Vector3D delta = air_target->plane->position - owner->plane->position;
    float distance = delta.Length();
    int ahead = (int) owner->plane->forward.AngleBetween(delta);
    int tail_aspect = (int) air_target->plane->forward.AngleBetween(delta);
    bool crossing = tail_aspect > 40 && tail_aspect < 140;
    RSIntel &intel = owner->mission->intel;
    // AI_SelectWeaponMask_9665 : ennemi dans les six heures du joueur -> word_722EA
    if (air_target == this->playerActor() && ahead < 30 && tail_aspect < 30 && distance < (float) intel.range_long) {
        owner->mission->player_tail_seen = owner;
    }
    uint16_t loaded = this->loadedWeaponMask();
    bool has_gun = (loaded & 0x800) != 0;
    bool has_short_missile = false;
    bool has_ir_missile = false;
    bool has_long_missile = false;
    if (intel.status_flag) {
        has_short_missile = (loaded & 0x1) != 0;
        has_ir_missile = (loaded & 0x3) != 0;
        has_long_missile = (loaded & 0x700) != 0;
    }

    if (ahead >= 90) {
        return 0;
    }
    if (distance < intel.range_gun && has_gun) {
        return 0x800;
    }
    if (distance > intel.range_medium && has_long_missile) {
        return 0x700;
    }
    if (has_ir_missile && distance < intel.range_long) {
        return (crossing && has_short_missile) ? 0x1 : 0x3;
    }
    return 0;
}

int SCAIBrain::computeFireSolutionQuality() {
    if (air_target == nullptr || air_target->plane == nullptr) {
        return 0;
    }
    Vector3D delta = air_target->plane->position - owner->plane->position;
    float distance = delta.Length();
    int ahead = (int) owner->plane->forward.AngleBetween(delta);
    int tail_aspect = (int) air_target->plane->forward.AngleBetween(delta);
    bool crossing = tail_aspect > 50 && tail_aspect < 130;
    RSIntel &intel = owner->mission->intel;
    if (ahead >= 90) {
        return 0;
    }
    int quality = 10 - (ahead * 10) / 35;
    if (weapon_mask == 0x800) {
        if (distance >= intel.range_gun) {
            return 0;
        }
        // erreur de visee : caps et elevations monde de D et de W (= le nez), ecart de cap non ramene a +/-180
        Vector3D nose = owner->plane->forward;
        float d_elevation = delta.Elevation() - nose.Elevation();
        float d_heading = delta.Azimuth() - nose.Azimuth();
        float error = sqrtf(d_heading * d_heading + d_elevation * d_elevation);
        // tolerance = arctan(vitesse de la cible / d), 90 deg si d <= 0
        float tolerance = distance > 0.0f ? radToDegree(atanf(air_target->plane->worldVelocity().Length() / distance)) : 90.0f;
        if (tolerance <= 0.0f) {
            return 0;
        }
        float difference = error - tolerance;
        if (difference < 0.0f) {
            quality = (int) floorf(8.0f - 2.0f * difference / tolerance);
        } else {
            quality = (int) floorf(8.0f - 4.0f * difference / tolerance);
        }
        if (crossing) {
            quality -= 4;
        }
    } else {
        if (crossing) {
            quality -= 4;
        }
        switch (weapon_mask) {
            case 0x1:
            case 0x2:
            case 0x3:
                if (distance >= intel.range_long) {
                    quality -= 10;
                } else if (distance < intel.range_close) {
                    quality -= crossing ? 10 : 3;
                }
                break;
            case 0x100:
            case 0x700:
                if (distance >= intel.range_far) {
                    quality -= 10;
                } else if (distance < intel.range_medium) {
                    quality -= crossing ? 10 : 3;
                }
                break;
            default:
                quality = 0;
                break;
        }
    }
    if (quality > 10) {
        return 10;
    }
    if (quality < 0) {
        return 0;
    }
    return quality;
}

bool SCAIBrain::reactionThreshold(int quality) {
    return 2 * quality >= owner->profile->ai.atrb.AA;
}

RSEntity *SCAIBrain::loadedMissile(uint16_t mask) {
    for (auto weap : owner->plane->weaps_load) {
        if (weap == nullptr || weap->nb_weap <= 0) {
            continue;
        }
        int weapon_id = weap->objct->wdat->weapon_id;
        if (weapon_id >= 1 && (mask & (1 << (weapon_id - 1))) != 0) {
            return weap->objct;
        }
    }
    return nullptr;
}

bool SCAIBrain::testMissileLock(RSEntity *missile) {
    SCMissionActors *locked = owner->plane->seekerSelect(missile, air_target, owner->mission);
    if (locked != air_target && debug_ticks % 25 == 0) {
        Vector3D delta = air_target->plane->position - owner->plane->position;
        Vector3D target_velocity = air_target->plane->worldVelocity();
        float same_direction = owner->plane->worldVelocity().DotProduct(&target_velocity);
        printf("AI %s#%d seeker aspec=%d no lock on %s (seeker holds %s) d=%.0f range=%u angle=%.0f cone=%d signature=%d sees=%d same_direction=%d\n",
               owner->actor_name.c_str(), owner->actor_id, missile->wdat->weapon_aspec, air_target->actor_name.c_str(), locked != nullptr ? locked->actor_name.c_str() : "nothing",
               delta.Length(), missile->wdat->target_range, owner->plane->forward.AngleBetween(delta), missile->wdat->tracking_cone,
               SCPlane::SeekerSignature(missile, air_target, owner->plane->worldVelocity()), owner->plane->seekerSees(missile, air_target) ? 1 : 0, same_direction >= 0.0f ? 1 : 0);
    }
    return locked == air_target;
}

// AI_WeaponRecoveryBusy_9027 : delai apres un tir, t = secondes depuis le dernier tir (horloge +0x175 - +0x109)
bool SCAIBrain::weaponRecoveryBusy() {
    float elapsed = retarget.clock - (float) last_fire_second;
    int flying = owner->profile->ai.atrb.FL;
    if (elapsed < (float) (flying - 13) && last_fired_weapon != 0x800 && last_fired_weapon != 0) {
        // missile encore en vol (categorie 8, lance par moi) ; Pilot_SkillCheck_B1 : un seul tolere
        bool tolerate = this->skillCheck(owner->profile->ai.atrb.TH, 0);
        bool tolerated = false;
        for (auto weapon : owner->weapons_shooted) {
            if (weapon == nullptr || !weapon->alive || weapon->obj->entity_type != EntityType::missiles) {
                continue;
            }
            if (!tolerated && tolerate) {
                tolerated = true;
                continue;
            }
            return true;
        }
        return false;
    }
    if (last_fired_weapon == 0x800) {
        return elapsed < (float) (16 - owner->profile->ai.atrb.TH) / 8.0f;
    }
    if (last_fired_weapon == 0) {
        return elapsed < (float) (16 - flying) / 8.0f + 0.5f;
    }
    return false;
}

// AI_BehaviorSelector : tir puis poursuite pure de la cible aerienne ; vrai = agit (le comportement en cours est abandonne)
bool SCAIBrain::behaviorSelector() {
    if (air_target == nullptr || air_target->plane == nullptr) {
        return false;
    }
    SCPilot *pilot = owner->pilot;
    Vector3D direction = air_target->plane->position - owner->plane->position;
    selector_ran = true;
    pilot->BeginManual();
    bool busy = this->weaponRecoveryBusy();
    bool trigger = false;
    bool snapped = false;
    // registre si non calcule pendant le delai apres tir : pointeur vers l'avion de la cible, positif en pratique
    int quality = 1;
    if (burst_remaining > 0 && last_fired_weapon == 0x800) {
        // rafale en cours : gachette sans remettre +0x109 a l'heure
        burst_remaining--;
        pilot->Fire(0x800, air_target);
        pilot->CmdGuidance(direction);
        return true;
    }
    if (!busy) {
        weapon_mask = this->selectWeaponMask();
        if (weapon_mask != last_weapon_mask) {
            printf("AI %s#%d weapon_mask=0x%X\n", owner->actor_name.c_str(), owner->actor_id, weapon_mask);
            last_weapon_mask = weapon_mask;
        }
        snapped = this->gunSnap(air_target);
        quality = this->computeFireSolutionQuality();
        fire_solution_quality = quality;
        if (weapon_mask == 0x800) {
            if (snapped && quality > 5) {
                quality = 10;
            }
            if (quality >= 2 && this->reactionThreshold(quality)) {
                burst_remaining = (uint8_t) (((std::rand() & 3) + 4) * quality) / 10;
                if (burst_remaining > 1) {
                    trigger = true;
                } else {
                    burst_remaining = 0;
                }
            }
        } else if (quality > 0) {
            // AI_FireWeaponTrigger : point d'emport compatible engage
            RSEntity *missile = this->loadedMissile(weapon_mask);
            if (missile != nullptr) {
                if (station_tracked == air_target) {
                    trigger = this->testMissileLock(missile);
                } else {
                    // bit 6 de +0x1B : demande de suivi, pas de tir ce tick
                    station_tracked = air_target;
                    printf("AI %s#%d seeker tracking target=%s weapon_id=%d aspec=%d cone=%d range=%u\n", owner->actor_name.c_str(), owner->actor_id, air_target->actor_name.c_str(), missile->wdat->weapon_id, missile->wdat->weapon_aspec, missile->wdat->tracking_cone, missile->wdat->target_range);
                }
            }
        }
    }
    if (debug_ticks % 25 == 0) {
        RSIntel &intel = owner->mission->intel;
        printf("AI %s#%d selector target=%s d=%.0f ahead=%.0f busy=%d elapsed=%.1f last_fired=0x%X loaded=0x%X missiles_allowed=%d mask=0x%X quality=%d snapped=%d tracked=%s trigger=%d (gun<%.0f long>%.0f short<%.0f)\n",
               owner->actor_name.c_str(), owner->actor_id, air_target->actor_name.c_str(), direction.Length(), owner->plane->forward.AngleBetween(direction),
               busy ? 1 : 0, retarget.clock - (float) last_fire_second, last_fired_weapon, this->loadedWeaponMask(), intel.status_flag ? 1 : 0,
               busy ? 0 : weapon_mask, quality, snapped ? 1 : 0, station_tracked != nullptr ? station_tracked->actor_name.c_str() : "none", trigger ? 1 : 0,
               (float) intel.range_gun, (float) intel.range_medium, (float) intel.range_long);
    }
    if (trigger) {
        last_fired_weapon = weapon_mask;
        last_fire_second = (int) floorf(retarget.clock);
        pilot->Fire(weapon_mask, air_target);
        printf("AI %s#%d fire weapon=0x%X quality=%d burst=%d\n", owner->actor_name.c_str(), owner->actor_id, weapon_mask, quality, burst_remaining);
    }
    if (!trigger && quality <= 0) {
        return false;
    }
    // AI_GuidanceSolution_Major(D, W = AI_Sensor_WeaponVelocityCache = le nez) : poursuite pure
    if (!snapped) {
        pilot->CmdGuidance(direction);
    }
    return true;
}

void SCAIBrain::resetGroundAttack(bool finished) {
    bool running = ground.target != nullptr;
    if (ground.phase == 3) {
        owner->pilot->disengageAutopilot();
    }
    ground.phase = 0;
    ground.weapon = nullptr;
    ground.released = nullptr;
    ground.target = nullptr;
    ground_attack_active = false;
    // le noeud ID19 lance par le tournoi se termine par endManeuver
    if (running && maneuver.id != 19) {
        this->endBehavior(BEHAVIOR_GROUND_ATTACK, finished);
    }
}

// AIEntity_OnBehaviorEnded_A1DF : methode +0x1C de l'entite, fin (1) ou abandon (0) du comportement en cours
void SCAIBrain::onBehaviorEnded(bool finished) {
    SCPilot *pilot = owner->pilot;
    pilot->CmdGearUp();
    pilot->disengageAutopilot();
    if (finished) {
        this->acquireBestThreat(false);
    }
    if (formation.active && reaction_level != REACT_NONE) {
        this->followAllyExec(this->followLeader());
    }
    // objet +0x59 = 0 : la physique reprend
    pilot->CmdKinematic(false, owner->plane->worldVelocity(), owner->plane->forward, pilot->NosePitch());
}

float SCAIBrain::headingDelta(Vector3D direction) {
    return std::fabs(signed180(owner->plane->forward.Azimuth() - direction.Azimuth()));
}

// methode +0x14 du modele d'arme : BombModel_TestGuidedLockCone_41735 (GBU-15), loc_42F71 (MISS, AGM-65D)
bool SCAIBrain::guidedWeaponLock(RSEntity *weapon, SCMissionActors *target) {
    Vector3D to_target = target->object->position - owner->plane->position;
    float distance = to_target.Length();
    if (weapon->entity_type != EntityType::bomb) {
        return distance < (float) weapon->wdat->effective_range;
    }
    if (weapon->bomb_guided == 0) {
        return false;
    }
    float speed = owner->plane->worldVelocity().Length();
    float flight_time = speed != 0.0f ? distance / speed : 0.0f;
    float cone = cosf(degreeToRad(fmodf((float) weapon->bomb_lock_cone_rate * flight_time, 360.0f)));
    to_target.Normalize();
    return to_target.DotProduct(&owner->plane->forward) > cone;
}

RSEntity *SCAIBrain::selectGroundWeapon() {
    static const int order[] = {ID_AGM65D, ID_GBU15, ID_MK20, ID_MK82, ID_DURANDAL, ID_LAU3};
    for (int weapon_id : order) {
        for (auto weap : owner->plane->weaps_load) {
            if (weap != nullptr && weap->nb_weap > 0 && weap->objct->wdat->weapon_id == weapon_id) {
                return weap->objct;
            }
        }
    }
    return nullptr;
}

Vector3D SCAIBrain::predictBombImpact(RSEntity *bomb) {
    GunSimulatedObject *simulated = new GunSimulatedObject();
    Vector3D initial_thrust = owner->plane->getWeaponIntialVector(1.0f);
    simulated->obj = bomb;
    simulated->x = owner->plane->x;
    simulated->y = owner->plane->y;
    simulated->z = owner->plane->z;
    simulated->vx = initial_thrust.x;
    simulated->vy = initial_thrust.y;
    simulated->vz = initial_thrust.z;
    simulated->weight = bomb->weight_in_kg;
    simulated->azimuthf = owner->plane->yaw;
    simulated->elevationf = owner->plane->pitch;
    simulated->target = nullptr;
    simulated->mission = owner->mission;
    Vector3D impact{0.0f, 0.0f, 0.0f};
    Vector3D velocity{0.0f, 0.0f, 0.0f};
    std::tie(impact, velocity) = simulated->ComputeTrajectoryUntilGround(owner->plane->tps);
    delete simulated;
    return impact;
}

void SCAIBrain::updateGroundAttack(SCMissionActors *target) {
    bool is_ground = target != nullptr && !target->is_destroyed && target->object != nullptr && target->object->entity != nullptr && target->object->entity->target_type == 2;
    if (!is_ground || reaction_level == REACT_MISSILE || owner->plane->on_ground) {
        this->resetGroundAttack(false);
        return;
    }
    if (ground.target != target) {
        this->resetGroundAttack(false);
        // GroundAttack_CanEngage_77000 : une arme sol chargee (id 3 a 8), sinon GOAL suivant
        if (this->selectGroundWeapon() == nullptr) {
            return;
        }
        ground.target = target;
        // GroundAttack_Start_7709A ; lance par le tournoi, c'est la manoeuvre 19 qui est empilee
        if (maneuver.id != 19) {
            this->pushBehavior(BEHAVIOR_GROUND_ATTACK);
        }
    }
    Vector3D own_position = owner->plane->position;
    if (debug_ticks != ground.last_tick + 1) {
        ground.last_position = own_position;
    }
    ground.last_tick = debug_ticks;
    Vector3D own_velocity = (own_position - ground.last_position) * (1.0f / TICK_DURATION);
    float own_speed = own_velocity.Length();
    ground.last_position = own_position;
    Vector3D target_position = target->object->position;
    // la cible au sol (+0x283) ne remplace pas la cible de mission (+0x137)
    owner->current_target = target->actor_id;
    owner->pilot->target_waypoint = target_position;
    Vector3D aim_point = target_position;
    aim_point.y += 1000.0f;
    Vector3D away = own_position - aim_point;
    float horizontal_distance = sqrtf(away.x * away.x + away.z * away.z);
    float angle = this->headingDelta(away);
    int previous_phase = ground.phase;
    float heading_error = 0.0f;
    float pitch_error = 0.0f;
    float ground_y = owner->plane->groundlevel;
    float nose_elevation = owner->pilot->NosePitch();

    if (ground.phase != 3) {
        owner->pilot->disengageAutopilot();
    }
    if (ground.phase == 4) {
        if (ground.released == nullptr) {
            for (auto weapon : owner->plane->weaps_object) {
                if ((weapon->obj == ground.weapon || ground.weapon->wdat->weapon_id == ID_LAU3) && std::find(ground.known_objects.begin(), ground.known_objects.end(), weapon) == ground.known_objects.end()) {
                    ground.released = weapon;
                    printf("AI %s#%d bomb released weapon=%d\n", owner->actor_name.c_str(), owner->actor_id, ground.weapon->wdat->weapon_id);
                    break;
                }
            }
            if (ground.released == nullptr && --ground.release_wait <= 0) {
                printf("AI %s#%d bomb release not seen, attack restarts\n", owner->actor_name.c_str(), owner->actor_id);
                this->resetGroundAttack(false);
                return;
            }
        } else if (std::find(owner->plane->weaps_object.begin(), owner->plane->weaps_object.end(), ground.released) == owner->plane->weaps_object.end()) {
            this->resetGroundAttack(true);
            return;
        }
        owner->pilot->SetPitchCommand(5.0f, 5.0f);
        owner->pilot->target_speed_ms = (float) owner->object->entity->jdyn->ai_speed_cruise;
        ground_attack_active = true;
        return;
    }

    bool wings_level = false;
    if (ground.phase <= 1) {
        bool aligned = angle > 170.0f;
        bool close = horizontal_distance < 5000.0f;
        if (!aligned && close) {
            ground.phase = 0;
        } else if (aligned && close) {
            wings_level = true;
            owner->pilot->BeginManual();
            owner->pilot->CmdThrottle(5);
            if (owner->pilot->CmdRollTo(0.0f, 5.0f)) {
                owner->pilot->CmdThrottle(3);
                ground.phase = 2;
            }
        } else if (horizontal_distance > 8000.0f || aligned) {
            ground.phase = 1;
        }
    }

    if (ground.phase == 2) {
        Vector3D autopilot_velocity = aim_point - own_position;
        autopilot_velocity.Normalize();
        owner->pilot->engageAutopilot(aim_point, autopilot_velocity * 100.0f);
        ground.autopilot_target = target_position;
        ground.phase3_time = 0.0f;
        ground.phase = 3;
    } else if (ground.phase == 3) {
        if (target_position.x != ground.autopilot_target.x || target_position.y != ground.autopilot_target.y || target_position.z != ground.autopilot_target.z) {
            Vector3D autopilot_velocity = aim_point - own_position;
            autopilot_velocity.Normalize();
            owner->pilot->setAutopilotTarget(aim_point, autopilot_velocity * 100.0f);
            ground.autopilot_target = target_position;
        }
    } else if (!wings_level) {
        Vector3D direction = ground.phase == 0 ? away : -away;
        if (ground.phase == 0) {
            direction.y = 0.0f;
        }
        this->computeAttitudeError(direction, heading_error, pitch_error);
        if (angle > 169.0f && horizontal_distance > 9000.0f) {
            owner->pilot->SetPitchCommand(0.0f, 5.0f);
        } else if (ground.phase == 1 && angle > 169.0f && own_position.y - aim_point.y > 2000.0f) {
            owner->pilot->SetPitchCommand(-40.0f, 5.0f);
        } else {
            owner->pilot->SetGuidanceDirection(direction);
        }
    }
    owner->pilot->target_speed_ms = (float) owner->object->entity->jdyn->ai_speed_cruise;
    ground_attack_active = true;

    if (ground.phase == 3) {
        if (ground.weapon == nullptr) {
            ground.weapon = this->selectGroundWeapon();
        }
        // WeaponStation_FindLoadedCompatible : plus d'arme -> phase 6, fin de l'attaque
        bool loaded = false;
        for (auto weap : owner->plane->weaps_load) {
            if (weap != nullptr && weap->nb_weap > 0 && weap->objct == ground.weapon) {
                loaded = true;
            }
        }
        if (!loaded) {
            this->resetGroundAttack(true);
            return;
        }
        if (ground.weapon->wdat->weapon_id == ID_AGM65D || ground.weapon->wdat->weapon_id == ID_GBU15) {
            if (this->guidedWeaponLock(ground.weapon, target)) {
                ground.known_objects = owner->plane->weaps_object;
                owner->pilot->Fire(1 << (ground.weapon->wdat->weapon_id - 1), target);
                owner->pilot->disengageAutopilot();
                ground.released = nullptr;
                ground.release_wait = 25;
                ground.phase = 4;
                printf("AI %s#%d guided weapon=%d fired d=%.0f\n", owner->actor_name.c_str(), owner->actor_id, ground.weapon->wdat->weapon_id, (target_position - own_position).Length());
            } else if (owner->pilot->autopilotReached()) {
                owner->pilot->disengageAutopilot();
                ground.phase = 0;
            }
            return;
        }
        if (ground.weapon->wdat->weapon_id == ID_LAU3) {
            ground.phase3_time += TICK_DURATION;
            if (ground.phase3_time >= 3.0f) {
                ground.known_objects = owner->plane->weaps_object;
                owner->pilot->Fire(1 << (ground.weapon->wdat->weapon_id - 1), target);
                owner->pilot->disengageAutopilot();
                ground.released = nullptr;
                ground.release_wait = 25;
                ground.phase = 4;
                printf("AI %s#%d rockets fired d=%.0f own_y=%.0f\n", owner->actor_name.c_str(), owner->actor_id, horizontal_distance, own_position.y);
            } else if (owner->pilot->autopilotReached()) {
                owner->pilot->disengageAutopilot();
                ground.phase = 0;
            }
            return;
        }
        Vector3D impact;
        if (ground.weapon->wdat->weapon_id == ID_DURANDAL) {
            // BombModel_PredictImpact_41311, n = 5 : 2500 m devant, direction horizontale de la vitesse
            Vector3D ahead = own_velocity;
            ahead.y = 0.0f;
            ahead.Normalize();
            impact = own_position + ahead * 2500.0f;
        } else {
            impact = this->predictBombImpact(ground.weapon);
        }
        float drop_height = own_position.y - target_position.y;
        float descent_speed = -own_velocity.y;
        float ballistic_time = drop_height > 0.0f ? (-descent_speed + sqrtf(descent_speed * descent_speed + 2.0f * 9.8f * drop_height)) / 9.8f : 0.0f;
        float ballistic_range = sqrtf(own_velocity.x * own_velocity.x + own_velocity.z * own_velocity.z) * ballistic_time;
        float miss = sqrtf((impact.x - target_position.x) * (impact.x - target_position.x) + (impact.z - target_position.z) * (impact.z - target_position.z));
        float tolerance = 20.0f + own_speed * TICK_DURATION;
        if ((std::rand() & 0xF) > owner->profile->ai.atrb.AG) {
            tolerance += 150.0f;
        }
        if (miss <= tolerance) {
            ground.known_objects = owner->plane->weaps_object;
            owner->pilot->Fire(1 << (ground.weapon->wdat->weapon_id - 1), target);
            owner->pilot->disengageAutopilot();
            ground.released = nullptr;
            ground.release_wait = 25;
            ground.phase = 4;
            printf("AI %s#%d bomb release requested weapon=%d miss=%.0f tolerance=%.0f d=%.0f own_y=%.0f speed=%.0f\n", owner->actor_name.c_str(), owner->actor_id, ground.weapon->wdat->weapon_id, miss, tolerance, horizontal_distance, own_position.y, own_speed);
        } else if (owner->pilot->autopilotReached()) {
            owner->pilot->disengageAutopilot();
            ground.phase = 0;
        }
        if (debug_ticks % 25 == 0) {
            printf("AI %s#%d ground attack phase=%d target=%s d=%.0f angle=%.0f miss=%.0f tolerance=%.0f impact=(%.0f,%.0f) impact_y=%.0f target_y=%.0f range=%.0f ballistic_range=%.0f own_y=%.0f speed=%.0f autopilot=%d reached=%d\n", owner->actor_name.c_str(), owner->actor_id, ground.phase, target->actor_name.c_str(), horizontal_distance, angle, miss, tolerance, impact.x, impact.z, impact.y, target_position.y, sqrtf((impact.x - own_position.x) * (impact.x - own_position.x) + (impact.z - own_position.z) * (impact.z - own_position.z)), ballistic_range, own_position.y, own_speed, owner->pilot->autopilotActive() ? 1 : 0, owner->pilot->autopilotReached() ? 1 : 0);
        }
    } else if (debug_ticks % 25 == 0) {
        printf("AI %s#%d ground attack phase=%d target=%s d=%.0f angle=%.0f heading_err=%.1f pitch_err=%.1f own_y=%.0f speed=%.0f\n", owner->actor_name.c_str(), owner->actor_id, ground.phase, target->actor_name.c_str(), horizontal_distance, angle, heading_error, pitch_error, own_position.y, own_speed);
    }
    if (ground.phase != previous_phase) {
        printf("AI %s#%d ground attack phase %d -> %d\n", owner->actor_name.c_str(), owner->actor_id, previous_phase, ground.phase);
    }
}

int SCAIBrain::missileDistanceBand() {
    Vector3D missile_position = {missile_threat->x, missile_threat->y, missile_threat->z};
    float distance = (missile_position - owner->plane->position).Length();
    int weapon_id = missile_threat->obj->wdat->weapon_id;
    int missile_mask = weapon_id >= 1 ? (1 << (weapon_id - 1)) : 0;
    RSIntel &intel = owner->mission->intel;
    if ((missile_mask & 0x700) != 0) {
        if (distance < intel.range_medium) {
            return 1;
        }
        return distance < intel.range_far ? 2 : 3;
    }
    if ((missile_mask & 0x3) != 0) {
        if (distance < intel.range_close) {
            return 1;
        }
        return distance < intel.range_long ? 2 : 3;
    }
    return 0;
}

// AI_MissileEvasionReaction_9A77
bool SCAIBrain::reactToMissile() {
    bool aims_me = missile_threat != nullptr && missile_threat->alive && missile_threat->target == owner;
    if (!aims_me) {
        if (reaction_level == REACT_MISSILE) {
            reaction_level = REACT_NONE;
        }
        missile_threat = nullptr;
        return false;
    }
    if (reaction_level != REACT_MISSILE) {
        return false;
    }
    int band = this->missileDistanceBand();
    if (band == 0) {
        return false;
    }
    Vector3D own_position = owner->plane->position;
    Vector3D missile_position = {missile_threat->x, missile_threat->y, missile_threat->z};
    Vector3D to_missile = missile_position - own_position;
    float floor = this->floorAltitude();
    to_missile.y = own_position.y <= floor ? 2.0f * floor - own_position.y : floor - own_position.y;
    Vector3D escape = -to_missile;
    if (band != 3) {
        SCMissionActors *player = this->playerActor();
        if (missile_threat != complained_missile && missile_threat->shooter != nullptr && missile_threat->shooter == player && player->team_id == owner->team_id) {
            owner->setMessage(0x0E);
            complained_missile = missile_threat;
        }
        this->missileCountermeasures();
        escape = {to_missile.z, 0.0f, -to_missile.x};
        Vector3D forward_horizontal = {owner->plane->forward.x, 0.0f, owner->plane->forward.z};
        if (escape.DotProduct(&forward_horizontal) < 0.0f) {
            escape = -escape;
        }
        escape.y = band == 1 ? 0.0f : to_missile.y;
    }
    owner->pilot->BeginManual();
    bool aligned = owner->pilot->CmdGuidance(escape);
    JDYN *jdyn = owner->object->entity->jdyn;
    this->speedThrottle(aligned ? (float) jdyn->ai_speed_max : (float) jdyn->ai_speed_cruise);
    if (debug_ticks % 25 == 0) {
        printf("AI %s#%d evade band=%d missile_d=%.0f\n", owner->actor_name.c_str(), owner->actor_id, band, (missile_position - own_position).Length());
    }
    return true;
}

void SCAIBrain::missileCountermeasures() {
    bool aims_me = missile_threat != nullptr && missile_threat->alive && missile_threat->target == owner;
    if (!aims_me) {
        if (reaction_level == REACT_MISSILE) {
            reaction_level = REACT_NONE;
        }
        missile_threat = nullptr;
        return;
    }
    last_fired_weapon = 0;
    int active_decoys = 0;
    for (auto actor : owner->mission->actors) {
        if (actor->plane == nullptr) {
            continue;
        }
        for (auto weapon : actor->plane->weaps_object) {
            if (weapon != nullptr && weapon->alive && dynamic_cast<DumbSimulatedObject *>(weapon) != nullptr) {
                active_decoys++;
            }
        }
    }
    if (active_decoys < 20 && this->skillCheck(owner->profile->ai.atrb.FL, 0) && !this->weaponRecoveryBusy()) {
        int weapon_id = missile_threat->obj->wdat->weapon_id;
        int missile_mask = weapon_id >= 1 ? (1 << (weapon_id - 1)) : 0;
        bool chaff = (missile_mask & 0x700) != 0;
        owner->plane->DeployCountermeasure(chaff);
        this->decide(chaff ? "leurre : paillettes" : "leurre : fusees");
    }
    last_fire_second = (int) floorf(retarget.clock);
}

void SCAIBrain::computeAttitudeError(Vector3D direction, float &heading_error, float &pitch_error) {
    heading_error = -signed1800(SCPilot::YawOf(direction) - owner->plane->yaw) / 10.0f;
    pitch_error = direction.Elevation() - owner->pilot->NosePitch();
}

Vector3D SCAIBrain::runwayAxis(Vector3D direction) {
    const float zero = 1.0f / 256.0f;
    if (fabsf(direction.x) < zero && fabsf(direction.z) >= zero) {
        return Vector3D(0.0f, 0.0f, direction.z > 0.0f ? 1.0f : -1.0f);
    }
    if (fabsf(direction.z) < zero && fabsf(direction.x) >= zero) {
        return Vector3D(direction.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
    }
    printf("AI %s#%d runway not axial (%.3f, %.3f), using the nose direction\n", owner->actor_name.c_str(), owner->actor_id, direction.x, direction.z);
    Vector3D horizontal(direction.x, 0.0f, direction.z);
    horizontal.Normalize();
    return horizontal;
}

void SCAIBrain::endGroundOp() {
    owner->pilot->EndGroundOps();
    BehaviorKind kind = ground_ops.kind == GROUND_OP_TAKEOFF ? BEHAVIOR_TAKEOFF : BEHAVIOR_LANDING;
    ground_ops.kind = GROUND_OP_NONE;
    this->endBehavior(kind, true);
}

bool SCAIBrain::takeoffOrder() {
    SCPlane *plane = owner->plane;
    SCPilot *pilot = owner->pilot;
    RSEntity *entity = plane->object->entity;
    if (ground_ops.kind != GROUND_OP_TAKEOFF) {
        if (owner->taken_off || plane->worldVelocity().Length() > 10.0f) {
            owner->taken_off = true;
            return true;
        }
        ground_ops.kind = GROUND_OP_TAKEOFF;
        this->pushBehavior(BEHAVIOR_TAKEOFF);
        ground_ops.phase = 0;
        ground_ops.time = 0.0f;
        ground_ops.stick = 0.0f;
        ground_ops.landing_done = false;
        ground_ops.axis = this->runwayAxis(plane->forward);
        pilot->BeginGroundOps();
        printf("AI %s#%d takeoff accel=%d rotate=%d pitch=%d gain=%d\n", owner->actor_name.c_str(), owner->actor_id, entity->takeoff_roll_accel, entity->takeoff_rotate_speed, entity->takeoff_climb_pitch, entity->takeoff_pitch_gain);
    }
    switch (ground_ops.phase) {
        case 0: {
            pilot->CmdGroundControls(0.0f, 10, plane->GetFlaps(), 1, 0);
            ground_ops.time += TICK_DURATION;
            float speed = entity->takeoff_roll_accel * ground_ops.time;
            if (speed > entity->takeoff_rotate_speed) {
                pilot->CmdKinematic(false, ground_ops.axis * speed, ground_ops.axis, 0.0f);
                ground_ops.phase = 1;
            } else {
                pilot->CmdKinematic(true, ground_ops.axis * speed, ground_ops.axis, 0.0f);
            }
            break;
        }
        case 1: {
            if (plane->y - plane->groundlevel > 300.0f) {
                pilot->CmdGroundControls(ground_ops.stick, 5, 1, 1, 0);
                ground_ops.phase = 2;
                break;
            }
            // AI_PitchAttitudeHold_126CC
            float gain = (float) entity->takeoff_pitch_gain;
            float error = (float) entity->takeoff_climb_pitch - pilot->NosePitch();
            ground_ops.stick = floorf((std::clamp)(error * gain / 8.0f, -gain, gain));
            pilot->CmdGroundControls(ground_ops.stick, 10, 1, 1, 0);
            break;
        }
        case 2:
            pilot->CmdGroundControls(ground_ops.stick, 5, 0, 0, 0);
            ground_ops.phase = 3;
            break;
        case 3:
            if (pilot->NosePitch() > 17.0f) {
                ground_ops.stick = -16.0f;
            } else {
                ground_ops.stick = 8.0f;
                ground_ops.phase = 4;
            }
            pilot->CmdGroundControls(ground_ops.stick, 5, 0, 0, 0);
            break;
        default:
            this->endGroundOp();
            owner->taken_off = true;
            printf("AI %s#%d takeoff done\n", owner->actor_name.c_str(), owner->actor_id);
            return true;
    }
    return false;
}

bool SCAIBrain::landingOrder(uint8_t approach_spot, uint8_t touchdown_spot) {
    if (ground_ops.landing_done) {
        return true;
    }
    SCPlane *plane = owner->plane;
    SCPilot *pilot = owner->pilot;
    RSEntity *entity = plane->object->entity;
    std::vector<SPOT *> &spots = owner->mission->mission->mission_data.spots;
    if (ground_ops.kind != GROUND_OP_LANDING) {
        // Player_ResolveAttachPointN_5305A : index hors table -> (0, 0, 0)
        Vector3D approach(0.0f, 0.0f, 0.0f);
        Vector3D touchdown(0.0f, 0.0f, 0.0f);
        if (approach_spot < spots.size()) {
            approach = spots[approach_spot]->position;
        }
        if (touchdown_spot < spots.size()) {
            touchdown = spots[touchdown_spot]->position;
        }
        // LandingBehavior_Start_75746 : l'IA est teleportee au point d'approche
        ground_ops.origin = approach;
        // Landing_Phase1_SetupApproach_75D51
        ground_ops.landing_target = touchdown + Vector3D(0.0f, (float) entity->landing_aim_height, 0.0f);
        Vector3D delta = ground_ops.landing_target - ground_ops.origin;
        ground_ops.axis = this->runwayAxis(delta);
        ground_ops.landing_speed = (float) entity->landing_speed;
        ground_ops.landing_duration = delta.Length() / ground_ops.landing_speed;
        ground_ops.landing_dir = delta;
        ground_ops.landing_dir.Normalize();
        ground_ops.landing_counter = 0;
        ground_ops.landing_pitch = 0.0f;
        ground_ops.landing_leveled = false;
        ground_ops.time = 0.0f;
        ground_ops.kind = GROUND_OP_LANDING;
        this->pushBehavior(BEHAVIOR_LANDING);
        ground_ops.phase = 2;
        pilot->BeginGroundOps();
        pilot->CmdGroundControls(0.0f, plane->GetThrottle() / 10, plane->GetFlaps(), 1, 0);
        pilot->CmdPlaceAt(ground_ops.origin, ground_ops.axis, 0.0f);
        pilot->target_waypoint = ground_ops.landing_target;
        printf("AI %s#%d landing approach=%d touchdown=%d speed=%d aim=%d steps=%d\n", owner->actor_name.c_str(), owner->actor_id, approach_spot, touchdown_spot, entity->landing_speed, entity->landing_aim_height, entity->landing_pitch_steps);
        return false;
    }
    switch (ground_ops.phase) {
        case 2: {
            // Landing_Phase2_Approach_76325
            ground_ops.time += TICK_DURATION;
            ground_ops.landing_counter -= 1;
            if (ground_ops.landing_counter < entity->landing_pitch_steps) {
                ground_ops.landing_counter = entity->landing_pitch_steps;
            } else {
                ground_ops.landing_pitch += 1.0f;
            }
            pilot->CmdKinematic(true, ground_ops.landing_dir * ground_ops.landing_speed, ground_ops.axis, ground_ops.landing_pitch);
            if (ground_ops.time >= ground_ops.landing_duration) {
                ground_ops.time = 0.0f;
                ground_ops.phase = 3;
            }
            break;
        }
        case 3: {
            // Landing_Phase3_TouchdownRoll_765B2
            if (ground_ops.time == 0.0f) {
                pilot->CmdPlaceAt(ground_ops.landing_target, ground_ops.axis, ground_ops.landing_pitch);
            }
            ground_ops.landing_counter += 1;
            if (ground_ops.landing_counter >= entity->landing_pitch_steps / 6) {
                ground_ops.landing_counter = 0;
            } else {
                ground_ops.landing_pitch -= 1.0f;
            }
            ground_ops.time += TICK_DURATION;
            if (ground_ops.landing_counter == 0 && !ground_ops.landing_leveled) {
                ground_ops.landing_pitch = 0.0f;
                ground_ops.landing_leveled = true;
                ground_ops.time = 0.0f;
                ground_ops.phase = 4;
            }
            pilot->CmdKinematic(true, ground_ops.axis * ground_ops.landing_speed, ground_ops.axis, ground_ops.landing_pitch);
            break;
        }
        case 4: {
            // Landing_Phase4_Braking_76C09 : la vitesse baisse par secondes entieres
            ground_ops.time += TICK_DURATION;
            int speed = (int) ground_ops.landing_speed - 2 * (int) ground_ops.time;
            pilot->CmdKinematic(true, ground_ops.axis * (float) speed, ground_ops.axis, 0.0f);
            if (speed <= 0) {
                ground_ops.phase = 5;
            }
            break;
        }
        default: {
            // Landing_Phase5_Stop_76E67
            Vector3D parked(plane->x, plane->groundlevel, plane->z);
            pilot->CmdGroundControls(0.0f, -1, 0, 1, 0);
            pilot->CmdPlaceAt(parked, ground_ops.axis, 0.0f);
            ground_ops.landing_done = true;
            this->endGroundOp();
            printf("AI %s#%d landed\n", owner->actor_name.c_str(), owner->actor_id);
            return true;
        }
    }
    return false;
}

// Goal_SetObjective_A307 : cas 0xAA (etat d'ailier +0x149 = 0) si l'ordre a ete pose, puis queue commune :
// ordre autre que 0xAA et formation tenue -> Goal_FollowAllyExec (sortie de formation)
void SCAIBrain::onObjectiveSet(bool assigned) {
    if (assigned && owner->current_command == OP_SET_OBJ_FOLLOW_ALLY) {
        formation.leader_state = 0;
    }
    // cas par defaut (0xBE) : +0x10F vide, +0x111 = ma position, objectif 0xBF
    if (assigned && owner->current_command == OP_SET_OBJ_BE) {
        nav.reference = nullptr;
        if (owner->plane != nullptr) {
            nav.center = owner->plane->position;
        }
    }
    if (owner->current_command != OP_SET_OBJ_FOLLOW_ALLY && formation.active) {
        this->followAllyExec(this->followLeader());
    }
}

SCMissionActors *SCAIBrain::followLeader() {
    if (owner->current_command != OP_SET_OBJ_FOLLOW_ALLY) {
        formation.leader = nullptr;
        formation.leader_arg = 0xFF;
        return nullptr;
    }
    if (owner->current_command_arg != formation.leader_arg) {
        formation.leader_arg = owner->current_command_arg;
        formation.leader = this->leaderActor();
    }
    return formation.leader;
}

int SCAIBrain::escortQueryLeader(SCMissionActors *leader, SCMissionActors *&engage) {
    // Escort_QueryLeaderValid : d'apres la reaction en cours du leader
    SCAIBrain *lead = leader->brain;
    int result = 0;
    if (lead->reaction_level == REACT_ENGAGED && lead->air_target != nullptr) {
        engage = lead->air_target;
        result = 3;
    } else if (lead->reaction_level == REACT_MISSILE && lead->missile_threat != nullptr) {
        SCMissionActors *shooter = lead->missile_threat->shooter;
        if (shooter != nullptr && shooter->plane != nullptr) {
            engage = shooter;
            result = 3;
        }
    }
    lead->escort_leader_free = result == 0;
    return result;
}

bool SCAIBrain::followAllyExec(SCMissionActors *leader) {
    // Goal_FollowAllyExec
    bool held_elsewhere = ground_ops.kind != GROUND_OP_NONE || ground_ops.landing_done;
    bool may_hold = formation.active || !held_elsewhere;
    if (leader != nullptr && leader->plane != nullptr && leader->plane->ejected) {
        formation.leader = nullptr;
        leader = nullptr;
    }
    bool result = false;
    if (leader != nullptr && leader->plane != nullptr && owner->current_command == OP_SET_OBJ_FOLLOW_ALLY && may_hold &&
        formation.leader_state == 0 && !owner->plane->ejected && !leader->plane->on_ground && reaction_level == REACT_NONE) {
        if (leader->brain != nullptr) {
            SCMissionActors *engage = nullptr;
            formation.leader_state = (uint8_t) this->escortQueryLeader(leader, engage);
            if (formation.leader_state == 3) {
                if (engage != owner->target) {
                    printf("AI %s#%d switching engage target from %s#%d to %s#%d\n", owner->actor_name.c_str(), owner->actor_id,
                           owner->target ? owner->target->actor_name.c_str() : "none", owner->target ? owner->target->actor_id : -1,
                           engage->actor_name.c_str(), engage->actor_id);
                }
                owner->target = engage;
                air_target = engage;
                engage_target = engage;
            } else {
                result = this->formationGuidance(leader);
            }
        } else {
            result = this->formationGuidance(leader);
        }
    }
    if (formation.active && !result) {
        formation.history_ready = false;
        owner->pilot->EndGroundOps();
        owner->pilot->CmdKinematic(false, owner->plane->worldVelocity(), owner->plane->forward, owner->pilot->NosePitch());
    }
    formation.active = result;
    return result;
}

Vector3D SCAIBrain::formationSlot(SCMissionActors *leader) {
    // Goal_FollowAllyFormation : poste = cote * S + avant * N + haut * U dans le repere du leader
    SCPlane *lead = leader->plane;
    Vector3D nose = lead->forward;
    Vector3D slot = owner->follow_slot;
    SCMissionActors *player = this->playerActor();
    bool player_side = player != nullptr && owner->team_id == player->team_id;
    if (owner->profile->radi.spch != 9 && player_side && !mood.enemies_active) {
        slot = Vector3D(300.0f, -200.0f, 50.0f);
    }
    Vector3D side(nose.z, 0.0f, -nose.x);
    side.Normalize();
    Vector3D up = side.CrossProduct(&nose);
    if (up.y < 0.0f) {
        up = up * -1.0f;
    }
    Vector3D rel = owner->plane->position - lead->position;
    float lateral = fabsf(slot.x);
    if (rel.x * side.x + rel.z * side.z < 0.0f) {
        lateral = -lateral;
    }
    Vector3D offset = side * lateral + nose * slot.y + up * slot.z;
    float leader_height = lead->y - lead->groundlevel;
    if (leader_height + offset.y < 500.0f) {
        offset.y = 500.0f;
    }
    return offset;
}

bool SCAIBrain::formationGuidance(SCMissionActors *leader) {
    // Formation_GuidanceSolution : l'ailier est deplace en cinematique (physique suspendue, objet +0x59)
    SCPlane *plane = owner->plane;
    SCPlane *lead = leader->plane;
    const float dt = TICK_DURATION;
    bool active = formation.active;
    Vector3D lead_nose = lead->forward;
    Vector3D own_nose = plane->forward;
    Vector3D lead_span(lead->ptw.v[0][0], lead->ptw.v[0][1], lead->ptw.v[0][2]);
    Vector3D own_span(plane->ptw.v[0][0], plane->ptw.v[0][1], plane->ptw.v[0][2]);
    float step = lead->worldVelocity().Length() * dt;
    Vector3D offset = this->formationSlot(leader);
    float offset_length = offset.Length();
    Vector3D to_slot = lead->position + offset - plane->position;
    float slot_distance = to_slot.Length();
    Vector3D own_flat(own_nose.x, 0.0f, own_nose.z);
    Vector3D slot_flat(to_slot.x, 0.0f, to_slot.z);
    bool behind = own_flat.AngleBetween(slot_flat) > 160.0f;
    if (slot_distance > 4.0f * offset_length) {
        active = false;
    } else if (!active) {
        Vector3D lead_flat(lead_nose.x, 0.0f, lead_nose.z);
        if ((int) lead_flat.AngleBetween(own_flat) < 15) {
            active = true;
        }
    }
    if (!active) {
        return false;
    }
    owner->pilot->target_waypoint = lead->position + offset;
    if (!formation.active || !formation.history_ready) {
        for (int i = 0; i < 32; i++) {
            formation.nose[i] = own_nose;
            formation.span[i] = own_span;
        }
        formation.index = 0;
        formation.clock = 0.0f;
        formation.history_ready = true;
    }
    if (step < slot_distance) {
        float extra = slot_distance - step;
        float cap = (slot_distance > 3000.0f && behind ? 50.0f : 20.0f) * dt;
        step += (std::min)(extra, cap);
    } else {
        step = slot_distance;
    }
    float lead_stick = fabsf(lead->elevator) * 16.0f;
    owner->pilot->BeginGroundOps();
    owner->pilot->CmdGroundControls(0.0f, lead->GetThrottle() / 10, plane->GetFlaps(), 0, 0);
    float correction = step * (behind ? 76.0f / 256.0f : 25.0f / 256.0f);
    step -= correction;
    while (formation.clock >= 0.125f) {
        formation.nose[formation.index] = lead_nose;
        if (lead_stick > 2.0f) {
            formation.span[formation.index] = (formation.span[formation.index] + lead_span) * 0.5f;
        } else {
            Matrix level;
            level.Identity();
            level.rotateM(degreeToRad((-own_nose).Azimuth()), 0, 1, 0);
            formation.span[formation.index] = Vector3D(level.v[0][0], level.v[0][1], level.v[0][2]);
        }
        formation.index = (formation.index + 1) & 31;
        formation.clock -= 0.125f;
    }
    formation.clock += dt;
    Vector3D mean_nose(0.0f, 0.0f, 0.0f);
    Vector3D mean_span(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 32; i++) {
        mean_nose = mean_nose + formation.nose[i];
        mean_span = mean_span + formation.span[i];
    }
    mean_nose = mean_nose * (1.0f / 32.0f);
    mean_span = mean_span * (1.0f / 32.0f);
    Vector3D motion = mean_nose * step;
    Vector3D toward = to_slot - motion;
    toward.Normalize();
    motion = motion + toward * correction;
    // orientation : nez du leader, envergure moyenne
    PlaneKinematicEvent event;
    event.plane = plane;
    event.engaged = true;
    event.velocity = motion * (1.0f / dt);
    SCPilot::AttitudeFromAxes(lead_nose, mean_span, event.yaw, event.pitch, event.roll);
    MessageBus::getInstance().publish(std::make_unique<PlaneKinematicEvent>(event));
    return true;
}

void SCAIBrain::followWaypoints(SCMissionActors *leader) {
    // Goal_FollowWaypoints
    if (this->behaviorRunning()) {
        this->tickBehavior();
        return;
    }
    if (leader == nullptr || leader->plane == nullptr) {
        return;
    }
    if (leader->plane->ejected) {
        formation.leader = nullptr;
        return;
    }
    SCPlane *lead = leader->plane;
    if (lead->y - lead->groundlevel > 333.0f) {
        // noeud entite+0xD5 = ID7 (poursuite du leader)
        this->scoreManeuver(7, leader);
        this->applyManeuver(7, leader, REACT_NONE);
        this->tickManeuver();
        return;
    }
    // noeud entite+0xD1 = ID21 : 1000 m au-dessus du leader, vitesse = nez * 200 + vitesse du leader
    Vector3D point = lead->position + Vector3D(0.0f, 1000.0f, 0.0f);
    Vector3D velocity = lead->forward * 200.0f + lead->worldVelocity();
    this->applyNavigation(point, velocity, 2.0f);
}
