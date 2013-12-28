//
//  IFFFile.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/14/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "IFFReader.h"
#include "string.h"

#define CHUNK_HEADER_SIZE 8

IFFReader::IFFReader() :
 data(NULL),
 size(0){
}

IFFReader::~IFFReader(){
    ReleaseIFF();
}

void IFFReader::InitWithFile(const char* filename){
    
    FILE* file = fopen(filename,"r");
    
    if (!file){
        printf("Unable to open iff file at:'%s'.\n",filename);
        return ;
    }
    
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    
    uint8_t* data = (uint8_t*)malloc(size);
    
    fread(data, 1, size, file);
    
    InitWithRAM(data,size);
    
    fclose(file);
}

void IFFReader::InitWithRAM(uint8_t* data, size_t size){
    this->data = data;
    this->size = size;
    this->cursor = data;
}

void IFFReader::ReleaseIFF(void){
}


void IFFReader::ReadID(char id[5]){
    id[0] = *(cursor++);
    id[1] = *(cursor++);
    id[2] = *(cursor++);
    id[3] = *(cursor++);
    id[4] = '\0';
}

void IFFReader::ReadInt32BE(uint32_t* result){
    
    uint32_t toLittleEndian = 0;
    toLittleEndian |= *(cursor++)   << 24 ;
    toLittleEndian |= *(cursor++)   << 16 ;
    toLittleEndian |= *(cursor++)   <<  8 ;
    toLittleEndian |= *(cursor++)   <<  0 ;
    
    *result = toLittleEndian;
}


void IFFReader::ReadInt32LE( uint32_t* result){
    
    uint32_t* intPointer = (uint32_t*)cursor;
    
    MoveCursor(4);
    
    *result = *intPointer;
}

void IFFReader::MoveCursor(size_t bytes){
    this->cursor += bytes;
}

void WriteTabs(size_t numTabs){
    for(size_t i=0 ;i < numTabs ; i++){
        putchar(' ');
        putchar(' ');
    }
}



void IFFReader::void AddChunkReader(ChunkReader* chunkReader){
    
    uint32_t* intID = (uint32_t*)id;
    handlers[*intID] = hander;
}


size_t IFFReader::ParseFORM(size_t numTabs){
    

    uint32_t chunkSize ;
    ReadInt32BE(&chunkSize);
    
    WriteTabs(numTabs);
    printf("%s\n","FORM");
    
    WriteTabs(numTabs);
    printf("%u\n",chunkSize);
    
    
    
    char subId[5];
    ReadID(subId);
    
    WriteTabs(numTabs);
    printf("FORM.%s\n",subId);
    
    int bytesToParse = chunkSize - 4 ; //-4 beacuse of the subtype we just read
    while (bytesToParse > 0){
        
       // WriteTabs(numTabs);
       // printf("Bytes to parse = %d.\n",bytesToParse);
        size_t lastChunkSize = ParseChunk(numTabs+2);
        
        bytesToParse -= lastChunkSize;
    }
    
    if (chunkSize % 2 != 0)
        chunkSize++;
    
    return chunkSize+CHUNK_HEADER_SIZE+4;
}

size_t IFFReader::ParseLIST(size_t numTabs){
    printf("Parsing LIST not implemented.\n");
    exit(0);
}

size_t IFFReader::ParseCAT(size_t numTabs){
    printf("Parsing CAT  not implemented.\n");
    exit(0);
}

size_t IFFReader::ParseChunk(size_t numTabs){
    
    char id[5];
    
    ReadID(id);
    
    if (!strncmp(id,"FORM",4)){
        return ParseFORM( numTabs);
    }
    
    else if (!strncmp(id,"CAT ",4)){
        return ParseCAT(numTabs);
    }
    
    else if (!strncmp(id,"LIST",4)){
        return ParseLIST( numTabs);
    }
    else{
        
        //Unknow chunk :( !
        //Check if we have a hander.
        uint32_t chunkSize ;
        ReadInt32BE(&chunkSize);
        
        WriteTabs(numTabs);
        printf("%s\n",id);
        WriteTabs(numTabs);
        printf("%d\n",chunkSize);
        
        if (chunkSize % 2 != 0)
            chunkSize++;
        
        //Check if we have something that can parse this chunk....
        ChunkReader handler = GetChunkReader(id);
        if (handler != NULL){
            uint8_t* savedCursor = this->cursor;
            handler(id,this->cursor,chunkSize);
            this->cursor = savedCursor;
        }
        MoveCursor(chunkSize);
        
        return chunkSize+CHUNK_HEADER_SIZE;
    }
}

void IFFReader::Parse(void){
    
    int32_t bytesToParse = this->size;
    
    while(bytesToParse > 0){
        
        uint32_t byteParsed ;
        
        char id[5];
        ReadID(id);
        
        //Check this is a FORM
        if (!strncmp(id,"FORM",4)){
            byteParsed= ParseFORM( 0);
        }
        
        else if (!strncmp(id,"CAT ",4)){
            byteParsed= ParseCAT(0);
        }
        
        else if (!strncmp(id,"LIST",4)){
            byteParsed= ParseLIST( 0);
        }
        else{
            //printf("This is not an IFF fike");
            return;
        }

        bytesToParse -= byteParsed;
    }
}

