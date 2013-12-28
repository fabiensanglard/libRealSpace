//
//  Base.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Base.h"

static char base[512];

const char* GetBase(void){
    return base;
}

void SetBase(const char* newBase){
    
    strncpy(base,newBase,512);
    
    //If the last character is not '/', add it
    if (base[strlen(base)-1] != '/')
        strcat(base,"/");
    
}