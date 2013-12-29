//
//  IffLexer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "IffLexer.h"

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

void Tab(int tab){
    for(int i=0 ; i <tab ;i++)
        putchar('   ');
}

void PrintGetChunkIdText(uint32_t id,int tab){
    char*idTetx = (char*)&id;
    
    Tab(tab);
    
    for(int i=3 ; i >=0 ;i--)
        putchar(idTetx[i]);
    
    putchar('\n');
}



size_t IffLexer::ParseFORM(int tabs){
    
    
    
    IffChunk chunk;
    
    //FORM id
    stream.ReadUInt32BE();
    
    chunk.size = stream.ReadUInt32BE();
    if (chunk.size % 2 != 0)
        chunk.size++;
    size_t bytesToParse = chunk.size;
    
    //Form subtype
    chunk.id = stream.ReadUInt32BE();
    
    
    bytesToParse-=4;
    
    //PrintGetChunkIdText(chunk.id,tabs);
    
    Tab(tabs);
    printf("FORM.");
    
    PrintGetChunkIdText(chunk.id,0);
    
    while (bytesToParse > 0){
        
        Tab(tabs);
        size_t byteParsed = ParseChunk(tabs+1);
    
        Tab(tabs);
        
        bytesToParse -= byteParsed;
    }
    
    
    
    return chunk.size+8;
}

size_t IffLexer::ParseChunk(int tabs){
    
    ByteStream peek(stream);
    
    IffChunk chunk;
    chunk.id = peek.ReadUInt32BE();
    
    switch (chunk.id) {
        case 'FORM':
            return ParseFORM(tabs+1);
            break;
        case 'CAT ':
            return ParseFORM(tabs+1);
            break;
        case 'LIST':
            return ParseFORM(tabs+1);
            break;
        case 'UVXY':
            printf("UVXY");
        default:
        {
            
            chunk.id = stream.ReadUInt32BE();
            
            PrintGetChunkIdText(chunk.id,tabs+1);
            //Unknow chunk :( !
            //Check if we have a hander.
            chunk.size = stream.ReadUInt32BE();
            
            if (chunk.size % 2 != 0)
                chunk.size++;
            
            //That this chunk
            chunk.data = stream.GetPosition();
            stream.MoveForward(chunk.size);
            
            chunks[chunk.id] = chunk;
            
            
            
            return chunk.size+8;
        }
            break;
    }
    
}

size_t IffLexer::ParseCAT(int tabs){
    return ParseFORM(tabs);
}

//Return how many bytes were moved forward
size_t IffLexer::ParseLIST(int tabs){
    return ParseFORM(tabs);
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
        
        printf("bytesToParse %lu\n",bytesToParse);
        size_t byteParsed =ParseChunk(0);
        printf("byteParsed = %lu",byteParsed);
        bytesToParse -= byteParsed;
    }

}

bool IffLexer::HasMoreChunks(void){
    return false;
}

IffChunk* IffLexer::GetNextChunk(void){
    return NULL;
}