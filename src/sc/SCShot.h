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

    void Init();
    void SetShotId(uint8_t shotid);
    virtual void RunFrame(void);


protected:
    RSOption optionParser;
    PakArchive optShps;
    PakArchive optPals;
    std::vector<shotBackground *>layers;
    uint8_t* rawPalette;
    std::vector<uint8_t*> palettes;
    uint8_t* forPalette;
    
    RSImageSet* getShape(uint8_t shpid);
    virtual void CheckKeyboard(void);
    int fps;
};

class EndMissionScene : public SCShot {
    protected:
        virtual void CheckKeyboard(void);
        int part{0};
    public:
        EndMissionScene(PakArchive *optShps, PakArchive *optPals) : SCShot() {};
        void Init();
        void RunFrame(void);
    };
#endif
