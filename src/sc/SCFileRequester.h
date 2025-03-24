#pragma once
#include "precomp.h"

extern AssetManager Assets;
class SCFileRequester {
private:
    RSImageSet *uiImageSet;
    std::vector<SCButton*> buttons;
    std::vector<SCZone *> zones;
    std::vector<std::string> files;
    void cancel();
    RSFont *font;
    int texte_x{0};
    std::string current_file{""};
    
public:
    std::string requested_file{""};
    bool opened{false};
    SCFileRequester();
    ~SCFileRequester();
    void draw(FrameBuffer *fb);
    void checkevents();
    SCButton *checkButtons(void);
    SCZone *checkZones();
    void loadFiles();
    void fileUp();
    void fileDown();
    void loadFile();
    void selectFile(void *unused, int index);
};