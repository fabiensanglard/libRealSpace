//
//  Texture.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Texture.h"
#include "RSImage.h"
#include "SCRenderer.h"

extern SCRenderer Renderer;

Texture::Texture()
 : data(0),
  locFlag(DISK)

{
    
}

Texture::~Texture(){
    
    if (id)
        Renderer.DeleteTextureInGPU(this);
    if (data)
        free(data);
}

void Texture::Set(RSImage* image){
    
    strncpy_s(this->name,image->name,8);
    this->width = image->width;
    this->height = image->height;
    this->data = (uint8_t*)malloc(width*height*4);
    locFlag = RAM;
    
    //UpdateContent(image);
}

uint32_t Texture::GetTextureID(void){
    
    
    return id;
    
}


void Texture::UpdateContent(RSImage* image){
    
    uint8_t* src = image->data;
    uint8_t* dst = this->data;
    VGAPalette* palette = image->palette;
    
    for(int i=0 ; i < image->height ; i++){
    for(int j=0 ; j < image->width  ; j++){
        
        uint8_t* srcIndex = src + j + i* image->width;
        
        const Texel* src = palette->GetRGBColor( (*srcIndex) );
        
        dst[0] = src->r;
        dst[1] = src->g;
        dst[2] = src->b;
        dst[3] = src->a;
        
        dst+=4;
    }
    }
}
