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
private:
    enum FragmentType {FRAG_END,FRAG_COMPOSITE,FRAG_RAW} ;
    enum FragmentSubType {SUB_FRAG_RAW =0x0, SUB_FRAG_COMPRESSED=0x1} ;

    typedef struct RLEFragment{
        FragmentType type;
        bool isCompressed;
        uint16_t numTexels;
        
        int16_t dx;
        int16_t dy;
    } RLEFragment;

    ByteStream stream;
    size_t size;
    
    int16_t leftDist;
    int16_t topDist;
    int16_t rightDist;
    int16_t botDist;
    uint8_t colorOffset;
    
    
    void ReadFragment  (RLEFragment* frag);
    bool ExpandFragment(RLEFragment* frag, uint8_t* dst );
    bool ExpandFragmentWithBox(RLEFragment* frag, uint8_t* dst, int bx1, int bx2, int by1, int by2 );
    bool WriteColor(uint8_t* dst,int16_t dx, int16_t dy, uint8_t color);
    bool WriteColorWithBox(uint8_t* dst,int16_t dx, int16_t dy, uint8_t color, int bx1, int bx2, int by1, int by2);
    
public:
    Point2D position;
    Point2D buffer_size{320,200};
    uint8_t* data;
     RLEShape();
    ~RLEShape();
    
    void Init(uint8_t* data, size_t size);
    void InitWithPosition(uint8_t* data, size_t size,Point2D* position );
    
    bool Expand(uint8_t* dst, size_t* byteRead);
    bool ExpandWithBox(uint8_t* dst, size_t* byteRead, int bx1, int bx2, int by1, int by2);
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
    int32_t GetLeft(void) { return leftDist;}
    int32_t GetBottom(void){ return botDist;}
    void SetColorOffset(uint8_t offset){ this->colorOffset = offset;}

};

