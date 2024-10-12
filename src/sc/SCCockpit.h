//
//  SCCockpit.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#pragma once
#ifndef __libRealSpace__SCCockpit__
#define __libRealSpace__SCCockpit__

#include "precomp.h"

struct HudLine {
    Point2D start;
    Point2D end;
};

class SCCockpit {
private:
    VGAPalette palette;
    std::vector<HudLine> horizon;
    void RenderHudHorizonLinesSmall();
    void RenderAltitude();
    void RenderTargetWithCam();
    void RenderHeading();
    void RenderSpeed();
    void RenderMFDS(Point2D mfds);
    RSFont *font;
    
public:
    RSCockpit* cockpit;
    RSHud* hud;
    
    float pitch{0.0f};
    float roll{0.0f};
    float yaw{0.0f};

    float speed{0.0f};
    float altitude{0.0f};
    float heading{0.0f};

    bool gear{false};
    bool flaps{false};
    bool airbrake{false};

    int throttle{0};

    MISN_PART *target;
    MISN_PART *player;
    Camera *cam;
    Vector2D weapoint_coords;

    SCCockpit();
    ~SCCockpit();
    void Init( );
    void Render(int face);
};
#endif