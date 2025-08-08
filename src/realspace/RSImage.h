//
//  RSImage.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/31/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <stdint.h>
#include "../commons/Maths.h"
#include "../engine/Texture.h"


class RSPalette;
struct VGAPalette;

enum IMAGE_FLAG
{
	IMAGE_FLAG_COPY_PALINDEX_TO_ALPHA = 1,
};

class RSImage{
    
public:

    size_t width;
    size_t height;
    uint16_t nbframes{1};
    char name[512];
    
    uint8_t* data;
    VGAPalette* palette;
    uint32_t flags;

    RSImage();
    ~RSImage();
    
    void Create(const char name[8],uint32_t width,uint32_t height, uint32_t flags);
    void UpdateContent(uint8_t* data);
    void SetPalette(VGAPalette* palette);
    void ClearContent(void);
    void GetNextFrame();
    Texture* GetTexture(void);
    void SyncTexture(void);
    uint8_t* GetData(void);
    
    
private:
    uint8_t *sub_frame_buffer{nullptr};
    uint8_t current_frame{0};
    Texture texture;
    bool dirty;
};
