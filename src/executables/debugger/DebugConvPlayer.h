#pragma once
#include "../../strike_commander/precomp.h"

class DebugConvPlayer : public SCConvPlayer {

public:
    DebugConvPlayer();
    ~DebugConvPlayer();

    void renderMenu() override;
    void renderUI() override;
};
