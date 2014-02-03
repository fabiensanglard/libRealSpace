//
//  ConAssetManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/1/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"



ConvAssetManager::ConvAssetManager(){
    
}

ConvAssetManager::~ConvAssetManager(){
    
}

void ConvAssetManager::Init(void){
    BuildDB();
}

NPCChar* ConvAssetManager::GetPNCChar(const char* name){
    
    NPCChar* npc = this->nps[name];
    
    if (npc == NULL){
        static NPCChar dummy;
        dummy.appearance = RLEShape::GetEmptyShape();
        npc = &dummy;
        Game.Log("ConvAssetManager: Cannot find npc '%s', returning dummy npc instead.\n",name);
    }
    
    return npc;
}

RLEShape* ConvAssetManager::GetBackGround(const char* name){
    
    RLEShape* shape = this->locations[name];
    
    if (shape == NULL){
        Game.Log("ConvAssetManager: Cannot find loc '%s', returning dummy loc instead.\n",name);
        shape = RLEShape::GetEmptyShape();
    }
    
    return shape;    
}



void ConvAssetManager::BuildDB(void){
    
    RLEShape* s ;
    
    
    TreEntry* convShapEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVSHPS.PAK");
    PakArchive convShapeArchive;
    convShapeArchive.InitFromRAM("",convShapEntry->data,convShapEntry->size);
    
    
    PakEntry* ba2_cuEntry = convShapeArchive.GetEntry(ba2_cu);
    PakArchive ba2_cu;
    ba2_cu.InitFromRAM("", ba2_cuEntry->data,ba2_cuEntry->size);
    s = new RLEShape();
    s->Init(ba2_cu.GetEntry(0)->data, ba2_cu.GetEntry(0)->size);
    locations["ba2_cu"] = s ;
    
    PakEntry* ba1_cuEntry = convShapeArchive.GetEntry(ba1_cu);
    PakArchive ba1_cu;
    ba1_cu.InitFromRAM("", ba1_cuEntry->data,ba1_cuEntry->size);
    s = new RLEShape();
    s->Init(ba1_cu.GetEntry(0)->data, ba1_cu.GetEntry(0)->size);
    locations["ba1_cu"] = s ;
    
    
    
    TreEntry* convPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK");
    PakArchive convPalettePak;
    convPalettePak.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
    
}