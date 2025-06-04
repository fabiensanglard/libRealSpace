#include "../../strike_commander/precomp.h"

class DebugStrike : public SCStrike {
private:
    void radar();
    void simInfo();
    void simConfig();
    void loadPlane();
    void showActorDetails(SCMissionActors* actor);
    void showOffCamera();
public:
    DebugStrike();
    ~DebugStrike();
    void init();
    void setMission(std::string mission_name);
    void renderMenu();
    void renderUI() override;
};