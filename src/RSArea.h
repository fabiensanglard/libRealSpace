//
//  RSMap.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSMap__
#define __libRealSpace__RSMap__

//DIRTY HACK !!! DELETE ME ASAP
#define  HEIGHT_DIVIDER 17


typedef struct MapObject{
    
    char name[9];
    char destroyedName[9];
    
    uint32_t position[3];
    
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

#define BLOCK_LOD_MAX 0
#define BLOCK_LOD_MED 1
#define BLOCK_LOD_MIN 2

#define NUM_LODS 3

#define BLOCK_PER_MAP_SIDE 18
#define BLOCKS_PER_MAP (BLOCK_PER_MAP_SIDE*BLOCK_PER_MAP_SIDE)

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
    float elevation[BLOCKS_PER_MAP];

    
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

#endif /* defined(__libRealSpace__RSMap__) */
