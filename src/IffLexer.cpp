//
//  IffLexer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "IffLexer.h"


IffChunk::IffChunk() :
    subId(0){
    
}

//A CHUNK_HEADER_SIZE features a 4 bytes ID and a 4 bytes size;
#define CHUNK_HEADER_SIZE 8

IffLexer::IffLexer(){
    
}

IffLexer::~IffLexer(){
    
}

bool IffLexer::InitFromFile(const char* filepath){
    char fullPath[512] ;
    fullPath[0] = '\0';
    
    strcat(fullPath, GetBase());
    strcat(fullPath, filepath);
    
    
    FILE* file = fopen(fullPath, "r");
    
    if (!file){
        printf("Unable to open TRE archive: '%s'.\n",filepath);
        return false;
    }
    
    fseek(file, 0,SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file,0 ,SEEK_SET);
    
    uint8_t* fileData = new uint8_t[fileSize];
    fread(fileData, 1, fileSize, file);
    
   
    
    return InitFromRAM(fileData,fileSize);
    
}

bool IffLexer::InitFromRAM(uint8_t* data, size_t size){
    
    this->data = data;
    this->size = size;
    
    stream.Set(this->data);
    
    Parse();
    
    return true;
}




size_t IffLexer::ParseFORM(IffChunk* chunk){
 
    
    //FORM id
    chunk->id = stream.ReadUInt32BE();
    
    chunk->size = stream.ReadUInt32BE();
    
    if (chunk->size % 2 != 0)
        chunk->size++;
    size_t bytesToParse = chunk->size;
    
    //Form subtype
    chunk->subId = stream.ReadUInt32BE();
    
    
    bytesToParse-=4;
    
    while (bytesToParse > 0){
        
        IffChunk* child = new IffChunk();
        size_t byteParsed = ParseChunk(child);
        chunk->childs.push_back(child);
        
        
        bytesToParse -= byteParsed;
    }
    
    
    
    return chunk->size+8;
}

size_t IffLexer::ParseChunk(IffChunk* chunk){
    
    ByteStream peek(stream);
    
    
    chunk->id = peek.ReadUInt32BE();
    
    switch (chunk->id) {
        case 'FORM':
            return ParseFORM(chunk);
            break;
        case 'CAT ':
            return ParseFORM(chunk);
            break;
        case 'LIST':
            return ParseFORM(chunk);
            break;
        default:
        {
            
            chunk->id = stream.ReadUInt32BE();
            
            chunk->size = stream.ReadUInt32BE();
            
            if (chunk->size % 2 != 0)
                chunk->size++;
            
            //That this chunk
            chunk->data = stream.GetPosition();
            stream.MoveForward(chunk->size);
            
            chunks[chunk->id] = chunk;
            
            
            
            return chunk->size+8;
        }
            break;
    }
    
}

size_t IffLexer::ParseCAT(IffChunk* chunk){
    return ParseFORM(chunk);
}

//Return how many bytes were moved forward
size_t IffLexer::ParseLIST(IffChunk* chunk){
    return ParseFORM(chunk);
}

void IffLexer::Parse(void){
    
    size_t bytesToParse = this->size;
    
    
    ByteStream peek(stream);
    uint32_t header = peek.ReadUInt32BE();
    
    switch (header) {
        case 'FORM':
        case 'CAR ':
        case 'LIST':
            break;
        default:
        {
            printf("ERROR, this is not an IFF file.\n");
            return;
            break;
        }
    }
    
    
   
    while(bytesToParse > 0){
        

        IffChunk* child = new IffChunk();
    
        size_t byteParsed =ParseChunk(child);
        
        masterChunk.childs.push_back(child);
        
        
        bytesToParse -= byteParsed;
        
    }

}

IffChunk* IffLexer::GetChunkByID(uint32_t id){
    return this->chunks[id];
}

void Tab(int tab){
    for(int i=0 ; i <tab*2 ;i++)
        putchar(' ');
}

char textIDs[5];
char* GetChunkTextID(uint32_t id){
    char* cursor = (char*)&id;
    for (int i=3 ; i >=0 ; i--)
        textIDs[i] = cursor[3-i];
    
    
    return textIDs;
}

void ListChunkContent(uint32_t level,IffChunk* chunk){
    
    Tab(level);
    printf("%s\n",GetChunkTextID(chunk->id));
    
    Tab(level);
    printf("%lu\n",chunk->size);
    
    if (chunk->subId != 0){
        Tab(level);
        printf("%s\n",GetChunkTextID(chunk->subId));
        
    }
    
    for(size_t i = 0 ; i < chunk->childs.size() ; i++){
        IffChunk* child =  chunk->childs[i];
        ListChunkContent(level+1,child);
    }
    
}

void IffLexer::List(FILE* output){
    
    uint32_t level = 0;
    
    for(size_t i = 0 ; i < masterChunk.childs.size() ; i++){
        IffChunk* child =  masterChunk.childs[i];
        ListChunkContent(level,child);
    }
}
