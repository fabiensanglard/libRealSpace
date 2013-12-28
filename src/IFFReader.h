//
//  IFFFile.h
//  iff
//
//  Created by Fabien Sanglard on 12/14/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__IFFFile__
#define __iff__IFFFile__





class IFFReader{
    
public:
    
    IFFReader();
    ~IFFReader();
    
    void InitWithFile(const char* filename);
    void InitWithRAM(uint8_t* data, size_t size);
    void ReleaseIFF(void);
    
    void AddReader(IFFChunkReader* chunkReader);
    
    void Parse(void);
    
    
private:
    uint8_t* data;
    size_t size;
    uint8_t* cursor;
    
    void ReadID( char id[5] );
    void ReadInt32BE( uint32_t* result);
    void ReadInt32LE(uint32_t* result);
    void MoveCursor(size_t bytes);
    
    size_t ParseFORM(size_t numTabs);
    size_t ParseLIST(size_t numTabs);
    size_t ParseCAT(size_t numTabs);
    size_t ParseChunk(size_t numTabs);
    
    

    
    std::vector<IFFChunkReader> chunkReaders;
    
};

#endif /* defined(__iff__IFFFile__) */
