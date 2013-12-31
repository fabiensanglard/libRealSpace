//
//  PakArchive.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


PakArchive::PakArchive(){
    
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
    
    
    ByteStream peek(this->stream);
    uint32_t offset = peek.ReadUInt32LE();
    offset &= 0x00FFFFFF ; //Remove the leading 0xE0
    
    uint32_t numEntries = (offset-4)/4;
    
    //First pass to read all the offsets
    for(int i =0 ; i < numEntries ; i ++){
        
        PakEntry* entry = new PakEntry();
        
        offset = stream.ReadUInt32LE();
        offset &= 0x00FFFFFF ; //Remove the leading 0xE0
        
        entry->data = this->data + offset;
        
        entries.push_back(entry);
    }
    
    //Second pass to calculate the sizes.
    int i =0;
    for( ; i < numEntries-1 ; i ++){
        
        PakEntry* entry = entries[i];
        
        entry->size = entries[i+1]->data - entry->data;
    }
    PakEntry* entry = entries[i];
    entry->size = this->data+this->size - entries[i-1]->data;
    
}

bool PakArchive::InitFromFile(const char* filepath){
    char fullPath[512] ;
    fullPath[0] = '\0';
    
    strcat(fullPath, GetBase());
    strcat(fullPath, filepath);
    
    
    FILE* file = fopen(fullPath, "r");
    
    if (!file){
        printf("Unable to open PAK archive: '%s'.\n",filepath);
        return false;
    }
    
    fseek(file, 0,SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file,0 ,SEEK_SET);
    
    uint8_t* fileData = new uint8_t[fileSize];
    fread(fileData, 1, fileSize, file);
    
    strcpy(this->path, filepath);
    
    InitFromRAM(fileData,fileSize);
    
    
    fclose(file);
    
    return true;

}

void PakArchive::InitFromRAM(uint8_t* data, size_t size){
    
    if (this->path[0] == '\0')
        strcpy(this->path, "From RAM");
    
    this->data = data;
    this->size = size;
    
    stream.Set(this->data);
    
    Parse();

}

bool PakArchive::Decompress(const char* dstDirectory){
    
    const char* suffix = ".CONTENT/";
    const char* filePattern = "FILE%d.UNKNOWN";
    char fullDstPath[512];
    
    for( size_t i = 0 ; i < this->entries.size() ; i++){
        
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
        
        
        sprintf(fullDstPath+strlen(fullDstPath),filePattern,i);
        
        //Make sure we have all the directories
        CreateDirectories(fullDstPath);
        
        
        //Write content.
        FILE* dstFile = fopen(fullDstPath,"w");
        PakEntry* entry = entries[i];
        fwrite(entry->data,1, entry->size, dstFile);
        fclose(dstFile);
        
        printf("Extracting file: '%s. (size: %lu).'\n",fullDstPath,entry->size);
    }

    return true;
}

size_t PakArchive::GetNumEntries(void){
    return this->entries.size();
}

PakEntry* PakArchive::GetEntry(size_t index){
    return this->entries[index];
}

void PakArchive::List(FILE* output){
    fprintf(output,"Listing content of PAK archives '%s'\n",this->path);
    for(size_t i =0; i < GetNumEntries() ; i++){
        PakEntry* entry = entries[i];
        if (entry->size != 0)
            fprintf(output,"    Entry [%3lu] size: %7lu bytes.\n",i, entry->size);
        else
            fprintf(output,"    Entry [%3lu] size: %7lu bytes (DUPLICATE).\n",i, entry->size);
    }
}

void PakArchive::GuessContent(FILE* output){
    
}
