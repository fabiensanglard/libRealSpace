//
//  main.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "../../strike_commander/precomp.h"
#include "DebugGame.h"
#include "DebugScreen.h"
#define FLOPPY 1
//Here are all the mean subsystems interacting together
DebugGame debugGameInstance;
SCState         GameState;
GameEngine      *Game = &debugGameInstance;
DebugScreen     debugScreen;
RSScreen        *Screen = &debugScreen;
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
    Assets.intel_root_path = "..\\..\\DATA\\INTEL\\";
    Assets.mission_root_path = "..\\..\\DATA\\MISSIONS\\";
    Assets.object_root_path = "..\\..\\DATA\\OBJECTS\\";
    Assets.sound_root_path = "..\\..\\DATA\\SOUNDS\\";
    Assets.texture_root_path = "..\\..\\DATA\\TXM\\";
    Assets.gameflow_root_path = "..\\..\\DATA\\GAMEFLOW\\";

    Assets.gameflow_filename = Assets.gameflow_root_path+"GAMEFLOW.IFF";
    Assets.optshps_filename = Assets.gameflow_root_path+"OPTSHPS.PAK";
    Assets.optpals_filename = Assets.gameflow_root_path+"OPTPALS.PAK";
    Assets.optfont_filename = Assets.gameflow_root_path+"OPTFONT.IFF";
    Assets.navmap_filename = Assets.gameflow_root_path+"NAVMAP.IFF";
    Assets.conv_pak_filename = Assets.gameflow_root_path+"CONVSHPS.PAK";
    Assets.option_filename = Assets.gameflow_root_path+"OPTIONS.IFF";
    Assets.conv_data_filename = Assets.gameflow_root_path+"CONVDATA.IFF";
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