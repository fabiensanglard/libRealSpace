//
//  SCAnimationPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "../realspace/RSMidgame.h"

typedef struct MIDGAME_LAYER_REF {
    MIDGAME_LAYER_COMMON *common{nullptr};
    Point2D *position{nullptr};
} MIDGAME_LAYER_REF;

typedef struct MIDGAME_DRAW_ENTRY {
    int16_t depth;
    int category;
    int index;
} MIDGAME_DRAW_ENTRY;

class SCAnimationPlayer: public IActivity {
protected:
    AssetManager &Assets = AssetManager::getInstance();
    RSMixer &Mixer = RSMixer::getInstance();
    GameEngine *Game = &GameEngine::instance();
    ConvAssetManager &ConvAssets = ConvAssetManager::getInstance();
    RSVGA &VGA = RSVGA::getInstance();
    std::vector<RSFont*> fonts;
    PakArchive midgames;
    PakArchive optShps;
    PakArchive optPals;
    PakArchive convShps;
    PakArchive convPals;
    RSMidgame *midgame_text_db{nullptr};
    std::vector<PakArchive*> mid;
    std::vector<PakArchive*> midvoc;
    
    int shot_counter{0};
    int fps_counter{0};
    int fps{1};
    int fps_timer{0};
    int current_music{255};
    
    bool auto_stop{true};
    void initMid1();
    bool stay_on_current_shot{false};
    VGAPalette original_palette;
    VGAPalette previous_palette;
    uint32_t frame_timer{0};

    bool isInWindow(uint16_t frame_start, uint16_t nb_frames, int frame);
    MIDGAME_LAYER_REF findLayer(MIDGAME_SHOT *shot, int16_t layer_id);
    Point2D screenPosition(MIDGAME_SHOT *shot, const MIDGAME_LAYER_COMMON &common, Point2D position);
    void applyPath(MIDGAME_LAYER_COMMON &common, Point2D &position, int frame);
    void moveTowardsEnd(Point2D &position, Point2D end, Point2D velocity);
    void jitterStep(MIDGAME_LAYER_COMMON &common);
    void blitLayer(FrameBuffer *fb, FrameBuffer *texture, Point2D position, const MIDGAME_LAYER_COMMON &common, uint8_t mask);
    int nextLipMouth(MIDGAME_SHOT_CHARACTER *chara, const std::string &text);
    void updateLips(MIDGAME_SHOT_CHARACTER *chara, int frame);
    int imageIndex(const MIDGAME_LAYER_COMMON &common, int value);
    uint8_t sequenceValue(const MIDGAME_LAYER_COMMON &common, int frame, bool repeat);
    void applyPaletteRef(const MIDGAME_PALETTE_REF &ref);
    int textWidth(RSFont *font, const std::string &text);
    void drawBackground(FrameBuffer *fb, MIDGAME_SHOT *shot, MIDGAME_SHOT_BG *bg, int frame, int fpsupdate);
    void drawCharacter(FrameBuffer *fb, MIDGAME_SHOT *shot, MIDGAME_SHOT_CHARACTER *chara, int frame, int fpsupdate);
    void drawSprite(FrameBuffer *fb, MIDGAME_SHOT *shot, MIDGAME_SHOT_SPRITE *sprt, int frame, int fpsupdate);
    void advanceSprite(MIDGAME_SHOT_SPRITE *sprt);
    int spriteLast(MIDGAME_SHOT_SPRITE *sprt);
    uint32_t fade_start{0};
    bool fade_done{false};
public:

    bool pause{false};
    std::unordered_map<uint8_t, std::vector<MIDGAME_SHOT *>> midgames_shots;

    SCAnimationPlayer();
    ~SCAnimationPlayer();
    virtual void init();
    virtual void runFrame(void);
    virtual void renderMenu() {};
    virtual void renderUI(void) {};
    void loadShot(std::string filename);
    void resetShot(MIDGAME_SHOT *shot);

};
