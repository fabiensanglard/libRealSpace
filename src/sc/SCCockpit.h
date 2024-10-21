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

struct SCAiPlane {
    SCPlane *plane;
    SCPilot *pilot;
    MISN_PART *object;
    RSProf *ai;
    std::string name;
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
    void RenderMFDSWeapon(Point2D pmfd_right);
    void RenderMFDSRadar(Point2D pmfd_left, float range, int mode);
    void RenderMFDSComm(Point2D pmfd_left, int mode);
    RSFont *font;
    RSFont *big_font;
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

    bool show_radars{false};
    bool show_weapons{false};
    bool show_damage{false};
    bool show_comm{false};
    bool show_cam{false};

    int radar_zoom{1};
    int throttle{0};

    MISN_PART *target;
    MISN_PART *player;
    RSProf *player_prof;
    std::vector<MISN_PART *> parts;
    std::vector<SCAiPlane *> ai_planes;
    Camera *cam;
    Vector2D weapoint_coords;

    SCCockpit();
    ~SCCockpit();
    void Init( );
    void Render(int face);
};
#endif