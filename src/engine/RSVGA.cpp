//
//  VGA.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSVGA.h"
#include "../realspace/RSImage.h"
#include "../realspace/RSPalette.h"
#include "SDL2/SDL_opengl_glext.h"

void applyEagle2x(uint32_t* src, uint32_t* dst, int width, int height) {
    int dst_width = width * 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t A = (x > 0 && y > 0) ? src[(y-1) * width + (x-1)] : src[y * width + x];
            uint32_t B = (y > 0) ? src[(y-1) * width + x] : src[y * width + x];
            uint32_t C = (x < width-1 && y > 0) ? src[(y-1) * width + (x+1)] : src[y * width + x];
            uint32_t D = (x > 0) ? src[y * width + (x-1)] : src[y * width + x];
            uint32_t E = src[y * width + x];
            uint32_t F = (x < width-1) ? src[y * width + (x+1)] : src[y * width + x];
            uint32_t G = (x > 0 && y < height-1) ? src[(y+1) * width + (x-1)] : src[y * width + x];
            uint32_t H = (y < height-1) ? src[(y+1) * width + x] : src[y * width + x];
            uint32_t I = (x < width-1 && y < height-1) ? src[(y+1) * width + (x+1)] : src[y * width + x];
            
            // Eagle2x algorithm
            dst[(y*2) * dst_width + (x*2)] = (D == B && D != H && B != F) ? B : E;
            dst[(y*2) * dst_width + (x*2+1)] = (B == F && B != D && F != H) ? F : E;
            dst[(y*2+1) * dst_width + (x*2)] = (D == H && D != B && H != F) ? D : E;
            dst[(y*2+1) * dst_width + (x*2+1)] = (F == H && F != B && H != D) ? H : E;
        }
    }
}

RSVGA::RSVGA() {
    this->frameBufferA = new FrameBuffer(320,200);
    this->frameBufferB = new FrameBuffer(320,200);
    this->upscaled_framebuffer = (uint32_t *)calloc(640*400, sizeof(uint32_t));
}

RSVGA::~RSVGA() {
    if (this->frameBufferA != nullptr) {
        this->frameBufferA->Clear();
        delete this->frameBufferA;
    }
    if (this->frameBufferB != nullptr) {
        this->frameBufferB->Clear();
        delete this->frameBufferB;
    }
    if (this->upscaled_framebuffer != nullptr) {
        free(this->upscaled_framebuffer);
    }
}

void RSVGA::SwithBuffers() {
    if (this->frameBuffer == this->frameBufferA) {
        this->frameBuffer = this->frameBufferB;
    } else {
        this->frameBuffer = this->frameBufferA;
    }
}
void RSVGA::init(int width, int height, AssetManager *amana) {
    this->width = width;
    this->height = height;
    this->assets = amana;

    RSPalette palette;
    palette.initFromFileData(this->assets->GetFileData("PALETTE.IFF"));
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
        Texel *rgba = palette.GetRGBColor(frameBuffer->framebuffer[i]);
        if (frameBuffer->framebuffer[i] == 255) {
            rgba->a = 0;
        }
        *dst = *rgba;
        dst++;
    }
    int w = (int) ((float) this->height * (4.0f/3.0f));
    int x = (this->width - w) /2;
    glViewport(x,0,w,this->height);
	
    
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    if (this->upscale)  {
        applyEagle2x((uint32_t *)data, upscaled_framebuffer, 320, 200);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 400, 0, GL_RGBA, GL_UNSIGNED_BYTE, upscaled_framebuffer);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

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

