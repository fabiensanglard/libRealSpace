//
//  PakArchive.h
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
#pragma once
#include <vector>
#include <map>
#include <stdint.h>

#include "Base.h"
#include "ByteStream.h"

typedef struct PakEntry{
    
    uint8_t* data;
    
    size_t size;
    
    uint8_t type ;
    // The mystery bytes at the end of the table.
    // 0xE0 seems to indicate content. 0xFF indicates an empty entry.
    enum ContentType {EMPTY = 0xFF, CONTENT = 0xE0};
    
    static inline bool Compare(PakEntry* any, PakEntry* other){
        return any->data < other->data;
    }
    
    
    
} PakEntry;

class PakArchive{
    
    
    
public:
    PakArchive();
    ~PakArchive();
    
    bool InitFromFile(const char* filepath);
    void InitFromRAM(const char* name,uint8_t* data, size_t size);
    

    bool Decompress(const char* dstDirectory, const char* extension);
    
    size_t GetNumEntries(void);
    PakEntry* GetEntry(size_t index);
    
    void List(FILE* output);
    
    void GuessContent(FILE* output);
    void GuessPakEntryContent(PakEntry* entry);
    
    char* GetName(void);
    
    inline bool IsReady(void){ return this->ready;}
    
    inline uint8_t* GetData(void){ return this->data;}
    
private:
    
    bool ready;

    ByteStream stream;
    
    void Parse(void);
    uint8_t* data;
    size_t size;
    char path[512] ;
    std::vector<PakEntry*> entries;
    
    // allows to know if we should free the TRE data
    bool initalizedFromFile ;
};

