//
//  main.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "../../strike_commander/precomp.h"
#include "DebugGame.h"
#define FLOPPY 1
//Here are all the mean subsystems interacting together
static DebugGame debugGameInstance;
SCState         GameState;
GameEngine      *Game = &debugGameInstance;
RSScreen        Screen;
RSVGA           VGA;
SCMouse         Mouse;
AssetManager    Assets;
SCRenderer      Renderer;
ConvAssetManager ConvAssets;
RSFontManager FontManager;
RSMixer Mixer;

int main(int argc, char* argv[]) {
    Assets.SetBase("./assets");
    // Load all TREs and PAKs
    if (FLOPPY) {
        std::vector<std::string> treFiles = {
            "GAMEFLOW.TRE",
            "OBJECTS.TRE",
            "MISC.TRE",
            "SOUND.TRE",
            "MISSIONS.TRE",
            "TEXTURES.TRE"
        };
        Assets.init(treFiles);
    } else {
        std::vector<std::string> cdTreFiles = {
            "BIGTRE.TRE",
            "LILTRE.TRE",
            "VOCLIST.TRE"
        };
        Assets.ReadISOImage("./SC.DAT");
        Assets.init(cdTreFiles);
    }
    
    Mixer.init(&Assets);
    FontManager.init(&Assets);

    // Load assets needed for Conversations (char and background)
    ConvAssets.init();
    Game->init();
    //Add MainMenu activity on the game stack.
    SCMainMenu* main = new SCMainMenu();
    main->init();
    Game->AddActivity(main);
    SCAnimationPlayer *intro = new SCAnimationPlayer();
    intro->init();
    Game->AddActivity(intro);
    Game->Run();

    
    return EXIT_SUCCESS;
}