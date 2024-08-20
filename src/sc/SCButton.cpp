//
//  Button.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


SCButton::SCButton(){
    this->enabled = true;
    this->onClick = nullptr;
    this->position = Point2D({0,0});
    this->dimension = Point2D({0,0});
}

SCButton::~SCButton(){
    
}

void SCButton::InitBehavior(ActionFunction fct, Point2D position,Point2D dimension){
    this->onClick = fct;
    this->position = position;
    this->dimension = dimension;
}

void SCButton::OnAction(void){
    this->onClick();
}
