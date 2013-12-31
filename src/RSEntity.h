//
//  RSEntity.h
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSEntity__
#define __libRealSpace__RSEntity__

#define LOD_LEVEL_MAX 0
#define LOD_LEVEL_MED 1
#define LOD_LEVEL_MIN 2

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
    
    uint8_t property;
    uint8_t ids[3];
    
    uint8_t color;
    uint8_t flags[3];
    
} Triangle ;

typedef struct Lod{
    
    uint32_t dist;
    uint16_t numTriangles;
    uint16_t triangleIDs[256];
} Lod;



class RSEntity{
    
public:
    
    RSEntity();
    ~RSEntity();
    
    void InitFromIFF(IffLexer* lexer);
    
    void AddImage(RSImage* image);
    void AddVertex(Vertex* vertex);
    void AddUV(uvxyEntry* uv);
    void AddLod(Lod* lod);
    void AddTriangle(Triangle* triangle);
    
    void SetPalette(VGAPalette* palette);
    
    
    
    uint32_t numImages;
    RSImage* images[256];
    
    Vertex vertices[65536];
    uint32_t numVertices;
    
    uvxyEntry uvs[256];
    uint32_t numUVs;
    
    Lod lods[5];
    uint32_t numLods;
    
    Triangle triangles[65536];
    uint32_t numTriangles;
    
    enum Property { TRANSPARENT = 0x02};
    
private:
    
    VGAPalette* palette;
    
    
    void ParseVERT(IffChunk* chunk);
    void ParseLVL(IffChunk* chunk);
    void ParseVTRI(IffChunk* chunk);
    void ParseTXMS(IffChunk* chunk);
    void ParseUVXY(IffChunk* chunk);
    void ParseTXMP(IffChunk* chunk);
};

#endif /* defined(__libRealSpace__RSEntity__) */
