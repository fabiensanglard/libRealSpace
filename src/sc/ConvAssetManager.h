//
//  ConAssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/1/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__ConAssetManager__
#define __libRealSpace__ConAssetManager__

#define CONV_TOP_BAR_HEIGHT 23
#define CONV_BOTTOM_BAR_HEIGHT 48

typedef struct CharFace{
    
    char name[9];
    RSImageSet* appearances;
    // size_t paletteID;
} CharFace;


typedef struct FacePalette{
    
    char name[9];
    uint8_t index;
    
} FacePalette;

typedef struct CharFigure{
    
    char name[9];
    RLEShape* appearance;
    size_t paletteID;
    
} CharFigure;


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
    
    CharFace* GetCharFace(char* name);
    ConvBackGround* GetBackGround(char* name);
    CharFigure* GetFigure(char* name);
    
    uint8_t GetFacePaletteID(char* name);
    
    
    
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
  
    
    
    
    std::map<char*, CharFace*,Char_String_Comparator> faces;
    std::map<char*, FacePalette* ,Char_String_Comparator> facePalettes;
    std::map<char*, ConvBackGround*,Char_String_Comparator> backgrounds;
    std::map<char*, CharFigure*,Char_String_Comparator> figures;

    
    PakArchive convShps;
    PakArchive convPals;
    PakArchive optShps;
    PakArchive optPals;
    void ParseBGLayer(uint8_t* data, size_t layerID,ConvBackGround* back );
    
};


#endif /* defined(__libRealSpace__ConAssetManager__) */
