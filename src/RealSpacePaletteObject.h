//
//  RealSpacePaletteObject.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__RealSpacePaletteObject__
#define __iff__RealSpacePaletteObject__

#include "stdint.h"
#include "gfx.h"

class RealSpacePaletteObject{
    
public:
    
    RealSpacePaletteObject();
    ~RealSpacePaletteObject();
    
    Palette* GetColorPalette(void);
    void SetColorFlag(uint32_t flag);
    
    Palette* GetBWPalette(void);
    void SetBWFlag(uint32_t flag);
    
    
private:
    
    int32_t colorFlag;
    Palette colors;
    
    int32_t bwFlag;
    Palette bwColors;
};

#endif /* defined(__iff__RealSpacePaletteObject__) */
