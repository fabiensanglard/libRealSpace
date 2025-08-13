//
//  Game.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Game__
#define __libRealSpace__Game__

#define HEIGHT 1080
#define WIDTH 1920
#define FULLSCREEN 1
#include <stack>
#include <vector>
#include <string>
#include <SDL.h>
#include "../commons/Maths.h"
#include "SCMouse.h"
#include "RSScreen.h"
#include "RSVGA.h"
#include "SCRenderer.h"
#include "IActivity.h"
#include "Loader.h"

class GameEngine{
    
public:
    
    GameEngine();
    ~GameEngine();
    
    virtual void init(void);
    virtual void Run(void);
    virtual void Terminate(const char* reason, ...);
    
    virtual void Log(const char* text, ...);
    virtual void LogError(const char* text, ...);
    
    //Add an activity on the top of the stack.
    virtual void AddActivity(IActivity* activity);
    virtual void StopTopActivity(void);
    virtual IActivity* GetCurrentActivity(void);
    
    virtual void PumpEvents(void);
    
protected:
    Loader *loader;
private:
    
    std::stack<IActivity*> activities;
    
    
};


#endif /* defined(__libRealSpace__Game__) */
