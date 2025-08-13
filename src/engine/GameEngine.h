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
    virtual void run(void);
    virtual void terminate(const char* reason, ...);
    
    virtual void log(const char* text, ...);
    virtual void logError(const char* text, ...);
    virtual void addActivity(IActivity* activity);
    virtual void stopTopActivity(void);
    virtual IActivity* getCurrentActivity(void);
    
    virtual void pumpEvents(void);
    
protected:
    Loader *loader;
private:
    
    std::stack<IActivity*> activities;
    
    
};


#endif /* defined(__libRealSpace__Game__) */
