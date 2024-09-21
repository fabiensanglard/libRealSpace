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
    void line(int x1, int y1, int x2, int y2, uint8_t color);
    void rect_slow(int left, int top, int right, int bottom, uint8_t color);
    void circle_slow(int x,int y, int radius, uint8_t color);
    void DrawText(RSFont* font, Point2D* coo, char* text, uint8_t color,size_t start, uint32_t size,size_t interLetterSpace, size_t spaceSize);
    void SwithBuffers();
    void VSync(void);
    
    void Clear(void);
    void FillWithColor(uint8_t color);
    inline uint8_t* GetFrameBuffer(void){ return frameBuffer;}
    
    void FillLineColor(size_t lineIndex, uint8_t color);
    void plot_pixel(int x, int y, uint8_t color);
private:
    
    VGAPalette palette;
    uint8_t* frameBuffer;
    uint8_t frameBufferA[320*200];
    uint8_t frameBufferB[320*200];
    
    uint32_t textureID;
};
