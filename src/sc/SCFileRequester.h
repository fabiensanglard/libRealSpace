#pragma once
#include "precomp.h"

extern AssetManager Assets;
class SCFileRequester {
    RSImageSet *uiImageSet;
public:
    bool opened{false};
    SCFileRequester();
    ~SCFileRequester();
    void draw(FrameBuffer *fb);
    void checkevents();
};