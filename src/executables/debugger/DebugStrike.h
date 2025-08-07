#include "../../strike_commander/precomp.h"

class DebugStrike : public SCStrike {
private:
    void radar();
    void simInfo();
    void simConfig();
    void loadPlane();
    void showActorDetails(SCMissionActors* actor);
    void showOffCamera();
    void showTextures();
    enum class DebugEntityMode {
        None,
        Actor,
        Scene,
        Area,
        Spot,
        Entity
    };
    DebugEntityMode debug_entity_mode{DebugEntityMode::Actor};
    RSEntity* debug_entity{nullptr};
    MISN_SCEN* debug_scene{nullptr};
    AREA* debug_area{nullptr};
    SPOT* debug_spot{nullptr};

public:
    DebugStrike();
    ~DebugStrike();
    void init();
    void setMission(std::string mission_name);
    void renderMenu();
    void renderUI() override;
    void renderWorkingRegisters();
};