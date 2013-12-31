//
//  Texture.h
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__Texture__
#define __iff__Texture__

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Texel;

typedef struct VGAPalette{
    
    Texel palette[256];
    
    void SetColor(uint8_t value,Texel* texel){
        
        Texel* paletteColor ;
        
        paletteColor = &palette[value] ;
        
        *paletteColor = *texel;
    }
    
    const Texel* GetRGBColor(uint8_t value){
        return &palette[value];
    }
    
} VGAPalette ;


class RSImage;

class Texture{
    
public:
    Texture();
    ~Texture();
    
    void Set(RSImage* image );
    size_t width;
    size_t height;
    char name[8];
    uint8_t* data;


    
    enum Location{ DISK=0x1,RAM=0x2,VRAM=0x4};
    uint8_t locFlag;
    
    //GPU stuff
    uint32_t id;
    uint32_t GetTextureID(void);
    

    
    void UpdateContent(RSImage* image);

private:
    
};

#endif /* defined(__iff__Texture__) */
