//
//  RealSpacePaletteObject.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


RSPalette::RSPalette(){

}

RSPalette::~RSPalette(){
    
}

Palette* RSPalette::GetColorPalette(void){
    return &this->colors;
}

Palette* RSPalette::GetBWPalette(void){
    return &this->bwColors;
}

void RSPalette::SetColorFlag(uint32_t flag){
    this->colorFlag = flag;
}

void RSPalette::SetBWFlag(uint32_t flag){
    this->bwFlag  =flag;
}

void RSPalette::ParsePALT(IffChunk* chunk){
    
    ByteStream stream(chunk->data);
    
    this->colorFlag = stream.ReadUInt32LE();

    Texel texel;
    
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = stream.ReadByte();
        texel.g = stream.ReadByte();
        texel.b = stream.ReadByte();
        
        if (i != 255)
            texel.a = 255;
        else
            texel.a = 0;
        
        //Since VGA palette RGB are on 6 bits and not 8, we need to adjust
        texel.r *= 255/64.0f;
        texel.g *= 255/64.0f;
        texel.b *= 255/64.0f;
        
        colors.SetColor(i, &texel);
    }
    
}

void RSPalette::ParseBLWH(IffChunk* chunk){
    
    ByteStream stream(chunk->data);
    
    this->bwFlag = stream.ReadUInt32LE();

    
    Texel texel;
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = stream.ReadByte();
        texel.g = stream.ReadByte();
        texel.b = stream.ReadByte();
        
        if (i != 255)
            texel.a = 255;
        else
            texel.a = 0;
        
        texel.r *= 255/64.0f;
        texel.g *= 255/64.0f;
        texel.b *= 255/64.0f;
        bwColors.SetColor(i, &texel);
    }
}

void RSPalette::ParseCMAP(IffChunk* chunk){
    
    ByteStream stream(chunk->data);
    
    Texel texel;
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = stream.ReadByte();
        texel.g = stream.ReadByte();
        texel.b = stream.ReadByte();
        texel.a = 255;
        colors.SetColor(i, &texel);
    }
    
}


void RSPalette::InitFromIFF(IffLexer* lexer){
    
    IffChunk* chunk = NULL;
    
    chunk = lexer->GetChunkByID('PALT');
    if (chunk != NULL){
        this->ParsePALT(chunk);
    }
    
    chunk = lexer->GetChunkByID('BLWH');
    if (chunk != NULL){
        this->ParseBLWH(chunk);
        
    }
    
    chunk = lexer->GetChunkByID('CMAP');
    if (chunk != NULL){
        this->ParseCMAP(chunk);
    }
    
}