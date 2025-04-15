//
//  Game.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Game__
#define __libRealSpace__Game__

#define HEIGHT 800
#define WIDTH 1200
#define FULLSCREEN 0
#include <stack>
#include <vector>
#include <string>
#include <SDL.h>
#include "../commons/Maths.h"
#include "../sc/SCMouse.h"
#include "RSScreen.h"
#include "RSVGA.h"
#include "SCRenderer.h"
#include "IActivity.h"

class GameEngine{
    
public:
    
    GameEngine();
    ~GameEngine();
    
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


#endif /* defined(__libRealSpace__Game__) */
