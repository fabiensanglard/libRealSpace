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


struct sprtRect {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
};

struct animatedSprites {
    RSImageSet* img;
    std::vector<uint8_t>* frames;
    uint8_t frameCounter;
    sprtRect* rect;
    std::vector<Point2D*> *quad;
    std::vector<uint8_t>* efect;
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
    std::map<uint8_t, animatedSprites*> sprites;
    uint8_t current_miss;
    uint8_t current_scen;
    void clicked(uint8_t id);
    void runEffect();
    void CheckKeyboard(void);
    RSImageSet* getShape(uint8_t shpid);
    int fps;
    uint8_t currentOptCode;
    std::vector<uint8_t> *efect;
};

#endif
