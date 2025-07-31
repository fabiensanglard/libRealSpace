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
    int selectd_file_index{0};
    std::function<void(std::string)> callback;
    uint8_t *rawpal;
    uint8_t color_offset{0};
    uint8_t palette_index{0};
    
    PakArchive optPals;
public:
    uint8_t shape_id_offset{42};
    std::string requested_file{""};
    bool opened{false};
    SCFileRequester(std::function<void(std::string)> callback, uint8_t shape_id_offset);
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