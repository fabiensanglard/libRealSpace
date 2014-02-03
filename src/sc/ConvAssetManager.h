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
    
    uint8_t* palettePatch;
    RLEShape* appearance;
    
} ConvBackGround;



class ConvAssetManager{

public:
    
    ConvAssetManager();
    ~ConvAssetManager();
    
    void Init(void);
    
    NPCChar* GetPNCChar(const char* name);
    ConvBackGround* GetBackGround(const char* name);

private:
    
    void BuildDB(void);
    std::map<const char*, NPCChar*,Char_String_Comparator> nps;
    std::map<const char*, ConvBackGround*,Char_String_Comparator> locations;
};


#endif /* defined(__libRealSpace__ConAssetManager__) */
