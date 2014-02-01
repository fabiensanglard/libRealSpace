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
    
    uint8_t color;
    RLEShape* appearances;
    
} NPCChar;


class ConvAssetManager{

public:
    
    ConvAssetManager();
    ~ConvAssetManager();
    
    void Init(void);
    
    NPCChar* GetPNCChar(char* name);
    RLEShape* GetBackGround(char* name);

private:
    
    void BuildDB(void);
    static std::map<char*, NPCChar> nps;
    static std::map<char*, RLEShape> locations;
};


#endif /* defined(__libRealSpace__ConAssetManager__) */
