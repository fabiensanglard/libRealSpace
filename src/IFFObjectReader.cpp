//
//  IFFObjectReader.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "IFFObjectReader.h"


IFFChunkReaderObject::IFFChunkReaderObject(){
    
    AddCapability(this->ParseTXMP,"TXMP");
    AddCapability(ParseVERT,"VERT");
    AddCapability(ParseLVL0,"LVL0");
    AddCapability(ParseLVL0,"LVL1");
    AddCapability(ParseLVL0,"LVL2");
    AddCapability(ParseVTRI,"VTRI");
    AddCapability(ParseUVXY,"UVXY");
}

IFFChunkReaderObject::~IFFChunkReaderObject(){
    
}

void IFFChunkReaderObject::ParseTXMP(char id[4],uint8_t* data, size_t size){
    
    Texture* texture = new Texture();
    
    char name[8];
    uint32_t width=0;
    uint32_t height=0;
    
    for(int i=0; i < 8 ; i++)
        name[i] = (char)*data++;
    
    
    
    
    uint16_t* dim = (uint16_t*)data;
    width = *dim;
    data+=2;
    
    dim = (uint16_t*)data;
    height = *dim;
    data+=2;
    
    texture->Set(name, width, height, data);
    
    object.AddTexture(texture);
}

void IFFChunkReaderObject::ParseVERT(char id[4],uint8_t* data, size_t size){
    
    Vertex vertex ;//(Vertex*)data;
    
    int32_t* coo = (int32_t*)data;
    
    for (int i = 0; i < size/12 ; i++) {
        vertex.x = (*coo>>8) + (*coo&0x000000FF)/256.0; coo++;
        vertex.y = (*coo>>8) + (*coo&0x000000FF)/256.0; coo++;
        vertex.z = (*coo>>8) + (*coo&0x000000FF)/256.0; coo++;
        object.AddVertex(&vertex);
    }
    
}

void IFFChunkReaderObject::ParseLVL0(char id[4],uint8_t* data, size_t size){
    
    Lod lod ;
    
    lod.numTriangles = (size-4)/2;
    
    
    uint32_t* dist = (uint32_t*)data;
    lod.dist = *dist++;
    
    data+=4;
    
    uint16_t* dim =(uint16_t*)data;
    
    for (int i = 0 ; i < lod.numTriangles ; i++) {
        lod.triangleIDs[i] = *dim++;
    }
    
    object.AddLod(&lod);
}


void IFFChunkReaderObject::ParseVTRI(char id[4],uint8_t* data, size_t size){
    
    
    Triangle triangle ;
    for (int i = 0; i < size/8; i++) {
        
        triangle.flag0 = *data++;
        
        
        
        triangle.ids[0] = *data++;
        triangle.ids[1] = *data++;
        triangle.ids[2] = *data++;
        
        
        triangle.color = *data++;
        
        triangle.flags[1] = *data++;
        triangle.flags[1] = *data++;
        triangle.flags[2] = *data++;
        
        object.AddTriangle(&triangle);
    }
    
}

void IFFChunkReaderObject::ParseUVXY(char id[4],uint8_t* data, size_t size){
    
    size_t numEntries = size/8;
    
    uvxyEntry uvEntry;
    for (size_t i=0; i < numEntries; i++) {
        
        uvEntry.triangleID = *data++;
        uvEntry.textureID = *data++;
        
        
        uvEntry.uvs[0].u = *data++;
        uvEntry.uvs[0].v = *data++;
    
        uvEntry.uvs[1].u = *data++;
        uvEntry.uvs[1].v = *data++;
        
        uvEntry.uvs[2].u = *data++;
        uvEntry.uvs[2].v = *data++;
        
        object.AddUV(&uvEntry);
        
    }
}



