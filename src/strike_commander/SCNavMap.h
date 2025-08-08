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
    
    void init();
    void SetName(char *name);
    void runFrame(void);
    RSMission *missionObj;
    SCMission *mission{nullptr};
    uint8_t *current_nav_point;
    RSNavMap* navMap;
    
private:
    int color{134};
    std::string *name;
    
    
    VGAPalette palette;
    VGAPalette objpal;

    bool show_area{false};
    bool show_obj{true};
    bool show_waypoint{true};

    void checkKeyboard(void);
    void showArea(AREA *area, float center, float map_width, int w, int h, int t, int l, int c);
};