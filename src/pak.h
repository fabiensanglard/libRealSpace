//
//  pak.h
//  iff
//
//  Created by Fabien Sanglard on 12/25/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef iff_pak_h
#define iff_pak_h

typedef struct Image{
    
    uint8_t* data;
    uint32_t offset;
    uint32_t size;
    
} Image;

class Animation{
    
public:
    Animation();
    ~Animation();
    
    uint32_t offset;
    uint32_t size;
//    enum Type {UNKNOWN,IFF,ANIMATION_IMAGE,ANIMATION_PALETTE,VOC};
//    Type type;
    uint8_t* data;
    
    std::vector<Image*> elements;
    
    Palette* palette;
};




class Entry{
    
    
public:
    
    Entry();
    ~Entry();
    
    uint8_t* data  ;
    uint32_t offset;
    uint32_t size;
    enum Type {UNKNOWN,IFF,ANIMATION,IMAGE,VOC,IFF_XMIDI,PAK,TEXTURE};
    Type type;
    
};

class PakArchive{
    
public:
    char path[512];
    uint32_t size;
    FILE* file;
    uint8_t* data;
    
    
    Entry* entries;
    uint32_t numEntries;
    
    //Offset are sometimes repeated so numFiles can be lower than numOffets
    uint32_t numDifferentOffsets;
    
    
};


#endif
