#pragma once
#include "precomp.h"

extern AssetManager Assets;
class SCFileRequester {
private:
    RSImageSet *uiImageSet;
    std::vector<SCButton*> buttons;
    std::vector<std::string> files;
    void cancel();
    RSFont *font;
public:
    bool opened{false};
    SCFileRequester();
    ~SCFileRequester();
    void draw(FrameBuffer *fb);
    void checkevents();
    SCButton *checkButtons(void);
    void loadFiles();
};