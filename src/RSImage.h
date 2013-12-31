//
//  RSImage.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/31/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSImage__
#define __libRealSpace__RSImage__

class RSPalette;

class RSImage{
    
public:
    RSImage();
    ~RSImage();
    
    void Create(const char name[8],uint32_t width,uint32_t height);
    
    void UpdateContent(uint8_t* data);
    
    void SetPalette(VGAPalette* palette);
    
    Texture* GetTexture(void);
    void SyncTexture(void);
    
    size_t width;
    size_t height;
    char name[512];
    
    uint8_t* data;
    VGAPalette* palette;
    
    
    //Run Length Encoding stuff
    void SetRLECenterCoo(int16_t left,int16_t right,int16_t top,int16_t bottom);
    uint8_t* rleCenter;
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
    bool WriteRLETexel(int16_t dx,int16_t dy, uint8_t color);
    
private:
    
    Texture texture;
    
    
    bool dirty;
};

#endif /* defined(__libRealSpace__RSImage__) */
