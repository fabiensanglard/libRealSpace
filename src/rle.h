//
//  rle.h
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __pak__rle__
#define __pak__rle__



enum FragmentType {FRAG_END,FRAG_COMPOSITE,FRAG_RAW} ;

enum FragmentSubType {SUB_FRAG_RAW =0x0, SUB_FRAG_COMPRESSED=0x1} ;

typedef struct RLEFragment{
    
    FragmentType type;
    bool isCompressed;
    uint16_t numTexels;
    
    int16_t dx;
    int16_t dy;
    
} RLEFragment;

bool ReadImage(uint8_t* src, Texture* dst, size_t* byteRead);

#endif /* defined(__pak__rle__) */
