//
//  ConAssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/1/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__ConAssetManager__
#define __libRealSpace__ConAssetManager__



typedef struct NPCChar{
    
    char name[9];
    RLEShape* appearance;
    
} NPCChar;



typedef struct ConvBackGround{
    std::vector<RLEShape*> layers;
    std::vector<uint8_t*> palettes;
    char name[9];
} ConvBackGround;



class ConvAssetManager{

public:
    
    ConvAssetManager();
    ~ConvAssetManager();
    
    void Init(void);
    
    NPCChar* GetPNCChar(char* name);
    ConvBackGround* GetBackGround(char* name);

private:
    
    void BuildDB(void);
    void ReadBackGrounds(const IffChunk* chunk);
    void ReadFaces(const IffChunk* chunk);
    void ReadFigures(const IffChunk* chunk);
    void ReadPFigures(const IffChunk* chunk);
    //I have no idea what is in there.
    void ReadFCPL(const IffChunk* chunk);
    //I have no idea what is in there.
    void ReadFGPL(const IffChunk* chunk);
  
    std::map<char*, NPCChar*,Char_String_Comparator> nps;
    std::map<char*, ConvBackGround*,Char_String_Comparator> backgrounds;
    
    PakArchive convShps;
    PakArchive convPals;
    PakArchive optShps;
    PakArchive optPals;
    void ParseBGLayer(uint8_t* data, size_t layerID,ConvBackGround* back );
    
};


#endif /* defined(__libRealSpace__ConAssetManager__) */
