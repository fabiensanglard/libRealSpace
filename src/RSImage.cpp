//
//  RSImage.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/31/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


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

    strcpy(this->name,name);
    this->width = width;
    this->height = height;
    this->data = (uint8_t*)malloc(this->width*this->height);
    this->palette = renderer.GetDefaultPalette();
    
    this->texture.Set(this);
}

void RSImage::UpdateContent(uint8_t* src){
    
    memccpy(this->data,src, 1, width * height);
    this->dirty = true;
    
}

void RSImage::SyncTexture(void){
    
    //Check that we have a texture with an id on the GPU
    if ((texture.locFlag & Texture::VRAM) != Texture::VRAM){
        //Create texture in the GPU
        renderer.CreateTextureInGPU(&texture);
    }
    
    //Check if we are synchornized with GPU
    if (this->dirty){
        texture.UpdateContent(this);
        renderer.UploadTextureContentToGPU(&texture);
        dirty = false;
    }
    
    
}

void RSImage::SetPalette(VGAPalette* palette){
    this->palette = palette;
}

Texture* RSImage::GetTexture(void){
    return &this->texture;
}


void RSImage::SetRLECenterCoo(int16_t left,int16_t right,int16_t top,int16_t bottom){
    
    //
    rleCenter= this->data + abs(left) + abs(top) * this->width;
    this->left=left;
    this->right=right;
    this->top=top;
    this->bottom=bottom;
}


bool RSImage::WriteRLETexel(int16_t dx,int16_t dy, uint8_t color){
    
    
    uint8_t* dst = rleCenter;
    dst+=dx;
    dst+=dy*this->width;
    
    
    
    if (dst < (this->data+this->width*this->height) && dst >= this->data)
        *dst = color;
    else{
        //printf("Error, trying to write outside texture.\n");
        return true;
    }
    
    return false;
}