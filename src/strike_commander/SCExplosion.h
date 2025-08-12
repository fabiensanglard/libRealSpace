#pragma once
#include "precomp.h"

class SCExplosion {
public:
    RSEntity *obj{nullptr};
    int fps{0};
    int current_frame{0};
    Vector3D position{0, 0, 0};
    bool is_finished{false};

    void render();
    SCExplosion(RSEntity *obj, Vector3D position);
};