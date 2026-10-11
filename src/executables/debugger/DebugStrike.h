#pragma once
#include "../../strike_commander/precomp.h"

class DebugStrike : public SCStrike {
protected:
    virtual void radar();
    virtual void simInfo();
    virtual void simConfig();
    virtual void loadPlane();
    virtual void showActorDetails(SCMissionActors* actor);
    virtual void showOffCamera();
    virtual void showTextures();
    virtual void renderMissionLoadingDialog(bool &show_mission);
    virtual void playerAIConfig();
    void enablePlayerAI(RSProf *profile);
    void disablePlayerAI();
    void playerAIOrders();
    void restoreDebugOrder();
    void giveDebugOrder(prog_op command, uint8_t arg, uint8_t arg2);
    void playerAIControls();
    void playerAIState();
    void evaluateTournament();
    void maneuverTestConfig();
    void maneuverTestTick();
    SCMissionActors *testManeuverTarget();
    bool spotCombo(const char *label, uint8_t &spot, bool allow_none);
    bool actorCombo(const char *label, uint8_t &actor_id);
    enum class DebugEntityMode { None, Actor, Scene, Area, Spot, Entity };
    DebugEntityMode debug_entity_mode{DebugEntityMode::Actor};
    RSEntity* debug_entity{nullptr};
    MISN_SCEN* debug_scene{nullptr};
    AREA* debug_area{nullptr};
    SPOT* debug_spot{nullptr};
    SCState &GameState = SCState::getInstance();
    RSProf *player_profile_saved{nullptr};
    float player_ai_accumulator{0.0f};
    prog_op debug_order_command{prog_op::OP_NOOP};
    uint8_t debug_order_arg{0};
    uint8_t debug_order_arg2{0xFF};
    bool debug_order_executed{false};
    SCMissionActors *debug_order_target{nullptr};
    prog_op saved_command{prog_op::OP_NOOP};
    uint8_t saved_command_arg{0};
    uint8_t saved_command_arg2{0xFF};
    bool saved_command_executed{false};
    SCMissionActors *saved_target{nullptr};
    std::vector<std::tuple<int, int, int>> tournament_scores;
    int test_maneuver_index{6};
    uint8_t test_maneuver_target{0xFF};
    bool maneuver_test{false};
    int maneuver_test_policy{0};
    bool maneuver_test_reflexes{true};
    int maneuver_test_runs{0};
public:
    DebugStrike();
    ~DebugStrike();
    void init() override;
    virtual void setMission(std::string mission_name);
    virtual void renderMenu() override;
    virtual void renderUI() override;
    virtual void renderWorkingRegisters();
    void runFrame(void) override;
};