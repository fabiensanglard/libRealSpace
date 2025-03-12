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

typedef struct MIDGAME_SHOT {
    std::vector<MIDGAME_SHOT_BG *>background;
    RSPalette *palette_bg;
    RSImageSet *foreground;
    RSPalette *palette_fg;
} MIDGAME_SHOT;

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
