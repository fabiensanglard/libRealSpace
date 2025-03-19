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

RSVGA::RSVGA() {
    this->frameBufferA = new FrameBuffer(320,200);
    this->frameBufferB = new FrameBuffer(320,200);
}

RSVGA::~RSVGA() {
    delete this->frameBufferA;
    delete this->frameBufferB;
}

void RSVGA::SwithBuffers() {
    if (this->frameBuffer == this->frameBufferA) {
        this->frameBuffer = this->frameBufferB;
    } else {
        this->frameBuffer = this->frameBufferA;
    }
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
        Texel *rgba = palette.GetRGBColor(frameBuffer->framebuffer[i]);
        if (frameBuffer->framebuffer[i] == 255) {
            rgba->a = 0;
        }
        *dst = *rgba;
        dst++;
    }
    glViewport(67,0,1066,800);			// Reset The Current Viewport
	
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

