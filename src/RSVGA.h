//
//  VGA.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>

#include "RLEShape.h"
#include "RSFont.h"
#include "Texture.h"
#include "FrameBuffer.h"

#define sgn(x) ((x<0)?-1:((x>0)?1:0))

struct VGAPalette;

class RSVGA{
    
public:
    
    RSVGA();
    ~RSVGA();
    
    
    void init(int width, int height, AssetManager* amana);
    
    void Activate(void);
    void SetPalette(VGAPalette* newPalette);
    VGAPalette* GetPalette(void);
    void SwithBuffers();
    void VSync(void);
    FrameBuffer* GetFrameBuffer(void){ return frameBuffer;};
private:
    int width;
    int height;
    AssetManager* assets;
    VGAPalette palette;
    FrameBuffer* frameBuffer;
    FrameBuffer* frameBufferA;
    FrameBuffer* frameBufferB;
    
    uint32_t textureID;
};
