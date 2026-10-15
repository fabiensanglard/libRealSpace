//
//  RealSpacePaletteObject.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include "RSPalette.h"


RSPalette::RSPalette(){

}

RSPalette::~RSPalette(){
    
}

VGAPalette* RSPalette::GetColorPalette(void){
    return &this->colors;
}

VGAPalette* RSPalette::GetBWPalette(void){
    return &this->bwColors;
}

void RSPalette::SetColorFlag(uint32_t flag){
    this->colorFlag = flag;
}

void RSPalette::SetBWFlag(uint32_t flag){
    this->bwFlag  =flag;
}

void RSPalette::copyFrom(VGAPalette* other){
    for (int i=0; i< 256; i++) {
        if (other->colors[i].a == 0) {
            continue;
        }   
        this->colors.colors[i] = other->colors[i];
    }
}
void RSPalette::initFromFileData(FileData* fileData) {
    this->initFromFileRam(fileData->data, fileData->size);
}
void RSPalette::initFromFileRam(uint8_t *data, size_t size, int size_offset) {
    IFFSaxLexer lexer;
    lexer.size_offset = size_offset;
    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["PALT"] = std::bind(&RSPalette::parsePALT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PAL "] = std::bind(&RSPalette::parsePAL, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSPalette::parsePAL(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["CMAP"] = std::bind(&RSPalette::parsePAL_CMAP, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSPalette::parsePAL_CMAP(uint8_t *data, size_t size){
    ByteStream stream(data, size);
    
    Texel texel;
    if (size == 772) {
        this->colorFlag = stream.ReadUInt32LE();
    }
    for(int i = 0 ; i < 256 ; i++){
        texel.r = stream.ReadByte();
        texel.g = stream.ReadByte();
        texel.b = stream.ReadByte();
        texel.a = 255;
        
        colors.SetColor(i, &texel);
    }
}
void RSPalette::parsePALT(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSPalette::parsePALT_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PALT"] = std::bind(&RSPalette::parsePALT_PALT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["BLWH"] = std::bind(&RSPalette::parsePALT_BLWH, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SKYC"] = std::bind(&RSPalette::parsePALT_SKYC, this, std::placeholders::_1, std::placeholders::_2);
    handlers["RANG"] = std::bind(&RSPalette::parsePALT_RANG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TABL"] = std::bind(&RSPalette::parsePALT_TABL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["BILD"] = std::bind(&RSPalette::parsePALT_BILD, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSPalette::parsePALT_INFO(uint8_t *data, size_t size){
    
}
void RSPalette::parsePALT_TABL(uint8_t *data, size_t size){
    
}

void RSPalette::parsePALT_PALT(uint8_t *data, size_t size){
    ByteStream stream(data, size);
    
    //this->colorFlag = stream.ReadUInt32LE();
    if (size == 772) {
        this->colorFlag = stream.ReadUInt32LE();
    }
    Texel texel;
    
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = stream.ReadByte();
        texel.g = stream.ReadByte();
        texel.b = stream.ReadByte();
        
        texel.a = 255;
        if (i == 255) {
            texel.a = 0;
        } else {
            texel.a = 255;
        }
            
        uint8_t r = texel.r;
        uint8_t g = texel.g;
        uint8_t b = texel.b;

        r = convertFrom6To8(r);
        g = convertFrom6To8(g);
        b = convertFrom6To8(b);

        texel.r = r;
        texel.g = g;
        texel.b = b;
        
        colors.SetColor(i, &texel);
    }
}
void RSPalette::parsePALT_BLWH(uint8_t *data, size_t size){
    ByteStream stream(data, size);
    
    //this->bwFlag = stream.ReadUInt32LE();

    
    Texel texel;
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = stream.ReadByte();
        texel.g = stream.ReadByte();
        texel.b = stream.ReadByte();
        
        if (i == 255) {
            texel.a = 0;
        } else {
            texel.a = 255;
        }
        
        uint8_t r = texel.r;
        uint8_t g = texel.g;
        uint8_t b = texel.b;

        r = convertFrom6To8(r);
        g = convertFrom6To8(g);
        b = convertFrom6To8(b);
        
        texel.r = r;
        texel.g = g;
        texel.b = b;

        
        bwColors.SetColor(i, &texel);
    }
}
void RSPalette::parsePALT_SKYC(uint8_t *data, size_t size){
    
}
void RSPalette::parsePALT_RANG(uint8_t *data, size_t size){
    ByteStream stream(data, size);
    uint16_t count = stream.ReadUShort();
    for (int i = 0; i < count; i++) {
        PaletteRange range;
        range.first = stream.ReadUShort();
        range.shades = stream.ReadUShort();
        range.fog = stream.ReadUShort();
        for (int k = 0; k < 7; k++) {
            stream.ReadByte();
        }
        this->ranges.push_back(range);
    }
}
void RSPalette::parsePALT_BILD(uint8_t *data, size_t size){
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSPalette::parsePALT_BILD_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TABL"] = std::bind(&RSPalette::parsePALT_BILD_TABL, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}
void RSPalette::parsePALT_BILD_INFO(uint8_t *data, size_t size){
    
}
void RSPalette::parsePALT_BILD_TABL(uint8_t *data, size_t size){
    
}