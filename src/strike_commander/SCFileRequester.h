#pragma once
#include "precomp.h"
#include <functional>
#include <string>
#include <vector>
#include "../engine/keyboard.h"  // Inclure notre nouveau gestionnaire d'événements

extern AssetManager Assets;

class SCFileRequester {
public:
    SCFileRequester(std::function<void(std::string)> callback, uint8_t shape_id_offset = 42);
    SCFileRequester(std::function<void(std::string)> callback, uint8_t shape_id_offset, bool save);
    ~SCFileRequester();
    
    void draw(FrameBuffer *fb);
    void checkevents();
    void cancel();
    void quitToDos();
    void loadFiles();
    void fileUp();
    void fileDown();
    void loadFile();

    SCButton* checkButtons(void);
    SCZone* checkZones();
    void selectFile(void *unused, int index);

    std::function<void(std::string)> callback;
    void activateTextEditor(void* unused, int id);
    void deactivateTextEditor();
    SCZone* m_textInputZone = nullptr;
    bool m_isEditingText = false;
    RSImageSet *uiImageSet;
    bool opened = true;
    bool save_mode = false;
    std::string current_file;
    std::string requested_file;
    std::vector<std::string> files;
    std::vector<SCButton*> buttons;
    std::vector<SCZone*> zones;
    int texte_x = 0;
    int selectd_file_index = -1;
    uint8_t shape_id_offset = 42;
    RSFont *font = nullptr;
    PakArchive optPals;
    uint8_t *rawpal;
    uint8_t color_offset = 49;
    uint8_t palette_index = 4;
    
    // Nouveaux membres pour la gestion des événements
    Keyboard* m_keyboard;
    std::shared_ptr<Keyboard::TextEditor> m_textEditor;
};