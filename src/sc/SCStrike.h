#pragma once
//
//  SCStrike.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"

#ifndef __libRealSpace__SCStrike__
#define __libRealSpace__SCStrike__

#define SCSTRIKE_MAX_MISSIONS 46
static const char *mission_list[] = {
    "MISN-1A.IFF",  "MISN-1B.IFF",  "MISN-1C.IFF",  "MISN-2A.IFF",  "MISN-3A.IFF",  "MISN-3B.IFF",  "MISN-3C.IFF",
    "MISN-4A.IFF",  "MISN-4B.IFF",  "MISN-4C.IFF",  "MISN-5A.IFF",  "MISN-5B.IFF",  "MISN-5BB.IFF", "MISN-5C.IFF",
    "MISN-6A.IFF",  "MISN-6B.IFF",  "MISN-6C.IFF",  "MISN-6D.IFF",  "MISN-7A.IFF",  "MISN-7B.IFF",  "MISN-7C.IFF",
    "MISN-7D.IFF",  "MISN-8A.IFF",  "MISN-8B.IFF",  "MISN-8C.IFF",  "MISN-9AA.IFF", "MISN-9A.IFF",  "MISN-9B.IFF",
    "MISN-10A.IFF", "MISN-10B.IFF", "MISN-10C.IFF", "MISN-10D.IFF", "MISN-11A.IFF", "MISN-11B.IFF", "MISN-11C.IFF",
    "MISN-12A.IFF", "MISN-12B.IFF", "MISN-13A.IFF", "MISN-13B.IFF", "MISN-14A.IFF", "MISN-14B.IFF", "MISN-15A.IFF",
    "MISN-3X.IFF",  "MISN-6X.IFF",  "MISN-8X.IFF",  "MISN-10X.IFF", "MISN-11X.IFF",
};
static int mission_idx = 0;

enum View { FRONT = 0, FOLLOW, RIGHT, LEFT, REAR, REAL };

class SCStrike : public IActivity {

public:
    SCStrike();
    ~SCStrike();

    void Init();
    void SetMission(char const *missionName);
    void CheckKeyboard(void);
    void RunFrame(void);
    void RenderMenu();

private:
    uint8_t camera_mode;
    bool mouse_control;
    Point3D *position;
    Camera *camera;
    Point3D camera_pos;
    float yaw;
    Point3D newPosition;
    Point2D pilote_lookat;
    Quaternion newOrientation;
    RSArea area;
    RSMission *missionObj;
    SCNavMap *nav_screen;
    SCPlane *player_plane;
    SCCockpit *cockpit;
    float counter;
    std::map<std::string, RSEntity *> objectCache;
};

#endif /* defined(__libRealSpace__SCStrike__) */
