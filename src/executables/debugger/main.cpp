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
#define FLOPPY 0
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
    

    Loader loader;
    
    Screen->init(1200,800,0);
    Assets.SetBase("./assets");
    loader.init();

    // Démarrer le chargement des assets en arrière-plan
    
    loader.startLoading([](Loader* loader) {
        // Simuler le chargement des assets
        const int totalAssets = 100;
        loader->setProgress(0.0f);
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
            loader->setProgress(80.0f);
        } else {
            std::vector<std::string> cdTreFiles = {
                "BIGTRE.TRE",
                "LILTRE.TRE",
                "VOCLIST.TRE"
            };
            loader->setProgress(10.0f);
            Assets.ReadISOImage("./SC.DAT");
            loader->setProgress(20.0f);
            Assets.init(cdTreFiles);
            loader->setProgress(30.0f);
        }
        Mixer.init(&Assets);
        loader->setProgress(35.0f);
        FontManager.init(&Assets);
        loader->setProgress(40.0f);
        // Load assets needed for Conversations (char and background)
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
        
        Assets.navmap_filename = "..\\..\\DATA\\COCKPITS\\NAVMAP.IFF";
        Assets.conv_pak_filename = Assets.gameflow_root_path+"CONVSHPS.PAK";
        Assets.option_filename = Assets.gameflow_root_path+"OPTIONS.IFF";
        Assets.conv_data_filename = Assets.gameflow_root_path+"CONVDATA.IFF";
        Assets.conv_pal_filename = Assets.gameflow_root_path+"CONVPALS.PAK";
        Assets.txm_filename = Assets.texture_root_path+"TXMPACK.PAK";
        Assets.acc_filename = Assets.texture_root_path+"ACCPACK.PAK";
        Assets.convpak_filename = Assets.gameflow_root_path+"CONV.PAK";
        // Load assets needed for Conversations (char and background)
        loader->setProgress(100.0f);
    });

    // Boucle de chargement
    while (!loader.isLoadingComplete()) {
        // Dessiner l'écran de chargement
        loader.runFrame();
        
        // Mettre à jour l'affichage
        Screen->Refresh();
    }

    Game->init();
    Game->Run();

    
    return EXIT_SUCCESS;
}