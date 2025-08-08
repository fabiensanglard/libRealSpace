#pragma once
#include "precomp.h"

class SCSmokeSet {
    RSSmokeSet *smoke_set;
    VGAPalette palette;
public: 
    std::vector<Texture *> textures;
    std::vector<std::vector<Texture *>> smoke_textures;
    SCSmokeSet();
    ~SCSmokeSet();
    void init();
};