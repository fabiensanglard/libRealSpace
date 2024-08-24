//
//  VGA.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSVGA.h"
#include "RSImage.h"
#include "IffLexer.h"
#include "RSPalette.h"



RSVGA::RSVGA(){
    
}

RSVGA::~RSVGA(){
    
}

void RSVGA::Clear(void){
    memset(frameBuffer, 0, 320*200);
    //glClear(GL_COLOR_BUFFER_BIT);
}

bool RSVGA::DrawShape(RLEShape* shape){
    if (shape != nullptr) {
        size_t byteRead=0;
        return shape->Expand(GetFrameBuffer(), &byteRead);
    }
    return (false);
}

void RSVGA::Init(void){
    
    
    //Load the default palette
    IffLexer lexer ;
    lexer.InitFromFile("PALETTE.IFF");
    //lexer.List(stdout);
    
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();
    
    
    
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glEnable(GL_TEXTURE_2D);
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    uint8_t* data = (uint8_t* )calloc(1, 320*200*4);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    free(data);
}

void RSVGA::Activate(void){
    
    
    glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
    glOrtho(0, 320, 0, 200, -10, 10) ;
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();
    

    glColor4f(1, 1, 1,1);
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

void RSVGA::SetPalette(VGAPalette* newPalette){
    this->palette = *newPalette;
}

VGAPalette* RSVGA::GetPalette(void){
    return &palette;
}

void RSVGA::VSync(void){
    
    Texel data[320*200];
    
    Texel* dst = data;
    for (size_t i = 0; i < 320*200; i++) {
        Texel* rgba = palette.GetRGBColor(frameBuffer[i]);
        *dst = *rgba;
        dst++;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    //Transform the frameBuffer into something OpenGL can understand and paint it.
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex2d(0,0);
    
    glTexCoord2f(1, 1);
    glVertex2d(320,0);
    
    glTexCoord2f(1, 0);
    glVertex2d(320, 200);
    
    glTexCoord2f(0, 0);
    glVertex2d(0,200);
    glEnd();
    
}

void RSVGA::FillLineColor(size_t lineIndex, uint8_t color){
    memset(frameBuffer+lineIndex*320, color, 320);
}
void RSVGA::plot_pixel(int x, int y, byte color) {
    /*  y*320 = y*256 + y*64 = y*2^8 + y*2^6   */
    frameBuffer[(y << 8) + (y << 6) + x] = color;
}

/**************************************************************************
 *  line                                                                  *
 *    draws a line using Bresenham's line-drawing algorithm, which uses   *
 *    no multiplication or division.                                      *
 **************************************************************************/

void RSVGA::line(int x1, int y1, int x2, int y2, byte color) {
    if (x1 > 320 || x2 > 320 || y1 > 200 || y2 > 200)
        return;
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1;      /* the horizontal distance of the line */
    dy = y2 - y1;      /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    frameBuffer[(py << 8) + (py << 6) + px] = color;

    if (dxabs >= dyabs) /* the line is more horizontal than vertical */
    {
        for (i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            plot_pixel(px, py, color);
        }
    } else /* the line is more vertical than horizontal */
    {
        for (i = 0; i < dyabs; i++) {
            x += dxabs;
            if (x >= dyabs) {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            plot_pixel(px, py, color);
        }
    }
}

/**************************************************************************
 *  rect_slow                                                             *
 *    Draws a rectangle by calling the line function four times.          *
 **************************************************************************/

void RSVGA::rect_slow(int left, int top, int right, int bottom, byte color) {
    line(left, top, right, top, color);
    line(left, top, left, bottom, color);
    line(right, top, right, bottom, color);
    line(left, bottom, right, bottom, color);
}

void RSVGA::DrawText(RSFont* font, Point2D* coo, char* text, uint8_t color,size_t start, uint32_t size,size_t interLetterSpace, size_t spaceSize){
    
    
    if (text == NULL)
        return;
    
    if (size <= 0)
        return;
    
    
    for (size_t i =0; i < size; i++) {
        
        char chartoDraw = text[start+i];
        
        RLEShape* shape = font->GetShapeForChar(chartoDraw);

        shape->SetColorOffset(color);
        //Adjust height
        int32_t lineHeight = coo->y;
        coo->y -= shape->GetHeight();
        
        if (chartoDraw== 'p' ||
            chartoDraw== 'y' ||
            chartoDraw== 'g' )
            coo->y += 1;
        
        shape->SetPosition(coo);
        DrawShape(shape);
        coo->y = lineHeight;
        
        if (chartoDraw == ' ')
            coo->x += static_cast<int32_t>(spaceSize);
        else
            coo->x = static_cast<int32_t>(coo->x + shape->GetWidth() + interLetterSpace);
        

    }
}


