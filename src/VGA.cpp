//
//  VGA.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"




RSVGA::RSVGA(){
    
}

RSVGA::~RSVGA(){
    
}

void RSVGA::Clear(void){
    memset(frameBuffer, 0, 320*200);
    //glClear(GL_COLOR_BUFFER_BIT);
}

bool RSVGA::DrawShape(RLEShape* shape){
    
    
    size_t byteRead;
    return shape->Expand(GetFrameBuffer(), &byteRead);
    
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
    
    uint8_t data[320*200*4];
    
    Texel* dst = (Texel*)data;
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


