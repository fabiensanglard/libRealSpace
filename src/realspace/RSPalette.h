//
//  RealSpacePaletteObject.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
#pragma once

#include <stdint.h>

#include "AssetManager.h"
#include "../engine/Texture.h"
#include "RSEntity.h"

struct VGAPalette;

class RSPalette{
    
public:
    
    RSPalette();
    ~RSPalette();
    
    void initFromFileData(FileData* fileData);
    void initFromFileRam(uint8_t *data, size_t size);
    VGAPalette* GetColorPalette(void);
    void SetColorFlag(uint32_t flag);
    
    VGAPalette* GetBWPalette(void);
    void SetBWFlag(uint32_t flag);
    
    void copyFrom(VGAPalette* other);
    
private:

    
    uint32_t colorFlag;
    VGAPalette colors;
    
    uint32_t bwFlag;
    VGAPalette bwColors;
    
    void parsePAL(uint8_t *data, size_t size);
    void parsePAL_CMAP(uint8_t *data, size_t size);

    void parsePALT(uint8_t *data, size_t size);
    void parsePALT_INFO(uint8_t *data, size_t size);
    void parsePALT_PALT(uint8_t *data, size_t size);
    void parsePALT_BLWH(uint8_t *data, size_t size);
    void parsePALT_SKYC(uint8_t *data, size_t size);
    void parsePALT_RANG(uint8_t *data, size_t size);
    void parsePALT_TABL(uint8_t *data, size_t size);
    void parsePALT_BILD(uint8_t *data, size_t size);
    void parsePALT_BILD_INFO(uint8_t *data, size_t size);
    void parsePALT_BILD_TABL(uint8_t *data, size_t size);
};
