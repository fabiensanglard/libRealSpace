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

void FillAlphaWithAppropriateColors(size_t width, size_t height, uint8_t* pixels)
{
	std::vector<int> distances(width * height, 10000);

	auto copyPixels = [pixels, width, height] (int fromOffset, int delta) {
		uint8_t* from = pixels + (fromOffset);
		uint8_t* to = pixels + (fromOffset + 4 * delta);
		for (int i = 0; i < 3; ++i) {
			to[i] = from[i];
		}
	};

	for (int y = 0; y < height; ++y) {
		int distance = 0;
		for (int x = 0; x < width; ++x) {
			int bufferOffset = y * (int) width + x;
			int pixelOffset = 4 * bufferOffset;
			bool isTransparent = pixels[pixelOffset + 3] == 0;
			distance = isTransparent ? distance + 1 : 0;
			if (isTransparent && x - distance >= 0) {
				distances[bufferOffset] = distance;
				copyPixels(pixelOffset, -distance);
			}
		}
		distance = 0;
		for (int x = (int) width - 1; x >= 0; --x) {
			int bufferOffset = y * (int) width + x;
			int pixelOffset = 4 * bufferOffset;
			bool isTransparent = pixels[pixelOffset + 3] == 0;
			distance = isTransparent ? distance + 1 : 0;
			if (isTransparent && x + distance < width && distances[bufferOffset] > distance) {
				distances[bufferOffset] = distance;
				copyPixels(pixelOffset, distance);
			}
		}
	}

	for (int x = 0; x < width; ++x) {
		int distance = 0;
		for (int y = 0; y < height; ++y) {
			int bufferOffset = y * int(width) + x;
			int pixelOffset = 4 * bufferOffset;
			bool isTransparent = pixels[pixelOffset + 3] == 0;
			distance = isTransparent ? distance + 1 : 0;
			if (isTransparent && y - distance >= 0 && distances[bufferOffset] > distance) {
				distances[bufferOffset] = distance;
				copyPixels(pixelOffset, -distance * (int)width);
			}
		}
		distance = 0;
		for (int y = int(height) - 1; y >= 0; --y) {
			int bufferOffset = y * int(width) + x;
			int pixelOffset = 4 * bufferOffset;
			bool isTransparent = pixels[pixelOffset + 3] == 0;
			distance = isTransparent ? distance + 1 : 0;
			if (isTransparent && y + distance < height && distances[bufferOffset] > distance) {
				distances[bufferOffset] = distance;
				copyPixels(pixelOffset, distance * int(width));
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

    if (hasAlpha) {
		//FillAlphaWithAppropriateColors(image->width, image->height, data);
	}
}
