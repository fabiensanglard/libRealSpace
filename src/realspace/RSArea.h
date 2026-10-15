//
//  RSMap.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "precomp.h"
#include "block_def.h"
#include "../commons/IFFSaxLexer.h"

typedef struct MapObject{
    
    char name[9];
    char destroyedName[9];
    
    Vector3D position{0,0,0};
    
    RSEntity* entity{nullptr};
    float orientation[3][3];
    std::vector<uint8_t> unknowns;
    
} MapObject;

struct CloudPuff {
    float ox, oy, oz;   // offset relatif au centre du nuage
    float rx, ry, rz;   // demi-axes de l'ellipsoïde
};

struct Cloud {
    Vector3D position;           // centre monde (altitude fixe)
    std::vector<CloudPuff> puffs;
    float alpha;
};


typedef struct AreaBlock{
    
    size_t width;
    size_t height;
    
    int sideSize;
    
    //To be delete later when we can parse it properly
    MapVertex vertice[400];
    
    inline MapVertex* GetVertice(int x, int y){
        return &vertice[x + y * this->sideSize];
    }
    
} AreaBlock;
struct AoVPoints {
    int32_t x, z, y;      // i32 LE chacun  (es:[si], es:[si+4], es:[si+8])
    uint8_t pad;          // +0x0C, non lu par le jeu
};

struct AreaOverlayTriangles {
    uint8_t  flag0;            // +0x00  (toujours 0 dans les données SC)
    uint16_t verticesIdx[3];   // +0x01 / +0x03 / +0x05
    uint8_t  type;             // +0x07  (hypothèse : 6 = texturé)
    uint16_t color;            // +0x08  (index couleur / texture)
    UV       uv[3];            // +0x0A  (3 × {u8 u; u8 v;})
};
struct AreaOverlay {
    AoVPoints* vertices;
    std::vector<AoVPoints> verticesVec;
    AreaOverlayTriangles trianles[400];
    int lx, ly, hx, hy;
    int nbTriangles;
    uint16_t              param2 = 0, param3 = 0;
    std::vector<uint16_t> param3Table;   // section 3, brute
    std::vector<uint16_t> grid;          // section 4, brute
    std::vector<uint16_t> drawList;      // polygones à dessiner (résolu depuis la grille)
};
#define BLOCK_LOD_MAX 0
#define BLOCK_LOD_MED 1
#define BLOCK_LOD_MIN 2

#define NUM_LODS 3



class RSMapTextureSet;
class RSImage;
class PakArchive;
struct SkirtVertex {
    float x, y, z;
    float r, g, b;
};
struct SkirtTriangle {
    SkirtVertex v[3];
};
struct PrecomputedSkirts {
    std::vector<SkirtTriangle> tris;
};
class RSArea{
public:
    
    RSArea();
    ~RSArea();
    
    void InitFromPAKFileName(const char* pakFilename);
    void InitFromZipFileName(std::string zipFilename);
    void InitFromRam(const char *pakFilename, uint8_t *data, size_t size);
    inline AreaBlock* GetAreaBlockByID(int lod,int blockID){
        if (blockID < 0 || blockID >= BLOCKS_PER_MAP || lod < 0 || lod >= NUM_LODS) {
            return nullptr;
        }
        return &this->blocks[lod][blockID];
    }
    
    inline AreaBlock* GetAreaBlockByCoo(int lod, int x, int y){
        return &this->blocks[lod][x + y * BLOCK_PER_MAP_SIDE];
    }
    
    RSImage* GetImageByID(size_t ID);
   
    //Per block objects list
    std::vector<MapObject> objects;
    std::vector<AreaOverlay> objectOverlay;
    std::vector<Cloud> clouds;
    float elevation[BLOCKS_PER_MAP];
	TreArchive *tre;
    float getGroundLevel(int BLOC, float x, float y);
    float getY(float x, float z);
    void BuildSkirts();                  // pré-calcul des jupes (une fois après ParseHeightMap)
    void generateClouds(int count, float altitude, float spread);
    const PrecomputedSkirts& GetSkirts() const { return skirts_; }
private:
    AssetManager &assetsManager = AssetManager::getInstance();
    SCRenderer &Renderer = SCRenderer::getInstance();
    void ParseObjects(void );
    
    void ParseTrigo(void );
    void ParseTriFile(PakEntry* entry);
    
    //Temporary name: I don't know yet what is in there.
    void ParseHeightMap(void);
    void ParseBlocks(size_t lod,PakEntry* entry,size_t verticePerBlock);
    
    void ParseElevations(void);
    
    std::vector<RSMapTextureSet*> textures;
    RSMapTextureSet overlay_textures;
    PakArchive* archive;
    
    // An area is made of 18*18 (324) blocks each block has 3 levels of details
    // Level 0 blocks are 20*20;
    // Level 1 blocks are 10*10;
    // Level 0 blocks are  5* 5;
    AreaBlock blocks[NUM_LODS][BLOCKS_PER_MAP];
    
    void parseTERA(uint8_t *data, size_t size);
    void parseTERA_VERS(uint8_t *data, size_t size);
    void parseTERA_INFO(uint8_t *data, size_t size);
    void parseTERA_BLOX(uint8_t *data, size_t size);
    void parseTERA_BLOX_ELEV(uint8_t *data, size_t size);
    void parseTERA_BLOX_ATRI(uint8_t *data, size_t size);
    void parseTERA_BLOX_OBJS(uint8_t *data, size_t size);
    void parseTERA_TXML(uint8_t *data, size_t size);
    void parseTERA_TXML_INFO(uint8_t *data, size_t size);
    void parseTERA_TXMS_MAPS(uint8_t *data, size_t size);

    char name[16];
    PrecomputedSkirts skirts_;
};
