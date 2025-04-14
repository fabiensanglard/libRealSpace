//
//  RSMap.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "precomp.h"


typedef struct MapObject{
    
    char name[9];
    char destroyedName[9];
    
    Vector3D position{0,0,0};
    
    RSEntity* entity{nullptr};
    std::vector<uint16_t> progs_id;
    
} MapObject;

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
struct AreaOverlayTriangles {
    int verticesIdx[3];
    uint8_t color;
    uint8_t u0, u1, u2, u3, u4;
    uint8_t u5, u6, u7, u8, u9;
    uint8_t u10,u11;
};
struct AoVPoints {
    int x;
    int y;
    int z;
    int u0;
    int u1;
    int u2;
};
struct AreaOverlay {
    AoVPoints* vertices;
    AreaOverlayTriangles trianles[400];
    int lx, ly, hx, hy;
    int nbTriangles;
};
#define BLOCK_LOD_MAX 0
#define BLOCK_LOD_MED 1
#define BLOCK_LOD_MIN 2

#define NUM_LODS 3



class RSMapTextureSet;
class RSImage;
class PakArchive;

class RSArea{
public:
    
    RSArea();
    RSArea(AssetManager *amanager);
    ~RSArea();
    
    void InitFromPAKFileName(const char* pakFilename);
    
    inline AreaBlock* GetAreaBlockByID(int lod,int blockID){
        return &this->blocks[lod][blockID];
    }
    
    inline AreaBlock* GetAreaBlockByCoo(int lod, int x, int y){
        return &this->blocks[lod][x + y * BLOCK_PER_MAP_SIDE];
    }
    
    RSImage* GetImageByID(size_t ID);
    AssetManager *assetsManager{nullptr};
    //Per block objects list
    std::vector<MapObject> objects;
    std::vector<AreaOverlay> objectOverlay;
    float elevation[BLOCKS_PER_MAP];
	TreArchive *tre;
    float getGroundLevel(int BLOC, float x, float y);
    float getY(float x, float z);
    
private:
    
    void ParseMetadata(void );
    void ParseObjects(void );
    
    void ParseTrigo(void );
    void ParseTriFile(PakEntry* entry);
    
    //Temporary name: I don't know yet what is in there.
    void ParseHeightMap(void);
    void ParseBlocks(size_t lod,PakEntry* entry,size_t verticePerBlock);
    
    void ParseElevations(void);
    
    std::vector<RSMapTextureSet*> textures;
    PakArchive* archive;
    
    // An area is made of 18*18 (324) blocks each block has 3 levels of details
    // Level 0 blocks are 20*20;
    // Level 1 blocks are 10*10;
    // Level 0 blocks are  5* 5;
    AreaBlock blocks[NUM_LODS][BLOCKS_PER_MAP];
    
    
    char name[16];
    
};
