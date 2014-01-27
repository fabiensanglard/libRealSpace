//
//  rle.h
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __pak__rle__
#define __pak__rle__

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
    
private:
    
    ByteStream stream;
    size_t size;
    Point2D position;
    
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
    
    bool WriteColor(uint8_t* dst,int16_t dx, int16_t dy, uint8_t color);
};






#endif /* defined(__pak__rle__) */
