#pragma once
//
//  SCShot.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#ifndef __libRealSpace__SCShot__
#define __libRealSpace__SCShot__

#include "precomp.h"


struct sprtRect {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
};

struct animatedSprites {
    RSImageSet* img {nullptr};
    std::vector<uint8_t>* frames {nullptr};
    uint8_t frameCounter {0};
    sprtRect* rect {nullptr};
    bool cliked {false};
    std::vector<Point2D*> *quad {nullptr};
    std::vector<EFCT *>* efect {nullptr}; 
};

class SCShot : public IActivity {

public:

    SCShot();
    ~SCShot();

    void Init();
    void RunFrame(void);


private:
    RSOption optionParser;
    PakArchive optShps;
    PakArchive optPals;
    std::vector<RLEShape *>layers;
    std::vector<RSImageSet *>mobil;
    uint8_t* rawPalette;
    uint8_t* forPalette;
    
    RSImageSet* getShape(uint8_t shpid);
    void CheckKeyboard(void);
    int fps;
};

#endif
