#pragma once
#include "precomp.h"

struct texture {
    GLuint texture_id{0};
    Texel *data{nullptr};
};

class SCSmokeSet {
    RSSmokeSet *smoke_set;
    std::vector<texture *> textures;
    VGAPalette palette;
public: 
    SCSmokeSet();
    ~SCSmokeSet();
    void Init();
};