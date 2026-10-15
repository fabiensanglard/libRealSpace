#include "DebugStrike.h"
#include "../../strike_commander/SCAIBrain.h"
#include "../../engine/gametimer.h"
#include <cctype>
#include <cmath>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>
#include <optional>
#include <tuple>

struct imperial_plane_data {
    float LmaxDef{0.0f};
    float LminDef{0.0f};
    float Fmax{0.0f};
    float Fmin{0.0f};
    float ELEVF_CSTE{0.0f};
    float ROLLF_CSTE{0.0f};
    float s{0.0f};
    float W{0.0f};
    float fuel_weight{0.0f};
    float Mthrust{0.0f};
    float b{0.0f};
    float ie_pi_AR{0.0f};
    float MIN_LIFT_SPEED{0.0f};
};

std::unordered_map<std::string, imperial_plane_data> imperial_planes = {
    {"747", {4.0f, -2.0f, 50.0f, 80.0f, 25.0f, 50.0f, 5500.0f, 500000.0f, 100000.0f, 200000.0f, 220.0f, .83f, 200.0f}},
    {"F-18", {9.0f, - 6.0f, 50.0f, 60.0f, 30.0f, 100.0f, 510.0f, 24000.0f, 12000.0f, 32000.0f, 38.0f, .90f, 200.0f}},
    {"F-16DES", {10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f, 23000.0f, 32.0f, .93f, 120.0f}},
    {"F-16GRAY", {10.0f, -7.0f, 40.0f, 40.0f, 30.0f, 100.0f, 390.0f, 18000.0f, 8000.0f, 23000.0f, 32.0f, .93f, 120.0f}},
    {"F-15", {8.0f, -5.0f, 30.0f, 60.0f, 32.0f, 140.0f, 608.0f, 28000.0f, 14000.0f, 46000.0f, 43.0f, .87f, 100.0f}}};

DebugStrike::DebugStrike() : SCStrike() {}

DebugStrike::~DebugStrike() {}

void DebugStrike::setMission(std::string mission_name) { 
    if (this->current_mission != nullptr) {
        this->disablePlayerAI();
    }
    target = nullptr;
    SCStrike::setMission(mission_name.c_str()); 
}

void DebugStrike::init() { SCStrike::init();}
void DebugStrike::runFrame(void) {
    SCMissionActors *player = this->current_mission->player;
    if (player->brain != nullptr && !this->pause_simu && this->camera_mode != View::AUTO_PILOT) {
        player->pilot->plane = this->player_plane;
        this->player_ai_accumulator += GameTimer::getInstance().getDeltaTime();
        if (this->player_ai_accumulator > 1.0f) {
            this->player_ai_accumulator = 1.0f;
        }
        while (this->player_ai_accumulator >= SCAIBrain::TICK_DURATION) {
            this->player_ai_accumulator -= SCAIBrain::TICK_DURATION;
            if (this->bench_active) {
                this->benchTick();
            } else if (this->maneuver_test) {
                this->maneuverTestTick();
            } else {
                this->restoreDebugOrder();
                player->brain->tick();
                this->debug_order_executed = player->current_command_executed;
            }
        }
        player->pilot->FlyTo();
    }
    SCStrike::runFrame();
    this->decisionTracePoll();
}
static std::string actorLabel(SCMissionActors *actor);
// changer d'acteur suivi repart d'une trace vide, pour ne pas melanger deux acteurs
void DebugStrike::decisionSelect(int actor_id) {
    if (actor_id == this->decision_actor_id) {
        return;
    }
    this->decision_actor_id = actor_id;
    this->decision_lines.clear();
    this->decision_last.clear();
    this->decision_time = 0.0f;
}
// acteur suivi par la trace, choisi dans la liste des acteurs de la mission qui ont un cerveau
SCMissionActors *DebugStrike::decisionActor() {
    for (auto actor : this->current_mission->actors) {
        if (actor->actor_id == this->decision_actor_id && actor->brain != nullptr) {
            return actor;
        }
    }
    return nullptr;
}
// le chemin sans les valeurs entre parentheses (distances, caps), qui changent a chaque tick
std::string DebugStrike::decisionKey(const std::string &path) {
    std::string key;
    int depth = 0;
    for (char c : path) {
        if (c == '(') {
            depth++;
        } else if (c == ')') {
            depth--;
        } else if (depth == 0) {
            key += c;
        }
    }
    return key;
}
// une ligne a chaque changement du chemin de decision (le cerveau le reecrit a chaque tick d'IA)
void DebugStrike::decisionTracePoll() {
    if (!this->decision_trace_active || this->pause_simu) {
        return;
    }
    this->decision_time += GameTimer::getInstance().getDeltaTime();
    SCMissionActors *actor = this->decisionActor();
    if (actor == nullptr || actor->plane == nullptr) {
        return;
    }
    std::string path = actor->brain->decision;
    std::string key = DebugStrike::decisionKey(path);
    if (path.empty() || key == this->decision_last) {
        return;
    }
    this->decision_last = key;
    SCAIBrain *brain = actor->brain;
    float reference_distance = -1.0f;
    if (brain->nav.reference != nullptr) {
        Vector3D reference = brain->nav.reference->plane != nullptr ? brain->nav.reference->plane->position : brain->nav.reference->object->position;
        reference_distance = (reference - actor->plane->position).Length();
    }
    float target_distance = -1.0f;
    if (brain->air_target != nullptr && brain->air_target->plane != nullptr) {
        target_distance = (brain->air_target->plane->position - actor->plane->position).Length();
    }
    char line[512];
    snprintf(line, sizeof(line), "t=%7.2f s  %s : %s  | d reference %.0f  d cible air %.0f  alt %.0f", this->decision_time, actor->actor_name.c_str(), path.c_str(),
             reference_distance, target_distance, actor->plane->y);
    this->decision_lines.push_back(line);
    if (this->decision_lines.size() > 2000) {
        this->decision_lines.erase(this->decision_lines.begin());
    }
    printf("DECISION %s\n", line);
}
void DebugStrike::decisionTraceWindow() {
    SCMissionActors *actor = this->decisionActor();
    ImGui::PushItemWidth(260.0f);
    if (ImGui::BeginCombo("Acteur suivi", actor != nullptr ? actorLabel(actor).c_str() : "aucun", 0)) {
        for (auto candidate : this->current_mission->actors) {
            if (candidate->brain == nullptr) {
                continue;
            }
            std::string label = actorLabel(candidate) + (candidate->is_active ? "" : " (inactif)") + (candidate->is_destroyed ? " (detruit)" : "");
            if (ImGui::Selectable(label.c_str(), candidate->actor_id == this->decision_actor_id)) {
                this->decisionSelect(candidate->actor_id);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    actor = this->decisionActor();
    ImGui::Checkbox("Enregistrer", &this->decision_trace_active);
    ImGui::SameLine();
    if (ImGui::Button("Effacer")) {
        this->decision_lines.clear();
        this->decision_last.clear();
        this->decision_time = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Copier")) {
        std::string all;
        for (auto &line : this->decision_lines) {
            all += line + "\n";
        }
        ImGui::SetClipboardText(all.c_str());
    }
    if (actor != nullptr) {
        ImGui::TextWrapped("Dernier tick : %s", actor->brain->decision);
    }
    ImGui::BeginChild("Lignes de decision", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (auto &line : this->decision_lines) {
        ImGui::TextUnformatted(line.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}
void DebugStrike::enablePlayerAI(RSProf *profile) {
    SCMissionActors *player = this->current_mission->player;
    this->player_profile_saved = player->profile;
    player->profile = profile;
    player->pilot = new SCPilot();
    player->pilot->actor = player;
    player->pilot->plane = this->player_plane;
    player->pilot->throttle = (float) this->player_plane->GetThrottle();
    player->brain = new SCAIBrain(player);
    this->player_ai_accumulator = 0.0f;
    this->saved_command = player->current_command;
    this->saved_command_arg = player->current_command_arg;
    this->saved_command_arg2 = player->current_command_arg2;
    this->saved_command_executed = player->current_command_executed;
    this->saved_target = player->target;
    this->debug_order_command = player->current_command;
    this->debug_order_arg = player->current_command_arg;
    this->debug_order_arg2 = player->current_command_arg2;
    this->debug_order_executed = player->current_command_executed;
    this->debug_order_target = player->target;
    // les PROG de mission sont ignores : le cerveau demarre en errance (0xBE) et ne suit que les ordres de test
    this->giveDebugOrder(OP_PACIFY_TARGET, 0, 0xFF);
    this->maneuver_test = false;
    this->bench_active = false;
    this->autopilot = true;
}
void DebugStrike::disablePlayerAI() {
    SCMissionActors *player = this->current_mission->player;
    if (player->brain == nullptr) {
        return;
    }
    player->pilot->disengageAutopilot();
    player->pilot->CmdKinematic(false, this->player_plane->worldVelocity(), this->player_plane->forward, player->pilot->NosePitch());
    PlaneControlEvent release;
    release.plane = this->player_plane;
    release.normalized_stick = false;
    release.control_stick_x = 0;
    release.control_stick_y = 0;
    release.throttle = (float) this->player_plane->GetThrottle();
    release.flaps = this->player_plane->GetFlaps();
    release.spoilers = this->player_plane->GetSpoilers();
    release.wheel = this->player_plane->GetWheel();
    MessageBus::getInstance().publish(std::make_unique<PlaneControlEvent>(release));
    MessageBus::getInstance().processEvents();
    delete player->brain;
    player->brain = nullptr;
    delete player->pilot;
    player->pilot = nullptr;
    player->profile = this->player_profile_saved;
    this->player_profile_saved = nullptr;
    this->bench_active = false;
    player->current_command = this->saved_command;
    player->current_command_arg = this->saved_command_arg;
    player->current_command_arg2 = this->saved_command_arg2;
    player->current_command_executed = this->saved_command_executed;
    player->target = this->saved_target;
    this->player_plane->alignVelocityToNose();
    this->autopilot = false;
}
// le script du joueur repasse par setObjective a chaque frame : l'ordre de test est repose avant chaque tick du cerveau
void DebugStrike::restoreDebugOrder() {
    SCMissionActors *player = this->current_mission->player;
    player->current_command = this->debug_order_command;
    player->current_command_arg = this->debug_order_arg;
    player->current_command_arg2 = this->debug_order_arg2;
    player->current_command_executed = this->debug_order_executed;
    if (this->debug_order_command == OP_SET_OBJ_DESTROY_TARGET || this->debug_order_command == OP_SET_OBJ_DEFEND_TARGET) {
        player->target = this->debug_order_target;
    }
}
void DebugStrike::giveDebugOrder(prog_op command, uint8_t arg, uint8_t arg2) {
    SCMissionActors *player = this->current_mission->player;
    this->restoreDebugOrder();
    player->SCMissionActors::setObjective(command, arg);
    player->current_command_arg2 = arg2;
    this->debug_order_command = player->current_command;
    this->debug_order_arg = player->current_command_arg;
    this->debug_order_arg2 = player->current_command_arg2;
    this->debug_order_executed = player->current_command_executed;
    this->debug_order_target = player->target;
}
bool DebugStrike::spotCombo(const char *label, uint8_t &spot, bool allow_none) {
    std::vector<SPOT *> &spots = this->current_mission->mission->mission_data.spots;
    std::string preview = spot < spots.size() ? "Spot " + std::to_string(spots[spot]->id) : "aucun";
    bool changed = false;
    if (ImGui::BeginCombo(label, preview.c_str(), 0)) {
        if (allow_none && ImGui::Selectable("aucun", spot >= spots.size())) {
            spot = 0xFF;
            changed = true;
        }
        for (size_t i = 0; i < spots.size(); i++) {
            std::string name = "Spot " + std::to_string(spots[i]->id) + " (" + std::to_string((int) spots[i]->position.x) + ", " +
                               std::to_string((int) spots[i]->position.y) + ", " + std::to_string((int) spots[i]->position.z) + ")";
            if (ImGui::Selectable(name.c_str(), spot == i)) {
                spot = (uint8_t) i;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}
bool DebugStrike::actorCombo(const char *label, uint8_t &actor_id) {
    std::string preview = "aucun";
    for (auto actor : this->current_mission->actors) {
        if (actor->actor_id == actor_id) {
            preview = actor->actor_name + " [" + std::to_string(actor->actor_id) + "]";
        }
    }
    bool changed = false;
    if (ImGui::BeginCombo(label, preview.c_str(), 0)) {
        for (auto actor : this->current_mission->actors) {
            if (actor == this->current_mission->player || actor->is_destroyed || actor->object == nullptr) {
                continue;
            }
            std::string name = actor->actor_name + " [" + std::to_string(actor->actor_id) + "]";
            if (ImGui::Selectable(name.c_str(), actor->actor_id == actor_id)) {
                actor_id = (uint8_t) actor->actor_id;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}
void DebugStrike::playerAIOrders() {
    static const prog_op orders[] = {
        OP_SET_OBJ_TAKE_OFF,
        OP_SET_OBJ_LAND,
        OP_SET_OBJ_FLY_TO_WP,
        OP_SET_OBJ_DESTROY_TARGET,
        OP_SET_OBJ_DEFEND_TARGET,
        OP_SET_OBJ_DEFEND_AREA,
        OP_SET_OBJ_FOLLOW_ALLY,
        OP_PACIFY_TARGET,
    };
    static const char *order_names[] = {
        "Decoller",
        "Atterrir",
        "Aller au waypoint",
        "Detruire la cible",
        "Defendre la cible",
        "Defendre la zone",
        "Suivre un allie",
        "Errance (0xBE)",
    };
    static int order_index = 3;
    static uint8_t spot_a = 0;
    static uint8_t spot_b = 0xFF;
    static uint8_t actor_arg = 0;
    SCMissionActors *player = this->current_mission->player;
    if (ImGui::BeginCombo("Ordre", order_names[order_index], 0)) {
        for (int i = 0; i < IM_ARRAYSIZE(order_names); i++) {
            if (ImGui::Selectable(order_names[i], i == order_index)) {
                order_index = i;
            }
        }
        ImGui::EndCombo();
    }
    prog_op command = orders[order_index];
    uint8_t arg = 0;
    uint8_t arg2 = 0xFF;
    switch (command) {
    case OP_SET_OBJ_LAND:
        this->spotCombo("Spot d'approche", spot_a, false);
        this->spotCombo("Spot de toucher", spot_b, true);
        arg = spot_a;
        arg2 = spot_b;
        break;
    case OP_SET_OBJ_FLY_TO_WP:
        this->spotCombo("Point", spot_a, false);
        this->spotCombo("Vitesse", spot_b, true);
        arg = spot_a;
        arg2 = spot_b;
        break;
    case OP_SET_OBJ_DEFEND_AREA:
        this->spotCombo("Centre", spot_a, false);
        arg = spot_a;
        break;
    case OP_SET_OBJ_DESTROY_TARGET:
    case OP_SET_OBJ_DEFEND_TARGET:
    case OP_SET_OBJ_FOLLOW_ALLY:
        this->actorCombo("Acteur", actor_arg);
        ImGui::SameLine();
        if (ImGui::Button("Cible radar") && this->target != nullptr) {
            actor_arg = (uint8_t) this->target->actor_id;
        }
        arg = actor_arg;
        break;
    default:
        break;
    }
    if (ImGui::Button("Donner l'ordre")) {
        this->giveDebugOrder(command, arg, arg2);
    }
    ImGui::Text("Ordre : %s arg %d arg2 %d, termine %d, verrouille %d", prog_op_names[this->debug_order_command].c_str(), this->debug_order_arg,
                this->debug_order_arg2, this->debug_order_executed, player->brain->objective_locked);
}
static std::string actorLabel(SCMissionActors *actor) {
    if (actor == nullptr) {
        return "aucun";
    }
    return actor->actor_name + " [" + std::to_string(actor->actor_id) + "]";
}
static void vectorText(const char *label, Vector3D v) {
    ImGui::Text("%s (%.0f, %.0f, %.0f)", label, v.x, v.y, v.z);
}
// AI_ManeuverTournament : scores de chaque candidat, etat de manoeuvre et contexte de combat preserves
void DebugStrike::evaluateTournament() {
    SCAIBrain *brain = this->current_mission->player->brain;
    static const int fixed_ids[8] = {
        20,
        14,
        15,
        16,
        21,
        7,
        19,
        4,
    };
    std::vector<int> ids;
    std::vector<int> weights;
    for (int i = 0; i < 8; i++) {
        ids.push_back(fixed_ids[i]);
        weights.push_back(0);
    }
    for (auto &entry : this->current_mission->player->profile->ai.mvrs) {
        bool fixed = false;
        for (int i = 0; i < 8; i++) {
            if (ids[i] == entry.node_id) {
                weights[i] = entry.value;
                fixed = true;
            }
        }
        if (!fixed && ids.size() < 25) {
            ids.push_back(entry.node_id);
            weights.push_back(entry.value);
        }
    }
    ManeuverState saved_maneuver = brain->maneuver;
    CombatContext saved_ctx = brain->ctx;
    this->tournament_scores.clear();
    for (size_t i = 0; i < ids.size(); i++) {
        brain->maneuver.phase_hint = 0;
        brain->maneuver.bits = 0;
        int score = brain->scoreManeuver(ids[i], brain->air_target);
        this->tournament_scores.push_back(std::make_tuple(ids[i], score, weights[i]));
    }
    brain->maneuver = saved_maneuver;
    brain->ctx = saved_ctx;
}
// manoeuvres traitees par tickManeuver (ID21 est la navigation, hors tournoi direct)
static const int test_maneuver_ids[] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    13,
    14,
    15,
    16,
    19,
};
// geometrie vers la cible de manoeuvre, recalculee a chaque image (pas le contexte du cerveau, qui ne bouge qu'au tick)
void DebugStrike::targetGeometry() {
    SCMissionActors *test_target = this->testManeuverTarget();
    if (test_target == nullptr) {
        ImGui::TextDisabled("Pas de cible de manoeuvre");
        return;
    }
    Vector3D target_position = test_target->plane != nullptr ? test_target->plane->position : test_target->object->position;
    Vector3D delta = target_position - this->player_plane->position;
    float distance = delta.Length();
    float horizontal = sqrtf(delta.x * delta.x + delta.z * delta.z);
    float nose = this->player_plane->forward.AngleBetween(delta);
    float heading = signed180(this->player_plane->forward.Azimuth() - delta.Azimuth());
    ImVec4 ok_color(0.3f, 0.8f, 0.3f, 1.0f);
    ImVec4 ko_color(0.9f, 0.4f, 0.3f, 1.0f);
    ImGui::Text("Distance %.0f m (horizontale %.0f m)", distance, horizontal);
    ImGui::SameLine();
    ImGui::TextColored(distance >= 17700.0f ? ok_color : ko_color, distance >= 17700.0f ? "[> 17 700 : ID 13 possible]" : "[< 17 700 : ID 13 s'arrete]");
    ImGui::SameLine();
    ImGui::TextColored(distance < 1800.0f ? ok_color : ko_color, distance < 1800.0f ? "[< 1800 : recalage de visee]" : "[> 1800]");
    ImGui::Text("Angle nez %.1f deg, ecart de cap %.1f deg, elevation %.1f deg, altitude relative %.0f m", nose, heading, delta.Elevation(), delta.y);
    if (test_target->plane != nullptr) {
        JDYN *jdyn = this->player_plane->object->entity->jdyn;
        ImGui::Text("Vitesse cible %.0f, ma vitesse indiquee %.0f (croisiere %d, mini %d)", test_target->plane->worldVelocity().Length(),
                    this->player_plane->indicatedAirspeed(), jdyn->ai_speed_cruise, jdyn->ai_speed_min);
    }
}
SCMissionActors *DebugStrike::testManeuverTarget() {
    for (auto actor : this->current_mission->actors) {
        if (actor->actor_id == this->test_maneuver_target && !actor->is_destroyed) {
            return actor;
        }
    }
    return nullptr;
}
// tick du mode test : minuterie et reflexes comme AI_TopLevelThink, puis uniquement la manoeuvre testee (ni GOAL ni ordre)
void DebugStrike::maneuverTestTick() {
    SCMissionActors *player = this->current_mission->player;
    SCAIBrain *brain = player->brain;
    SCMissionActors *test_target = this->testManeuverTarget();
    player->pilot->ClearGuidance();
    brain->selector_ran = false;
    brain->updateTimers();
    brain->air_target = test_target;
    bool reacted = false;
    if (this->maneuver_test_reflexes && !this->player_plane->on_ground) {
        reacted = !brain->behaviorRunning() && brain->runReflexes();
        if (!reacted && brain->reaction_level <= REACT_COLLISION) {
            reacted = brain->scanCollisionThreats();
        }
    }
    if (!reacted) {
        if (brain->behaviorRunning()) {
            brain->tickBehavior();
        } else if (this->maneuver_test_policy == 0) {
            int id = test_maneuver_ids[this->test_maneuver_index];
            brain->maneuver.phase_hint = 0;
            brain->maneuver.bits = 0;
            if (test_target != nullptr) {
                brain->scoreManeuver(id, test_target);
            }
            brain->applyManeuver(id, test_target, REACT_NONE);
            this->maneuver_test_runs++;
            brain->tickManeuver();
        } else if (this->maneuver_test_policy == 1 && test_target != nullptr) {
            if (brain->runTournament()) {
                this->maneuver_test_runs++;
            }
        } else {
            brain->wander();
        }
    }
    brain->just_hit = false;
    brain->escort_leader_free = false;
}
// banc de manoeuvre : le tick de la manoeuvre seul, sans score, tournoi, reflexes ni GOAL
void DebugStrike::benchStart() {
    this->maneuver_test = false;
    this->current_mission->player->pilot->throttle = (float) this->player_plane->GetThrottle();
    this->current_mission->player->brain->abandonBehavior();
    this->benchSituation();
    this->benchApply();
    SCAIBrain *brain = this->current_mission->player->brain;
    this->bench_active = true;
    this->bench_runs = 1;
    this->bench_time = 0.0f;
    this->bench_run_start = 0.0f;
    this->bench_last_phase = brain->maneuver.phase;
    this->bench_last_bits = brain->maneuver.bits;
    this->bench_next_sample = 0.5f;
    this->bench_trace.clear();
    std::string start_label = "depart, situation " + std::to_string(this->bench_situation) + ", ordre " + prog_op_names[this->debug_order_command] + " " +
                              std::to_string(this->debug_order_arg);
    this->benchRecord(start_label.c_str());
}
void DebugStrike::benchSituation() {
    SCMissionActors *player = this->current_mission->player;
    printf("BANC situation %d : player_plane %p, acteur %p, pilote %p\n", this->bench_situation, (void *) this->player_plane, (void *) player->plane,
           (void *) player->pilot->plane);
    if (this->bench_situation == 0) {
        return;
    }
    SCPlane *plane = this->player_plane;
    player->pilot->disengageAutopilot();
    plane->kinematic_mode = false;
    JDYN *jdyn = plane->object->entity->jdyn;
    float pitch = 0.0f;
    float speed = (float) jdyn->ai_speed_min;
    if (this->bench_situation == 1) {
        pitch = -10.0f;
        speed = (float) jdyn->ai_speed_cruise;
        plane->y = plane->groundlevel + 150.0f;
    } else if (this->bench_situation == 2) {
        pitch = 45.0f;
    }
    Vector3D level(plane->forward.x, 0.0f, plane->forward.z);
    level.Normalize();
    float cos_pitch = cosf(degreeToRad(pitch));
    Vector3D nose(level.x * cos_pitch, sinf(degreeToRad(pitch)), level.z * cos_pitch);
    plane->pitch = pitch * 10.0f;
    plane->roll = 0.0f;
    plane->forward = nose;
    plane->velocity = nose * speed;
    plane->pitch_speed = 0.0f;
    plane->yaw_speed = 0.0f;
    plane->roll_speed = 0.0f;
}
void DebugStrike::benchApply() {
    SCAIBrain *brain = this->current_mission->player->brain;
    int id = test_maneuver_ids[this->test_maneuver_index];
    SCMissionActors *bench_target = this->testManeuverTarget();
    brain->maneuver.bits = 0;
    brain->maneuver.phase_hint = 3;
    if (this->bench_use_score && bench_target != nullptr) {
        // effets du score seulement (bits, point memorise, phase de depart), resultat ignore
        brain->scoreManeuver(id, bench_target);
    }
    if (this->bench_start_phase > 0) {
        brain->maneuver.phase_hint = this->bench_start_phase;
    }
    brain->applyManeuver(id, bench_target, REACT_NONE);
    if (this->bench_start_phase > 0) {
        brain->maneuver.phase = this->bench_start_phase;
    }
}
void DebugStrike::benchTick() {
    SCMissionActors *player = this->current_mission->player;
    SCAIBrain *brain = player->brain;
    this->bench_time += SCAIBrain::TICK_DURATION;
    player->pilot->ClearGuidance();
    brain->selector_ran = false;
    int last_phase = brain->maneuver.phase;
    brain->tickManeuver();
    if (brain->maneuver.id == 0) {
        std::string reason = "fin : ";
        if (brain->maneuver.end_reason[0] != '\0') {
            reason += brain->maneuver.end_reason;
        } else {
            reason += brain->maneuver.timer <= 0.0f ? "minuteur" : "phase " + std::to_string(last_phase);
        }
        reason += ", ordre " + prog_op_names[player->current_command];
        this->benchRecord(reason.c_str());
        if (!this->bench_repeat) {
            this->bench_active = false;
            return;
        }
        if (this->bench_time - this->bench_run_start < 1.5f * SCAIBrain::TICK_DURATION) {
            this->benchRecord("relance arretee : fin immediate");
            this->bench_active = false;
            return;
        }
        // comme le tournoi qui reprend le meme gagnant : nouvelle application, sans les autres candidats
        this->benchApply();
        this->bench_runs++;
        this->bench_run_start = this->bench_time;
        this->bench_last_phase = brain->maneuver.phase;
        this->bench_last_bits = brain->maneuver.bits;
        std::string relaunch = "relance " + std::to_string(this->bench_runs);
        this->benchRecord(relaunch.c_str());
        return;
    }
    if (brain->maneuver.phase != this->bench_last_phase) {
        this->bench_last_phase = brain->maneuver.phase;
        this->bench_last_bits = brain->maneuver.bits;
        this->benchRecord("phase");
    } else if (brain->maneuver.bits != this->bench_last_bits) {
        this->bench_last_bits = brain->maneuver.bits;
        this->benchRecord("bits");
    } else if (this->bench_time >= this->bench_next_sample) {
        this->benchRecord("");
    } else {
        return;
    }
    while (this->bench_next_sample <= this->bench_time) {
        this->bench_next_sample += 0.5f;
    }
}
void DebugStrike::benchRecord(const char *label) {
    SCMissionActors *player = this->current_mission->player;
    SCAIBrain *brain = player->brain;
    char line[320];
    snprintf(line, sizeof(line),
             "t=%6.2f s  %-60s ID %d phase %d bits %02X  assiette %5.0f  incl %5.0f  vit %5.0f  gaz %3d  minuteur %5.1f  alt %6.0f  d %6.0f  nez %4.0f",
             this->bench_time, label, test_maneuver_ids[this->test_maneuver_index], brain->maneuver.phase, brain->maneuver.bits,
             player->pilot->NosePitch(), player->pilot->BankAngle(), this->player_plane->indicatedAirspeed(), this->player_plane->GetThrottle(),
             brain->maneuver.timer, this->player_plane->y, brain->ctx.dist, brain->ctx.nose_angle);
    std::string full = line;
    SCPilot *pilot = player->pilot;
    char guidance[160];
    snprintf(guidance, sizeof(guidance), "  | h %6.2f v %6.2f r %7.1f t %6.2f s %5.2f  manche roulis %6.2f tangage %6.2f", pilot->guidance_log_h,
             pilot->guidance_log_v, pilot->guidance_log_r, pilot->guidance_log_t, pilot->guidance_log_s, pilot->roll_stick, pilot->pitch_stick);
    full += guidance;
    SCJetpPlane *jetp = dynamic_cast<SCJetpPlane *>(this->player_plane);
    if (jetp != nullptr) {
        Vector3D velocity = jetp->worldVelocity();
        char flight[160];
        snprintf(flight, sizeof(flight), "  | pente %6.2f alpha %6.2f consigne %6.2f vtang %7.2f", velocity.Elevation(), jetp->alpha_deg, jetp->alpha_command_deg,
                 jetp->pitch_speed);
        full += flight;
    }
    this->bench_trace.push_back(full);
    printf("BANC %s\n", full.c_str());
}
void DebugStrike::benchConfig() {
    static const char *phase_names[] = {
        "auto (application)",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
    };
    static const char *situation_names[] = {
        "actuelle",
        "pres du sol, en descente (ID 14)",
        "nez haut, lent (ID 15)",
        "lent, a plat (ID 16)",
    };
    SCAIBrain *brain = this->current_mission->player->brain;
    if (!ImGui::CollapsingHeader("Banc de manoeuvre", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    ImGui::TextWrapped("Execute seulement le tick de la manoeuvre choisie ci-dessus : ni score, ni tournoi, ni reflexes, ni GOAL. Cible facultative.");
    ImGui::PushItemWidth(160.0f);
    ImGui::Combo("Phase de depart", &this->bench_start_phase, phase_names, IM_ARRAYSIZE(phase_names));
    ImGui::PopItemWidth();
    ImGui::Text("Situation de depart :");
    for (int i = 0; i < IM_ARRAYSIZE(situation_names); i++) {
        ImGui::SameLine();
        ImGui::RadioButton(situation_names[i], &this->bench_situation, i);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Preparer avec le score", &this->bench_use_score);
    ImGui::SameLine();
    ImGui::Checkbox("Relancer a la fin", &this->bench_repeat);
    if (!this->bench_active) {
        if (ImGui::Button("Executer")) {
            this->benchStart();
        }
    } else {
        if (ImGui::Button("Arreter")) {
            brain->endManeuver(false);
            brain->maneuver.end_reason = "arret manuel";
            this->benchRecord("arret manuel");
            this->bench_active = false;
        }
        ImGui::SameLine();
        ImGui::Text("en cours : ID %d, phase %d, minuteur %.1f, t = %.1f s", brain->maneuver.id, brain->maneuver.phase, brain->maneuver.timer, this->bench_time);
    }
    if (!this->bench_trace.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Copier la trace")) {
            std::string all;
            for (auto &line : this->bench_trace) {
                all += line + "\n";
            }
            ImGui::SetClipboardText(all.c_str());
        }
        ImGui::BeginChild("Trace du banc", ImVec2(0, 160), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (auto &line : this->bench_trace) {
            ImGui::TextUnformatted(line.c_str());
        }
        ImGui::EndChild();
    }
}
void DebugStrike::maneuverTestConfig() {
    SCAIBrain *brain = this->current_mission->player->brain;
    if (!ImGui::CollapsingHeader("Mode test MVRS", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    if (ImGui::Checkbox("Mode test actif", &this->maneuver_test)) {
        this->bench_active = false;
        brain->abandonBehavior();
        this->maneuver_test_runs = 0;
        if (!this->maneuver_test) {
            brain->air_target = nullptr;
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Reflexes (decrochage, sol, collision)", &this->maneuver_test_reflexes);
    ImGui::RadioButton("Repeter la manoeuvre choisie", &this->maneuver_test_policy, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Tournoi MVRS", &this->maneuver_test_policy, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Une fois puis errance", &this->maneuver_test_policy, 2);
    if (this->maneuver_test_policy == 2) {
        ImGui::SameLine();
        if (ImGui::Button("Lancer") && this->maneuver_test) {
            int id = test_maneuver_ids[this->test_maneuver_index];
            SCMissionActors *test_target = this->testManeuverTarget();
            brain->abandonBehavior();
            brain->maneuver.phase_hint = 0;
            brain->maneuver.bits = 0;
            if (test_target != nullptr) {
                brain->scoreManeuver(id, test_target);
            }
            brain->applyManeuver(id, test_target, REACT_NONE);
            this->maneuver_test_runs++;
        }
    }
    ImGui::Text("Executions %d, manoeuvre en cours ID %d, derniere terminee %d", this->maneuver_test_runs, brain->maneuver.id,
                brain->last_finished_behavior);
    if (this->maneuver_test_policy == 1 && this->testManeuverTarget() == nullptr) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Le tournoi demande une cible");
    }
}
void DebugStrike::playerAIControls() {
    SCAIBrain *brain = this->current_mission->player->brain;
    std::string maneuver_preview = "ID " + std::to_string(test_maneuver_ids[this->test_maneuver_index]);
    if (ImGui::BeginCombo("Manoeuvre", maneuver_preview.c_str(), 0)) {
        for (int i = 0; i < IM_ARRAYSIZE(test_maneuver_ids); i++) {
            std::string name = "ID " + std::to_string(test_maneuver_ids[i]);
            if (ImGui::Selectable(name.c_str(), i == this->test_maneuver_index)) {
                this->test_maneuver_index = i;
            }
        }
        ImGui::EndCombo();
    }
    this->actorCombo("Cible de manoeuvre", this->test_maneuver_target);
    ImGui::SameLine();
    if (ImGui::Button("Cible radar##manoeuvre") && this->target != nullptr) {
        this->test_maneuver_target = (uint8_t) this->target->actor_id;
    }
    ImGui::SameLine();
    if (ImGui::Button("Sans cible")) {
        this->test_maneuver_target = 0xFF;
    }
    this->targetGeometry();
    this->benchConfig();
    this->maneuverTestConfig();
    if (!ImGui::CollapsingHeader("Forcage")) {
        return;
    }
    SCMissionActors *chosen = this->testManeuverTarget();
    if (chosen == nullptr) {
        chosen = brain->air_target;
    }
    ImGui::Text("Cible utilisee : %s", actorLabel(chosen).c_str());
    if (ImGui::Button("Forcer la manoeuvre") && chosen != nullptr) {
        // comme AI_ManeuverTournament : le score prepare phase_hint, bits et point, puis la manoeuvre est appliquee
        brain->abandonBehavior();
        brain->maneuver.phase_hint = 0;
        brain->maneuver.bits = 0;
        brain->scoreManeuver(test_maneuver_ids[this->test_maneuver_index], chosen);
        brain->applyManeuver(test_maneuver_ids[this->test_maneuver_index], chosen, REACT_NONE);
    }
    ImGui::SameLine();
    if (ImGui::Button("Finir la manoeuvre")) {
        brain->endManeuver(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Abandonner le comportement")) {
        brain->abandonBehavior();
    }
    if (ImGui::Button("Prendre comme cible air") && chosen != nullptr) {
        brain->air_target = chosen;
    }
    ImGui::SameLine();
    if (ImGui::Button("Effacer la cible air")) {
        brain->air_target = nullptr;
    }
    ImGui::SameLine();
    if (ImGui::Button("Deverrouiller l'ordre")) {
        brain->objective_locked = false;
    }
    if (ImGui::Button("Evaluer le tournoi (cible air)") && brain->air_target != nullptr) {
        this->evaluateTournament();
    }
    if (!this->tournament_scores.empty() && ImGui::BeginTable("Tournoi", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Score");
        ImGui::TableSetupColumn("Poids MVRS");
        ImGui::TableSetupColumn("Total (+/-1 aleatoire)");
        ImGui::TableHeadersRow();
        for (auto &entry : this->tournament_scores) {
            int id = std::get<0>(entry);
            int score = std::get<1>(entry);
            int weight = std::get<2>(entry);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", score);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", weight);
            ImGui::TableSetColumnIndex(3);
            if (score == 0) {
                ImGui::Text("exclu");
            } else {
                ImGui::Text("%d", score + weight);
            }
        }
        ImGui::EndTable();
    }
}
void DebugStrike::brainState(SCMissionActors *actor) {
    static const char *reaction_names[] = {
        "aucune",
        "engage",
        "missile",
        "collision",
        "evitement sol",
        "sortie de decrochage",
    };
    static const char *behavior_names[] = {
        "manoeuvre",
        "navigation",
        "attaque sol",
        "decollage",
        "atterrissage",
    };
    static const char *ground_op_names[] = {
        "aucune",
        "decollage",
        "atterrissage",
    };
    SCMissionActors *player = actor;
    SCAIBrain *brain = player->brain;
    SCPilot *pilot = player->pilot;
    if (ImGui::CollapsingHeader("Ordre et decision", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto order = prog_op_names.find(player->current_command);
        ImGui::Text("Ordre %s, arg %d, arg2 %d, termine %d, verrouille %d", order != prog_op_names.end() ? order->second.c_str() : "?",
                    player->current_command_arg, player->current_command_arg2, player->current_command_executed, brain->objective_locked);
        ImGui::Text("Cible de mission %s, objet de navigation %s", actorLabel(player->target).c_str(), actorLabel(brain->nav.reference).c_str());
        ImGui::TextWrapped("Decision du dernier tick : %s", brain->decision);
    }
    if (ImGui::CollapsingHeader("Etat du cerveau", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Reaction %s (%d), moral %d, qualite de tir %d", brain->reaction_level < 6 ? reaction_names[brain->reaction_level] : "?",
                    brain->reaction_level, brain->morale, brain->fire_solution_quality);
        ImGui::Text("Cible air %s", actorLabel(brain->air_target).c_str());
        ImGui::Text("Cible sol %s", actorLabel(brain->ground_target).c_str());
        ImGui::Text("Cible de mission %s, engagee %s, annoncee %s", actorLabel(player->target).c_str(), actorLabel(brain->engage_target).c_str(),
                    actorLabel(brain->announced_target).c_str());
        ImGui::Text("Missile menacant %p, alerte %d", (void *) brain->missile_threat, brain->threat_alerted);
        ImGui::Text("Touche %d par %s", brain->just_hit, actorLabel(brain->last_attacker).c_str());
        ImGui::Text("Masque d'armes 0x%04X, attaque sol %d", brain->weapon_mask, brain->ground_attack_active);
        ImGui::Text("Ordre verrouille %d, selecteur passe %d, leader libre %d", brain->objective_locked, brain->selector_ran, brain->escort_leader_free);
        ImGui::Text("Candidats air %d, sol %d, missiles %d", brain->last_air_candidates, brain->last_ground_candidates, brain->last_missile_candidates);
        ImGui::Text("Ciblage : horloge %.2f, masque %d, fenetre lente %d, rapide %d", brain->retarget.clock, brain->retarget.mask, brain->retarget.slow_fired,
                    brain->retarget.fast_fired);
    }
    if (ImGui::CollapsingHeader("Comportements")) {
        std::string stack = "";
        for (auto kind : brain->behaviors) {
            stack += std::string(kind < 5 ? behavior_names[kind] : "?") + " > ";
        }
        ImGui::Text("Pile : %s", stack.empty() ? "vide" : stack.c_str());
        ImGui::Text("Dernier comportement termine : %d", brain->last_finished_behavior);
    }
    if (ImGui::CollapsingHeader("Manoeuvre")) {
        ImGui::Text("ID %d, cible %s, niveau %d", brain->maneuver.id, actorLabel(brain->maneuver.target).c_str(), brain->maneuver.level);
        ImGui::Text("Minuterie %.2f, phase %d (indice %d), segments %d, cote %d, bits 0x%02X", brain->maneuver.timer, brain->maneuver.phase,
                    brain->maneuver.phase_hint, brain->maneuver.segments, brain->maneuver.side, brain->maneuver.bits);
        vectorText("Point", brain->maneuver.point);
        vectorText("Segment", brain->maneuver.segment);
        ImGui::Text("Minuterie du segment %.2f, cap initial %.1f, aspect poursuite %.1f", brain->maneuver.segment_timer, brain->maneuver.start_heading,
                    brain->maneuver.pursuit_aspect);
        std::string uses = "";
        for (int i = 0; i < 32; i++) {
            if (brain->maneuver.uses[i] > 0) {
                uses += "ID" + std::to_string(i) + "x" + std::to_string(brain->maneuver.uses[i]) + " ";
            }
        }
        ImGui::Text("Utilisations : %s", uses.c_str());
    }
    if (ImGui::CollapsingHeader("Contexte de combat")) {
        CombatContext &ctx = brain->ctx;
        ImGui::Text("Cible %s, avion %d", actorLabel(ctx.target).c_str(), ctx.aircraft);
        ImGui::Text("Vitesse %.0f, croisiere %.0f, mini %.0f, IAS %.0f", ctx.my_speed, ctx.cruise, ctx.min_speed, ctx.ias);
        vectorText("D", ctx.D);
        vectorText("Position relative future", ctx.future_rel);
        ImGui::Text("Distance %.0f, angle nez %.1f, angle futur %.1f, angle vitesse cible %.1f", ctx.dist, ctx.nose_angle, ctx.future_angle,
                    ctx.target_vel_angle);
        ImGui::Text("Aspect %.1f, vitesse cible %.0f, cote %.1f, derriere %d, face a face %d", ctx.aspect, ctx.target_speed, ctx.side, ctx.behind,
                    ctx.head_on);
    }
    if (ImGui::CollapsingHeader("Tir")) {
        ImGui::Text("Rafale restante %d, derniere arme %d, dernier tir a %d s", brain->burst_remaining, brain->last_fired_weapon, brain->last_fire_second);
        ImGui::Text("Objet suivi par le point d'emport %s", actorLabel(brain->station_tracked).c_str());
        ImGui::Text("Missile signale %p", (void *) brain->complained_missile);
    }
    if (ImGui::CollapsingHeader("Navigation")) {
        ImGui::Text("Reference %s", actorLabel(brain->nav.reference).c_str());
        vectorText("Centre", brain->nav.center);
        ImGui::Text("Rayon %.0f, altitude %.0f, vitesse %.0f", brain->nav.radius, brain->nav.altitude, brain->nav.speed);
        vectorText("Point", brain->nav.point);
        vectorText("Vitesse voulue", brain->nav.velocity);
        ImGui::Text("Minuterie %.2f, atteint %d", brain->nav.timer, brain->nav.reached);
    }
    if (ImGui::CollapsingHeader("Attaque sol")) {
        ImGui::Text("Cible %s, phase %d, arme %s", actorLabel(brain->ground.target).c_str(), brain->ground.phase,
                    brain->ground.weapon != nullptr ? brain->ground.weapon->name.c_str() : "aucune");
        ImGui::Text("Largue %p, attente %d, temps phase 3 %.2f", (void *) brain->ground.released, brain->ground.release_wait, brain->ground.phase3_time);
        vectorText("Point d'autopilote", brain->ground.autopilot_target);
    }
    if (ImGui::CollapsingHeader("Decollage / atterrissage")) {
        GroundOpsState &ops = brain->ground_ops;
        ImGui::Text("Operation %s, phase %d, temps %.2f, manche %.1f", ops.kind < 3 ? ground_op_names[ops.kind] : "?", ops.phase, ops.time, ops.stick);
        vectorText("Axe", ops.axis);
        vectorText("Origine", ops.origin);
        vectorText("Point de toucher", ops.landing_target);
        vectorText("Direction d'approche", ops.landing_dir);
        ImGui::Text("Vitesse %.0f, duree %.2f, assiette %.1f, compteur %d, palier %d, termine %d", ops.landing_speed, ops.landing_duration, ops.landing_pitch,
                    ops.landing_counter, ops.landing_leveled, ops.landing_done);
    }
    if (ImGui::CollapsingHeader("Formation")) {
        ImGui::Text("Leader %s (arg %d), etat %d, active %d, historique %d, index %d, horloge %.2f", actorLabel(brain->formation.leader).c_str(),
                    brain->formation.leader_arg, brain->formation.leader_state, brain->formation.active, brain->formation.history_ready, brain->formation.index,
                    brain->formation.clock);
        vectorText("Poste", player->follow_slot);
    }
    if (ImGui::CollapsingHeader("Moral")) {
        ImGui::Text("Discipline %d (minuterie %.2f), minuterie %.2f, mutinerie %d, fuite %d", brain->mood.disciplined, brain->mood.discipline_timer,
                    brain->mood.timer, brain->mood.mutiny, brain->mood.fleeing);
        vectorText("Destination", brain->mood.destination);
        vectorText("Base", brain->mood.home);
        ImGui::Text("Ennemis vivants %d, pertes %d, ennemis actifs %d", brain->mood.enemies_alive, brain->mood.own_losses, brain->mood.enemies_active);
    }
    if (ImGui::CollapsingHeader("Pilote")) {
        ImGui::Text("Autopilote %d, atteint %d, operations au sol %d", pilot->autopilotActive(), pilot->autopilotReached(), pilot->GroundOpsActive());
        ImGui::Text("Inclinaison %.1f, assiette %.1f, manche roulis %.1f", pilot->BankAngle(), pilot->NosePitch(), pilot->RollStickValue());
        ImGui::Text("Mode attitude %d : erreur cap %.1f, tangage %.1f, zone morte %.1f", pilot->attitude_mode, pilot->attitude_heading_error,
                    pilot->attitude_pitch_error, pilot->attitude_deadband);
        ImGui::Text("Manche %d %d, gaz %d, cinematique %d", player->plane->control_stick_x, player->plane->control_stick_y,
                    player->plane->GetThrottle(), player->plane->kinematicMode());
    }
}
// vue d'ensemble des cerveaux IA : une ligne par acteur, detail du cerveau de la ligne choisie
void DebugStrike::aiOverview() {
    static const char *reaction_names[] = {
        "-",
        "engage",
        "missile",
        "collision",
        "sol",
        "decrochage",
    };
    static int selected_id = -1;
    static bool active_only = true;
    ImGui::Checkbox("Acteurs actifs seulement", &active_only);
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable("Cerveaux IA", 9, flags, ImVec2(0, 260))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Acteur");
        ImGui::TableSetupColumn("Camp");
        ImGui::TableSetupColumn("Ordre");
        ImGui::TableSetupColumn("Reaction");
        ImGui::TableSetupColumn("Comportement");
        ImGui::TableSetupColumn("Cible air");
        ImGui::TableSetupColumn("Vitesse / alt");
        ImGui::TableSetupColumn("Gaz");
        ImGui::TableSetupColumn("Decision");
        ImGui::TableHeadersRow();
        for (auto actor : this->current_mission->actors) {
            if (actor->brain == nullptr || actor->plane == nullptr || actor->pilot == nullptr) {
                continue;
            }
            if (active_only && !actor->is_active && actor != this->current_mission->player) {
                continue;
            }
            SCAIBrain *brain = actor->brain;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            std::string label = actorLabel(actor) + (actor->is_destroyed ? " (detruit)" : "");
            if (ImGui::Selectable(label.c_str(), selected_id == actor->actor_id, ImGuiSelectableFlags_SpanAllColumns)) {
                selected_id = actor->actor_id;
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", actor->team_id);
            ImGui::TableSetColumnIndex(2);
            auto order = prog_op_names.find(actor->current_command);
            std::string order_name = order != prog_op_names.end() ? order->second : "?";
            if (order_name.rfind("OP_SET_OBJ_", 0) == 0) {
                order_name = order_name.substr(11);
            }
            ImGui::Text("%s %d%s%s", order_name.c_str(), actor->current_command_arg, actor->current_command_executed ? " fini" : "",
                        brain->objective_locked ? " verrou" : "");
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", brain->reaction_level < 6 ? reaction_names[brain->reaction_level] : "?");
            ImGui::TableSetColumnIndex(4);
            if (brain->maneuver.id != 0) {
                ImGui::Text("%s ph %d", brain->behaviorLabel().c_str(), brain->maneuver.phase);
            } else {
                ImGui::Text("%s", brain->behaviorLabel().c_str());
            }
            ImGui::TableSetColumnIndex(5);
            if (brain->air_target != nullptr && brain->air_target->plane != nullptr) {
                float distance = (brain->air_target->plane->position - actor->plane->position).Length();
                ImGui::Text("%s %.0f m", brain->air_target->actor_name.c_str(), distance);
            } else {
                ImGui::TextDisabled("-");
            }
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%.0f / %.0f", actor->plane->indicatedAirspeed(), actor->plane->y);
            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%d", actor->plane->GetThrottle());
            ImGui::TableSetColumnIndex(8);
            ImGui::TextUnformatted(brain->decision);
        }
        ImGui::EndTable();
    }
    SCMissionActors *selected = nullptr;
    for (auto actor : this->current_mission->actors) {
        if (actor->actor_id == selected_id && actor->brain != nullptr && actor->plane != nullptr && actor->pilot != nullptr) {
            selected = actor;
        }
    }
    if (selected == nullptr) {
        ImGui::TextDisabled("Cliquer une ligne pour afficher le detail de son cerveau");
        return;
    }
    ImGui::Separator();
    ImGui::Text("Detail : %s", actorLabel(selected).c_str());
    ImGui::SameLine();
    if (ImGui::Button("Suivre dans la trace de decision")) {
        this->decisionSelect(selected->actor_id);
    }
    ImGui::BeginChild("Detail du cerveau", ImVec2(0, 0), false);
    ImGui::PushID(selected->actor_id);
    this->brainState(selected);
    ImGui::PopID();
    ImGui::EndChild();
}
void DebugStrike::playerAIConfig() {
    static RSProf *selected_profile = nullptr;
    SCMissionActors *player = this->current_mission->player;
    if (player->brain != nullptr) {
        ImGui::Text("Profil %s", player->profile->radi.info.callsign.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Desactiver l'IA")) {
            this->disablePlayerAI();
            return;
        }
        this->playerAIOrders();
        this->playerAIControls();
        this->brainState(this->current_mission->player);
        return;
    }
    std::vector<SCMissionActors *> candidates;
    std::string selected_name = "";
    for (auto actor : this->current_mission->actors) {
        if (actor == player || actor->profile == nullptr || !actor->profile->ai.isAI || actor->profile->ai.goal.empty()) {
            continue;
        }
        candidates.push_back(actor);
        if (actor->profile == selected_profile) {
            selected_name = actor->actor_name;
        }
    }
    if (selected_name.empty()) {
        selected_profile = nullptr;
    }
    if (ImGui::BeginCombo("Profil IA", selected_name.c_str(), 0)) {
        for (auto actor : candidates) {
            if (ImGui::Selectable(actor->actor_name.c_str(), actor->profile == selected_profile)) {
                selected_profile = actor->profile;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Activer l'IA") && selected_profile != nullptr) {
        this->enablePlayerAI(selected_profile);
    }
}
void DebugStrike::showOffCamera() {
    ImVec2 avail_size = ImGui::GetContentRegionAvail();
    ImGui::Image((void *)(intptr_t)Renderer.texture, avail_size, {0, 1}, {1, 0});
}
void DebugStrike::simInfo() {
    ImGui::Text("Speed %d, Mach %.3f, Altitude %.0f, Heading %.0f", this->player_plane->airspeed,
                this->player_plane->mach, new_position.y, this->player_plane->azimuthf);
    ImGui::Text("Throttle %d", this->player_plane->GetThrottle());
    ImGui::Text("Control Stick %d %d", this->player_plane->control_stick_x, this->player_plane->control_stick_y);

    ImGui::Text("Pitch %.3f, Yaw %.3f, roll %.3f", this->player_plane->elevationf, this->player_plane->azimuthf,
                this->player_plane->twist);
    ImGui::Text("Pich speed %.3f, Roll speed %.3f, Yaw speed %.3f", this->player_plane->elevation_speedf, this->player_plane->roll_speed, this->player_plane->azimuth_speedf);
    ImGui::Text("CCIRP pitch[%.3f] yaw[%.3f]", this->player_plane->m_pitch_var, this->player_plane->m_yaw_var);
    ImGui::Text("Y %.3f, On ground %d", this->player_plane->y, this->player_plane->on_ground);
    ImGui::Text("flight [roller:%4f, elevator:%4f, rudder:%4f]", this->player_plane->rollers,
                this->player_plane->elevator, this->player_plane->rudder);
    ImGui::Text("Acceleration (vx,vy,vz) [%.3f ,%.3f ,%.3f ]", this->player_plane->vx, this->player_plane->vy,
                this->player_plane->vz);
    ImGui::Text("Acceleration (ax,ay,az) [%.3f ,%.3f ,%.3f ]", this->player_plane->ax, this->player_plane->ay,
                this->player_plane->az);
    ImGui::Text("Forces : drag %.3f, gravity drag %.3f, lift drag %.3f, thrust %.3f, lift %.3f, weight %.3f",
                this->player_plane->drag_force, this->player_plane->gravity_drag_force,
                this->player_plane->lift_drag_force, this->player_plane->thrust_force, this->player_plane->lift_force,
                this->player_plane->gravity_force);
    ImGui::Text("Lift %.3f, cl %.3f, ae %.3f, cdp %.3f, titl %.3f, stall %d", this->player_plane->lift,
                this->player_plane->Cl, this->player_plane->ae, this->player_plane->Cdp,
                this->player_plane->tilt_factor, this->player_plane->wing_stall);
    ImGui::Text("Gravity %.3f", this->player_plane->gravity);
    ImGui::Text("Drag %.3f", this->player_plane->drag);
    ImGui::Text("Inv mass %.8f", this->player_plane->inverse_mass);
    ImGui::Text("Thrust %.3f", this->player_plane->thrust_force);
    ImGui::Text("G-Load %.3f", this->player_plane->g_load);
    ImGui::Text("ptw");
    for (int o = 0; o < 4; o++) {
        ImGui::Text("PTW[%d]=[%f,%f,%f,%f]", o, this->player_plane->ptw.v[o][0], this->player_plane->ptw.v[o][1],
                    this->player_plane->ptw.v[o][2], this->player_plane->ptw.v[o][3]);
    }
    ImGui::Text("incremental");
    for (int o = 0; o < 4; o++) {
        ImGui::Text("INC[%d]=[%f,%f,%f,%f]", o, this->player_plane->incremental.v[o][0],
                    this->player_plane->incremental.v[o][1], this->player_plane->incremental.v[o][2],
                    this->player_plane->incremental.v[o][3]);
    }
}
void DebugStrike::loadPlane() {
    std::vector<std::string> planes;
    planes = {
        "747",      "A-10", "AWACS",  "B-1B",    "C130DES", "C130GRN", "F-4",      "F-15",    "F-16DES", "F-16GRAY",
        "F-18",     "F-22", "F-117",  "LEARJET", "MIG21",   "MIG29",   "MIRAGE",   "SU27",    "TORNCG",  "TU-20",
        "YF23",     "F4F3", "TBD",    "SBD",     "TBF",     "HELLCAT", "HELLDIVE", "CORSAIR", "BEARCAT", "ZERO",
        "KATETORP", "VAL",  "CLAUDE", "FRANK",   "JUDY",    "BETTY",   "EMILY",    "BAKA",    "F4F4",
    };
    static float envergure = 0.0f;
    static float thrust = 0.0f;
    static float weight = 0.0f;
    static float fuel = 0.0f;
    static float surface = 0.0f;
    static float wing_aspec_ratio = 0.0f;
    static float ie_pi_AR = 0.0f;
    static float roll_rate_max = 0.0f;
    static float pitch_rate_max = 0.0f;
    static std::string plane_name = "";
    static bool simple_simulation = true;
    static bool jdyn_simulation = false;

    if (ImGui::BeginCombo("Plane", plane_name.c_str(), 0)) {
        for (auto plane : planes) {
            if (ImGui::Selectable(plane.c_str(), !plane_name.compare(plane))) {
                plane_name = plane;
            }
            if (!plane_name.compare(plane)) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // ImGui::SameLine();
    static int sim_type = 0;
    const char *sim_types[] = {"simple sim", "jdyn sim", "sgi sim", "Jetp sim"};
    for (int i = 0; i < IM_ARRAYSIZE(sim_types); ++i) {
        ImGui::RadioButton(sim_types[i], &sim_type, i);
        if (i < IM_ARRAYSIZE(sim_types) - 1)
            ImGui::SameLine();
    }

    if (ImGui::Button("Load") && plane_name != "") {
        // this->plane_to_load = planes.indexOf(plane);
        RSEntity *plane_to_load = new RSEntity();
        TreEntry *entry = Assets.GetEntryByName(Assets.object_root_path + plane_name + ".IFF");
        if (entry == nullptr) {
            printf("Unable to load plane %s\n", plane_name.c_str());
            return;
        }
        plane_to_load->InitFromRAM(entry->data, entry->size, Assets.object_root_path + plane_name + ".IFF");
        BoudingBox *bb = plane_to_load->GetBoudingBpx();
        envergure = (bb->max.z - bb->min.z) / 2.0f;
        surface = plane_to_load->wing_area;

        thrust = plane_to_load->thrust_in_newton / 4.448f;
        weight = plane_to_load->weight_in_kg * 2.208588957f;
        fuel = plane_to_load->jdyn->fuel_capacity * 2.208588957f;

        ie_pi_AR = 4000.0f / plane_to_load->drag;
        roll_rate_max = plane_to_load->jdyn->rate_limit_dps;
        pitch_rate_max = plane_to_load->jdyn->max_turn_rate_dps;
        SCPlane *new_plane = nullptr;
        surface = surface * 10.7639f;
        envergure = envergure * 3.28084f;
        wing_aspec_ratio = (envergure * envergure) / surface;
        float twist_rate = plane_to_load->jdyn->max_turn_rate_dps;
        float roll_rate = plane_to_load->jdyn->rate_limit_dps;
        switch (sim_type) {
        case 0:
            thrust = (float) plane_to_load->thrust_in_newton;
            envergure = (bb->max.z - bb->min.z) / 2.0f;
            surface = plane_to_load->wing_area;
            weight =(float)  plane_to_load->weight_in_kg;
            fuel = (float) plane_to_load->jdyn->fuel_capacity;
            new_plane = new SCSimplePlane(10.0f, -7.0f, 40.0f, 40.0f, twist_rate, roll_rate, surface, weight, fuel,
                                          thrust, envergure, 0.83f, 120, this->current_mission->area, player_plane->x,
                                          player_plane->y, player_plane->z);
            new_plane->yaw = player_plane->azimuthf;
            break;
        case 1:
            thrust = (float) plane_to_load->thrust_in_newton;
            surface = plane_to_load->wing_area;
            envergure = sqrtf(surface / ((float)M_PI * 0.83f * plane_to_load->jdyn->induced_drag_k));
            weight = (float) plane_to_load->weight_in_kg;
            fuel = (float) plane_to_load->jdyn->fuel_capacity;
            twist_rate = (float) plane_to_load->jdyn->pitch_rate_limit_dps/3.0f;
            roll_rate = (float) plane_to_load->jdyn->rate_limit_dps/3.0f;
            new_plane = new SCJdynPlane(plane_to_load->jdyn->max_g, -(float) plane_to_load->jdyn->max_g/2.0f, 40.0f, 60.0f, twist_rate, roll_rate, surface, weight, fuel,
                                        thrust, envergure, plane_to_load->jdyn->induced_drag_k,
                                        (int) plane_to_load->jdyn->stall_speed_ms,
                                        this->current_mission->area, player_plane->x,
                                        player_plane->y, player_plane->z);
            new_plane->yaw = player_plane->azimuthf;
            break;
        case 2:
            if (!imperial_planes.count(plane_name)) {
                twist_rate = 30.0f;
                roll_rate = 100.0f;
                fuel = 0;
                new_plane = new SCPlane(10.0f, -7.0f, 40.0f, 40.0f, twist_rate, roll_rate, surface, weight, fuel,
                                        thrust, envergure, 0.83f, 120, this->current_mission->area, player_plane->x,
                                        player_plane->y, player_plane->z);
            } else {
                imperial_plane_data &plane_data = imperial_planes[plane_name];
                new_plane = new SCPlane(plane_data.LmaxDef, plane_data.LminDef, plane_data.Fmax, plane_data.Fmin,
                                        plane_data.ELEVF_CSTE, plane_data.ROLLF_CSTE, plane_data.s, plane_data.W,
                                        plane_data.fuel_weight, plane_data.Mthrust, plane_data.b, plane_data.ie_pi_AR,
                                        (int) plane_data.MIN_LIFT_SPEED, this->current_mission->area, player_plane->x,
                                        player_plane->y, player_plane->z);
                twist_rate = plane_data.ELEVF_CSTE;
                roll_rate = plane_data.ROLLF_CSTE;
                envergure = plane_data.b;
                surface = plane_data.s;
                weight = plane_data.W;
                fuel = plane_data.fuel_weight;
                thrust = plane_data.Mthrust;
            }
            break;
        case 3:
            thrust =(float)  plane_to_load->thrust_in_newton;
            envergure = (bb->max.z - bb->min.z) / 2.0f;
            surface = plane_to_load->wing_area;
            weight =(float)  plane_to_load->weight_in_kg;
            fuel = (float) plane_to_load->jdyn->fuel_capacity;
            twist_rate = 30.0f;
            roll_rate = 100.0f;
            new_plane = new SCJetpPlane(10.0f, -7.0f, 40.0f, 40.0f, twist_rate, roll_rate, surface, weight, fuel,
                                          thrust, envergure, 0.83f, 120, this->current_mission->area, player_plane->x,
                                          player_plane->y, player_plane->z);
            new_plane->yaw = player_plane->azimuthf;
            break;
        }

        new_plane->simple_simulation = false;
        new_plane->object = player_plane->object;
        new_plane->object->entity = plane_to_load;
        new_plane->yaw = player_plane->yaw;
        new_plane->pilot = player_plane->pilot;
        new_plane->InitLoadout();
        this->player_plane = new_plane;
        this->current_mission->player->plane = this->player_plane;

        this->cockpit->player_plane = this->player_plane;
        this->cockpit->init();
    }
    ImGui::Text("Plane name: %s", plane_name.c_str());
    ImGui::Text("Envergure: %.2f", envergure);
    ImGui::Text("Thrust: %.2f", thrust);
    ImGui::Text("Weight: %.2f", weight);
    ImGui::Text("Fuel: %.2f", fuel);
    ImGui::Text("Surface: %.2f", this->player_plane->s);
    ImGui::Text("Wing aspect ratio: %.2f", wing_aspec_ratio);
    ImGui::Text("Induced efficiency: %.2f", this->player_plane->ie_pi_AR);
    ImGui::Text("Roll rate max: %.2f", roll_rate_max);
    ImGui::Text("Pitch rate max: %.2f", pitch_rate_max);
    ImGui::Text("G-Load: Max %.2f, Min %.2f", this->player_plane->Lmax, this->player_plane->Lmin);
}
void DebugStrike::simConfig() {
    static bool azymuth_control = false;
    static int altitude = 0;
    static int azimuth = 0;
    static int throttle = 0;
    static int speed = 0;
    static bool go_to_nav = false;
    static bool go_autopilot = false;
    ImGui::DragInt("set altitude", &altitude, 100, 0, 30000);
    ImGui::DragInt("set throttle", &throttle, 10, 0, 100);
    ImGui::DragInt("set azimuth", &azimuth, 1, 0, 360);
    ImGui::DragInt("set speed", &speed, 2, 0, 30);
    if (ImGui::Button("set")) {
        this->player_plane->vz = (float)-speed;
        this->player_plane->y = (float)altitude;
        this->player_plane->last_py = this->player_plane->y;
        this->player_plane->rollers = 0;
        this->player_plane->roll_speed = 0;
        this->player_plane->elevation_speedf = 0;
        this->player_plane->elevator = 0;
        this->player_plane->SetThrottle(throttle);
        this->player_plane->ptw.Identity();
        this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);

        this->player_plane->ptw.rotateM(tenthOfDegreeToRad(azimuth * 10.0f), 0, 1, 0);
        this->player_plane->ptw.rotateM(tenthOfDegreeToRad(this->player_plane->elevationf), 1, 0, 0);
        this->player_plane->ptw.rotateM(tenthOfDegreeToRad(this->player_plane->twist), 0, 0, 1);
        this->player_plane->Simulate();

        this->pause_simu = true;
    }
    ImGui::SameLine();
    ImGui::PushID(1);
    if (go_autopilot) {
        this->pilot.target_speed = -speed;
        this->pilot.target_climb = altitude;
        this->pilot.target_azimut = (float)azimuth * 10.0f;
        this->autopilot = true;
        this->pilot.FlyTo();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
    }
    if (ImGui::Button("Autopilot")) {
        this->pilot.plane = this->player_plane;
        go_autopilot = !go_autopilot;
        if (go_autopilot) {
            this->autopilot = true;
        } else {
            this->autopilot = false;
        }
        go_to_nav = false;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    ImGui::PushID(1);
    if (go_to_nav) {
        this->pilot.plane = this->player_plane;
        this->pilot.target_speed = -speed;
        this->autopilot = true;
        this->pilot.SetTargetWaypoint(this->current_mission->waypoints[this->nav_point_id]->spot->position);
        this->pilot.FlyTo();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
    }
    ImGui::SameLine();
    if (ImGui::Button("Go To Waypoint")) {
        this->pilot.plane = this->player_plane;
        this->pilot.SetTargetWaypoint(this->current_mission->waypoints[this->nav_point_id]->spot->position);
        go_to_nav = !go_to_nav;
        if (go_to_nav) {
            this->autopilot = true;
        } else {
            this->autopilot = false;
        }
        go_autopilot = false;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    ImGui::Text("turnstate %d", this->pilot.turnState);
    ImGui::Text("target speed %d", this->pilot.target_speed);
    ImGui::Text("target azimuth %.2f", this->pilot.target_azimut);
    ImGui::Text("target climb %d", this->pilot.target_climb);
    ImGui::Text("target roll %.2f", this->pilot.targetRoll);
    if (this->current_mission->player->brain != nullptr) {
        go_autopilot = false;
        go_to_nav = false;
    }
}
void DebugStrike::radar() {
    // Get the canvas parameters for converting world-coordinates to canvas coordinates.
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    static float zoom = 1.0f;
    static ImVec2 pan_offset = ImVec2(0, 0);
    ImGuiIO &io = ImGui::GetIO();

    // Check if the mouse is hovering over the window

    if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
        float prev_zoom = zoom;
        zoom += io.MouseWheel * zoom * 0.1f; // Adjust zoom sensitivity as needed.
        if (zoom < 0.1f)
            zoom = 0.1f;
        ImVec2 mouse_pos = io.MousePos;
        pan_offset.x = pan_offset.x * (zoom / prev_zoom) + (mouse_pos.x - canvas_pos.x) * (1 - (zoom / prev_zoom));
        pan_offset.y = pan_offset.y * (zoom / prev_zoom) + (mouse_pos.y - canvas_pos.y) * (1 - (zoom / prev_zoom));
    }

    bool is_titlebar_hovered = false;
    {
        // Vérifier si la souris est dans la barre de titre
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 mouse_pos = ImGui::GetIO().MousePos;

        float title_bar_height = ImGui::GetFrameHeight(); // Hauteur approximative de la barre de titre

        // La souris est sur la barre de titre si elle est dans la partie supérieure de la fenêtre
        if (mouse_pos.x >= window_pos.x && mouse_pos.x <= window_pos.x + window_size.x && mouse_pos.y >= window_pos.y &&
            mouse_pos.y <= window_pos.y + title_bar_height) {
            is_titlebar_hovered = true;
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !is_titlebar_hovered) {
        pan_offset.x += io.MouseDelta.x;
        pan_offset.y += io.MouseDelta.y;
    }

    // Adjust the canvas position and size using the pan and zoom.
    canvas_pos.x += pan_offset.x;
    canvas_pos.y += pan_offset.y;
    canvas_size.x *= zoom;
    canvas_size.y *= zoom;
    // Set a scale factor to map actor positions to the child window size.
    // You may need to adjust this value depending on your world-to-canvas mapping.

    float scale_x = canvas_size.x / (BLOCK_WIDTH * 18.0f);
    float scale_z = canvas_size.y / (BLOCK_WIDTH * 18.0f);

    // Draw a rectangle to represent the canvas area.
    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                             IM_COL32(6, 57, 112, 255));
    // Optionally, center the drawing within the canvas.
    ImVec2 canvas_center = ImVec2(canvas_pos.x + canvas_size.x * 0.5f, canvas_pos.y + canvas_size.y * 0.5f);
    // Draw a grid for better visualization.
    for (int i = -9; i <= 9; ++i) {
        // Vertical lines
        ImVec2 line_start = ImVec2(canvas_center.x + i * BLOCK_WIDTH * scale_x, canvas_pos.y);
        ImVec2 line_end = ImVec2(line_start.x, canvas_pos.y + canvas_size.y);
        draw_list->AddLine(line_start, line_end, IM_COL32(200, 200, 200, 255));

        // Horizontal lines
        line_start = ImVec2(canvas_pos.x, canvas_center.y + i * BLOCK_WIDTH * scale_z);
        line_end = ImVec2(canvas_pos.x + canvas_size.x, line_start.y);
        draw_list->AddLine(line_start, line_end, IM_COL32(200, 200, 200, 255));
    }

    for (auto runway : this->current_mission->area->objectOverlay) {
        if (runway.vertices && runway.nbTriangles > 0) {
            ImVec2 runway_pos;
            for (int i = 0; i < runway.nbTriangles; i++) {
                AreaOverlayTriangles triangle = runway.trianles[i];
                std::vector<Vector3D> vertices;
                for (auto vid : triangle.verticesIdx) {
                    Vector3D v = {(float)runway.vertices[vid].x, (float)runway.vertices[vid].y,
                                  (float)runway.vertices[vid].z};
                    vertices.push_back(v);
                }
                ImVec2 p1 =
                    ImVec2(canvas_center.x + vertices[0].x * scale_x, canvas_center.y - vertices[0].z * scale_z);
                ImVec2 p2 =
                    ImVec2(canvas_center.x + vertices[1].x * scale_x, canvas_center.y - vertices[1].z * scale_z);
                ImVec2 p3 =
                    ImVec2(canvas_center.x + vertices[2].x * scale_x, canvas_center.y - vertices[2].z * scale_z);
                runway_pos = p3;
                draw_list->AddTriangleFilled(p1, p2, p3, IM_COL32(0, 255, 0, 100));
            }
            draw_list->AddText(runway_pos, IM_COL32(255, 255, 255, 255), "RUNWAY");
        }
    }
    for (auto area : this->current_mission->mission->mission_data.areas) {
        if (area) {
            // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
            float world_x = area->position.x;
            float world_z = -area->position.z;
            ImVec2 area_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x, canvas_center.y - world_z * scale_z);
            ImVec2 top_left = ImVec2(area_canvas_pos.x - area->AreaWidth * scale_x * 0.5f,
                                     area_canvas_pos.y - area->AreaHeight * scale_z * 0.5f);
            ImVec2 bottom_right = ImVec2(area_canvas_pos.x + area->AreaWidth * scale_x * 0.5f,
                                         area_canvas_pos.y + area->AreaHeight * scale_z * 0.5f);

            draw_list->AddRect(top_left, bottom_right, IM_COL32(0, 255, 0, 255));

            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y), IM_COL32(255, 255, 255, 255), area->AreaName);
            if (ImGui::IsMouseHoveringRect(top_left, bottom_right)) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    this->debug_area = area;
                    this->debug_entity_mode = DebugEntityMode::Area;
                }
            }
        }
    }
    for (auto spot : this->current_mission->mission->mission_data.spots) {
        if (spot) {
            // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
            float world_x = spot->position.x;
            float world_z = -spot->position.z;
            ImVec2 spot_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x, canvas_center.y - world_z * scale_z);

            ImVec2 top_left = ImVec2(spot_canvas_pos.x - 5.0f * 0.5f, spot_canvas_pos.y - 5.0f * 0.5f);
            ImVec2 bottom_right = ImVec2(spot_canvas_pos.x + 5.0f * 0.5f, spot_canvas_pos.y + 5.0f * 0.5f);

            draw_list->AddRect(top_left, bottom_right, IM_COL32(0, 255, 255, 255));

            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y), IM_COL32(255, 255, 255, 255),
                               ("SPOT: " + std::to_string(spot->id)).c_str());
            if (ImGui::IsMouseHoveringRect(top_left, bottom_right)) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    this->debug_spot = spot;
                    this->debug_entity_mode = DebugEntityMode::Spot;
                }
            }
        }
    }
    for (auto actor : this->current_mission->actors) {
        if (actor && actor->plane) {
            // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
            float world_x = actor->plane->x;
            float world_z = -actor->plane->z;

            ImVec2 actor_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x, canvas_center.y - world_z * scale_z);

            // Define square size.
            float square_size = 10.0f;

            // Draw a square centered at the actor's computed canvas position.
            ImVec2 top_left = ImVec2(actor_canvas_pos.x - square_size * 0.5f, actor_canvas_pos.y - square_size * 0.5f);
            ImVec2 bottom_right =
                ImVec2(actor_canvas_pos.x + square_size * 0.5f, actor_canvas_pos.y + square_size * 0.5f);
            ImU32 actor_color = IM_COL32(128, 128, 128, 255);
            if (actor->is_active) {
                actor_color = IM_COL32(0, 255, 0, 255);
            } else if (actor->is_destroyed) {
                actor_color = IM_COL32(128, 0, 128, 255);
            } else if (actor->plane == this->player_plane) {
                actor_color = IM_COL32(255, 0, 0, 255);
            }
            draw_list->AddRect(top_left, bottom_right, actor_color);
            for (auto wp : actor->plane->weaps_object) {
                if (wp) {
                    // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
                    float wp_world_x = wp->x;
                    float wp_world_z = -wp->z;
                    ImVec2 wp_canvas_pos =
                        ImVec2(canvas_center.x + wp_world_x * scale_x, canvas_center.y - wp_world_z * scale_z);

                    // Draw a square centered at the weapon's computed canvas position.
                    ImVec2 wp_top_left = ImVec2(wp_canvas_pos.x - 5.0f * 0.5f, wp_canvas_pos.y - 5.0f * 0.5f);
                    ImVec2 wp_bottom_right = ImVec2(wp_canvas_pos.x + 5.0f * 0.5f, wp_canvas_pos.y + 5.0f * 0.5f);

                    draw_list->AddRect(wp_top_left, wp_bottom_right, IM_COL32(255, 255, 0, 255));
                }
            }
            if (actor->pilot != nullptr) {
                ImVec2 pilot_waypoin = ImVec2(canvas_center.x + actor->pilot->target_waypoint.x * scale_x,
                                              canvas_center.y + actor->pilot->target_waypoint.z * scale_z);
                // Draw a square centered at the pilot's target waypoint.
                ImVec2 pilot_top_left = ImVec2(pilot_waypoin.x - 5.0f * 0.5f, pilot_waypoin.y - 5.0f * 0.5f);
                ImVec2 pilot_bottom_right = ImVec2(pilot_waypoin.x + 5.0f * 0.5f, pilot_waypoin.y + 5.0f * 0.5f);
                draw_list->AddRect(pilot_top_left, pilot_bottom_right, IM_COL32(255, 0, 255, 255));
                draw_list->AddLine(ImVec2(actor_canvas_pos.x, actor_canvas_pos.y),
                                   ImVec2(pilot_waypoin.x, pilot_waypoin.y), IM_COL32(255, 0, 255, 255));
                if (actor->target != nullptr) {
                    ImVec2 actor_target = ImVec2(
                        canvas_center.x + actor->target->object->position.x * scale_x,
                        canvas_center.y + actor->target->object->position.z * scale_z);
                    draw_list->AddLine(
                        ImVec2(actor_canvas_pos.x, actor_canvas_pos.y),
                        ImVec2(actor_target.x, actor_target.y), IM_COL32(255, 0, 0, 255)
                    );
                }
                
            }
            if (actor->is_active || actor->plane == this->player_plane) {
                ImVec2 plane_direction =
                    ImVec2(actor->plane->x - actor->plane->last_px, actor->plane->z - actor->plane->last_pz);
                draw_list->AddLine(ImVec2(actor_canvas_pos.x, actor_canvas_pos.y),
                                   ImVec2(actor_canvas_pos.x + plane_direction.x * 5.0f,
                                          actor_canvas_pos.y + plane_direction.y * 5.0f),
                                   actor_color);
            }

            // Optionally, display the actor id near the square.
            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y), IM_COL32(255, 255, 255, 255),
                               actor->actor_name.c_str());
            // if clicked, open a window with actor details
            if (ImGui::IsMouseHoveringRect(top_left, bottom_right)) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    this->target = actor;
                    this->current_mission->player->target = actor;
                    this->debug_entity_mode = DebugEntityMode::Actor;
                }
                ImGui::OpenPopup("Actor Details");
                if (ImGui::BeginPopup("Actor Details")) {
                    ImGui::Text("Actor: %s", actor->actor_name.c_str());
                    ImGui::SameLine();
                    ImGui::Text("ID: %d", actor->actor_id);
                    if (actor->plane) {
                        ImGui::Text("Air Speed: %d", actor->plane->airspeed);
                        ImGui::Text("Altitude: %.0f", actor->plane->y);
                        ImGui::Text("Heading: %.0f", 360 - (actor->plane->azimuthf / 10.0f));
                        ImGui::Text("Position: (%.2f, %.2f, %.2f)", actor->plane->x, actor->plane->y, actor->plane->z);
                    } else {
                        ImGui::Text("Position: (%.2f, %.2f, %.2f)", actor->object->position.x,
                                    actor->object->position.y, actor->object->position.z);
                    }
                    ImGui::Text("Current Target: %d" , actor->current_target);

                    ImGui::EndPopup();
                }
            }
        } else if (actor) {
            // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
            float world_x = actor->object->position.x;
            float world_z = -actor->object->position.z;
            ImVec2 actor_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x, canvas_center.y - world_z * scale_z);
            // Define square size.
            float square_size = 10.0f;

            // Draw a square centered at the actor's computed canvas position.
            ImVec2 top_left = ImVec2(actor_canvas_pos.x - square_size * 0.5f, actor_canvas_pos.y - square_size * 0.5f);
            ImVec2 bottom_right =
                ImVec2(actor_canvas_pos.x + square_size * 0.5f, actor_canvas_pos.y + square_size * 0.5f);
            ImVec2 circle_center = ImVec2(actor_canvas_pos.x, actor_canvas_pos.y);
            ImU32 color = IM_COL32(255, 255, 0, 255);
            if (actor->object == nullptr || actor->object->entity == nullptr) {
                continue;
            }
            switch (actor->object->entity->entity_type) {
            case EntityType::swpn:
                color = IM_COL32(0, 255, 0, 255);
                if (!actor->is_active) {
                    color = IM_COL32(0, 128, 0, 255);
                } 
                break;
            case EntityType::ornt:
                color = IM_COL32(255, 255, 0, 255);
                break;
            case EntityType::ground:
                color = IM_COL32(240, 255, 255, 255);
                if (!actor->is_active) {
                    color = IM_COL32(120, 128, 128, 255);
                } 
                break;
            case EntityType::destroyed_object:
                color = IM_COL32(128, 0, 128, 255);
                break;
            default:
                color = IM_COL32(255, 255, 0, 255);
                break;
            }
            
            if (actor->is_destroyed) {
                color = IM_COL32(128, 128, 128, 255);
            }
            draw_list->AddCircle(circle_center, square_size, color);
            draw_list->AddRect(top_left, bottom_right, color);
            if (ImGui::IsMouseHoveringRect(top_left, bottom_right)) {
                ImGui::OpenPopup("Actor Details");
                if (ImGui::BeginPopup("Actor Details")) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        this->target = actor;
                        this->debug_entity_mode = DebugEntityMode::Actor;
                    }
                    ImGui::Text("Actor: %s", actor->object->member_name.c_str());
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", actor->object->position.x, actor->object->position.y,
                                actor->object->position.z);
                    ImGui::EndPopup();
                }
            }
            // Optionally, display the actor id near the square.
            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y), IM_COL32(255, 255, 255, 255),
                               actor->object->member_name.c_str());
            for (auto wp : actor->weapons_shooted) {
                if (wp) {
                    // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
                    float wp_world_x = wp->x;
                    float wp_world_z = -wp->z;
                    ImVec2 wp_canvas_pos =
                        ImVec2(canvas_center.x + wp_world_x * scale_x, canvas_center.y - wp_world_z * scale_z);

                    // Draw a square centered at the weapon's computed canvas position.
                    ImVec2 wp_top_left = ImVec2(wp_canvas_pos.x - 5.0f * 0.5f, wp_canvas_pos.y - 5.0f * 0.5f);
                    ImVec2 wp_bottom_right = ImVec2(wp_canvas_pos.x + 5.0f * 0.5f, wp_canvas_pos.y + 5.0f * 0.5f);
                    
                    draw_list->AddRect(wp_top_left, wp_bottom_right, IM_COL32(255, 255, 0, 255));
                }
            }
        }
    }
    int cpt_scenes = 0;
    for (auto scene : this->current_mission->mission->mission_data.scenes) {
        if (scene) {
            int area_id = scene->area_id;
            float world_x = 0.0f;
            float world_z = 0.0f;
            if (area_id < 0 || area_id >= this->current_mission->mission->mission_data.areas.size()) {

            } else {
                world_x = this->current_mission->mission->mission_data.areas[area_id]->position.x;
                world_z = -this->current_mission->mission->mission_data.areas[area_id]->position.z;
            }

            ImVec2 scene_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x, canvas_center.y - world_z * scale_z);

            // Define square size.
            float square_size = 30.0f;

            // Draw a square centered at the scene's computed canvas position.
            ImVec2 top_left = ImVec2(scene_canvas_pos.x - square_size * 0.5f, scene_canvas_pos.y - square_size * 0.5f);
            ImVec2 bottom_right =
                ImVec2(scene_canvas_pos.x + square_size * 0.5f, scene_canvas_pos.y + square_size * 0.5f);
            draw_list->AddRect(top_left, bottom_right, IM_COL32(0, 255, 255, 255));
            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y), IM_COL32(255, 255, 255, 255),
                               ("Scene: " + std::to_string(cpt_scenes) + (scene->is_active?" ACTIVE ": " NON ACTIVE") ).c_str());
            if (ImGui::IsMouseHoveringRect(top_left, bottom_right)) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    this->debug_scene = scene;
                    this->debug_entity_mode = DebugEntityMode::Scene;
                }
            }
            cpt_scenes++;
        }
    }
}
void printProgTable(std::vector<PROG> &prog, SCMissionActors *actor, int prog_id = -1) {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    SCState &GameState = SCState::getInstance();
    if (ImGui::BeginTable("Flags", 2, flags)) {
        ImGui::TableSetupColumn("Opcode", ImGuiTableColumnFlags_WidthStretch, 0.8f);
        ImGui::TableSetupColumn("Valeur", ImGuiTableColumnFlags_WidthStretch, 0.2f);
        ImGui::TableHeadersRow();
        auto it = GameState.requierd_flags.begin();
        int cpt = 0;
        for (auto op : prog) {
            ImGui::TableNextRow();
            bool executed = false;
            if (prog_id >=0 && actor != nullptr) {
                for (auto opt_traced : actor->mission->progs_traces[prog_id]) {
                    if (opt_traced == cpt) {
                        executed = true;
                        break;
                    }
                }
            } else if (actor != nullptr) {
                for (auto opt_traced : actor->executed_opcodes) {
                    if (opt_traced == cpt) {
                        executed = true;
                        break;
                    }
                }
            }
            ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
            if (executed) {
                cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.35f));
            } else {
                cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.35f));
            }

            ImGui::TableSetColumnIndex(0);
            float tableWidth = ImGui::GetContentRegionAvail().x;
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            ImGui::Text("[%03d] %s ", op.opcode, prog_op_names[prog_op(op.opcode)].c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            ImGui::Text("[%d]", op.arg);
            cpt++;
        }
        ImGui::EndTable();
    }
}
void DebugStrike::showActorDetails(SCMissionActors *actor) {
    if (actor->object != nullptr) {
        ImGui::Text("Actor ID: %d", actor->actor_id);
        ImGui::Text("Actor Team Id: %d", actor->team_id);
        ImGui::Text("Active: %d", actor->is_active);
        ImGui::Text("Destroyed: %d", actor->is_destroyed);
        if (ImGui::TreeNode("Object")) {
            RSEntity *entity = actor->object->entity;
            ImGui::Text("Name: %s", actor->object->member_name.c_str());
            if (entity->explos != nullptr) {
                if (ImGui::TreeNode("Explosion")) {
                    ImGui::Text("Explosion: %s", entity->explos->name.c_str());
                    ImGui::Text("X %d, Y%d", entity->explos->x, entity->explos->y);
                    ImGui::TreePop();
                }
            }
            if (entity->jdyn != nullptr) {
                if (ImGui::TreeNode("JDYN")) {
                    ImGui::Text("WEIGHT: %d", entity->weight_in_kg);
                    ImGui::Text("THRUST: %d", entity->thrust_in_newton);
                    ImGui::Text("MAX G-LOAD: %d", entity->jdyn->max_g);
                    ImGui::Text("WING AREA: %.3f", entity->wing_area);
                    ImGui::TreePop();
                }
            }
            if (entity->sysm.size() > 0) {
                if (ImGui::TreeNode("SYSM")) {
                    for (auto& sys_pair : entity->sysm) {
                        if (ImGui::TreeNode(sys_pair.first.c_str())) {
                            for (auto& sys : sys_pair.second) {
                                ImGui::Text("Name: %s, pv %d", sys.first.c_str(), sys.second);
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("No object");
    }
    if (actor->profile != nullptr) {
        if (ImGui::TreeNode("Profile")) {
            ImGui::Text("Name %s", actor->profile->radi.info.name.c_str());
            ImGui::Text("CallSign %s", actor->profile->radi.info.callsign.c_str());
            ImGui::Text("Is AI %d", actor->profile->ai.isAI);
            if (ImGui::TreeNode("MSGS")) {
                for (auto msg : actor->profile->radi.msgs) {
                    ImGui::Text("- [%d]: %s", msg.first, msg.second.c_str());
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("ASKS")) {
                for (auto msg : actor->profile->radi.asks) {
                    ImGui::Text("- [%s]: %s", msg.first.c_str(), msg.second.c_str());
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("OPS")) {
                for (auto msg : actor->profile->radi.opts) {
                    ImGui::Text("- %c", msg);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("AI MVRS")) {
                for (auto ai : actor->profile->ai.mvrs) {
                    ImGui::Text("NODE [%d] - [%d]", ai.node_id, ai.value);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("AI GOAL")) {
                for (auto goal : actor->profile->ai.goal) {
                    ImGui::Text("[%d]", goal);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("AI ATRB")) {
                ImGui::Text("TH %d", actor->profile->ai.atrb.TH);
                ImGui::Text("CN %d", actor->profile->ai.atrb.CN);
                ImGui::Text("VB %d", actor->profile->ai.atrb.VB);
                ImGui::Text("LY %d", actor->profile->ai.atrb.LY);
                ImGui::Text("FL %d", actor->profile->ai.atrb.FL);
                ImGui::Text("AG %d", actor->profile->ai.atrb.AG);
                ImGui::Text("AA %d", actor->profile->ai.atrb.AA);
                ImGui::Text("SM %d", actor->profile->ai.atrb.SM);
                ImGui::Text("AR %d", actor->profile->ai.atrb.AR);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Score")) {
                ImGui::Text("Score %d", actor->score);
                ImGui::Text("Plane %d", actor->plane_down);
                ImGui::Text("Ground %d", actor->ground_down);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("Name %s", actor->actor_name.c_str());
        ImGui::Text("No profile");
    }
    if (ImGui::TreeNode("Progs")) {
        int cpt = 0;
        if (actor->on_is_activated.size() > 0) {
            if (ImGui::TreeNode("On is activated")) {
                if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_activated, "Prog %d", cpt)) {
                    printProgTable(actor->on_is_activated, actor, actor->object->on_is_activated);
                    ImGui::TreePop();
                    cpt++;
                }
                ImGui::TreePop();
            }
        }
        if (actor->on_update.size() > 0) {
            if (ImGui::TreeNode("On update")) {
                if (ImGui::TreeNode((void *)(intptr_t)&actor->on_update, "Prog %d", cpt)) {
                    printProgTable(actor->on_update, actor, actor->object->on_mission_update);
                    ImGui::TreePop();
                    cpt++;
                }

                ImGui::TreePop();
            }
        }
        if (actor->on_is_destroyed.size() > 0) {
            if (ImGui::TreeNode("On is destroyed")) {
                if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_destroyed, "Prog %d", cpt)) {
                    printProgTable(actor->on_is_destroyed, actor, actor->object->on_is_destroyed);
                    ImGui::TreePop();
                    cpt++;
                }
                ImGui::TreePop();
            }
        }
        if (actor->on_mission_start.size() > 0) {
            if (ImGui::TreeNode("On mission start")) {
                if (ImGui::TreeNode((void *)(intptr_t)&actor->on_mission_start, "Prog %d", cpt)) {
                    printProgTable(actor->on_mission_start, actor, actor->object->on_missions_init);
                    ImGui::TreePop();
                    cpt++;
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    if (actor->plane != nullptr) {
        if (ImGui::TreeNode("Plane")) {
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", actor->plane->x, actor->plane->y, actor->plane->z);
            ImGui::Text("Yaw: %.2f", actor->plane->yaw);
            ImGui::Text("Pitch: %.2f", actor->plane->pitch);
            ImGui::Text("Roll: %.2f", actor->plane->roll);
            ImGui::Text("Airspeed: %d", actor->plane->airspeed);
            ImGui::Text("Altitude: %.0f", actor->plane->y);
            if (actor->plane->weaps_load.size() > 0) {
                if (ImGui::TreeNode("Weapons")) {
                    for (auto weap : actor->plane->weaps_load) {
                        if (weap) {
                            ImGui::Text("Weapon: %s", weap->name.c_str());
                            ImGui::Text("Ammo: %d", weap->nb_weap);
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("No plane");
    }
}
void DebugStrike::renderMenu() {
    static bool show_simulation = false;
    static bool show_camera = false;
    static bool show_cockpit = false;
    static bool show_mission = false;
    static bool show_mission_parts_and_areas = false;
    static bool show_simulation_config = false;
    static bool show_textures = false;
    static bool show_music_player = false;
    static bool show_ai = false;
    static bool show_visual_config = false;

    static bool show_offcam = false;
    static bool show_load_plane = false;
    static bool show_radar = false;
    static bool show_gunsight_hud_offset = false;
    static bool show_player_ai = false;
    static bool show_decision_trace = false;
    if (ImGui::BeginMenu("SCStrike")) {
        ImGui::MenuItem("Mission", NULL, &show_mission);
        ImGui::MenuItem("Mission part and Area", NULL, &show_mission_parts_and_areas);
        ImGui::MenuItem("Music", NULL, &show_music_player);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Debug")) {
        ImGui::MenuItem("Simulation", NULL, &show_simulation);
        ImGui::MenuItem("Simulation config", NULL, &show_simulation_config);
        ImGui::MenuItem("Camera", NULL, &show_camera);
        ImGui::MenuItem("GunSight HUD angular offset", NULL, &show_gunsight_hud_offset);
        ImGui::MenuItem("Cockpit", NULL, &show_cockpit);
        ImGui::MenuItem("Show BBox", NULL, &this->show_bbox);
        ImGui::MenuItem("Ai pilot", NULL, &show_ai);
        ImGui::MenuItem("IA sur l'avion du joueur", NULL, &show_player_ai);
        ImGui::MenuItem("Trace de decision IA", NULL, &show_decision_trace);
        ImGui::MenuItem("Off camera", NULL, &show_offcam);
        ImGui::MenuItem("Load plane", NULL, &show_load_plane);
        ImGui::MenuItem("Radar", NULL, &show_radar);
        ImGui::MenuItem("Show Textures", NULL, &show_textures);
        ImGui::MenuItem("Visual config", NULL, &show_visual_config);
        ImGui::EndMenu();
    }
    int sceneid = -1;
    Vector3D plane_player_pos = {this->player_plane->x, this->player_plane->y, this->player_plane->z};
    uint8_t area_id = this->current_mission->getAreaID(plane_player_pos);
    ImGui::Text("Speed %d\tAltitude %.0f\tHeading %.0f\tTPS: %03d\tArea %s\tfilename: %s\tArea ID: %d",
                this->player_plane->airspeed, this->new_position.y, 360 - (this->player_plane->azimuthf / 10.0f),
                this->player_plane->tps, this->current_mission->mission->mission_data.name.c_str(),
                this->miss_file_name.c_str(), area_id);

    if (show_visual_config) {
        ImGui::Begin("Visual Config", &show_visual_config);
        ImGui::Text("Visual Config");
        SCRenderer &renderer = SCRenderer::getInstance();
        if (ImGui::SliderInt("Max view distance", &renderer.max_view_distance, 1000, 300000)) {
            renderer.resetCameraPerspective();
        }
        ImGui::SliderInt("World LOD", &this->world_lod, 0, 2);
        ImGui::SliderInt("Object LOD", &renderer.lodLevel, 0, 2);
        ImGui::Checkbox("Textures", &renderer.show_textured);
        ImGui::Checkbox("Show Fog", &renderer.show_fog);
        ImGui::Checkbox("Show Clouds", &renderer.show_clouds);
        ImGui::End();
    }
    if (show_textures) {
        this->showTextures();
    }
    if (show_player_ai) {
        ImGui::Begin("IA sur l'avion du joueur", &show_player_ai);
        this->playerAIConfig();
        ImGui::End();
    }
    if (show_decision_trace) {
        ImGui::Begin("Trace de decision IA", &show_decision_trace);
        this->decisionTraceWindow();
        ImGui::End();
    }
    if (show_offcam) {
        ImGui::Begin("Offcam", &show_offcam);
        showOffCamera();
        ImGui::End();
    }
    if (show_load_plane) {
        ImGui::Begin("Load plane", &show_load_plane);
        loadPlane();
        ImGui::End();
    }
    if (show_radar) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove;
        static bool is_dragging_window = false;

        if (ImGui::Begin("Actors Position", &show_radar, window_flags)) {
            // Vérifier si la souris est sur la barre de titre
            bool is_titlebar_hovered =
                ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(),
                                           ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x,
                                                  ImGui::GetWindowPos().y + ImGui::GetFrameHeight()),
                                           false);

            // Commencer à déplacer si on clique sur la barre de titre
            if (is_titlebar_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                is_dragging_window = true;

            // Arrêter de déplacer si on relâche le bouton
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                is_dragging_window = false;

            // Déplacer la fenêtre si on est en train de la faire glisser
            if (is_dragging_window && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetIO().MouseDelta.x,
                                           ImGui::GetWindowPos().y + ImGui::GetIO().MouseDelta.y));
            }

            radar();
            ImGui::End();
        }
    }
    if (show_ai) {
        ImGui::Begin("AI", &show_ai);
        this->aiOverview();
        ImGui::End();
    }
    if (show_music_player) {
        ImGui::Begin("Music Player", &show_music_player);
        static int music_id = 0;
        ImGui::PushItemWidth(80.0f);
        if (ImGui::BeginCombo("Music list", std::to_string(music_id).c_str(), 0)) {
            for (int i = 0; i < Mixer.music->musics[2].size(); i++) {
                if (ImGui::Selectable(std::to_string(i).c_str(), i == music_id)) {
                    music_id = i;
                }
                if (i == music_id) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        if (ImGui::Button("Play")) {
            Mixer.playMusic(music_id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            Mixer.stopMusic();
        }
        ImGui::SameLine();
        if (ImGui::Button("Next")) {
            music_id++;
            if (music_id >= Mixer.music->musics[2].size()) {
                music_id = 0;
            }
            Mixer.playMusic(music_id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Prev")) {
            music_id--;
            if (music_id < 0) {
                music_id = (int) Mixer.music->musics[2].size() - 1;
            }
            Mixer.playMusic(music_id);
        }
        ImGui::End();
    }
    if (show_simulation_config) {
        ImGui::Begin("Simulation Config", &show_simulation_config);
        simConfig();
        ImGui::End();
    }
    if (show_simulation) {
        ImGui::Begin("Simulation", &show_simulation);
        simInfo();
        ImGui::End();
    }
    if (show_camera) {
        ImGui::Begin("Camera", &show_camera);
        ImGui::Text("Tps %d", this->player_plane->tps);
        ImGui::Text("Camera mode %d", this->camera_mode);
        ImGui::Text("Position [%.3f,%.3f,%.3f]", this->camera_pos.x, this->camera_pos.y, this->camera_pos.z);
        ImGui::Text("Pilot lookat [%d,%d]", this->pilote_lookat.x, this->pilote_lookat.y);
        ImGui::SliderFloat("Camera pos x", &this->camera_pos.x, -150.0f, 150.0f);
        ImGui::SliderFloat("Camera pos y", &this->camera_pos.y, -150.0f, 150.0f);
        ImGui::SliderFloat("Camera pos z", &this->camera_pos.z, -150.0f, 150.0f);
        ImGui::Text("Light position [%.3f,%.3f,%.3f]", this->light.x, this->light.y, this->light.z);
        ImGui::Text("Renderer Light [%.3f,%.3f,%.3f]", Renderer.light.x,
                    Renderer.light.y, Renderer.light.z);
        ImGui::SliderFloat("Light x", &this->light.x, -150.0f, 150.0f);
        ImGui::SliderFloat("Light y", &this->light.y, -150.0f, 150.0f);
        ImGui::SliderFloat("Light z", &this->light.z, -150.0f, 150.0f);
        ImGui::End();
    }
    if (show_gunsight_hud_offset) {
        ImGui::Begin("GunSight HUD angular offset", &show_gunsight_hud_offset);
        ImGui::Text("Current offset: [%.2f, %.2f]", this->gunsight_hud_offset.x,
                    this->gunsight_hud_offset.y);
        ImGui::SliderFloat("Offset X", &this->gunsight_hud_offset.x, -1.0f, 1.0f);
        ImGui::SliderFloat("Offset Y", &this->gunsight_hud_offset.y, -1.0f, 1.0f);
        ImGui::End();
    }
    if (show_cockpit) {
        ImGui::Begin("Cockpit", &show_cockpit);
        ImGui::Text("Throttle %d", this->player_plane->GetThrottle());

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetWheel()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Gears")) {
            this->player_plane->SetWheel();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetFlaps()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Flaps")) {
            this->player_plane->SetFlaps();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetSpoilers()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Breaks")) {
            this->player_plane->SetSpoilers();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        if (this->mouse_control) {
            ImGui::SameLine();
            ImGui::Text("Mouse control enabled");
        }
        ImGui::Text("Target %s", this->current_mission->actors[this->current_target]->actor_name.c_str());
        if (this->target != nullptr && this->target->object != nullptr) {
            Vector3D targetPos = {this->target->object->position.x, this->target->object->position.y,
                                this->target->object->position.z};
            Vector3D playerPos = {this->new_position.x, this->new_position.y, this->new_position.z};

            Camera totarget;
            totarget.SetPosition(&playerPos);
            totarget.lookAt(&targetPos);
            float raw;
            float yawn;
            float pitch;
            raw = 0.0f;
            yawn = 0.0f;
            pitch = 0.0f;

            totarget.getOrientation().GetAngles(pitch, yawn, raw);

            Vector3D directionVector = {targetPos.x - playerPos.x, targetPos.y - playerPos.y, targetPos.z - playerPos.z};

            ImGui::Text("Target position [%.3f,%.3f,%.3f]", targetPos.x, targetPos.y, targetPos.z);
            ImGui::Text("Player position [%.3f,%.3f,%.3f]", playerPos.x, playerPos.y, playerPos.z);
            ImGui::Text("Azimuth to target %.3f",
                        (360 - (this->player_plane->azimuthf / 10.0f)) -
                            (atan2(targetPos.z - playerPos.z, targetPos.x - playerPos.x) * (180.0 / M_PI) + 90.0f));
            ImGui::Text("Q angles [%.3f,%.3f,%.3f] rel %.3f", pitch * 180.0 / M_PI, yawn * 180.0 / M_PI,
                        180.0f - (raw * 180.0 / M_PI),
                        (180.0f - (raw * 180.0 / M_PI)) - (360 - (this->player_plane->azimuthf / 10.0f)));
            if (ImGui::TreeNode("missiles")) {
                for (auto missils : this->player_plane->weaps_object) {
                    ImGui::Text("VX:%.3f\tVY:%.3f\tVZ:%.3f\televation:%.3f\tazimut:%.3f\tspeed%.3f", missils->vx,
                                missils->vy, missils->vz, missils->elevationf, missils->azimuthf,
                                Vector3D(missils->vx, missils->vy, missils->vz).Length());
                    ImGui::Text("X:%.3f\tY:%.3f\tZ:%.3f", missils->x + 180000.0f, missils->y, missils->z + 180000.0f);
                    if (missils->target != nullptr) {
                        ImGui::Text("Target X:%.0f\tY:%.0f\tZ:%.0f", missils->target->object->position.x,
                                    missils->target->object->position.y, missils->target->object->position.z);
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }
    if (show_mission) {
        renderMissionLoadingDialog(show_mission);
    }

    if (show_mission_parts_and_areas) {
        ImGui::Begin("Mission Parts and Areas", &show_mission_parts_and_areas);
        ImGui::Text("Mission %s", this->current_mission->mission->mission_data.name.c_str());
        ImGui::Text("Area %s", this->current_mission->mission->mission_data.world_filename.c_str());
        ImGui::Text("Player Coord %.0f %.0f %.0f", this->current_mission->mission->getPlayerCoord()->position.x,
                    this->current_mission->mission->getPlayerCoord()->position.y,
                    this->current_mission->mission->getPlayerCoord()->position.z);
        ImGui::Text("Messages %d", this->current_mission->mission->mission_data.messages.size());
        if (ImGui::TreeNode("Messages")) {
            for (auto msg : this->current_mission->mission->mission_data.messages) {
                ImGui::Text("- %s", msg->c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Areas")) {
            for (auto area : this->current_mission->mission->mission_data.areas) {
                if (ImGui::TreeNode((void *)(intptr_t)area->id, "Area id %d", area->id)) {
                    ImGui::Text("Area name %s", area->AreaName);
                    ImGui::Text("Area Type %c", area->AreaType);
                    ImGui::Text("Area x %.0f y %.0f z %.0f", area->position.x, area->position.y, area->position.z);
                    ImGui::Text("Area width %d height %d", area->AreaWidth, area->AreaHeight);
                    for (auto ub : area->unknown_bytes) {
                        ImGui::Text("ub %d", ub);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Area Objects")) {
            for (int g = 0; g < area->objects.size(); g++) {
                auto obj = &area->objects.at(g);
                if (ImGui::TreeNode((void *)(intptr_t)g, "Object id %d", g)) {
                    ImGui::Text("Object name %s", obj->name);
                    ImGui::Text("Object x %.3f y %.3f z %.3f", obj->position.x, obj->position.y, obj->position.z);
                    for (auto unknown_byte : obj->unknowns) {
                        ImGui::Text("ub %d", unknown_byte);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Area overlay")) {
            for (auto overlay : this->current_mission->area->objectOverlay) {
                ImGui::Text("Overlay nbtrianbles %d", overlay.nbTriangles);
                ImGui::Text("Overlay nbvertices %d", overlay.verticesVec.size());
                for (auto vertex : overlay.verticesVec) {
                    ImGui::Text("Overlay vertex x %d y %d z %d", vertex.x, vertex.y, vertex.z);
                }
                for (int i=0; i < overlay.verticesVec.size(); i++) {
                    ImGui::Text("Overlay vertices [%d] (%d, %d, %d)",i, overlay.vertices[i].x, overlay.vertices[i].y, overlay.vertices[i].z);
                }
                for (int i=0; i < overlay.nbTriangles; i++) {
                    ImGui::Text("Overlay triangle [%d] vertices:%d, %d, %d",i, 
                        overlay.trianles[i].verticesIdx[0],
                        overlay.trianles[i].verticesIdx[1],
                        overlay.trianles[i].verticesIdx[2]
                    );
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("SPOT")) {
            for (auto spot : this->current_mission->mission->mission_data.spots) {
                if (ImGui::TreeNode((void *)(intptr_t)spot, "Spot %d", spot->id)) {
                    ImGui::Text("Spot area_id %d", spot->area_id);
                    ImGui::Text("Spot x %.0f y %.0f z %.0f", spot->position.x, spot->position.y, spot->position.z);
                    ImGui::Text("Spot from file x %.0f y %.0f z %.0f", spot->origin.x, spot->origin.y, spot->origin.z);
                    ImGui::Text("u1 %d", spot->unknown1);
                    ImGui::Text("u2 %d", spot->unknown2);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("PROG")) {
            int id = 1;
            for (auto prog : this->current_mission->mission->mission_data.prog) {
                if (ImGui::TreeNode((void *)(intptr_t)prog, "Prog %d", id)) {
                    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                    if (ImGui::BeginTable("Flags", 2, flags)) {
                        auto it = GameState.requierd_flags.begin();
                        for (auto op : *prog) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("opcode:[%03d] %s ", op.opcode, prog_op_names[prog_op(op.opcode)].c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("arg:[%d]", op.arg);
                        }
                        ImGui::EndTable();
                    }

                    ImGui::TreePop();
                }
                id++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Flags")) {
            int cpt_flags = 0;
            for (auto flag : this->current_mission->mission->mission_data.flags) {
                ImGui::Text("Flag %d : %d", cpt_flags, flag);
                cpt_flags++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Scenes")) {
            int cpt_scenes = 0;
            for (auto scene : this->current_mission->mission->mission_data.scenes) {
                if (ImGui::TreeNode((void *)(intptr_t)scene, "Scene %d", cpt_scenes)) {
                    if (scene->area_id >= 0) {
                        ImGui::Text("Scene area_id %s",
                                    this->current_mission->mission->mission_data.areas[scene->area_id]->AreaName);
                    } else {
                        ImGui::Text("Scene area_id %d", scene->area_id);
                    }
                    ImGui::Text("IS ACTIVE %d", scene->is_active);
                    ImGui::Text("Is COORD RELATIVE TO AREA %d", scene->is_coord_on_area);
                    for (auto cast : scene->cast) {
                        ImGui::Text("Cast %d", cast);
                    }
                    for (auto prog : scene->progs_id) {
                        ImGui::Text("Prog %d", prog);
                    }
                    for (auto b : scene->unknown_bytes) {
                        ImGui::Text("ub %d", b);
                    }
                    ImGui::TreePop();
                }
                cpt_scenes++;
            }
            ImGui::TreePop();
        }
        ImGui::Text("Mission Parts %d", this->current_mission->mission->mission_data.parts.size());
        if (ImGui::TreeNode("Parts")) {
            for (auto part : this->current_mission->mission->mission_data.parts) {
                if (ImGui::TreeNode((void *)(intptr_t)part->id, "Parts id %d, area id %d", part->id, part->area_id)) {
                    ImGui::Text("u1 %d", part->unknown1);
                    ImGui::Text("u2 %d", part->unknown2);
                    ImGui::Text("x %.0f y %.0f z %.0f", part->position.x, part->position.y, part->position.z);
                    ImGui::Text("azymuth %d", part->azymuth);
                    ImGui::Text("u3 %d", part->unknown3);
                    ImGui::Text("Name %s", part->member_name.c_str());
                    ImGui::Text("Destroyed %s", part->member_name_destroyed.c_str());
                    for (auto obj : part->unknown_bytes) {
                        ImGui::Text("ub %d", obj);
                    }
                    ImGui::Text("Weapon load %s", part->weapon_load.c_str());
                    if (ImGui::TreeNode("RS ENTITY")) {
                        ImGui::Text("weight %d", part->entity->weight_in_kg);
                        ImGui::Text("Thrust %d", part->entity->thrust_in_newton);
                        if (ImGui::TreeNode("childs")) {
                            for (auto child : part->entity->chld) {
                                ImGui::Text("%s - %d %d %d", child->name.c_str(), child->x, child->y, child->z);
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("weapons")) {
                            for (auto weapon : part->entity->weaps) {
                                ImGui::Text("%s - %d", weapon->name.c_str(), weapon->nb_weap);
                                if (weapon->objct != nullptr) {
                                    if (ImGui::TreeNode("weapons")) {
                                        if (weapon->objct->wdat != nullptr) {
                                            ImGui::Text(
                                                "Weapon DATA %d, %d, %d, %d, %d", weapon->objct->wdat->damage,
                                                weapon->objct->wdat->effective_range, weapon->objct->wdat->target_range,
                                                weapon->objct->wdat->tracking_cone, weapon->objct->wdat->radius);
                                        }
                                        if (weapon->objct->dynn_miss != nullptr) {
                                            ImGui::Text("Dynamics: %d, %d, %d, %d",
                                                        weapon->objct->dynn_miss->velovity_m_per_sec,
                                                        weapon->objct->dynn_miss->turn_degre_per_sec,
                                                        weapon->objct->dynn_miss->proximity_cm,
                                                        weapon->objct->weight_in_kg);
                                        }
                                        ImGui::TreePop();
                                    }
                                }
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("hpts")) {
                            for (auto hpt : part->entity->hpts) {
                                ImGui::Text("%d - %d %d %d", hpt->id, hpt->x, hpt->y, hpt->z);
                            }
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Mission Actors")) {
            for (auto actor : current_mission->actors) {
                if (ImGui::TreeNode((void *)(intptr_t)actor, "Actor %d", actor->actor_id)) {
                    showActorDetails(actor);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Casting")) {
            for (auto cast : this->current_mission->mission->mission_data.casting) {
                if (ImGui::TreeNode((void *)(intptr_t)cast, "Actor %s", cast->actor.c_str())) {
                    if (cast->profile != nullptr) {
                        ImGui::Text("Name %s", cast->profile->radi.info.name.c_str());
                        ImGui::Text("CallSign %s", cast->profile->radi.info.callsign.c_str());
                        ImGui::Text("Is AI %d", cast->profile->ai.isAI);
                        if (ImGui::TreeNode("MSGS")) {
                            for (auto msg : cast->profile->radi.msgs) {
                                ImGui::Text("- [%d]: %s", msg.first, msg.second.c_str());
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("ASKS")) {
                            for (auto msg : cast->profile->radi.asks) {
                                ImGui::Text("- [%s]: %s", msg.first.c_str(), msg.second.c_str());
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("ASKS_VECTOR")) {
                            for (auto msg : cast->profile->radi.asks_vector) {
                                ImGui::Text("- %s", msg.c_str());
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }
}

void DebugStrike::renderUI() {
    if (ImGui::BeginTabBar("DebugTabs")) {
        if (ImGui::BeginTabItem("Mission Info")) {
            ImGui::BeginChild("Radar", ImVec2(0, 400), true);
            radar();
            ImGui::EndChild();
            ImGui::BeginChild("View", ImVec2(0, 200), true);
            ImGui::BeginChild("Camera", ImVec2(250, 0), true);
            showOffCamera();
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("Target", ImVec2(0, 0), true);
            if (this->target != nullptr && this->target->object != nullptr) {
                ImGui::Text("Target: %s", this->target->object->member_name.c_str());
                if (this->target->plane) {
                    ImGui::Text("Air Speed: %d", this->target->plane->airspeed);
                    ImGui::Text("Altitude: %.0f", this->target->plane->y);
                    ImGui::Text("Heading: %.0f", 360 - (this->target->plane->azimuthf / 10.0f));
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", this->target->plane->x, this->target->plane->y,
                                this->target->plane->z);
                } else {
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", this->target->object->position.x,
                                this->target->object->position.y, this->target->object->position.z);
                }
                if (this->target->pilot != nullptr) {
                    ImGui::Text("{TH %.0f TA %d}", this->target->pilot->target_azimut,
                                this->target->pilot->target_climb);
                    ImGui::Text("Turning %d, L/R/N %d, TRoll %.3f", this->target->pilot->turning,
                                this->target->pilot->turnState, this->target->pilot->targetRoll);
                    ImGui::Text("AI OBJ %d", this->target->current_objective);

                    ImGui::Text("AI Target %d", this->target->current_target);
                }
                if (ImGui::Button("Destroy")) {
                    this->target->object->alive = false;
                }
            }
            ImGui::EndChild();
            ImGui::EndChild();
            ImGui::BeginChild("Target info", ImVec2(0, 0), true);
            switch (debug_entity_mode) {
            case DebugEntityMode::None:
                ImGui::Text("No entity selected");
                break;
            case DebugEntityMode::Actor:
                if (this->target != nullptr) {
                    showActorDetails(this->target);
                } else {
                    ImGui::Text("No target selected");
                }
                break;
            case DebugEntityMode::Spot:
                if (this->debug_spot != nullptr) {
                    ImGui::Text("Spot ID: %d", this->debug_spot->id);
                    ImGui::Text("Position: (%.0f, %.0f, %.0f)", this->debug_spot->position.x,
                                this->debug_spot->position.y, this->debug_spot->position.z);
                    ImGui::Text("Origin: (%.0f, %.0f, %.0f)", this->debug_spot->origin.x, this->debug_spot->origin.y,
                                this->debug_spot->origin.z);
                    ImGui::Text("Area ID: %d", this->debug_spot->area_id);
                    ImGui::Text("Unknown1: %d", this->debug_spot->unknown1);
                    ImGui::Text("Unknown2: %d", this->debug_spot->unknown2);
                } else {
                    ImGui::Text("No spot selected");
                }
                break;
            case DebugEntityMode::Area:
                if (this->debug_area != nullptr) {
                    ImGui::Text("Area ID: %d", this->debug_area->id);
                    ImGui::Text("Area Name: %s", this->debug_area->AreaName);
                    ImGui::Text("Position: (%.0f, %.0f, %.0f)", this->debug_area->position.x,
                                this->debug_area->position.y, this->debug_area->position.z);
                    ImGui::Text("Width: %d, Height: %d", this->debug_area->AreaWidth, this->debug_area->AreaHeight);
                } else {
                    ImGui::Text("No area selected");
                }
                break;
            case DebugEntityMode::Scene:
                if (this->debug_scene != nullptr) {
                    ImGui::Text("Scene ID: %d", this->debug_scene->area_id);
                    ImGui::Text("Is Active: %d", this->debug_scene->is_active);
                    ImGui::Text("Is Coord Relative to Area: %d", this->debug_scene->is_coord_on_area);
                    if (this->debug_scene->cast.size() > 0) {
                        if (ImGui::TreeNode("Cast Actors")) {
                            for (auto cast_actor : this->debug_scene->cast) {
                                if (ImGui::TreeNode((void *)(intptr_t)cast_actor, "Cast Actor [SCENE ID:%d]", cast_actor)) {
                                    int i = 0;
                                    for (auto part : this->current_mission->mission->mission_data.parts) {
                                        if (i == cast_actor) {
                                            for (auto actor : this->current_mission->actors) {
                                                if (actor->actor_id == part->id) {
                                                    showActorDetails(actor);
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                        i++;
                                    }
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                    }

                    bool has_progs = false;
                    for (auto prog : this->debug_scene->progs_id) {
                        if (prog != -1) {
                            has_progs = true;
                            break;
                        }
                    }
                    if (has_progs) {
                        if (this->debug_scene->on_is_activated > 0) {
                            if (ImGui::TreeNode("On is activated Prog")) {
                                ImGui::Text("On is activated: %d", this->debug_scene->on_is_activated);
                                std::vector<PROG> &p = *(this->current_mission->mission->mission_data.prog[this->debug_scene->on_is_activated]);
                                printProgTable(p, this->current_mission->player, this->debug_scene->on_is_activated);
                                ImGui::TreePop();
                            }
                        }
                        if (this->debug_scene->on_mission_update > 0) {
                            if (ImGui::TreeNode("On updated Prog")) {
                                ImGui::Text("On updated: %d", this->debug_scene->on_mission_update);
                                std::vector<PROG> &p = *(this->current_mission->mission->mission_data.prog[this->debug_scene->on_mission_update]);
                                printProgTable(p, this->current_mission->player, this->debug_scene->on_mission_update);
                                ImGui::TreePop();
                            }
                        }
                        if (this->debug_scene->on_leaving > 0) {
                            if (ImGui::TreeNode("On leaving Prog")) {
                                ImGui::Text("On leaving: %d", this->debug_scene->on_leaving);
                                std::vector<PROG> &p = *(this->current_mission->mission->mission_data.prog[this->debug_scene->on_leaving]);
                                printProgTable(p, this->current_mission->player, this->debug_scene->on_leaving);
                                ImGui::TreePop();
                            }
                        }
                    }
                    if (ImGui::TreeNode("Unknown Bytes")) {
                        for (auto b : this->debug_scene->unknown_bytes) {
                            ImGui::Text("ub %d", b);
                        }
                        ImGui::TreePop();
                    }
                } else {
                    ImGui::Text("No scene selected");
                }
                break;
                default:
                    ImGui::Text("No entity selected");
                break;
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Simulation")) {
            ImGui::BeginChild("Simulation Info", ImVec2(0, 370), true);
            simInfo();
            ImGui::EndChild();
            ImGui::BeginChild("Simulation config", ImVec2(0, 250), true);
            simConfig();
            ImGui::EndChild();
            ImGui::BeginChild("Planes", ImVec2(0, 0), true);
            loadPlane();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Missions Scripts")) {
            ImGui::BeginChild("Working Registers", ImVec2(0, 400), true);
            renderWorkingRegisters();
            ImGui::EndChild();
            ImGui::BeginChild("Progs", ImVec2(0, 0), true);
            static int selectedProgIndex = 0;

            ImGui::Text("Mission Programs");
            ImGui::Separator();

            // Get the list of progs
            auto& progs = this->current_mission->mission->mission_data.prog;
            std::vector<std::string> progLabels;

            // Create labels for each prog
            for (int i = 0; i < progs.size(); i++) {
                progLabels.push_back("Program #" + std::to_string(i));
            }

            if (selectedProgIndex >= progs.size()) {
                selectedProgIndex = progs.size() - 1;
            }
            // Combo box for selecting a prog
            if (ImGui::BeginCombo("Select Program", progLabels.empty() ? "No programs" : progLabels[selectedProgIndex].c_str())) {
                for (int i = 0; i < progLabels.size(); i++) {
                    bool isSelected = (selectedProgIndex == i);
                    if (ImGui::Selectable(progLabels[i].c_str(), isSelected)) {
                        selectedProgIndex = i;
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Display the selected prog's contents
            if (!progLabels.empty() && selectedProgIndex >= 0 && selectedProgIndex < progs.size()) {
                ImGui::Separator();
                ImGui::Text("Program #%d Operations", selectedProgIndex);
                
                // Using the existing printProgTable function to display the program
                if (progs[selectedProgIndex]) {
                    printProgTable(*progs[selectedProgIndex], this->current_mission->player, selectedProgIndex);
                    if (ImGui::Button("Run Program")) {
                        SCProg *program = new SCProg(this->current_mission->player, *progs[selectedProgIndex], this->current_mission, selectedProgIndex);
                        program->execute();
                        delete program;
                    }
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Program is nullptr");
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        // Additional tabs can be added here.
        ImGui::EndTabBar();
    }
}

void DebugStrike::renderWorkingRegisters() {
    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    
    if (ImGui::BeginTable("Mission Flags", 2, tableFlags)) {
        ImGui::TableSetupColumn("Flag Index");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();
        
        int cpt_flags = 0;
        for (auto flag : this->current_mission->mission->mission_data.flags) {
            
            if (flag != 0) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", cpt_flags);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", flag);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.0f, 0.5f)));
            }
            cpt_flags++;
        }
        ImGui::EndTable();
    }
    if (ImGui::BeginTable("GameFlow Register", 2, tableFlags)) {
        ImGui::TableSetupColumn("GameFlow Register");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();
        
        int cpt_flags = 0;
        for (auto flag : this->current_mission->gameflow_registers) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", flag.first);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", flag.second);
            if (flag.second != 0) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.0f, 0.5f)));
            }
            cpt_flags++;
        }
        ImGui::EndTable();
    }
}
void DebugStrike::showTextures() {
    ImGui::Begin("Textures");
    std::vector<Texture *> textures;
    for (const auto part: this->current_mission->mission->mission_data.parts) {
        if (part->entity != nullptr) {
            for (const auto &texture : part->entity->images) {
                textures.push_back(texture->GetTexture());
            }
        }
    }
    ImGui::Text("Textures loaded: %d", textures.size());
    if (ImGui::BeginTable("TexturesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Texture Name");
        ImGui::TableSetupColumn("Texture ID");
        ImGui::TableHeadersRow();
        for (const auto &texture : textures) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", texture->name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Image((ImTextureID)(intptr_t)texture->id, ImVec2(100, 100));
        }
        ImGui::EndTable();
    }
    ImGui::End();
}
void DebugStrike::renderMissionLoadingDialog(bool &show_mission) {
    ImGui::Begin("Mission", &show_mission);
    std::vector<const char *> to_mission_list = {
        "MISN-a1.IFF", "MISN-a2.IFF", "MISN-b1.IFF", "MISN-bw.IFF", "MISN-b2.IFF", "MISN-b3.IFF",
        "MISN-b4.IFF", "MISN-c1.IFF", "MISN-cw.IFF", "MISN-c2.IFF", "MISN-c3.IFF", "MISN-cy.IFF",
        "MISN-c4.IFF", "MISN-cz.IFF", "MISN-d1.IFF", "MISN-dw.IFF", "MISN-d2.IFF", "MISN-e1.IFF",
        "MISN-e2.IFF", "MISN-e3.IFF", "MISN-e4.IFF", "MISN-f1.IFF", "MISN-f2.IFF", "MISN-fx.IFF",
        "MISN-f3.IFF", "MISN-f4.IFF", "MISN-g1.IFF", "MISN-g2.IFF", "MISN-g3.IFF", "MISN-h1.IFF"};
    static ImGuiComboFlags flags = 0;
    static int to_mission_idx = 0;
    ImGui::PushItemWidth(120.0f);
    if (ImGui::BeginCombo("Strike Commander", mission_list[mission_idx], flags)) {
        for (int n = 0; n < SCSTRIKE_MAX_MISSIONS; n++) {
            const bool is_selected = (mission_idx == n);
            if (ImGui::Selectable(mission_list[n], is_selected))
                mission_idx = n;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine(300);
    if (ImGui::Button("Load SC Mission")) {
        Assets.navmap_filename = "..\\..\\DATA\\COCKPITS\\NAVMAP.IFF";
        Assets.navmap_add_filename = "";
        this->setMission((char *)mission_list[mission_idx]);
    }
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(120.0f);
    if (ImGui::BeginCombo("Tactical Operation", to_mission_list[to_mission_idx], flags)) {
        for (int n = 0; n < to_mission_list.size(); n++) {
            const bool is_selected = (to_mission_idx == n);
            if (ImGui::Selectable(to_mission_list[n], is_selected))
                to_mission_idx = n;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine(300);
    if (ImGui::Button("Load TO Mission")) {
        Assets.navmap_filename = "..\\..\\DATA\\COCKPITS\\NAVMAP.IFF";
        Assets.navmap_add_filename = "..\\..\\DATA\\COCKPITS\\NAVMAP2.IFF";
        this->setMission((char *)to_mission_list[to_mission_idx]);
    }
    ImGui::PopItemWidth();
    ImGui::End();
}