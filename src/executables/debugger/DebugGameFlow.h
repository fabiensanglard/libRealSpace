#pragma once
#include "../../strike_commander/precomp.h"

class DebugGameFlow : public SCGameFlow {
protected:
    bool pause{false};
    void flyMission();
    void playConv(uint8_t convId);
    void renderGameState();
    void renderMissionInfos();
public:
    DebugGameFlow();
    ~DebugGameFlow();

    void renderMenu() override;
    void renderUI() override;
};
