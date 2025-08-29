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
#include "../../engine/Keyboard.h"
#include "../../engine/EventManager.h"  

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
    // Crée le clavier (abstraction)
    m_keyboard = new Keyboard();

    // Enregistrer actions souris (position + boutons)
    m_keyboard->registerAction(InputAction::MOUSE_POS_X);
    m_keyboard->registerAction(InputAction::MOUSE_POS_Y);
    m_keyboard->registerAction(InputAction::MOUSE_LEFT);
    m_keyboard->registerAction(InputAction::MOUSE_MIDDLE);
    m_keyboard->registerAction(InputAction::MOUSE_RIGHT);

    // Bind position absolue (axis: 0 = X, 1 = Y)
    m_keyboard->bindMousePositionToAction(InputAction::MOUSE_POS_X, 0, 1.0f);
    m_keyboard->bindMousePositionToAction(InputAction::MOUSE_POS_Y, 1, 1.0f);

    // Bind boutons (indices SDL : left=1, middle=2, right=3)
    m_keyboard->bindMouseButtonToAction(InputAction::MOUSE_LEFT,   SDL_BUTTON_LEFT);
    m_keyboard->bindMouseButtonToAction(InputAction::MOUSE_MIDDLE, SDL_BUTTON_MIDDLE);
    m_keyboard->bindMouseButtonToAction(InputAction::MOUSE_RIGHT,  SDL_BUTTON_RIGHT);

    m_keyboard->registerAction(InputAction::KEY_ESCAPE);
    m_keyboard->bindKeyToAction(InputAction::KEY_ESCAPE, SDL_SCANCODE_ESCAPE);
    EventManager::getInstance().enableImGuiForwarding(true);
    loadSC();
}

void DebugGame::pumpEvents(void) {
        // Met à jour tout (Keyboard encapsule InputActionSystem/EventManager)
    m_keyboard->update();

    // Position absolue (pixels fenêtre)
    int px, py;
    m_keyboard->getMouseAbsolutePosition(px, py);

    // Conversion vers l’espace 320x200 legacy
    Point2D newPosition;
    newPosition.x = static_cast<int>(px * 320.0f / Screen->width);
    newPosition.y = static_cast<int>(py * 200.0f / Screen->height);
    Mouse.SetPosition(newPosition);
    Mouse.SetVisible(true);
    // Mettre à jour les événements des trois boutons via transitions
    struct BtnMap {
        InputAction action;
        int legacyIndex; // 0=Left,1=Middle,2=Right (adapter si ordre différent)
    } maps[] = {
        { InputAction::MOUSE_LEFT,   0 },
        { InputAction::MOUSE_MIDDLE, 1 },
        { InputAction::MOUSE_RIGHT,  2 },
    };
    

    for (auto& m : maps) {
        auto& legacyBtn = Mouse.buttons[m.legacyIndex];
        if (m_keyboard->isActionJustPressed(m.action)) {
            legacyBtn.event = MouseButton::PRESSED;
        } else if (m_keyboard->isActionJustReleased(m.action)) {
            legacyBtn.event = MouseButton::RELEASED;
        } else {
            legacyBtn.event = MouseButton::NONE;
        }
    }

    if (EventManager::getInstance().shouldQuit()) {
        terminate("System request.");
        return;
    }
}

void DebugGame::run() {
    IActivity *currentActivity;
    while (activities.size() > 0) {

        pumpEvents();

        currentActivity = activities.top();
        if (currentActivity->isRunning()) {
            currentActivity->focus();
            currentActivity->runFrame();
        } else {
            activities.pop();
            delete currentActivity;
        }

        Screen->refresh();

        Mouse.FlushEvents(); // On peut le garder si sa logique interne reste valable.
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
