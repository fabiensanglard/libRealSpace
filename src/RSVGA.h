//
//  VGA.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__VGA__
#define __libRealSpace__VGA__

class RSVGA{
    
public:
    
    RSVGA();
    ~RSVGA();
    
    
    void Init(void);
    
    void Activate(void);
    void SetPalette(VGAPalette* newPalette);
    VGAPalette* GetPalette(void);
    
    bool DrawShape(RLEShape* shape);
    void DrawText (RSFont* font, Point2D* coo, char* text, uint8_t color);
    
    void VSync(void);
    
    void Clear(void);
    
    inline uint8_t* GetFrameBuffer(void){ return frameBuffer;}
    
private:
    
    VGAPalette palette;
    uint8_t frameBuffer[320*200];
    
    uint32_t textureID;
};

#endif /* defined(__libRealSpace__VGA__) */
