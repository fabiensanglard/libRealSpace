#pragma once


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
#include "../../engine/Loader.h"
class DebugGame : public GameEngine {
    
public:
    
    DebugGame();
    ~DebugGame();
    
    void init(void);
    void run(void);
    void terminate(const char* reason, ...);
    
    void log(const char* text, ...);
    void logError(const char* text, ...);
    
    //Add an activity on the top of the stack.
    void addActivity(IActivity* activity);
    void stopTopActivity(void);
    IActivity* getCurrentActivity(void);
    void pumpEvents(void);
    void loadSC();
    void loadSCCD();
    void loadTO();
    void loadPacific();
    void testMissionSC();
    void testObjects();

private:
    std::stack<IActivity*> activities;
    
    
};
