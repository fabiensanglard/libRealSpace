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


class Texture{
    
public:
    Texture();
    ~Texture();
    
    void Set(const char name[8],uint32_t width,uint32_t height,uint8_t* data );
    void CreateEmpty(const char name[8],uint32_t width,uint32_t height);

    void Clear(void);
    int32_t width;
    int32_t height;
    char name[8];
    uint8_t* data;
    
    //GPU stuff
    enum Location {DISK = 1, RAM = 2, VRAM = 4};
    uint32_t locFlag;
    uint32_t id;
    uint32_t GetTextureID(void);
    
    //Run Length Encoding stuff
    void SetRLECenterCoo(int16_t left,int16_t right,int16_t top,int16_t bottom);
    uint8_t* rleCenter;
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
    bool WriteRLETexel(int16_t dx,int16_t dy, uint8_t color);
};

#endif /* defined(__iff__Texture__) */
