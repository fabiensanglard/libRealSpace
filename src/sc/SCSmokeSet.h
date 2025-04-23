#pragma once
#include "precomp.h"

class SCSmokeSet {
    RSSmokeSet *smoke_set;
    VGAPalette palette;
public: 
    std::vector<Texture *> textures;
    SCSmokeSet();
    ~SCSmokeSet();
    void init();
};