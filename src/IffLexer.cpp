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

size_t IffLexer::ParseChunk(){
    
    IffChunk chunk;
    chunk.id = stream.ReadUInt32LE();
    
    if (!strncmp(chunk.textId,"FORM",4)){
        return ParseFORM();
    }
    
    else if (!strncmp(chunk.textId,"CAT ",4)){
        return ParseCAT();
    }
    
    else if (!strncmp(chunk.textId,"LIST",4)){
        return ParseLIST();
    }
    else{
        
        //Unknow chunk :( !
        //Check if we have a hander.
        chunk.size = stream.ReadUInt32BE();

        if (chunk.size % 2 != 0)
            chunk.size++;
        
        //Skip this content
        chunk.data = stream.GetPosition();
        
        stream.MoveForward(chunk.size);
        
        chunks[chunk.id] = chunk;
        
        printf("%4s\n",chunk.textId);
        
        return chunk.size;
    }
}

size_t IffLexer::ParseFORM(){
    
    uint32_t chunkSize ;
    chunkSize= stream.ReadUInt32BE();

    //Sub ID
    IffChunk chunk;
    chunk.id = stream.ReadUInt32LE();
    
    
    size_t bytesToParse = chunkSize  ; //-4 beacuse of the subtype and chunkSize we just read
    while (bytesToParse > 0){
        
        size_t lastChunkSize = ParseChunk();
        
        bytesToParse -= lastChunkSize + CHUNK_HEADER_SIZE+4;
    }
    
    if (chunkSize % 2 != 0)
        chunkSize++;
    
    return chunkSize+CHUNK_HEADER_SIZE+4;
}

size_t IffLexer::ParseCAT(){
    
    stream.ReadUInt32BE();
    size_t chunkSize = stream.ReadUInt32BE();
    stream.MoveForward(chunkSize);
    
    printf("***Warning**** CAT not implemented.\n");
    return chunkSize + CHUNK_HEADER_SIZE;
}

//Return how many bytes were moved forward
size_t IffLexer::ParseLIST(){
    
    ByteStream stream(data);
    
    stream.ReadUInt32BE();
    size_t chunkSize = stream.ReadUInt32BE();
    
    printf("***Warning**** LIST not implemented.\n");
    return chunkSize + CHUNK_HEADER_SIZE;
}

void IffLexer::Parse(void){
    
    size_t bytesToParse = this->size;
    
    while(bytesToParse > 0){
        
        size_t byteParsed =0;
        
        IffChunk chunk;
        chunk.id = stream.ReadUInt32LE();
        bytesToParse-=4;
        
        //Check this is a FORM
        if (!strncmp(chunk.textId,"FORM",4)){
            byteParsed= ParseFORM();
        }
        
        else if (!strncmp(chunk.textId,"CAT ",4)){
            byteParsed= ParseCAT();
        }
        
        else if (!strncmp(chunk.textId,"LIST",4)){
            byteParsed= ParseLIST();
        }
        else{
            //printf("This is not an IFF file !\n");
            byteParsed = bytesToParse;
        }
        bytesToParse -= byteParsed;
    }

}

bool IffLexer::HasMoreChunks(void){
    return false;
}

IffChunk* IffLexer::GetNextChunk(void){
    return NULL;
}