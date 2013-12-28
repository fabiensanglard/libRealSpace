//
//  RealSpacePaletteObject.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RealSpacePaletteObject.h"


RealSpacePaletteObject::RealSpacePaletteObject(){

}

RealSpacePaletteObject::~RealSpacePaletteObject(){
    
}

Palette* RealSpacePaletteObject::GetColorPalette(void){
    return &this->colors;
}

Palette* RealSpacePaletteObject::GetBWPalette(void){
    return &this->bwColors;
}

void RealSpacePaletteObject::SetColorFlag(uint32_t flag){
    this->colorFlag = flag;
}

void RealSpacePaletteObject::SetBWFlag(uint32_t flag){
    this->bwFlag  =flag;
}