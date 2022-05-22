//
//  tre.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include "Base.h"
#include "ByteStream.h"
#include "PakArchive.h"

struct Char_String_Comparator
{
    bool operator()(char const *a, char const *b) const
    {
        return strcmp(a, b) < 0;
    }
};

typedef struct TreEntry{
    
    uint8_t     unknownFlag;
    char        name[65];
    size_t      size;
    uint8_t*    data;
    
} TreEntry;



class TreArchive{
    
public:
    
     TreArchive();
    ~TreArchive();
 
    bool InitFromFile(const char* filepath);
    void InitFromRAM(const char* name, uint8_t* data, size_t size);
    
    char* GetPath(void);
    
    void List(FILE* output);
    
    //Direct access to a TRE entry.
    TreEntry* GetEntryByName(const char* entryName);
    
    //Build a pak directly
    bool GetPAKByName(const char* entryName,PakArchive* pak);
    
    //A way to iterate through all entries in the TRE.
    TreEntry* GetEntryByID(size_t entryID);
    size_t GetNumEntries(void);
    
    bool Decompress(const char* dstDirectory);
    
    
    static inline bool Compare(TreEntry* any, TreEntry* other){
        return any->data < other->data;
    }
    
    inline uint8_t* GetData(void){ return data; }
    
    inline bool IsValid(void){ return this->valid;}
    
    std::vector<TreEntry*> entries;

private:
    
    bool valid;
    
    
    void ReadEntry(ByteStream* stream, TreEntry* entry);
    void Parse(void);
    
    //
    char path[512];
    uint8_t* data;
    size_t   size;
    
    
    
    // allows to know if we should free the TRE data
    bool initalizedFromFile ;
    

    std::map<const char*,TreEntry*,Char_String_Comparator> mappedEntries;
};
