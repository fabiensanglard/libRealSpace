//
//  AssetManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

typedef struct TreNameID{
    AssetManager::TreID id ;
    const char* filename;
    
} TreNameID;

#define NUM_TRES 6
TreNameID nameIds[NUM_TRES] =
{
    {AssetManager::TRE_GAMEFLOW,"GAMEFLOW.TRE"},
    {AssetManager::TRE_OBJECTS,"OBJECTS.TRE"},
    {AssetManager::TRE_MISC,"MISC.TRE"},
    {AssetManager::TRE_SOUND,"SOUND.TRE"},
    {AssetManager::TRE_MISSIONS,"MISSIONS.TRE"},
    {AssetManager::TRE_TEXTURES,"TEXTURES.TRE"}
};

void AssetManager::SetBase(const char* newBase){
    ::SetBase(newBase);
}

void AssetManager::Init(void){
    
    //Load all TRE in RAM and store them.
    for (size_t i =0 ; i < NUM_TRES; i++) {
        TreArchive* tre = new TreArchive();
        tre->InitFromFile(nameIds[i].filename);
        
        if (tre->IsValid())
            this->tres.push_back(tre);
        else{
            Game.Terminate("Unable to load asset '%s' (Is the SC base set).",nameIds[i].filename);
            
        }
    }
}

AssetManager::AssetManager(){
    
}

AssetManager::~AssetManager(){
    
    while(!tres.empty()){
        TreArchive* tre = tres.back();
        tres.pop_back();
        delete tre;
    }
}



