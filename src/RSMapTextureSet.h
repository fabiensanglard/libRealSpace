//
//  RSMapTextureSet.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSMapTextureSet__
#define __libRealSpace__RSMapTextureSet__

class RSMapTextureSet{
  
public:
    
    RSMapTextureSet();
    ~RSMapTextureSet();
    
    void InitFromPAK(PakArchive* archive);
    
    size_t GetNumTextures(void);
    Texture* GetTextureById(size_t index);
    
    void List(FILE* output);
    
private:
    
    char name[512];
    
    void Parse(PakArchive* archive);
    std::vector<Texture*> textures;
  
};

#endif /* defined(__libRealSpace__RSMapTextureSet__) */
