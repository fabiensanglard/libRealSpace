//
//  SCFont.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "RLEShape.h"
#include "PakArchive.h"

class RSFont{
public:
    RSFont();
    ~RSFont();
    
    RLEShape* GetShapeForChar(char c);
    void InitFromPAK(PakArchive* fontArchive);
    
protected:
private:
   std::vector<RLEShape*> letters;
};
