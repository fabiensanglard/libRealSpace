//
//  IffLexer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__IffLexer__
#define __libRealSpace__IffLexer__



class IffChunk{
    
public:
    
    IffChunk();
    
    uint32_t id;
    uint8_t* data;
    size_t size;
    
    //In the case of FORM,CAT  and LIST
    uint32_t subId;
    std::vector<IffChunk*> childs;
} ;

class IffLexer{
    
public:
    
    IffLexer();
    ~IffLexer();
    
    bool InitFromFile(const char* filepath);
    bool InitFromRAM(uint8_t* data, size_t size);

    void List(FILE* output);
    
    IffChunk* GetChunkByID(uint32_t id);
    
private:
    
    size_t ParseChunk(IffChunk* child);
    size_t ParseFORM(IffChunk* child);
    size_t ParseCAT(IffChunk* child);
    size_t ParseLIST(IffChunk* child);
    
    void Parse(void);
    std::map<uint32_t,IffChunk*> chunks;
    
    ByteStream stream;
    uint8_t* data;
    size_t size;
    
    IffChunk masterChunk;
};

#endif /* defined(__libRealSpace__IffLexer__) */
