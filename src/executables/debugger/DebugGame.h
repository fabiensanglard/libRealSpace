#pragma once

#define HEIGHT 800
#define WIDTH 1200
#define FULLSCREEN 0
#include <stack>
#include <vector>
#include <string>
#include <SDL.h>
#include "../../commons/Maths.h"
#include "../../engine/GameEngine.h"
#include "../../engine/SCMouse.h"
#include "../../engine/RSScreen.h"
#include "../../engine/RSVGA.h"
#include "../../engine/SCRenderer.h"
#include "../../engine/IActivity.h"

class DebugGame : public GameEngine {
    
public:
    
    DebugGame();
    ~DebugGame();
    
    void init(void);
    void Run(void);
    void Terminate(const char* reason, ...);
    
    void Log(const char* text, ...);
    void LogError(const char* text, ...);
    
    //Add an activity on the top of the stack.
    void AddActivity(IActivity* activity);
    void StopTopActivity(void);
    IActivity* GetCurrentActivity(void);
    
    void PumpEvents(void);
    
private:
    
    std::stack<IActivity*> activities;
    
    
};
