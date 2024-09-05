//
//  PakArchive.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "PakArchive.h"
#include <errno.h>

PakArchive::PakArchive() :
    initalizedFromFile(false),
   ready(false){
    
}

PakArchive::~PakArchive(){
    if (initalizedFromFile)
        delete[] this->data;
}



void PakArchive::Parse(void){
    
    uint32_t advertisedSize = stream.ReadUInt32LE();
    
    if (advertisedSize != this->size){
        printf("'%s' is not a PAK archive !.\n",this->path);
        return;
    }
    
    ready = true;
    
    ByteStream peek(this->stream);
    uint32_t offset = peek.ReadUInt32LE();
    offset &= 0x00FFFFFF ; //Remove the leading 0xE0
    
    size_t numEntries = (offset-4)/4;
    
    //Hashmap to keep track of duplicates
    std::map<uint32_t,void*> uniqueOffsets;
    
    
    //First to read all the offsets
    for(int i =0 ; i < numEntries ; i ++){
        
        
        offset = stream.ReadUInt32LE();
        
        
        PakEntry* entry = new PakEntry();

        entry->type = (offset & 0xFF000000) >> 24;
        
        offset &= 0x00FFFFFF ; //Remove the leading 0xE0 or 0xFF
        
        
        
        entry->data = this->data + offset;
        
        //if (uniqueOffsets[offset] == NULL){
            entries.push_back(entry);
            uniqueOffsets[offset] = entry;
        //}
        
    }
    
   
    //numEntries = uniqueOffsets.size();
    
    
    //Second pass to calculate the sizes.
    int i =0;
    for( ; i < numEntries-1 ; i ++){
        PakEntry* entry = entries[i];
        entry->size = entries[i+1]->data - entry->data;
    }
    
    PakEntry* entry = entries[i];
    entry->size = (this->data + this->size) - entries[i]->data;
    
    //std::sort(entries.begin(), entries.end(),PakEntry::Compare);
}

bool PakArchive::InitFromFile(const char* filepath){
    char fullPath[512] ;
    fullPath[0] = '\0';
    
    strcat(fullPath, GetBase());
    strcat(fullPath, filepath);
    
    
    FILE* file = fopen(fullPath, "r+b");
    
    if (!file){
        printf("Unable to open PAK archive: '%s'.\n",filepath);
        return false;
    }
    
    fseek(file, 0,SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file,0 ,SEEK_SET);
    
    uint8_t* fileData = new uint8_t[fileSize];
    fread(fileData, 1, fileSize, file);
    
    initalizedFromFile = true;
    
    InitFromRAM(filepath,fileData,fileSize);
    
    
    fclose(file);
    
    return true;

}

void PakArchive::InitFromRAM(const char* name,uint8_t* data, size_t size){
    
    strcpy(this->path,name);
    
    this->data = data;
    this->size = size;
    
    stream.Set(this->data);
    
    ready = false;
    
    Parse();

}

bool PakArchive::Decompress(const char* dstDirectory,const char* extension){
    
    const char* suffix = ".CONTENT/";
    const char* filePattern = "FILE%d.%s";
    char fullDstPath[512];
    
    printf("Decompressing PAK %s (size: %llu bytes)\n.",this->path,this->size);
    
    for( size_t i = 0 ; i < this->entries.size() ; i++){
        
        PakEntry* entry = entries[i];
        
        if(entry->size == 0)
            continue;
        
        //Build dst path
        fullDstPath[0] = '\0';
        strcat(fullDstPath, dstDirectory);
        
        //Make sure we have a slash at the end of the dst.
        size_t pathLength = strlen(fullDstPath);
        if (fullDstPath[pathLength-1] != '/')
            strcat(fullDstPath,"/");
        
        //Append the PAK archive name
        strcat(fullDstPath, this->path);
        
        //Append the subdirectory name.
        strcat(fullDstPath, suffix);
        
        
        sprintf(fullDstPath+strlen(fullDstPath),filePattern,i,extension);
        //Convert '\\' to '/'
        size_t sizeFullPath = strlen(fullDstPath);
        for (int i =0 ; i < sizeFullPath ; i++){
            if (fullDstPath[i] =='\\')
                fullDstPath[i] = '/';
        }
        
        //Make sure we have all the directories
        CreateDirectories(fullDstPath);
        
        
        //Write content.
        FILE* dstFile = fopen(fullDstPath, "w+b");
        
        
        if (dstFile == NULL){
            printf("Unable to create destination file: '%s'.\n",fullDstPath);
            continue;
        }
        
        size_t byteWritten = fwrite(entry->data,1, entry->size, dstFile);
        
        if (byteWritten != entry->size){

            printf("*Error while writing entry (errono: %s) size(size: %llu).\n",strerror(errno),entry->size);
        }
        else
            printf("Extracted file: '%s. (size: %llu).'\n",fullDstPath,entry->size);
        fclose(dstFile);
        
        
    }

    return true;
}

size_t PakArchive::GetNumEntries(void){
    return this->entries.size();
}

PakEntry* PakArchive::GetEntry(size_t index){
    if (index < this->entries.size()) {
        return this->entries[index];
    }
    return nullptr;
}

void PakArchive::List(FILE* output){
    fprintf(output,"Listing content of PAK archives '%s'\n",this->path);
    for(size_t i =0; i < GetNumEntries() ; i++){
        PakEntry* entry = entries[i];
       
        if (entry->size != 0)
            fprintf(output,"    Entry [%3zu] offset[0x%8llX] size: %7llu bytes, type: %X.\n",i,entry->data-this->data, entry->size,entry->type);
        else
            fprintf(output,"    Entry [%3llu] offset[0x%8llX] size: %7llu bytes, type: %X (DUPLICATE).\n",i,entry->data-this->data, entry->size,entry->type);
    }
}


void PakArchive::GuessPakEntryContent(PakEntry* entry){
    
}

void PakArchive::GuessContent(FILE* output){
    for(size_t i=0 ; i < entries.size() ; i++){
        GuessPakEntryContent(entries[i]);
    }
}


char* PakArchive::GetName(void){
    return this->path;
}