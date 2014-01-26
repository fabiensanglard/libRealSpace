//
//  AssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__AssetManager__
#define __libRealSpace__AssetManager__

class AssetManager{
    
public:
    void SetBase(const char* base);
    void Init(void);
    
    enum TreID {TRE_GAMEFLOW, TRE_OBJECTS, TRE_MISC, TRE_SOUND, TRE_MISSIONS,TRE_TEXTURES } ;
    
    std::vector<TreArchive*> tres;
    
    
    AssetManager();
    ~AssetManager();
    
private:
    TreArchive* LoadTRE(const char* name);
    PakArchive* LoadPAK(const char* name);
};


#endif /* defined(__libRealSpace__AssetManager__) */
