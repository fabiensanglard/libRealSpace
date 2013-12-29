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


void RSEntity::Parse(IffLexer* lexer){
    
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