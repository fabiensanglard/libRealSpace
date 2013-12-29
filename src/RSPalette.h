//
//  RealSpacePaletteObject.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__RealSpacePaletteObject__
#define __iff__RealSpacePaletteObject__



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

class RSPalette{
    
public:
    
    RSPalette();
    ~RSPalette();
    
    void InitFromIFF(IffLexer* lexer);
    
    
    
    
    
    Palette* GetColorPalette(void);
    void SetColorFlag(uint32_t flag);
    
    Palette* GetBWPalette(void);
    void SetBWFlag(uint32_t flag);
    
    
private:
    
    void ParsePALT(IffChunk* chunk);
    void ParseBLWH(IffChunk* chunk);
    void ParseCMAP(IffChunk* chunk);
    
    uint32_t colorFlag;
    Palette colors;
    
    uint32_t bwFlag;
    Palette bwColors;
};

#endif /* defined(__iff__RealSpacePaletteObject__) */
