//
//  RSMapTextureSet.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <stdint.h>
#include "PakArchive.h"
#include "RSImage.h"

//TODO: CHANGE CONSTRUCTORE TO USE IFF and MapImageEntry!!!

typedef struct{
    
    uint16_t startingIDs;
    char name[9];
    
    uint8_t unknown0;
    uint8_t numImages;
    uint8_t unknown1;
    uint8_t unknown2;
    uint8_t unknown3;
    
} MapImageEntry;

class RSMapTextureSet{
  
public:
    
    RSMapTextureSet();
    ~RSMapTextureSet();
    
    void InitFromPAK(PakArchive* archive);
    
    size_t GetNumImages(void);
    RSImage* GetImageById(size_t index);
    
    void List(FILE* output);
    
private:
    
    char name[512];
    
    void Parse(PakArchive* archive);
    std::vector<RSImage*> images;
  
};
