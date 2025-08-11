#pragma once
#include "../../strike_commander/precomp.h"

class DebugObjectViewer : public SCObjectViewer {
    public:
    DebugObjectViewer();
    ~DebugObjectViewer();

    void renderMenu();
    void renderUI();
};