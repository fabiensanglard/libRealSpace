//
//  Game.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


GameEngine::GameEngine(){
    
}

GameEngine::~GameEngine(){
    
}

void GameEngine::Init(){
    
    //Load all TREs and PAKs
    Assets.Init();
    
    //Load Main Palette and Initialize the GL
    renderer.Init(4);
    
    //Load the Mouse Cursor
    Mouse.Init();
    
}

void GameEngine::PumpEvents(void){
    
    SDL_PumpEvents();
    
    //Mouse
    SDL_Event mouseEvents[5];
    int numMouseEvents= SDL_PeepEvents(mouseEvents,5,SDL_PEEKEVENT,SDL_MOUSEMOTION,SDL_MOUSEWHEEL);
    for(int i= 0 ; i < numMouseEvents ; i++){
        SDL_Event* event = &mouseEvents[i];
        
        switch (event->type) {
            case SDL_MOUSEMOTION:
                
                break;
            default:
                break;
        }
    }
    
    //Joystick
    
    //Keyboard
    SDL_Event keybEvents[5];
    int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYDOWN,SDL_TEXTINPUT);
    for(int i= 0 ; i < numKeybEvents ; i++){
        SDL_Event* event = &keybEvents[i];
        switch (event->type) {
            default:
                break;
        }
    }
    
    
    
    //Oculus VR
    
    
    
    
    //System events
    SDL_Event sysEvents[5];
    int numSysEvents = SDL_PeepEvents(sysEvents,5,SDL_PEEKEVENT,SDL_FIRSTEVENT,SDL_SYSWMEVENT);
    for(int i= 0 ; i < numSysEvents ; i++){
        SDL_Event* event = &sysEvents[i];
        
        switch (event->type) {
            case SDL_APP_TERMINATING:
                Terminate("System request.");
                break;
            case SDL_QUIT:
                Terminate("System request.");
                break;
            case SDL_WINDOWEVENT:
                if (event->window.event == SDL_WINDOWEVENT_ENTER){
                    
                    SDL_ShowCursor(SDL_DISABLE);
                    return;
                }
                
                if (event->window.event == SDL_WINDOWEVENT_LEAVE){
                    
                    SDL_ShowCursor(SDL_ENABLE);
                    return;
                }
                        
               
                break;
            default:
                break;
        }
    }
    
    
    
}


void GameEngine::Run(){
    
    IActivity* currentActivity;
    
    while (activities.size() > 0) {
        
        //Verify is we should be capturing the mouse or not.
        
        
        PumpEvents();
        
        //Clear the screen
        renderer.Clear();
        
        //Allow the active activity to Run and Render
        currentActivity = activities.top();
        
        if (currentActivity->IsRunning())
            currentActivity->Run();
        else{
            activities.pop();
            delete currentActivity;
        }
            
        
        // Render the mouse if we should
        
        renderer.Swap();
        
        //Flush all events since they should all have been interpreted.
        SDL_FlushEvents(SDL_FIRSTEVENT,SDL_LASTEVENT);
    }
}

void GameEngine::Terminate(const char* reason, ...){
    Log("Terminating: ");
    va_list args;
    va_start(args,reason);
    vfprintf(stdout,reason,args);
    va_end(args);
    Log("\n");
    exit(0);
}

void GameEngine::Log(const char* text, ...){
    va_list args;
    va_start(args,text);
    vfprintf(stdout,text,args);
    va_end(args);
}

void GameEngine::LogError(const char* text, ...){
    va_list args;
    va_start(args,text);
    vfprintf(stderr,text,args);
    va_end(args);
}


void GameEngine::AddActivity(IActivity* activity){
    this->activities.push(activity);
}












