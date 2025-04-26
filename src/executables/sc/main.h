//
//  main.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef libRealSpace_main_h
#define libRealSpace_main_h
#include "../strike_commander/precomp.h"

extern GameEngine *Game;
extern SCMouse Mouse;
extern RSScreen *Screen;
extern RSVGA    VGA;
extern AssetManager Assets;
extern SCRenderer Renderer;
extern ConvAssetManager ConvAssets;
extern RSFontManager FontManager;
extern RSMixer Mixer;
extern SCState GameState;

#endif