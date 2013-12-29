//
//  IffLexer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__IffLexer__
#define __libRealSpace__IffLexer__

typedef struct{
    
    uint32_t id;
    uint8_t* data;
    size_t size;
    
} IffChunk;

class IffLexer{
    
public:
    
    IffLexer();
    ~IffLexer();
    
    bool InitFromFile(const char* filepath);
    bool InitFromRAM(uint8_t* data, size_t size);

    bool HasMoreChunks(void);
    IffChunk* GetNextChunk(void);
    
private:
    
    size_t ParseChunk(int tabs);
    size_t ParseFORM(int tabs);
    size_t ParseCAT(int tabs);
    size_t ParseLIST(int tabs);
    
    void Parse(void);
    std::map<uint32_t,IffChunk> chunks;
    
    ByteStream stream;
    uint8_t* data;
    size_t size;
    
};

#endif /* defined(__libRealSpace__IffLexer__) */
