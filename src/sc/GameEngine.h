//
//  Game.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Game__
#define __libRealSpace__Game__

class GameEngine{
    
public:
    
    GameEngine();
    ~GameEngine();
    
    void Init(void);
    void Run(void);
    void Terminate(const char* reason, ...);
    
    void Log(const char* text, ...);
    void LogError(const char* text, ...);
    
    //Add an activity on the top of the stack.
    void AddActivity(IActivity* activity);
    void StopTopActivity(void);
    
    
    void PumpEvents(void);
    
private:
    
    std::stack<IActivity*> activities;
    
    
};


#endif /* defined(__libRealSpace__Game__) */
