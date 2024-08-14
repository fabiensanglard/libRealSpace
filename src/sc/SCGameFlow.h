#pragma once
//
//  File.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include "../../librealspace/RSGameflow.h"
#include "../../librealspace/RSOption.h"


class SCGameFlow : public IActivity {

public:

    SCGameFlow();
    ~SCGameFlow();

    void Init();
    void createMiss();
    void RunFrame(void);


private:
    RSGameFlow gameFlowParser;
    RSOption optionParser;
    PakArchive optShps;
    PakArchive optPals;
    RLEShape *layer;
    uint8_t* rawPalette;
    uint8_t* forPalette;
    std::vector<RLEShape*> sprites;
    uint8_t current_miss;
    uint8_t current_scen;
    void CheckKeyboard(void);
    RLEShape* getShape(uint8_t shpid);
};
