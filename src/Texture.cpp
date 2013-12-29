//
//  Texture.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"



Texture::Texture()
 : data(0),
  locFlag(DISK)

{
    
}

Texture::~Texture(){
    
    
    
    if ((locFlag & VRAM)){
        renderer.UnloadTextureToVRAM(this);
        locFlag &= ~VRAM;
    }
    
    if ((locFlag & RAM)){
        free(data);
        locFlag &= RAM;
    }
}

void Texture::Set(const char name[8],uint32_t width,uint32_t height,uint8_t* data ){
    
    CreateEmpty(name,width,height);
    memcpy(this->data,data, width * height);
    locFlag = RAM;
}

void Texture::CreateEmpty(const char name[8],uint32_t width,uint32_t height){
    
    strncpy(this->name,name,8);
    this->width = width;
    this->height = height;
    this->data = (uint8_t*)malloc(width*height);
    locFlag = RAM;
}

uint32_t Texture::GetTextureID(void){
    
    if ((locFlag & VRAM) == 0){
        
        renderer.UploadTextureToVRAM(this,Renderer::USE_DEFAULT_PALETTE);
        
        locFlag |= VRAM;
    }
    
    return this->id;
}

void Texture::SetRLECenterCoo(int16_t left,int16_t right,int16_t top,int16_t bottom){

    //
    rleCenter= this->data + abs(left) + abs(top) * this->width;
    this->left=left;
    this->right=right;
    this->top=top;
    this->bottom=bottom;
}


bool Texture::WriteRLETexel(int16_t dx,int16_t dy, uint8_t color){
    
    
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

void Texture::Clear(void){
    memset(this->data,0,this->width*this->height);
}
