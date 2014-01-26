//
//  IActivity.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


IActivity::IActivity(){
    
}

IActivity::~IActivity()
{
    
}


void IActivity::SetTitle(const char* title){
    renderer.SetTitle(title);
}