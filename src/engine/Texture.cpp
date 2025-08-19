//
//  Texture.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Texture.h"
#include "../realspace/RSImage.h"
#include "SCRenderer.h"
#include <cassert>

extern SCRenderer Renderer;

Texture::Texture()
 : data(0),
  locFlag(DISK)

{
    
}

Texture::~Texture(){
    
    if (id)
        Renderer.deleteTextureInGPU(this);
    if (data)
        free(data);
}

void Texture::Set(RSImage* image){
    
    strncpy(this->name,image->name,8);
    this->width = image->width;
    this->height = image->height;
    this->data = (uint8_t*)malloc(width*height*4);
    locFlag = RAM;
}

uint32_t Texture::GetTextureID(void){
    
    
    return id;
    
}
// Anti-halo: propage la couleur des voisins opaques dans les pixels totalement transparents
static void AlphaBleedRGBA8(uint8_t* data, int w, int h, int iterations = 8) {
    if (!data || w<=0 || h<=0) return;
    std::vector<uint8_t> src(data, data + (size_t)w*h*4);
    std::vector<uint8_t> dst = src;
    auto idx = [w](int x, int y){ return (y*w + x)*4; };

    for (int it=0; it<iterations; ++it) {
        dst = src;
        for (int y=0; y<h; ++y) {
            for (int x=0; x<w; ++x) {
                int i = idx(x,y);
                if (src[i+3] != 0) continue; // ne traite que les pixels totalement transparents
                int count=0; int r=0,g=0,b=0;
                const int nx[8]={-1,0,1,-1,1,-1,0,1};
                const int ny[8]={-1,-1,-1,0,0,1,1,1};
                for (int k=0;k<8;++k){
                    int xx = x+nx[k], yy = y+ny[k];
                    if (xx<0||yy<0||xx>=w||yy>=h) continue;
                    int j = idx(xx,yy);
                    if (src[j+3]==0) continue; // voisin transparent: ignore
                    r += src[j+0]; g += src[j+1]; b += src[j+2];
                    ++count;
                }
                if (count>0) {
                    dst[i+0] = (uint8_t)(r / count);
                    dst[i+1] = (uint8_t)(g / count);
                    dst[i+2] = (uint8_t)(b / count);
                    // alpha reste 0
                }
            }
        }
        src.swap(dst);
    }
    std::memcpy(data, src.data(), (size_t)w*h*4);
}

void Texture::UpdateContent(RSImage* image){
    
    uint8_t* src = image->data;
    uint8_t* dst = this->data;
    VGAPalette* palette = image->palette;
    bool hasAlpha = false;

    for(int i=0 ; i < image->height ; i++){
        for(int j=0 ; j < image->width  ; j++){
            
            uint8_t* srcIndex = src + j + i* image->width;
            const Texel* src = palette->GetRGBColor( (*srcIndex) );
            
            dst[0] = src->r;
            dst[1] = src->g;
            dst[2] = src->b;
			if (src->a == 0) {
                hasAlpha = true;
            }
            dst[3] = src->a;

            dst+=4;
        }
    }
    
    // Appliquer l’anti-halo une seule fois si la texture a de l’alpha
    if (hasAlpha && !this->animated) {
        AlphaBleedRGBA8(this->data, (int)this->width, (int)this->height, 8);
    }
}
