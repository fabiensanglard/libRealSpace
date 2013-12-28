//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__gfx__
#define __iff__gfx__

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT  768

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Texel;

typedef struct Palette{
    
  Texel palette[256];

  void SetColor(uint8_t value,Texel* texel){
    
    Texel* paletteColor ;
    
    paletteColor = &palette[value] ;

      *paletteColor = *texel;
  }
    
    const Texel* GetRGBColor(uint8_t value){
        return &palette[value];
    }

} Palette ;

void IMG_Init(void);
void IMG_ShowPalette(Palette* palette,int cellSize);
void IMG_ShowImage(uint8_t* image, uint16_t width, uint16_t height,Palette* palette,int zoom,bool wait);
void IMG_ShowModel(RealSpaceObject* object,Palette* palette );

#endif /* defined(__iff__gfx__) */
