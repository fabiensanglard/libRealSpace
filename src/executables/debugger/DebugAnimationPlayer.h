#pragma once
#include "../../strike_commander/precomp.h"
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

struct ShotSectionElement {
    std::string label;
    ImVec2 positionStart;
    ImVec2 positionEnd;
    void* elementPtr;
};

class DebugAnimationPlayer: public SCAnimationPlayer {
private:
    PakArchive *current_mid{nullptr};
    PakEntry *current_entry{nullptr};
    PakArchive *current_palette_mid{nullptr};
    PakArchive *current_midvoc{nullptr};
    int current_palette_entry_index{-1};
    int current_entry_index{-1};
    int current_midvoc_entry_index{-1};
    MIDGAME_SHOT_BG* p_bg_editor{nullptr};
    MIDGAME_SHOT_SPRITE* p_sprite_editor{nullptr};
    MIDGAME_SHOT_BG* p_foreground_editor{nullptr};
    MIDGAME_SHOT_CHARACTER* p_character_editor{nullptr};
    MIDGAME_SHOT* p_shot_editor{nullptr};
    MIDGAME_TEXT_LINE* p_textline_editor{nullptr};

    // Fonctions helper pour showEditor()
    void resetEditorSelection();
    void selectEditorElement(MIDGAME_SHOT_BG* bg, MIDGAME_SHOT_SPRITE* sprite, 
                             MIDGAME_SHOT_BG* fg, MIDGAME_SHOT_CHARACTER* character, 
                             MIDGAME_SHOT* shotPtr, MIDGAME_TEXT_LINE* textLine);
    MIDGAME_SHOT* createEmptyShot();
    float calculateSectionHeight(size_t elementCount, float sectionHeaderHeight, 
                                float elementHeight, float elementSpacing, 
                                float emptyMessageHeight);
    std::string formatElementPosition(int startX, int startY, int endX, int endY);
    void renderInsertShotButton(size_t shotIndex, float& xOffset, float timelineHeight, 
                               ImVec2 timelinePos, float insertButtonWidth, bool isBeforeShot);
    void deleteShotAtIndex(size_t shotIndex);
    void addNewBackground(MIDGAME_SHOT* shot);
    void addNewCharacter(MIDGAME_SHOT* shot);
    void addNewSprite(MIDGAME_SHOT* shot);
    void addNewForeground(MIDGAME_SHOT* shot);
    void addNewTextLine(MIDGAME_SHOT *shot);
    std::string buildBackgroundLabel(MIDGAME_SHOT_BG* bg, size_t index);
    std::string buildCharacterLabel(MIDGAME_SHOT_CHARACTER* character, size_t index);
    std::string buildSpriteLabel(MIDGAME_SHOT_SPRITE* sprite, size_t index);
    std::string buildForegroundLabel(MIDGAME_SHOT_BG* fg, size_t index);
    std::string buildTexteLineLabel(MIDGAME_TEXT_LINE* txt, size_t index);
    void editLayerCommon(MIDGAME_LAYER_COMMON &common);
    int nextFreeLayerId();

    void drawShotSection(
        const char* title,
        const char* emptyLabel,
        ImU32 color,
        int idOffset,
        size_t shotIndex,
        ImVec2 shotPos,
        float& yOffset,
        const std::vector<ShotSectionElement>& elements,
        const std::function<void()>& addElement,
        const std::function<void(void*)>& onSelect,
        float shotWidth,
        float textPadding,
        float sectionHeaderHeight,
        float emptyMessageHeight,
        float elementHeight,
        float elementSpacing
    );

    std::vector<ShotSectionElement> buildBackgroundElements(MIDGAME_SHOT* shot);
    std::vector<ShotSectionElement> buildCharacterElements(MIDGAME_SHOT* shot);
    std::vector<ShotSectionElement> buildSpriteElements(MIDGAME_SHOT* shot);
    std::vector<ShotSectionElement> buildForegroundElements(MIDGAME_SHOT* shot);
    std::vector<ShotSectionElement> buildTextLineElements(MIDGAME_SHOT* shot);
public:
    DebugAnimationPlayer();
    ~DebugAnimationPlayer();
    void renderMenu() override;
    void renderUI(void) override;
    void midgameChoser();
    void midgamePaletteChoser();
    void showEditor();
    void editMidGameShotBG(MIDGAME_SHOT_BG* bg);
    void editMidGameShotSprite(MIDGAME_SHOT_SPRITE* sprite);
    void editMidGameShotCharacter(MIDGAME_SHOT_CHARACTER* character);
    void editMidGameShot(MIDGAME_SHOT *shot);
    void editMidGameShotTextLine(MIDGAME_TEXT_LINE *textLine);
    void midvocChooser();
};
