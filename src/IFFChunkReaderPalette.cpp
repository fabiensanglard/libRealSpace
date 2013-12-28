//
//  IFFPaletteReader.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

IFFChunkReaderPalette::IFFChunkReaderPalette(){
    AddCapability(&IFFChunkReaderPalette::ParsePALT,"PALT");
    AddCapability(ParseBLWH,"BLWH");
    AddCapability(ParseRANG,"RANG");
    AddCapability(ParseCMAP,"CMAP");
}

IFFChunkReaderPalette::~IFFChunkReaderPalette(){
    
}





void IFFChunkReaderPalette::ParsePALT(char id[4],uint8_t* data, size_t size){

    palette.SetColorFlag(*((uint32_t*)data));
    
    data+=4;
    
    Texel texel;
    
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = *data++;
        texel.g = *data++;
        texel.b = *data++;
        
        if (i != 255)
            texel.a = 255;
        else
            texel.a = 0;
        
        //Since VGA palette RGB are on 6 bits and not 8, we need to adjust
        texel.r *= 255/64.0f;
        texel.g *= 255/64.0f;
        texel.b *= 255/64.0f;
        
        palette.GetColorPalette()->SetColor(i, &texel);
    }
        
}

void IFFChunkReaderPalette::ParseBLWH(char id[4],uint8_t* data, size_t size){
    
    palette.SetBWFlag(*((uint32_t*)data));
    
    data+=4;
    
    Texel texel;
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = *data++;
        texel.g = *data++;
        texel.b = *data++;
        
        if (i != 255)
            texel.a = 255;
        else
            texel.a = 0;
        
        texel.r *= 255/64.0f;
        texel.g *= 255/64.0f;
        texel.b *= 255/64.0f;
        palette.GetBWPalette()->SetColor(i, &texel);
    }
}

void IFFChunkReaderPalette::ParseCMAP(char id[4],uint8_t* data, size_t size){
    
    Texel texel;
    
    for(int i = 0 ; i < 256 ; i++){
        texel.r = *data++;
        texel.g = *data++;
        texel.b = *data++;
        texel.a = 255;
        palette.GetColorPalette()->SetColor(i, &texel);
    }

}

void IFFChunkReaderPalette::ParseRANG(char id[4],uint8_t* data, size_t size){
    
}