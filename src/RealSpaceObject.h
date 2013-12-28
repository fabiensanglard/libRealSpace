//
//  Object.h
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__Object__
#define __iff__Object__


typedef struct Vertex{
    float x;
    float y;
    float z;
} Vertex ;

typedef struct UV{
    uint8_t u;
    uint8_t v;
} UV ;

typedef struct uvxyEntry{
    
    uint8_t triangleID;
    uint8_t textureID;
    UV uvs[3];
} uvxyEntry;

typedef struct Triangle{
    
    uint8_t flag0;
    uint8_t ids[3];
    
    uint8_t color;
    uint8_t flags[3];
    
} Triangle ;

typedef struct Lod{
    
    uint32_t dist;
    uint16_t numTriangles;
    uint16_t triangleIDs[256];
} Lod;

class RealSpaceObject{
    
public:
    
    RealSpaceObject();
    ~RealSpaceObject();
    
    void AddTexture(Texture* texture);
    void AddVertex(Vertex* vertex);
    void AddUV(uvxyEntry* uv);
    void AddLod(Lod* lod);
    void AddTriangle(Triangle* triangle);
    

    uint32_t numTextures;
    Texture textures[256];
    
    Vertex vertices[65536];
    uint32_t numVertices;
    
    uvxyEntry uvs[256];
    uint32_t numUVs;
    
    Lod lods[5];
    uint32_t numLods;
    
    Triangle triangles[65536];
    uint32_t numTriangles;
    
};

#endif /* defined(__iff__Object__) */
