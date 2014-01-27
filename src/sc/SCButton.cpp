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

void SCButton::InitBehavior(ActionFunction fct, Point2D position){
    this->state = STATE_UP;
    this->onClick = fct;
    this->appearance[APR_DOWN].SetPosition(position);
    this->appearance[APR_UP].SetPosition(position);
}

void SCButton::OnAction(void){
    
}
