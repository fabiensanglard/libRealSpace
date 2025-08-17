//
//  main.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "../../strike_commander/precomp.h"
#define FLOPPY 0
//Here are all the mean subsystems interacting together
SCState         GameState;
GameEngine      *Game;
RSScreen        *Screen;
RSVGA           VGA;
SCMouse         Mouse;
AssetManager    Assets;
SCRenderer      Renderer;
ConvAssetManager ConvAssets;
RSFontManager FontManager;
RSMixer Mixer;

int main(int argc, char* argv[]) {

    Game = new GameEngine();
    Screen = new RSScreen();
    Screen->init(WIDTH,HEIGHT,FULLSCREEN);
    Loader loader;
    Screen->is_spfx_finished = false;
    while (!Screen->is_spfx_finished) {
        Screen->fxTurnOnTv();
        Screen->refresh();
        SDL_PumpEvents();
    }
    Assets.SetBase("./assets");
    loader.init();
    // Load all TREs and PAKs
    loader.startLoading([](Loader* loader) {
        loader->setProgress(0.0f);
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
        loader->setProgress(50.0f);
        Assets.intel_root_path = "..\\..\\DATA\\INTEL\\";
        Assets.mission_root_path = "..\\..\\DATA\\MISSIONS\\";
        Assets.object_root_path = "..\\..\\DATA\\OBJECTS\\";
        Assets.sound_root_path = "..\\..\\DATA\\SOUND\\";
        Assets.texture_root_path = "..\\..\\DATA\\TXM\\";
        Assets.gameflow_root_path = "..\\..\\DATA\\GAMEFLOW\\";

        Assets.gameflow_filename = Assets.gameflow_root_path+"GAMEFLOW.IFF";
        Assets.optshps_filename = Assets.gameflow_root_path+"OPTSHPS.PAK";
        Assets.optpals_filename = Assets.gameflow_root_path+"OPTPALS.PAK";
        Assets.optfont_filename = Assets.gameflow_root_path+"OPTFONT.IFF";
        Assets.navmap_filename = "..\\..\\DATA\\COCKPITS\\NAVMAP.IFF";
        Assets.conv_pak_filename = Assets.gameflow_root_path+"CONVSHPS.PAK";
        Assets.option_filename = Assets.gameflow_root_path+"OPTIONS.IFF";
        Assets.conv_data_filename = Assets.gameflow_root_path+"CONVDATA.IFF";
        Assets.conv_pal_filename = Assets.gameflow_root_path+"CONVPALS.PAK";
        Assets.txm_filename = Assets.texture_root_path+"TXMPACK.PAK";
        Assets.acc_filename = Assets.texture_root_path+"ACCPACK.PAK";
        Assets.convpak_filename = Assets.gameflow_root_path+"CONV.PAK";
        Mixer.init(&Assets);
        loader->setProgress(60.0f);
        FontManager.init(&Assets);
        loader->setProgress(70.0f);
        // Load assets needed for Conversations (char and background)
        ConvAssets.init();
        loader->setProgress(100.0f);
    });
    while (!loader.isLoadingComplete()) {
        // Dessiner l'écran de chargement
        loader.runFrame();
        
        // Mettre à jour l'affichage
        Screen->refresh();
        SDL_PumpEvents();
    }
    Game->init();
    //Add MainMenu activity on the game stack.
    SCMainMenu* main = new SCMainMenu();
    main->init();
    Game->addActivity(main);
    SCAnimationPlayer *intro = new SCAnimationPlayer();
    intro->init();
    Game->addActivity(intro);
    Game->run();

    
    return EXIT_SUCCESS;
}