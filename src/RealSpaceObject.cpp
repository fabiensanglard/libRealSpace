//
//  Object.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


RealSpaceObject::RealSpaceObject(){
    
}

RealSpaceObject::~RealSpaceObject(){
    
}


void RealSpaceObject::AddTexture(Texture* texture){
    this->textures[numTextures] = *texture;
    numTextures++;
}

void RealSpaceObject::AddVertex(Vertex* vertex){
    this->vertices[numVertices] = *vertex;
    numVertices++;
}

void RealSpaceObject::AddUV(uvxyEntry* uv){
    this->uvs[numUVs] = *uv;
    numUVs++;
}

void RealSpaceObject::AddLod(Lod* lod){
    this->lods[numLods] = *lod;
    numLods++;
}

void RealSpaceObject::AddTriangle(Triangle* triangle){
    this->triangles[numTriangles] = * triangle;
    numTriangles++;
}