//
//  Button.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCButton.h"


SCButton::SCButton(){
    this->enabled = true;
    this->onClick = nullptr;
    this->position = Point2D({0,0});
    this->dimension = Point2D({0,0});
    this->apre = APR_UP;
}

SCButton::~SCButton(){
    
}

void SCButton::InitBehavior(std::function<void()> fct, Point2D position,Point2D dimension){
    this->onClick = fct;
    this->position = position;
    this->dimension = dimension;
}

void SCButton::OnAction(void){
    if (this->onClick != nullptr)
        this->onClick();
}
