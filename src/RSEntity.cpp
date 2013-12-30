//
//  RSEntity.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSEntity.h"


RSEntity::RSEntity(){
    
}

RSEntity::~RSEntity(){
    
}

void RSEntity::SetPalette(VGAPalette* palette){
    this->palette = palette;
}

void RSEntity::ParseTXMP(IffChunk* chunk){


    ByteStream stream(chunk->data);
    
    Texture* texture = new Texture();
    
    char name[8];
    
    for(int i=0; i < 8 ; i++)
        name[i] = stream.ReadByte();
    
    uint32_t width = stream.ReadShort();
    uint32_t height= stream.ReadShort();
    
    texture->Set(name, width, height, stream.GetPosition());
    
    AddTexture(texture);

}

void RSEntity::ParseTXMS(IffChunk* chunk){
    
    if (chunk==NULL)
        return;
    
    if (chunk->childs.size() <1 ){
        //That is not expected, at minimun we should have an INFO chunk.
        printf("Error: This TXMS doesn't even have an INFO chunk.");
        return;
    }
    
    
    ByteStream stream(chunk->childs[0]->data);
    uint32_t version = stream.ReadUInt32LE();
    //printf("TXMS Version: %u.\n",);
    
    for(int i =1; i < chunk->childs.size() ; i++){
        IffChunk* maybeTXMS = chunk->childs[i];
        if (maybeTXMS->id == 'TXMP')
            ParseTXMP(maybeTXMS);
    }
    
}

void RSEntity::ParseVERT(IffChunk* chunk){
    
    if (chunk==NULL)
        return;
    
    ByteStream stream(chunk->data);
    
    size_t numVertice = chunk->size/12;
    
    for (int i = 0; i < numVertice ; i++) {
        
        int32_t coo ;
        
        
        Vertex vertex ;
        
        coo = stream.ReadInt32LE();
        vertex.z = (coo>>8) + (coo&0x000000FF)/255.0;
        
        coo = stream.ReadInt32LE();
        vertex.x = (coo>>8) + (coo&0x000000FF)/255.0;
        
        coo = stream.ReadInt32LE();
        vertex.y = (coo>>8) + (coo&0x000000FF)/255.0;
        AddVertex(&vertex);
    }
    
}

void RSEntity::ParseLVL(IffChunk* chunk){
    
    if (chunk==NULL)
        return;
    
    ByteStream stream(chunk->data);
    
    Lod lod ;
    
    lod.numTriangles = (chunk->size-4)/2;
    
    lod.dist = stream.ReadUInt32LE();
   
    for (int i = 0 ; i < lod.numTriangles ; i++) {
        lod.triangleIDs[i] = stream.ReadUShort();
    }
    
    AddLod(&lod);
}


void RSEntity::ParseVTRI(IffChunk* chunk){
    
    if (chunk==NULL)
        return;
    
    size_t numTriangle= chunk->size/8;
    ByteStream stream(chunk->data);
    
    Triangle triangle ;
    for (int i = 0; i < numTriangle ; i++) {
        
        triangle.property = stream.ReadByte();
        
        
        
        triangle.ids[0] = stream.ReadByte();
        triangle.ids[1] = stream.ReadByte();
        triangle.ids[2] = stream.ReadByte();
        
        
        triangle.color = stream.ReadByte();
        
        triangle.flags[0] = stream.ReadByte();
        triangle.flags[1] = stream.ReadByte();
        triangle.flags[2] = stream.ReadByte();
        
        AddTriangle(&triangle);
    }
    
}

void RSEntity::ParseUVXY(IffChunk* chunk){
    
    if (chunk==NULL)
        return;
    
    ByteStream stream(chunk->data);
    
    size_t numEntries = chunk->size/8;
    
    uvxyEntry uvEntry;
    for (size_t i=0; i < numEntries; i++) {
        
        uvEntry.triangleID = stream.ReadByte();
        uvEntry.textureID =  stream.ReadByte();
        
        
        uvEntry.uvs[0].u = stream.ReadByte();
        uvEntry.uvs[0].v = stream.ReadByte();
        
        uvEntry.uvs[1].u = stream.ReadByte();
        uvEntry.uvs[1].v = stream.ReadByte();
        
        uvEntry.uvs[2].u = stream.ReadByte();
        uvEntry.uvs[2].v = stream.ReadByte();
        
        AddUV(&uvEntry);
        
    }
}


void RSEntity::InitFromIFF(IffLexer* lexer){
    
    
    IffChunk* chunk;
    
    chunk = lexer->GetChunkByID('UVXY');
    ParseUVXY(chunk);
    
    chunk = lexer->GetChunkByID('VTRI');
    ParseVTRI(chunk);
    
    chunk = lexer->GetChunkByID('VERT');
    ParseVERT(chunk);
    
    chunk = lexer->GetChunkByID('TXMS');
    ParseTXMS(chunk);
    
    chunk = lexer->GetChunkByID('LVL0');
    if (chunk!= NULL)
        ParseLVL(chunk);
    
    chunk = lexer->GetChunkByID('LVL1');
    if (chunk!= NULL)
        ParseLVL(chunk);
    
    chunk = lexer->GetChunkByID('LVL2');
    if (chunk!= NULL)
        ParseLVL(chunk);
}


void RSEntity::AddTexture(Texture* texture){
    this->textures[numTextures] = *texture;
    numTextures++;
}

void RSEntity::AddVertex(Vertex* vertex){
    this->vertices[numVertices] = *vertex;
    numVertices++;
}

void RSEntity::AddUV(uvxyEntry* uv){
    this->uvs[numUVs] = *uv;
    numUVs++;
}

void RSEntity::AddLod(Lod* lod){
    this->lods[numLods] = *lod;
    numLods++;
}

void RSEntity::AddTriangle(Triangle* triangle){
    this->triangles[numTriangles] = * triangle;
    numTriangles++;
}