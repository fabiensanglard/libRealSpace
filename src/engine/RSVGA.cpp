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
// Ajouter après la fonction applyEagle2x existante
extern RSScreen *Screen;
// Fonction auxiliaire pour comparer deux couleurs (utilisée par SuperEagle)
bool colorEqual(uint32_t c1, uint32_t c2, int threshold = 0) {
    if (c1 == c2) return true;

    uint8_t r1 = (c1 >> 24) & 0xFF;
    uint8_t g1 = (c1 >> 16) & 0xFF;
    uint8_t b1 = (c1 >> 8) & 0xFF;
    uint8_t a1 = c1 & 0xFF;
    
    uint8_t r2 = (c2 >> 24) & 0xFF;
    uint8_t g2 = (c2 >> 16) & 0xFF;
    uint8_t b2 = (c2 >> 8) & 0xFF;
    uint8_t a2 = c2 & 0xFF;
    
    int dr = abs(r1 - r2);
    int dg = abs(g1 - g2);
    int db = abs(b1 - b2);
    
    return (dr <= threshold && dg <= threshold && db <= threshold && a1 == a2);
}

// Fonction pour mélanger deux couleurs
uint32_t colorBlend(uint32_t c1, uint32_t c2) {
    uint8_t r1 = (c1 >> 24) & 0xFF;
    uint8_t g1 = (c1 >> 16) & 0xFF;
    uint8_t b1 = (c1 >> 8) & 0xFF;
    uint8_t a1 = c1 & 0xFF;
    
    uint8_t r2 = (c2 >> 24) & 0xFF;
    uint8_t g2 = (c2 >> 16) & 0xFF;
    uint8_t b2 = (c2 >> 8) & 0xFF;
    uint8_t a2 = c2 & 0xFF;
    
    uint8_t r = (r1 + r2) / 2;
    uint8_t g = (g1 + g2) / 2;
    uint8_t b = (b1 + b2) / 2;
    uint8_t a = (a1 + a2) / 2;
    
    return (r << 24) | (g << 16) | (b << 8) | a;
}

// SuperEagle2x - Version améliorée de Eagle2x avec détection plus précise des contours
void applySuperEagle2x(uint32_t* src, uint32_t* dst, int width, int height) {
    int dst_width = width * 2;
    const int threshold = 30; // Seuil de tolérance pour la comparaison de couleurs
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Récupération des 9 pixels de la grille 3x3 autour du pixel courant
            uint32_t A = (x > 0 && y > 0) ? src[(y-1) * width + (x-1)] : src[y * width + x];
            uint32_t B = (y > 0) ? src[(y-1) * width + x] : src[y * width + x];
            uint32_t C = (x < width-1 && y > 0) ? src[(y-1) * width + (x+1)] : src[y * width + x];
            uint32_t D = (x > 0) ? src[y * width + (x-1)] : src[y * width + x];
            uint32_t E = src[y * width + x];  // Pixel central
            uint32_t F = (x < width-1) ? src[y * width + (x+1)] : src[y * width + x];
            uint32_t G = (x > 0 && y < height-1) ? src[(y+1) * width + (x-1)] : src[y * width + x];
            uint32_t H = (y < height-1) ? src[(y+1) * width + x] : src[y * width + x];
            uint32_t I = (x < width-1 && y < height-1) ? src[(y+1) * width + (x+1)] : src[y * width + x];
            
            // Position dans le buffer de destination
            uint32_t e0, e1, e2, e3; // Les 4 pixels de destination (2x2)
            
            // Règles SuperEagle pour le pixel supérieur gauche (e0)
            if (colorEqual(B, D, threshold) && !colorEqual(B, F, threshold) && !colorEqual(D, H, threshold)) {
                e0 = colorBlend(B, D);  // Mélange diagonal
            } else if (colorEqual(D, B, threshold) && !colorEqual(D, H, threshold) && !colorEqual(B, F, threshold)) {
                e0 = colorBlend(D, B);  // Mélange diagonal
            } else {
                e0 = E;  // Conserver le pixel central
            }
            
            // Règles pour le pixel supérieur droit (e1)
            if (colorEqual(B, F, threshold) && !colorEqual(B, D, threshold) && !colorEqual(F, H, threshold)) {
                e1 = colorBlend(B, F);  // Mélange diagonal
            } else if (colorEqual(F, B, threshold) && !colorEqual(F, H, threshold) && !colorEqual(B, D, threshold)) {
                e1 = colorBlend(F, B);  // Mélange diagonal
            } else {
                e1 = E;  // Conserver le pixel central
            }
            
            // Règles pour le pixel inférieur gauche (e2)
            if (colorEqual(D, H, threshold) && !colorEqual(D, B, threshold) && !colorEqual(H, F, threshold)) {
                e2 = colorBlend(D, H);  // Mélange diagonal
            } else if (colorEqual(H, D, threshold) && !colorEqual(H, F, threshold) && !colorEqual(D, B, threshold)) {
                e2 = colorBlend(H, D);  // Mélange diagonal
            } else {
                e2 = E;  // Conserver le pixel central
            }
            
            // Règles pour le pixel inférieur droit (e3)
            if (colorEqual(F, H, threshold) && !colorEqual(F, B, threshold) && !colorEqual(H, D, threshold)) {
                e3 = colorBlend(F, H);  // Mélange diagonal
            } else if (colorEqual(H, F, threshold) && !colorEqual(H, D, threshold) && !colorEqual(F, B, threshold)) {
                e3 = colorBlend(H, F);  // Mélange diagonal
            } else {
                e3 = E;  // Conserver le pixel central
            }
            
            // Détection de coins supplémentaires avec les pixels diagonaux A, C, G, I
            if (colorEqual(E, A, threshold) && colorEqual(E, D, threshold) && colorEqual(E, B, threshold) && 
                !colorEqual(E, C, threshold) && !colorEqual(E, G, threshold)) {
                e0 = colorBlend(E, A);
            }
            
            if (colorEqual(E, C, threshold) && colorEqual(E, B, threshold) && colorEqual(E, F, threshold) && 
                !colorEqual(E, A, threshold) && !colorEqual(E, I, threshold)) {
                e1 = colorBlend(E, C);
            }
            
            if (colorEqual(E, G, threshold) && colorEqual(E, D, threshold) && colorEqual(E, H, threshold) && 
                !colorEqual(E, A, threshold) && !colorEqual(E, I, threshold)) {
                e2 = colorBlend(E, G);
            }
            
            if (colorEqual(E, I, threshold) && colorEqual(E, F, threshold) && colorEqual(E, H, threshold) && 
                !colorEqual(E, C, threshold) && !colorEqual(E, G, threshold)) {
                e3 = colorBlend(E, I);
            }
            
            // Écriture des 4 pixels dans le buffer de destination
            dst[(y*2) * dst_width + (x*2)] = e0;
            dst[(y*2) * dst_width + (x*2+1)] = e1;
            dst[(y*2+1) * dst_width + (x*2)] = e2;
            dst[(y*2+1) * dst_width + (x*2+1)] = e3;
        }
    }
}
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


void RSVGA::fadeOut(int steps, int delayMs) {
    // Sauvegarde de l'état actuel de l'écran
    Texel originalData[320 * 200];
    memcpy(originalData, data, sizeof(data));
    
    // Facteur de réduction de luminosité pour chaque étape
    float fadeStep = 1.0f / steps;
    
    for (int step = 1; step <= steps; step++) {
        // Facteur d'assombrissement pour cette étape (de 1.0 à 0.0)
        float fadeFactor = 1.0f - (step * fadeStep);
        
        // Appliquer le facteur d'assombrissement à chaque pixel
        for (int i = 0; i < 320 * 200; i++) {
            // Préserver la transparence originale
            uint8_t alpha = originalData[i].a;
            
            // Assombrir progressivement les composantes RGB
            data[i].r = static_cast<uint8_t>(originalData[i].r * fadeFactor);
            data[i].g = static_cast<uint8_t>(originalData[i].g * fadeFactor);
            data[i].b = static_cast<uint8_t>(originalData[i].b * fadeFactor);
            data[i].a = alpha; // Conserver l'alpha d'origine
        }
        
        displayBuffer((uint32_t *)data, 320, 200);
        Screen->refresh();
        // Attente entre les étapes
        #ifdef _WIN32
        Sleep(delayMs);
        #else
        usleep(delayMs * 1000);
        #endif
    }
    
    // S'assurer que l'écran est complètement noir à la fin
    for (int i = 0; i < 320 * 200; i++) {
        data[i].r = 0;
        data[i].g = 0;
        data[i].b = 0;
        // Conserver l'alpha d'origine
        data[i].a = originalData[i].a;
    }
    displayBuffer((uint32_t *)data, 320, 200);
}

void RSVGA::displayBuffer(uint32_t *buffer, int width, int height) {
    int w = (int) ((float) this->height * (4.0f/3.0f));
    int x = (this->width - w) /2;
    glViewport(x,0,w,this->height);
	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    if (this->upscale)  {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
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

void RSVGA::VSync(void) {

    Texel *dst = data;
    for (size_t i = 0; i < 320 * 200; i++) {
        Texel *rgba = palette.GetRGBColor(frameBuffer->framebuffer[i]);
        if (frameBuffer->framebuffer[i] == 255) {
            rgba->a = 0;
        }
        *dst = *rgba;
        dst++;
    }
    if (this->upscale) {
        applySuperEagle2x((uint32_t *)data, upscaled_framebuffer, 320, 200);
        displayBuffer(upscaled_framebuffer, 640, 400);
    } else {
        displayBuffer((uint32_t *)data, 320, 200);
    }
}

