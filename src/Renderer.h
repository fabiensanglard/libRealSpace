//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__gfx__
#define __iff__gfx__

class RSEntity;

class Renderer{
    
public:
    
     Renderer();
    ~Renderer();
    
    void Init(int32_t width,int32_t height);
    
    void Clear(void);

    void ShowPalette(VGAPalette* palette);
    void DrawImage(uint8_t* image, uint16_t width, uint16_t height,VGAPalette* palette,int zoom);
    void DrawModel(RSEntity* object,VGAPalette* palette , size_t lodLevel);
    void DisplayModel(RSEntity* object,size_t lodLevel);
    
    void SetTitle(const char* title);
    
    void UploadTextureToVRAM(Texture* texture, VGAPalette* palette);
    void UnloadTextureToVRAM(Texture* texture);
    
    static  VGAPalette* const USE_DEFAULT_PALETTE;
    
private:
    
    void PumpEvents(void);
    bool initialized;
    
    int32_t width;
    int32_t height;
    
    uint8_t* backBuffer;
    
    VGAPalette defaultPalette;
    bool running;
    
    Camera camera;
    vec3_t light;
};


extern Renderer renderer;

/*
void IMG_Init(void);
void IMG_ShowPalette(Palette* palette,int cellSize);
void IMG_ShowImage(uint8_t* image, uint16_t width, uint16_t height,Palette* palette,int zoom,bool wait);
void IMG_ShowModel(RealSpaceObject* object,Palette* palette );
*/
#endif /* defined(__iff__gfx__) */
