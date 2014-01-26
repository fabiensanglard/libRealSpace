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

void GameEngine::Run(){
    
    IActivity* currentActivity;
    
    while (activities.size() > 0) {
        
        //Clear the screen
        
        //Allow the active activity to Run and Render
        currentActivity = activities.top();
        currentActivity->Run();
        
        // Render the mouse if we should
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












