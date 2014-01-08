//
//  RSMap.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSMap__
#define __libRealSpace__RSMap__

typedef struct  BlockVertex{
    uint8_t rawData[6];
    Vertex v;
} BlockVertex;

typedef struct AreaBlock{
    
    size_t width;
    size_t height;
    
    //To be delete later when we can parse it properly
    MapVertex vertice[400];
    
    
} AreaBlock;

#define BLOCK_LOD_MAX 0
#define BLOCK_LOD_MED 1
#define BLOCK_LOD_MIN 2

#define NUM_LODS 3
#define BLOCKS_PER_MAP 324


class RSArea{
public:
    
    RSArea();
    ~RSArea();
    
    void InitFromPAKFileName(const char* pakFilename);
    
    inline AreaBlock* GetAreaBlock(int lod, int blockID){
        return &this->blocks[lod][blockID];
    }
    
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
    
    float elevation[BLOCKS_PER_MAP];
};

#endif /* defined(__libRealSpace__RSMap__) */
