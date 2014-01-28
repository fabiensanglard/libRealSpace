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


SCButton* IActivity::CheckButtons(void){
    return NULL;
}

void IActivity::DrawButtons(void){
    
    
    for(size_t i = 0 ; i < buttons.size() ; i++){
        SCButton* button = buttons[i];
        if (button->IsEnabled())
            VGA.DrawShape(&button->appearance[SCButton::Appearance::APR_UP]);
        else
            VGA.DrawShape(&button->appearance[SCButton::Appearance::APR_DOWN]);
    }
    
}