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
    
    char* name;
    RLEShape* appearance;
    
} NPCChar;


class ConvAssetManager{

public:
    
    ConvAssetManager();
    ~ConvAssetManager();
    
    void Init(void);
    
    NPCChar* GetPNCChar(const char* name);
    RLEShape* GetBackGround(const char* name);

private:
    
    void BuildDB(void);
    static std::map<char*, NPCChar> nps;
    static std::map<char*, RLEShape> locations;
};


#endif /* defined(__libRealSpace__ConAssetManager__) */
