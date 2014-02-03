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
    Game.Log("We are not freeing the RAM from all the RLEs !!!\n");
}

void ConvAssetManager::Init(void){
    BuildDB();
}

NPCChar* ConvAssetManager::GetPNCChar(char* name){
    
    NPCChar* npc = this->nps[name];
    
    if (npc == NULL){
        static NPCChar dummy;
        dummy.appearance = RLEShape::GetEmptyShape();
        npc = &dummy;
        Game.Log("ConvAssetManager: Cannot find npc '%s', returning dummy npc instead.\n",name);
        this->nps[name] = npc;
    }
    
    return npc;
}

ConvBackGround* ConvAssetManager::GetBackGround(char* name){
    
    ConvBackGround* shape = this->backgrounds[name];
    
    if (shape == NULL){
        Game.Log("ConvAssetManager: Cannot find loc '%s', returning dummy loc instead.\n",name);
        static ConvBackGround dummy;
        uint8_t dummyPalettePatch[5] = { 0, 0, 0 ,0 , 0};
        dummy.palettes.push_back(dummyPalettePatch);
        dummy.layers.push_back(RLEShape::GetEmptyShape());
        shape = &dummy;
        this->backgrounds[name] = shape;
    }
    
    return shape;    
}

void ConvAssetManager::ParseBGLayer(uint8_t* data, size_t layerID,ConvBackGround* back ){
    
    ByteStream dataReader ;
    dataReader.Set(data + 5 * layerID);
    
    uint8_t type = dataReader.ReadByte();
    uint8_t shapeID = dataReader.ReadByte();
    uint8_t paletteID = dataReader.ReadByte();
    
    PakArchive* shapeArchive = NULL;
    PakArchive* paletteArchive = NULL;
    
    if (type == 0x00){
        // RLEShape is in CONVSHPS.PAK and Palette is in CONVPALS.PAK
        shapeArchive = & this->convShps;
        paletteArchive = &this->convPals;
    }
    
    if (type == 0x01){
        // RLEShape is in OPTSHPS.PAK and Palette is in OPTPALS.PAK
        shapeArchive = & this->optShps;
        paletteArchive = &this->optPals;
    }
    
    
    //Debug Display
    
        printf("\n%8s layer %lu :",back->name,layerID);
        for (size_t x=0; x < 5 ; x++) {
            printf("%3d ",*(data + 5 * layerID+x));
        }
    
    
    
    
    
    
    /*
    // The pack features some duplicate entries.
    while(convShapeArchive.GetEntry(shapeID)->size == 0)
        shapeID--;
    */
    
    RLEShape* s = new RLEShape();
    
    PakEntry* shapeEntry = shapeArchive->GetEntry(shapeID);
    PakArchive subPAK;
    subPAK.InitFromRAM("", shapeEntry->data,shapeEntry->size);
    
    if (!subPAK.IsReady()){
        
        //Sometimes the image is not in a PAK but as raw data.
        Game.Log("Error on Pak %d for layer %d in loc %8s => Using dummy instead\n",shapeID,layerID,back->name);
        
        //Using an empty shape for now...
        *s = *RLEShape::GetEmptyShape();
        return;
    }
    else
        s->Init(subPAK.GetEntry(0)->data, subPAK.GetEntry(0)->size);
    
    
    back->layers.push_back(s);
    back->palettes.push_back(paletteArchive->GetEntry(paletteID)->data);

}


void ConvAssetManager::ReadBackGrounds(const IffChunk* chunkRoot){
    
    for(size_t i = 0 ; i < chunkRoot->childs.size() ; i ++){
        IffChunk* chunk = chunkRoot->childs[i];
        if (chunk->id != 'FORM'){
            Game.Log("ConvAssetManager::ReadBackGrounds => Unexpected chunk (%s).\n",chunk->GetName());
            Game.Terminate("Unable to build CONV database.\n");
        }
        
        IffChunk* info = chunk->childs[0];
        
        ConvBackGround* back = new ConvBackGround();

        //Get the name
        memset(back->name, 0, 9);
        memcpy(back->name, info->data, info->size);
        
        //Parse layers and associated bgs.
        size_t numLayers = chunk->childs[1]->size / 5 ; //A layer entry is 5 bytes wide
        for (size_t layerID=0; layerID < numLayers; layerID++)
            ParseBGLayer(chunk->childs[1]->data,layerID,back);
        
        this->backgrounds[back->name] = back;
        //Game.Log("  Able to reach shape in CONVSHPS.PAK entry %d from background '%s'.\n",shapeID,back->name);
    }
}
void ConvAssetManager::ReadFaces(const IffChunk* chunk){}
void ConvAssetManager::ReadFigures(const IffChunk* chunk){}
void ConvAssetManager::ReadPFigures(const IffChunk* chunk){}
//I have no idea what is in there.
void ConvAssetManager::ReadFCPL(const IffChunk* chunk){}
//I have no idea what is in there.
void ConvAssetManager::ReadFGPL(const IffChunk* chunk){}

void ConvAssetManager::BuildDB(void){
    
    //This is were the background shapes are stored.
    TreEntry* convShapEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVSHPS.PAK");
    convShps.InitFromRAM("CONVSHPS.PAK",convShapEntry->data,convShapEntry->size);
    //convShapeArchive.List(stdout);
    
    //This is were the palette patches are stored
    TreEntry* convPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK");
    convPals.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
    //convPalettePak.List(stdout);

    //This is were the background shapes are stored.
    TreEntry* optShapEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    optShps.InitFromRAM("OPTSHPS.PAK",optShapEntry->data,optShapEntry->size);
    //optShps(stdout);
    
    //This is were the palette patches are stored
    TreEntry* optPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    //optPals(stdout);

    
    
    //Open the metadata
    TreEntry* convDataEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVDATA.IFF");
    IffLexer convDataLexer;
    convDataLexer.InitFromRAM(convDataEntry->data, convDataEntry->size);
    
    
    ReadBackGrounds(convDataLexer.GetChunkByID('BCKS'));
    
    ReadFaces(convDataLexer.GetChunkByID('FACE'));
    
    ReadFigures(convDataLexer.GetChunkByID('FIGR'));
    
    ReadPFigures(convDataLexer.GetChunkByID('PFIG'));
    
    //I have no idea what is in there.
    ReadFCPL(convDataLexer.GetChunkByID('FCPL'));

    //I have no idea what is in there.
    ReadFGPL(convDataLexer.GetChunkByID('FGPL'));

        
}