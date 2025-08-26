//
//  Game.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_sdl2.h"
#include "DebugGame.h"
#include "DebugStrike.h"
#include "DebugGameFlow.h"
#include "DebugObjectViewer.h"

#include "../../strike_commander/precomp.h"
extern RSScreen *Screen;
extern SCMouse Mouse;
extern AssetManager Assets;
extern SCRenderer Renderer;
extern RSVGA VGA;

DebugGame::DebugGame() {
    printf("DebugGame constructor\n");
}

DebugGame::~DebugGame() {}

void DebugGame::init() {

    // Load Main Palette and Initialize the GL
    
    VGA.init(1200,800, &Assets);
    Renderer.init(1200,800, &Assets);
    // Load the Mouse Cursor
    Mouse.init();
    loadSC();
}

void DebugGame::pumpEvents(void) {

    SDL_PumpEvents();

    // Mouse
    SDL_Event mouseEvents[5];
    int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEMOTION, SDL_MOUSEWHEEL);
    ImGui_ImplSDL2_ProcessEvent(mouseEvents);
    for (int i = 0; i < numMouseEvents; i++) {
        SDL_Event *event = &mouseEvents[i];

        switch (event->type) {
        case SDL_MOUSEMOTION:

            Point2D newPosition;
            newPosition.x = event->motion.x;
            newPosition.y = event->motion.y;

            newPosition.x = static_cast<int>(newPosition.x * 320 / Screen->width);
            newPosition.y = static_cast<int>(newPosition.y * 200 / Screen->height);

            Mouse.SetPosition(newPosition);

            break;

        case SDL_MOUSEBUTTONDOWN:
            // printf("SDL_MOUSEBUTTONDOWN %d\n",event->button.button);

            Mouse.buttons[event->button.button - 1].event = MouseButton::PRESSED;
            break;
        case SDL_MOUSEBUTTONUP:
            // printf("SDL_MOUSEBUTTONUP %d\n",event->button.button);
            Mouse.buttons[event->button.button - 1].event = MouseButton::RELEASED;
            break;
        default:
            break;
        }
    }


    // System events
    SDL_Event sysEvents[5];
    int numSysEvents = SDL_PeepEvents(sysEvents, 5, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_SYSWMEVENT);
    for (int i = 0; i < numSysEvents; i++) {
        SDL_Event *event = &sysEvents[i];

        switch (event->type) {
        case SDL_APP_TERMINATING:
            terminate("System request.");
            break;
        case SDL_QUIT:
            terminate("System request.");
            break;

        // Verify is we should be capturing the mouse or not.
        case SDL_WINDOWEVENT:
            if (event->window.event == SDL_WINDOWEVENT_ENTER) {
                Mouse.SetVisible(true);
                SDL_ShowCursor(SDL_DISABLE);
                return;
            }

            if (event->window.event == SDL_WINDOWEVENT_LEAVE) {
                Mouse.SetVisible(false);
                SDL_ShowCursor(SDL_ENABLE);
                return;
            }

            if (event->window.event == SDL_WINDOWEVENT_CLOSE) {
                terminate("System request.");
                break;
            }
            
            break;
        default:
            break;
        }
    }
}

void DebugGame::loadSC() {
    
    Assets.SetBase("./assets");
    // Load all TREs and PAKs
    
    std::vector<std::string> cdTreFiles = {
        "GAMEFLOW.TRE",
        "MISC.TRE",
        "MISSIONS.TRE",
        "OBJECTS.TRE",
        "SOUND.TRE",
        "TEXTURES.TRE"
    };
    Assets.init(cdTreFiles);
    
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
    
    FontManager.init(&Assets);

    // Load assets needed for Conversations (char and background)
    ConvAssets.init();

    //Add MainMenu activity on the game stack.
    DebugGameFlow* main = new DebugGameFlow();
    
    GameState.Reset();
    GameState.player_callsign = "Debug";
    GameState.player_name = "Debug Player";
    GameState.player_firstname = "Debug";
    for (int i=0; i<256; i++) {
        GameState.requierd_flags[i] = false;
    }
    main->init();
    Game->addActivity(main);
}


void DebugGame::loadSCCD() {
    
    Assets.SetBase("./assets");
    // Load all TREs and PAKs
    
    Assets.ReadISOImage("./SC.DAT");
    std::vector<std::string> treFiles = {
        "BIGTRE.TRE",
        "LILTRE.TRE",
        "VOCLIST.TRE"
    };
    Assets.init(treFiles);
    
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
    
    FontManager.init(&Assets);

    // Load assets needed for Conversations (char and background)
    ConvAssets.init();

    //Add MainMenu activity on the game stack.
    DebugGameFlow* main = new DebugGameFlow();
    
    GameState.Reset();
    GameState.player_callsign = "Debug";
    GameState.player_name = "Debug Player";
    GameState.player_firstname = "Debug";
    for (int i=0; i<256; i++) {
        GameState.requierd_flags[i] = false;
    }
    main->init();
    Game->addActivity(main);
}


void DebugGame::testMissionSC() {
    
    Assets.SetBase("./assets");
    // Load all TREs and PAKs
    Assets.ReadISOImage("./SC.DAT");
    std::vector<std::string> treFiles = {
        "BIGTRE.TRE",
        "LILTRE.TRE",
        "TOBIGTRE.TRE",
        "PACIFIC.DAT"
    };
    Assets.init(treFiles);
    
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
    
    FontManager.init(&Assets);

    //Add MainMenu activity on the game stack.
    DebugStrike * main = new DebugStrike();
    main->init();
    main->setMission("MISN-1A.IFF");
    Game->addActivity(main);
    
}

void DebugGame::loadTO() {
    Game->stopTopActivity();
    Assets.SetBase("./assets");
    // Load all TREs and PAKs
    
    std::vector<std::string> cdTreFiles = {
        "TOBIGTRE.TRE",
        "LILTRE.TRE",
        "TOVOCLST.TRE"
    };
    Assets.ReadISOImage("./SC.DAT");
    Assets.init(cdTreFiles);
    Assets.intel_root_path = "..\\..\\DATA\\INTEL\\";
    Assets.mission_root_path = "..\\..\\DATA\\MISSIONS\\";
    Assets.object_root_path = "..\\..\\DATA\\OBJECTS\\";
    Assets.sound_root_path = "..\\..\\DATA\\SOUND\\";
    Assets.texture_root_path = "..\\..\\DATA\\TXM\\";
    Assets.gameflow_root_path = "..\\..\\DATA\\GAMEFLOW\\";

    Assets.gameflow_filename = Assets.gameflow_root_path+"GAMEFLO2.IFF";
    Assets.optshps_filename = Assets.gameflow_root_path+"OPTSHPS.PAK";
    Assets.optpals_filename = Assets.gameflow_root_path+"OPTPALS.PAK";
    Assets.optfont_filename = Assets.gameflow_root_path+"OPTFONT.IFF";
    Assets.navmap_filename = "..\\..\\DATA\\COCKPITS\\NAVMAP2.IFF";
    Assets.conv_pak_filename = Assets.gameflow_root_path+"CONVSHPS.PAK";
    Assets.option_filename = Assets.gameflow_root_path+"OPTIONS.IFF";
    Assets.conv_data_filename = Assets.gameflow_root_path+"CONVDATA.IFF";
    Assets.conv_pal_filename = Assets.gameflow_root_path+"CONVPALS.PAK";
    Assets.txm_filename = Assets.texture_root_path+"TXMPACK.PAK";
    Assets.acc_filename = Assets.texture_root_path+"ACCPACK.PAK";
    Assets.convpak_filename = Assets.gameflow_root_path+"CONV2.PAK";
    
    FontManager.init(&Assets);
    // Load assets needed for Conversations (char and background)
    ConvAssets.init();
    
    //Add MainMenu activity on the game stack.
    DebugGameFlow* main = new DebugGameFlow();
    
    GameState.Reset();
    GameState.player_callsign = "Debug";
    GameState.player_name = "Debug Player";
    GameState.player_firstname = "Debug";
    for (int i=0; i<256; i++) {
        GameState.requierd_flags[i] = false;
    }
    main->init();
    Game->addActivity(main);
}


void DebugGame::testObjects() {
    Game->stopTopActivity();
    Assets.SetBase("./assets");
    // Load all TREs and PAKs
    std::vector<std::string> cdTreFiles = {
        "TOBIGTRE.TRE",
        "LILTRE.TRE"
    };
    Assets.ReadISOImage("./SC.DAT");
    Assets.init(cdTreFiles);
    
    FontManager.init(&Assets);

    // Load assets needed for Conversations (char and background)
    // ConvAssets.init();

    //Add MainMenu activity on the game stack.
    DebugObjectViewer* main = new DebugObjectViewer();
    main->init();
    Game->addActivity(main);
}

void DebugGame::loadPacific() {
    Game->stopTopActivity();
    Assets.SetBase("./assets");
    // Load all TREs and PAKs
    
    std::vector<std::string> cdTreFiles = {
        "PACIFIC.DAT",
    };
    Assets.init(cdTreFiles);
    
    FontManager.init(&Assets);

    // Load assets needed for Conversations (char and background)
    // ConvAssets.init();

    //Add MainMenu activity on the game stack.
    SCObjectViewer* main = new SCObjectViewer();
    main->init();
    Game->addActivity(main);
}

void DebugGame::run() {

    IActivity *currentActivity;

    while (activities.size() > 0) {

        pumpEvents();

        // Clear the screen
        // enderer.Clear();

        // Allow the active activity to Run and Render
        if (activities.empty()) {
            logError("No activity to run.\n");
            return;
        }
        currentActivity = activities.top();

        if (currentActivity->isRunning()) {
            currentActivity->focus();
            currentActivity->runFrame();
        } else {
            activities.pop();
            delete currentActivity;
        }

        // Swap GL buffer
        Screen->refresh();

        // Flush all events since they should all have been interpreted.
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

        // Also clear the Mouse flags.
        Mouse.FlushEvents();
    }
}

void DebugGame::terminate(const char *reason, ...) {
    log("Terminating: ");
    va_list args;
    va_start(args, reason);
    vfprintf(stdout, reason, args);
    va_end(args);
    log("\n");
    exit(0);
}

void DebugGame::log(const char *text, ...) {
    va_list args;
    va_start(args, text);
    vfprintf(stdout, text, args);
    va_end(args);
}

void DebugGame::logError(const char *text, ...) {
    va_list args;
    va_start(args, text);
    vfprintf(stderr, text, args);
    va_end(args);
}

void DebugGame::addActivity(IActivity *activity) {
    if (activities.size()>0) {
        IActivity *currentActivity;
        currentActivity = activities.top();
        currentActivity->unFocus();
    }
    activity->start();
    this->activities.push(activity);
}

void DebugGame::stopTopActivity(void) {
    IActivity *currentActivity;
    currentActivity = activities.top();
    currentActivity->stop();
}

IActivity *DebugGame::getCurrentActivity(void) { 
    if (activities.empty()) {
        return nullptr; // No activity is running
    }
    return activities.top();
}
