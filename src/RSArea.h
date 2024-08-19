//
//  RSMap.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
}
#endif

#include <stdint.h>
#include <vector>
#include "Matrix.h"
#include "Quaternion.h"
#include "TreArchive.h"
#include "PakArchive.h"
#include "RSEntity.h"
#include "RSMapTextureSet.h"
#include "IffLexer.h"


#define BLOCK_WIDTH 20000

typedef struct MapObject{
    
    char name[9];
    char destroyedName[9];
    
    int32_t position[3];
    
    RSEntity* entity;
    
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

#define BLOCK_PER_MAP_SIDE 18
#define BLOCKS_PER_MAP (BLOCK_PER_MAP_SIDE*BLOCK_PER_MAP_SIDE)

class RSMapTextureSet;
class RSImage;
class PakArchive;

class RSArea{
public:
    
    RSArea();
    ~RSArea();
    
    void InitFromPAKFileName(const char* pakFilename);
    
    inline AreaBlock* GetAreaBlockByID(int lod,int blockID){
        return &this->blocks[lod][blockID];
    }
    
    inline AreaBlock* GetAreaBlockByCoo(int lod, int x, int y){
        return &this->blocks[lod][x + y * BLOCK_PER_MAP_SIDE];
    }
    
    RSImage* GetImageByID(size_t ID);
    
    //Per block objects list
    std::vector<MapObject> objects[BLOCKS_PER_MAP];
    std::vector<AreaOverlay> objectOverlay;
    float elevation[BLOCKS_PER_MAP];

    size_t GetNumJets(void);
    RSEntity* GetJet(size_t jetID);
	std::map<std::string, RSEntity *> *objCache;
	TreArchive *tre;
    float getGroundLevel(int BLOC, float x, float y);
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
    
    void AddJet(TreArchive* tre, const char* name, Quaternion* orientation, Point3D* position);
    void AddJets(void);
    std::vector<RSEntity*> jets;
};
