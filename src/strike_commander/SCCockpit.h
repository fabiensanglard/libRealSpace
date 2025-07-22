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
    
    std::vector<HudLine> horizon;
    void RenderHudHorizonLinesSmall(Point2D center, FrameBuffer *fb);
    void RenderAltitude(Point2D alti_arrow, FrameBuffer *fb);
    void RenderTargetWithCam();
    void RenderTargetingReticle();
    void RenderBombSight();
    void RenderHeading(Point2D heading_pos, FrameBuffer *fb);
    void RenderSpeed(Point2D heading_pos, FrameBuffer *fb);
    void RenderMFDS(Point2D mfds, FrameBuffer *fb);
    void RenderMFDSRadarImplementation(Point2D pmfd_left, float range, const char* mode_name, bool air_mode, FrameBuffer *fb);
    
    RSFont *font;
    RSFont *big_font;
    int radio_mission_timer{0};
    
public:
    VGAPalette palette;
    RSCockpit* cockpit;
    RSHud* hud;
    FrameBuffer *hud_framebuffer{nullptr};
    FrameBuffer *mfd_right_framebuffer{nullptr};
    FrameBuffer *mfd_left_framebuffer{nullptr};
    float pitch{0.0f};
    float roll{0.0f};
    float yaw{0.0f};

    float speed{0.0f};
    float altitude{0.0f};
    float heading{0.0f};

    float pitch_speed{0.0f};
    float yaw_speed{0.0f};
    float roll_speed{0.0f};

    bool gear{false};
    bool flaps{false};
    bool airbrake{false};
    bool mouse_control{false};
    bool show_radars{false};
    bool show_weapons{false};
    bool show_damage{false};
    bool show_comm{false};
    bool show_cam{false};
    RadarMode radar_mode{RadarMode::AARD};
    int radar_zoom{1};
    int throttle{0};
    int comm_target{0};
    float way_az{0};
    MISN_PART *target{nullptr};
    MISN_PART *player{nullptr};
    RSProf *player_prof{nullptr};
    std::vector<MISN_PART *> parts;
    std::vector<SCAiPlane *> ai_planes;
    Camera *cam;
    Vector2D weapoint_coords;
    SCPlane *player_plane;
    SCMission *current_mission;
    uint8_t *nav_point_id{nullptr};
    SCCockpit();
    ~SCCockpit();
    void init( );
    void Update();
    void Render(int face);
    void RenderHUD();
    void RenderMFDSWeapon(Point2D pmfd_right, FrameBuffer *fb);
    void RenderMFDSRadar(Point2D pmfd_left, float range, int mode, FrameBuffer *fb);
    void RenderMFDSComm(Point2D pmfd_left, int mode, FrameBuffer *fb);
};
#endif