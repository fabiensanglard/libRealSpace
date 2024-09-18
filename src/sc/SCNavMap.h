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
    
private:
    std::string *name;
    RSNavMap* navMap;
    VGAPalette palette;
    VGAPalette objpal;
    void CheckKeyboard(void);
};