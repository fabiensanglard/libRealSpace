//
//  Button.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


SCButton::SCButton(){
    
}

SCButton::~SCButton(){
    
}

void SCButton::Init(RSImage* upApp, RSImage* downApp,ActionFunction fct){
    
    this->apperance[APR_UP] = upApp;
    this->apperance[APR_DOWN] = downApp;
    
    this->state = STATE_UP;
    
    this->onClick = fct;
}

void SCButton::OnAction(void){
    
}
