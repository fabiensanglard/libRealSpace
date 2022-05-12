//
//  RSImage.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/31/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSImage.h"
#include "SCRenderer.h"
#include "Texture.h"
#include "RSPalette.h"
#include "RSVGA.h"


extern SCRenderer Renderer;

RSImage::RSImage() :
data(0),
dirty(false),
palette(NULL){
    
}

RSImage::~RSImage(){
    if (data)
        free(data);
}

void RSImage::Create(const char name[8],uint32_t width,uint32_t height){

    strncpy(this->name,name,8);
    this->width = width;
    this->height = height;
    this->data = (uint8_t*)malloc(this->width*this->height);
    this->palette = Renderer.GetPalette();
    
    this->texture.Set(this);
    dirty = true;
}

void RSImage::UpdateContent(uint8_t* src){
    
    memcpy(this->data,src, width * height);
    this->dirty = true;
    
}

void RSImage::SyncTexture(void){
    
    //Check that we have a texture with an id on the GPU
    if ((texture.locFlag & Texture::VRAM) != Texture::VRAM){
        //Create texture in the GPU
        Renderer.CreateTextureInGPU(&texture);
        texture.locFlag |= Texture::VRAM;
    }
    
    //Check if we are synchornized with GPU
    if (this->dirty){
        texture.UpdateContent(this);
        Renderer.UploadTextureContentToGPU(&texture);
        dirty = false;
    }
    
    
}


uint8_t* RSImage::GetData(void){
    dirty = true;
    return data;
}

void RSImage::ClearContent(void){
    memset(this->data,0,this->width*this->height);
    dirty = true;
}

void RSImage::SetPalette(VGAPalette* palette){
    this->palette = palette;
    dirty = true;
}

Texture* RSImage::GetTexture(void){
    
    SyncTexture();
    
    return &this->texture;
}
