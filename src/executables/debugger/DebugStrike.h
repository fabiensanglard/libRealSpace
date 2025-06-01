#include "../../strike_commander/precomp.h"

class DebugStrike : public SCStrike {
public:
    DebugStrike();
    ~DebugStrike();
    void init();
    void setMission(std::string mission_name);
    void renderMenu();
};