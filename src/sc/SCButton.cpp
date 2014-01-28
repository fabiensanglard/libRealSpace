//
//  Button.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


SCButton::SCButton():
    enabled(true){
    
}

SCButton::~SCButton(){
    
}

void SCButton::InitBehavior(ActionFunction fct, Point2D position,Point2D dimension){
    this->state = STATE_UP;
    this->onClick = fct;
    this->dimension = dimension;
}

void SCButton::OnAction(void){
    
}
