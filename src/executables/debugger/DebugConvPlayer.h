#pragma once
#include "../../strike_commander/precomp.h"

class DebugConvPlayer : public SCConvPlayer {
protected:
    bool paused{false};
    void CheckFrameExpired(void) override;
public:
    DebugConvPlayer();
    ~DebugConvPlayer();

    void renderMenu() override;
    void renderUI() override;
};
