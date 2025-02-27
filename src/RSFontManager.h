//
//  SCFontManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <map>
#include <string.h>

#include "RSFont.h"
#include "TreArchive.h"

class RSFontManager{
public:
    RSFontManager();
    ~RSFontManager();
    
    void Init(void);
    RSFont* GetFont(const char* name);
    
protected:
private:
    std::map<std::string, RSFont*> fonts;
};

