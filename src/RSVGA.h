//
//  VGA.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <Windows.h>
#include <GL/GL.h>
#include "RLEShape.h"
#include "RSFont.h"
#include "Texture.h"

#define sgn(x) ((x<0)?-1:((x>0)?1:0))

struct VGAPalette;

class RSVGA{
    
public:
    
    RSVGA();
    ~RSVGA();
    
    
    void Init(void);
    
    void Activate(void);
    void SetPalette(VGAPalette* newPalette);
    VGAPalette* GetPalette(void);
    
    bool DrawShape(RLEShape* shape);
    void line(int x1, int y1, int x2, int y2, byte color);
    void rect_slow(int left, int top, int right, int bottom, byte color);
    void DrawText(RSFont* font, Point2D* coo, char* text, uint8_t color,size_t start, uint32_t size,size_t interLetterSpace, size_t spaceSize);
    
    void VSync(void);
    
    void Clear(void);
    
    inline uint8_t* GetFrameBuffer(void){ return frameBuffer;}
    
    void FillLineColor(size_t lineIndex, uint8_t color);
    void plot_pixel(int x, int y, byte color);
private:
    
    VGAPalette palette;
    uint8_t frameBuffer[320*200];
    
    uint32_t textureID;
};
