#pragma once
#include "../../strike_commander/precomp.h"

class DebugGameFlow : public SCGameFlow {
protected:
    void flyMission();
    void renderGameState();
    void renderMissionInfos();
public:
    DebugGameFlow();
    ~DebugGameFlow();

    void renderMenu() override;
    void renderUI() override;
};
