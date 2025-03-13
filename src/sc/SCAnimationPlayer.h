//
//  SCAnimationPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCAnimationPlayer__
#define __libRealSpace__SCAnimationPlayer__

typedef struct MIDGAME_SHOT_BG {
    RSImageSet *image;
    RSPalette *palette;
    Point2D position_start;
    Point2D position_end;
    Point2D velocity;
} MIDGAME_SHOT_BG;

typedef struct MIDGAME_SHAPE_DATA {
    PakArchive *pak;
    uint8_t shape_id;
    uint8_t sub_shape_id;
    uint8_t pal_id;
    Point2D start;
    Point2D end;
    Point2D velocity;
} MIDGAME_SHAPE_DATA;

typedef struct MIDGAME_SHOT {
    std::vector<MIDGAME_SHOT_BG *>background;
    RSPalette *palette_bg{nullptr};
    RSImageSet *foreground{nullptr};
    RSPalette *palette_fg{nullptr};
} MIDGAME_SHOT;

typedef struct MIDGAME_DATA_SHOT {
    std::vector<MIDGAME_SHAPE_DATA> background;
    std::vector<MIDGAME_SHAPE_DATA> middle;
    std::vector<MIDGAME_SHAPE_DATA> forground;
    int nbframe;
} MIDGAME_DATA_SHOT;

typedef struct MIDGAME_DATA {
    std::vector<MIDGAME_DATA_SHOT> shots;
} MIDGAME_DATA;


class SCAnimationPlayer: public IActivity {
private:
    PakArchive midgames;
    PakArchive optShps;
    PakArchive optPals;
    std::vector<PakArchive*> mid;

    std::map<uint8_t, std::vector<MIDGAME_SHOT *>> midgames_shots;
public:
    SCAnimationPlayer();
    ~SCAnimationPlayer();
    void CheckKeyboard(void);
    void Init();
    void RunFrame(void);
};

#endif /* defined(__libRealSpace__SCAnimationPlayer__) */
