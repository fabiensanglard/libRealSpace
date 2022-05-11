//
//  RSMusic.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <stdint.h>
class PakArchive;

class RSMusic{
    
public:
    void InitFromPAK(PakArchive* archive, uint32_t index);    
};

