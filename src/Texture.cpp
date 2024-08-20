//
//  Texture.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Texture.h"
#include "RSImage.h"
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
        Renderer.DeleteTextureInGPU(this);
    if (data)
        free(data);
}

void Texture::Set(RSImage* image){
    
    strncpy_s(this->name,image->name,8);
    this->width = image->width;
    this->height = image->height;
    this->data = (uint8_t*)malloc(width*height*4);
    locFlag = RAM;
    
    //UpdateContent(image);
}

uint32_t Texture::GetTextureID(void){
    
    
    return id;
    
}

void FillAlphaWithAppropriateColors(size_t w, size_t h, uint8_t* data)
{
	std::vector<int> dist(w * h, 10000);

	auto copy = [data, w, h] (int fo, int delta) {
		uint8_t* dst = data + (fo);
		uint8_t* src = data + (fo + 4 * delta);
		for (int i = 0; i < 3; ++i) {
			assert(fo + i >= 0 && fo + i < w * h * 4);
			dst[i] = src[i];
		}
	};

	for(int y = 0 ; y < h; ++y) {
		int d = 0;
		for(int x = 0 ; x < w; ++x) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && x - d >= 0) {
				dist[bo] = d;
				copy(fo, -d);
			}
		}
		d = 0;
		for(int x = w - 1; x >= 0; --x) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && x + d < w && dist[bo] > d) {
				dist[bo] = d;
				copy(fo, d);
			}
		}
	}

	for(int x = 0 ; x < w; ++x) {
		int d = 0;
		for(int y = 0 ; y < h; ++y) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && y - d >= 0 && dist[bo] > d) {
				dist[bo] = d;
				copy(fo, -d * w);
			}
		}
		d = 0;
		for(int y = h - 1; y >= 0; --y) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && y + d < h && dist[bo] > d) {
				dist[bo] = d;
				copy(fo, d * w);
			}
		}
	}
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
            dst[3] = src->a;

            if (src->r == 0 && src->g == 0 && src->b == 0) {
                dst[3] = 0;
            }
				
			// force alpha on delimiters...
			if (image->width == 64 && image->height == 64 && src->r == 174 && src->g == 28 && src->b == 0) {
                dst[3] = 0;
            }

			const bool alpha = dst[3] == 0;
			hasAlpha = hasAlpha || alpha;

            dst+=4;
        }
    }

    /*if (hasAlpha)
		FillAlphaWithAppropriateColors(image->width, image->height, data);
	*/
	if ((image->flags & IMAGE_FLAG_COPY_PALINDEX_TO_ALPHA) != 0) {
		dst = this->data;
		for(int i = 0 ; i < image->height; i++) {
			for(int j = 0 ; j < image->width; j++) {
				const uint8_t* psrcIndex = src + j + i * image->width;
				const uint8_t srcIndex = *psrcIndex;
				const bool alpha = dst[3] == 0;
				if (alpha)
					dst[3] = srcIndex;
				dst+=4;
			}
		}
	}
}
