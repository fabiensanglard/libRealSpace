//
//  main.c
//  tre
//
//  Created by fabien sanglard on 12/7/2013.
//  Copyright (c) 2013 fabien sanglard. All rights reserved.
//

#include "TreArchive.h"


TreArchive::TreArchive():
    initalizedFromFile(false),
    valid(false)
{

    this->path[0] = '\0';
}

TreArchive::~TreArchive(){
    
    if (initalizedFromFile)
        delete[] this->data;
    
    for(size_t i=0 ; i < entries.size() ; i++){
        delete entries[i];
    }
}

bool TreArchive::InitFromFile(const char* filepath){
    
    char fullPath[512] ;
    fullPath[0] = '\0';
    
    strcat(fullPath, GetBase());
    strcat(fullPath, filepath);
    
    
    FILE* file = fopen(fullPath, "r+b");
    
    if (!file){
        printf("Unable to open TRE archive: '%s'.\n",filepath);
        return false;
    }
    
    fseek(file, 0,SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file,0 ,SEEK_SET);
    
    uint8_t* fileData = new uint8_t[fileSize];
    fread(fileData, 1, fileSize, file);
    
    
    InitFromRAM(filepath,fileData,fileSize);
    
    
    fclose(file);
    
    return true;
}

void TreArchive::InitFromRAM(const char* name,uint8_t* data, size_t size){
    
    strcpy(this->path, name);
    
    this->data = data;
    this->size = size;
    
    Parse();
}


void TreArchive::ReadEntry(ByteStream* stream, TreEntry* entry){
    /*The format of a TRE entry is (see doc/p1_tre_format.txt):
     
     data type    |     size      |       description
     ------------------------------------------------------------------------------
     byte        |       1       |  always 0x1, it's meaning is not fully
     |               | understood.
     ----------------|---------------|---------------------------------------------
     string      |      65       |  The name of the file as the game reads it.
     |               | Privateer's internal file structure demands
     |               | that every file name starts with ..\..
     ----------------|---------------|---------------------------------------------
     dword       |       4       |  The offset on the TRE where the file starts.
     ----------------|---------------|---------------------------------------------
     dword       |       4       |  The size of the file.
     ----------------|---------------|---------------------------------------------
     

     
     */
    
    entry->unknownFlag = stream->ReadByte();
    
    for(int i=0 ; i < 65 ; i++)
        entry->name[i] = stream->ReadByte();
    
    entry->data = this->data + stream->ReadUInt32LE();
    
    entry->size = stream->ReadUInt32LE();
}





void TreArchive::Parse(void){

    ByteStream stream(this->data);
    
    size_t numEntries = stream.ReadUInt32LE() ;
    
    //The pointer to the start of the data. We are not using it.
    stream.ReadUInt32LE() ;
    
    
    //Now read all entries
    for(size_t i =0; i < numEntries; i++){
        TreEntry* entry = new TreEntry();
        ReadEntry(&stream,entry);
        
        mappedEntries[entry->name] = entry;
        
        // We use a vector so we can return the list of files in the order
        // they were listed in the TRE index.
        entries.push_back(entry);
    }
    
   // std::sort(entries.begin(), entries.end(),TreArchive::Compare);
    
    valid = true;
}

void TreArchive::List(FILE* output){
    
    fprintf(output,"Listing content of TRE archive '%s'.\n",this->path);
    fprintf(output,"    %lu entrie(s) found.\n",entries.size());

    for (size_t i=0 ; i < entries.size() ; i++){
        TreEntry* entry = entries[i];
        fprintf(output,"    Entry [%3lu] offset[0x%8lX]'%s' size: %lu bytes.\n",i,entry->data-this->data,entry->name,entry->size);
    }
}


//Direct access to a TRE entry.
TreEntry* TreArchive::GetEntryByName(const char* entryName){
    return mappedEntries[entryName];
}

bool TreArchive::GetPAKByName(const char* entryName,PakArchive* pak){\
    TreEntry* entry = GetEntryByName(entryName);
    if (entry == NULL)
        return false;
    
    pak->InitFromRAM(entryName, entry->data, entry->size);
    
    if (!pak->IsReady())
        return false;
    
    return true;
}

TreEntry* TreArchive::GetEntryByID(size_t entryID){
    return entries[entryID];
}

size_t TreArchive::GetNumEntries(void){
    return entries.size();
}



bool TreArchive::Decompress(const char* dstDirectory){
    
    for(size_t i=0 ; i < mappedEntries.size() ; i++){
        TreEntry* entry = entries[i];
        
        
        char fullPath[512];
        fullPath[0] = '\0';
        strcat(fullPath,dstDirectory);
        
        //Make sure the dstDirectory end with a /
        size_t dstSize = strlen(fullPath);
        if (fullPath[dstSize-1] != '/')
            strcat(fullPath,"/");
        
        //Remove the leading . and .. and /
        char* cursor = entry->name;
        while(*cursor == '.' ||
              *cursor == '/' ||
              *cursor == '\\')
            cursor++;
        strcat(fullPath, cursor);
        
        //Convert '\\' to '/'
        size_t sizeFullPath = strlen(fullPath);
        for (int i =0 ; i < sizeFullPath ; i++){
            if (fullPath[i] =='\\')
                fullPath[i] = '/';
        }
        
        //Recursively create the directories
        CreateDirectories(fullPath);
        
        //Write file !
        printf("Decompressing TRE file: %lu '%s' %lu (bytes).\n",i,fullPath,entry->size);
        FILE* file = fopen(fullPath,"w+b");
        fwrite(entry->data, 1, entry->size, file);
        fclose(file);
        
    }

    return true;
}



