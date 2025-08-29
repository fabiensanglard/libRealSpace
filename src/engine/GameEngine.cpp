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
#include "GameEngine.h"
#include "Loader.h"
#include "InputActionSystem.h"  
#include "EventManager.h"
#include "keyboard.h" 
extern RSScreen *Screen;
extern GameEngine *Game;
extern SCMouse Mouse;
extern AssetManager Assets;
extern SCRenderer Renderer;
extern RSVGA VGA;

GameEngine::GameEngine() {}

GameEngine::~GameEngine() {
    delete m_keyboard;
    m_keyboard = nullptr;
}


void GameEngine::init() {

    // Load Main Palette and Initialize the GL
    
    VGA.init(WIDTH,HEIGHT, &Assets);
    Renderer.init(WIDTH,HEIGHT, &Assets);
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
}

void GameEngine::pumpEvents(void) {

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

void GameEngine::run() {

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

void GameEngine::terminate(const char *reason, ...) {
    log("Terminating: ");
    va_list args;
    va_start(args, reason);
    vfprintf(stdout, reason, args);
    va_end(args);
    log("\n");
    exit(0);
}

void GameEngine::log(const char *text, ...) {
    va_list args;
    va_start(args, text);
    vfprintf(stdout, text, args);
    va_end(args);
}

void GameEngine::logError(const char *text, ...) {
    va_list args;
    va_start(args, text);
    vfprintf(stderr, text, args);
    va_end(args);
}

void GameEngine::addActivity(IActivity *activity) {
    if (activities.size()>0) {
        IActivity *currentActivity;
        currentActivity = activities.top();
        currentActivity->unFocus();
    }
    activity->start();
    this->activities.push(activity);
}

void GameEngine::stopTopActivity(void) {
    IActivity *currentActivity;
    currentActivity = activities.top();
    currentActivity->stop();
}

IActivity *GameEngine::getCurrentActivity(void) { return activities.top(); }
