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


typedef struct MapVertex{
    float x;
    float y;
    float z;

    uint8_t flag;
    uint8_t type;
    uint8_t textSet;
    uint8_t text;
    
    float color[3];
    
} MapVertex ;


typedef struct Vertex{
    float x;
    float y;
    float z;
} Vertex ;

typedef struct BoudingBox{
    Vertex min;
    Vertex max;
} BoudingBox;

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
    
    void InitFromRAM(uint8_t* data, size_t size);
    void InitFromIFF(IffLexer* lexer);
    
    void AddImage(RSImage* image);
    size_t NumImages(void);
    
    void AddVertex(Vertex* vertex);
    size_t NumVertice(void);
    
    void AddUV(uvxyEntry* uv);
    size_t NumUVs(void);
    
    void AddLod(Lod* lod);
    size_t NumLods(void);
    
    void AddTriangle(Triangle* triangle);
    size_t NumTriangles(void);
    
    void SetPalette(VGAPalette* palette);
    
    
    std::vector<RSImage*> images;
    std::vector<Vertex> vertices;
    std::vector<uvxyEntry> uvs;
    std::vector<Lod> lods;
    std::vector<Triangle> triangles;

    
    enum Property { TRANSPARENT = 0x02};
    
    BoudingBox* GetBoudingBpx(void);
    
    
private:
    
    BoudingBox bb;
    void CalcBoundingBox(void);
    
    VGAPalette* palette;
    
    
    void ParseVERT(IffChunk* chunk);
    void ParseLVL(IffChunk* chunk);
    void ParseVTRI(IffChunk* chunk);
    void ParseTXMS(IffChunk* chunk);
    void ParseUVXY(IffChunk* chunk);
    void ParseTXMP(IffChunk* chunk);
};

#endif /* defined(__libRealSpace__RSEntity__) */
