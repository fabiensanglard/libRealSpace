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


struct shotBackground {
    RSImageSet* img {nullptr};
    uint8_t frameCounter {0};
};

class SCShot : public IActivity {

public:

    SCShot();
    ~SCShot();

    void init();
    void SetShotId(uint8_t shotid);
    virtual void runFrame(void);


protected:
    RSOption optionParser;
    PakArchive optShps;
    PakArchive optPals;
    std::vector<shotBackground *>layers;
    uint8_t* rawPalette;
    std::vector<uint8_t*> palettes;
    uint8_t* forPalette;
    
    RSImageSet* getShape(uint8_t shpid);
    virtual void checkKeyboard(void);
    int fps;
};

class EndMissionScene : public SCShot {
protected:
    int part{0};
public:
    EndMissionScene(PakArchive *optShps, PakArchive *optPals) : SCShot() {};
    void init();
    void runFrame(void);
};
class MapShot : public SCShot {
    std::vector<uint8_t*> mapics;
    FrameBuffer *frameBuffer;
    FrameBuffer *frameBufferA;
    
    int x{0};
    int y{0};
    int x_max{820};
    int y_max{600};
    int fp_counter{0};
    std::vector<MAP_POINT *> *points{nullptr};
    RSFont *font{nullptr};
    int color{86};
    int point_counter;
    int next_point_counter;
    float point_x_move;
    float point_y_move;
    float current_x;
    float current_y;
public:
    MapShot() : SCShot() {};
    void init();
    void SetPoints(std::vector<MAP_POINT *> *points);
    void runFrame(void);
};
#endif
