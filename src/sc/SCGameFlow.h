#pragma once
//
//  File.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#ifndef __libRealSpace__SCGameFow__
#define __libRealSpace__SCGameFow__

#include "precomp.h"
#include "../../librealspace/RSGameflow.h"
#include "../../librealspace/RSOption.h"

struct sprtRect {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
};

struct animatedSprites {
    RSImageSet* img;
    std::vector<uint8_t>* frames;
    uint8_t frameCounter;
    sprtRect* rect;
};

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
    std::vector<animatedSprites*> sprites;
    uint8_t current_miss;
    uint8_t current_scen;
    void CheckKeyboard(void);
    RSImageSet* getShape(uint8_t shpid);
    int fps;
};

#endif
