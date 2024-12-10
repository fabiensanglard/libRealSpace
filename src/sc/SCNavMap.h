//
//  SCNavMap.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 31/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "precomp.h"

class SCNavMap : public IActivity {
    
public:
    SCNavMap();
    ~SCNavMap();
    
    void Init();
    void SetName(char *name);
    void RunFrame(void);
    RSMission *missionObj;
    SCMission *mission{nullptr};
    uint8_t *current_nav_point;
    
private:
    int color{134};
    std::string *name;
    RSNavMap* navMap;
    
    VGAPalette palette;
    VGAPalette objpal;

    bool show_area{false};
    bool show_obj{false};
    bool show_waypoint{true};

    void CheckKeyboard(void);
    void showArea(AREA *area, float center, float map_width, int w, int h, int t, int l, int c);
};