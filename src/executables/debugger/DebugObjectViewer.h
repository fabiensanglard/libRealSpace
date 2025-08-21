#pragma once
#include "../../strike_commander/precomp.h"

class DebugObjectViewer : public SCObjectViewer {
private:
    std::vector<Vector3D> vertices;
    bool moovingLight{true};
    bool autoRotate{true};
public:
    DebugObjectViewer();
    ~DebugObjectViewer();

    void runFrame(void) override;
    void renderMenu();
    void renderUI();
};