//
//  VGA.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSVGA.h"
#include "IffLexer.h"
#include "RSImage.h"
#include "RSPalette.h"
#include "SDL2/SDL_opengl_glext.h"

RSVGA::RSVGA() {}

RSVGA::~RSVGA() {}

void RSVGA::Clear(void) { this->FillWithColor(255); }
void RSVGA::FillWithColor(uint8_t color) { memset(frameBuffer, color, 320 * 200); }
void RSVGA::SwithBuffers() {
    if (this->frameBuffer == this->frameBufferA) {
        this->frameBuffer = this->frameBufferB;
    } else {
        this->frameBuffer = this->frameBufferA;
    }
}
bool RSVGA::DrawShape(RLEShape *shape) {
    if (shape != nullptr) {
        size_t byteRead = 0;
        return shape->Expand(GetFrameBuffer(), &byteRead);
    }
    return (false);
}
bool RSVGA::DrawShapeWithBox(RLEShape *shape, int bx1, int bx2, int by1, int by2) {
    if (shape != nullptr) {
        size_t byteRead = 0;
        return shape->ExpandWithBox(GetFrameBuffer(), &byteRead, bx1, bx2, by1, by2);
    }
    return (false);
}
void RSVGA::Init(void) {

    // Load the default palette
    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");

    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glEnable(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, textureID);
    uint8_t *data = (uint8_t *)calloc(1, 320 * 200 * 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    free(data);
    this->frameBuffer = this->frameBufferA;
}

void RSVGA::Activate(void) {

    glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
    glLoadIdentity();            // Reset The Projection Matrix
    glOrtho(0, 320, 0, 200, -10, 10);
    glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
    glLoadIdentity();

    glColor4f(1, 1, 1, 1);
}

void RSVGA::SetPalette(VGAPalette *newPalette) { this->palette = *newPalette; }

VGAPalette *RSVGA::GetPalette(void) { return &palette; }

void RSVGA::VSync(void) {

    Texel data[320 * 200];

    Texel *dst = data;
    for (size_t i = 0; i < 320 * 200; i++) {
        Texel *rgba = palette.GetRGBColor(frameBuffer[i]);
        if (frameBuffer[i] == 255) {
            rgba->a = 0;
        }
        *dst = *rgba;
        dst++;
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex2d(0, 0);

    glTexCoord2f(1, 1);
    glVertex2d(320, 0);

    glTexCoord2f(1, 0);
    glVertex2d(320, 200);

    glTexCoord2f(0, 0);
    glVertex2d(0, 200);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void RSVGA::FillLineColor(size_t lineIndex, uint8_t color) { memset(frameBuffer + lineIndex * 320, color, 320); }
void RSVGA::plot_pixel(int x, int y, uint8_t color) {
    /*  y*320 = y*256 + y*64 = y*2^8 + y*2^6   */
    if (x < 0 || x >= 320 || y < 0 || y >= 200)
        return;
    frameBuffer[(y << 8) + (y << 6) + x] = color;
}

/**************************************************************************
 *  line                                                                  *
 *    draws a line using Bresenham's line-drawing algorithm, which uses   *
 *    no multiplication or division.                                      *
 **************************************************************************/

void RSVGA::line(int x1, int y1, int x2, int y2, uint8_t color) {
    this->lineWithBoxWithSkip(x1, y1, x2, y2, color, 0, 320, 0, 200, 1);
}

void RSVGA::lineWithSkip(int x1, int y1, int x2, int y2, uint8_t color, int skip) {
    this->lineWithBoxWithSkip(x1, y1, x2, y2, color, 0, 320, 0, 200, skip);
}
void RSVGA::lineWithBox(int x1, int y1, int x2, int y2, uint8_t color, int bx1, int bx2, int by1, int by2) {
    this->lineWithBoxWithSkip(x1, y1, x2, y2, color, bx1, bx2, by1, by2, 1);
}

void RSVGA::lineWithBoxWithSkip(int x1, int y1, int x2, int y2, uint8_t color, int bx1, int bx2, int by1, int by2,
                                int skip) {
    if (x1 > 320 || x2 > 320 || y1 > 200 || y2 > 200)
        return;
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1; /* the horizontal distance of the line */
    dy = y2 - y1; /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    if (px >= bx1 && px <= bx2 && py >= by1 && py <= by2) {
        // frameBuffer[(py << 8) + (py << 6) + px] = color;
    }

    if (dxabs >= dyabs) /* the line is more horizontal than vertical */
    {
        for (i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            if (i % skip == 0) {
                if (px >= bx1 && px <= bx2 && py >= by1 && py <= by2) {
                    plot_pixel(px, py, color);
                }
            }
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
            if (i % skip == 0) {
                if (px >= bx1 && px <= bx2 && py >= by1 && py <= by2) {
                    plot_pixel(px, py, color);
                }
            }
        }
    }
}

/**************************************************************************
 *  rect_slow                                                             *
 *    Draws a rectangle by calling the line function four times.          *
 **************************************************************************/

void RSVGA::rect_slow(int left, int top, int right, int bottom, uint8_t color) {
    line(left, top, right, top, color);
    line(left, top, left, bottom, color);
    line(right, top, right, bottom, color);
    line(left, bottom, right, bottom, color);
}

/**************************************************************************
 *  circle_slow                                                           *
 *    Draws a circle by using floating point numbers and math fuctions.   *
 **************************************************************************/

void RSVGA::circle_slow(int x, int y, int radius, uint8_t color) {
    float n = 0, invradius = 1 / (float)radius;
    int dx = 0, dy = radius - 1;
    uint16_t dxoffset, dyoffset, offset = (y << 8) + (y << 6) + x;

    while (dx <= dy) {
        dxoffset = (dx << 8) + (dx << 6);
        dyoffset = (dy << 8) + (dy << 6);
        frameBuffer[offset + dy - dxoffset] = color;
        frameBuffer[offset + dx - dyoffset] = color;
        frameBuffer[offset - dx - dyoffset] = color;
        frameBuffer[offset - dy - dxoffset] = color;
        frameBuffer[offset - dy + dxoffset] = color;
        frameBuffer[offset - dx + dyoffset] = color;
        frameBuffer[offset + dx + dyoffset] = color;
        frameBuffer[offset + dy + dxoffset] = color;
        dx++;
        n += invradius;
        dy = radius * sin(acos(n));
    }
}
void RSVGA::PrintText(RSFont *font, Point2D *coo, char *text, uint8_t color, size_t start, uint32_t size,
                      size_t interLetterSpace, size_t spaceSize) 
    {
        this->PrintText_SM(font, coo, text, color, start, size, interLetterSpace, spaceSize, true);
}

void RSVGA::PrintText_SM(RSFont *font, Point2D *coo, char *text, uint8_t color, size_t start, uint32_t size,
                      size_t interLetterSpace, size_t spaceSize, bool isSmall) {

    if (text == NULL)
        return;

    if (size <= 0)
        return;
    int startx = coo->x;
    if (coo->y < 0)
        return;
    if (coo->y > 200)
        return;
    if (coo->x < 0)
        return;
    if (coo->x > 320)
        return;
    for (size_t i = 0; i < size; i++) {

        char chartoDraw = text[start + i];

        RLEShape *shape = font->GetShapeForChar(chartoDraw);
        if (shape == nullptr) {
            continue;
        }
        shape->SetColorOffset(color);
        // Adjust height
        int32_t lineHeight = coo->y;
        coo->y -= shape->GetHeight();

        if (isSmall && (chartoDraw == 'p' || chartoDraw == 'y' || chartoDraw == 'g' || chartoDraw == 'q' || chartoDraw == 'j'))
            coo->y += 1;
        if (chartoDraw == '\n') {
            RLEShape *sp = font->GetShapeForChar('A');
            coo->y += sp->GetHeight() + 2;
            lineHeight = coo->y;
            coo->x = startx;
        }
        if (coo->y < 0)
            continue;
        if (coo->y > 200)
            continue;
        if (coo->x < 0)
            continue;
        if (coo->x+static_cast<int32_t>(spaceSize) > 320)
            continue;
        shape->SetPosition(coo);
        DrawShape(shape);
        coo->y = lineHeight;

        if (chartoDraw == ' ')
            coo->x += static_cast<int32_t>(spaceSize);
        else
            coo->x = static_cast<int32_t>(coo->x + shape->GetWidth() + interLetterSpace);
    }
}
void RSVGA::blit(uint8_t* srcBuffer, int x, int y, int width, int height) {
    for (int row = 0; row < height; ++row) {
        int destRow = y + row;
        if (destRow < 0 || destRow >= 200)
            continue;
        int destOffset = destRow * 320;
        for (int col = 0; col < width; ++col) {
            int destCol = x + col;
            if (destCol < 0 || destCol >= 320)
                continue;
            frameBuffer[destOffset + destCol] = srcBuffer[row * width + col];
        }
    }
}
void RSVGA::blitLargeBuffer(uint8_t* srcBuffer, int srcWidth, int srcHeight, int srcX, int srcY, int destX, int destY, int width, int height) {

    for (int row = 0; row < height; ++row) {
        int srcRow = srcY + row;
        int destRow = destY + row;
        if (srcRow < 0 || srcRow >= srcHeight)
            continue;
        if (destRow < 0 || destRow >= 200)
            continue;
        int destOffset = destRow * 320;
        for (int col = 0; col < width; ++col) {
            int srcCol = srcX + col;
            int destCol = destX + col;
            if (srcCol < 0 || srcCol >= srcWidth)
                continue;
            if (destCol < 0 || destCol >= 320)
                continue;
            if (srcBuffer[srcRow * srcWidth + srcCol] == 255)
                continue;
            frameBuffer[destOffset + destCol] = srcBuffer[srcRow * srcWidth + srcCol];
        }
    }
}