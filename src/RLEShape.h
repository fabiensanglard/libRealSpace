//
//  rle.h
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <stdint.h>

#include "Base.h"
#include "ByteStream.h"
#include "Maths.h"

class RLEShape{

    
public:
    
     RLEShape();
    ~RLEShape();
    
    void Init(uint8_t* data, size_t size);
    void InitWithPosition(uint8_t* data, size_t size,Point2D* position );
    
    bool Expand(uint8_t* dst, size_t* byteRead);
    
    inline void SetPosition(Point2D* position){
        this->position = *position;
    }
    
    inline void SetPositionX(int32_t x){
        this->position.x = x;
    }

    
    static RLEShape* GetEmptyShape(void);
    
    int32_t GetWidth(void){ return leftDist + this->rightDist;}
    int32_t GetHeight(void){ return topDist+botDist;}
    int32_t GetTop(void){ return topDist;}
    int32_t GetBottom(void){ return botDist;}
    
    
    
    void SetColorOffset(uint8_t offset){ this->colorOffset = offset;}
    Point2D position;
private:
    
    ByteStream stream;
    size_t size;
    
    
    uint8_t* data;
    
    enum FragmentType {FRAG_END,FRAG_COMPOSITE,FRAG_RAW} ;
    enum FragmentSubType {SUB_FRAG_RAW =0x0, SUB_FRAG_COMPRESSED=0x1} ;
    
    typedef struct RLEFragment{
        FragmentType type;
        bool isCompressed;
        uint16_t numTexels;
        
        int16_t dx;
        int16_t dy;
    } RLEFragment;
    
    
    void ReadFragment  (RLEFragment* frag);
    bool ExpandFragment(RLEFragment* frag, uint8_t* dst );
    
    int16_t leftDist;
    int16_t topDist;
    int16_t rightDist;
    int16_t botDist;
    
    uint8_t colorOffset;
    bool WriteColor(uint8_t* dst,int16_t dx, int16_t dy, uint8_t color);
};

