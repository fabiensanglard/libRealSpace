//
//  AssetManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include <fstream>
#include <iostream>
#include <cstring>
typedef struct TreNameID{
    AssetManager::TreID id ;
    const char* filename;
    
} TreNameID;
const int SECTOR_SIZE = 2353;
#define NUM_TRES 6
TreNameID nameIds[NUM_TRES] =
{
    {AssetManager::TRE_GAMEFLOW,"GAMEFLOW.TRE"},
    {AssetManager::TRE_OBJECTS,"OBJECTS.TRE"},
    {AssetManager::TRE_MISC,"MISC.TRE"},
    {AssetManager::TRE_SOUND,"SOUND.TRE"},
    {AssetManager::TRE_MISSIONS,"MISSIONS.TRE"},
    {AssetManager::TRE_TEXTURES,"TEXTURES.TRE"}
};

void AssetManager::SetBase(const char* newBase){
    ::SetBase(newBase);
}

void AssetManager::Init(void){
    
    //Load all TRE in RAM and store them.
    for (size_t i =0 ; i < NUM_TRES; i++) {
        TreArchive* tre = new TreArchive();
        tre->InitFromFile(nameIds[i].filename);
        
        if (tre->IsValid())
            this->tres.push_back(tre);
        else{
            Game.Terminate("Unable to load asset '%s' (Did you set the SC base folder ?).",nameIds[i].filename);
            
        }
    }
}

AssetManager::AssetManager(){
    
}

AssetManager::~AssetManager(){
    
    while(!tres.empty()){
        TreArchive* tre = tres.back();
        tres.pop_back();
        delete tre;
    }
}

bool AssetManager::ReadISOImage(const std::string& isoPath) {
    std::ifstream isoFile(isoPath, std::ios::binary);
    if (!isoFile.is_open()) {
        std::cerr << "Failed to open ISO file: " << isoPath << std::endl;
        return false;
    }

    if (!ExtractFileIndex(isoFile)) {
        std::cerr << "Failed to extract file index from ISO file: " << isoPath << std::endl;
        return false;
    }

    isoFile.close();
    return true;
}

bool AssetManager::ExtractFileIndex(std::ifstream& isoFile) {
    // Seek to the start of the primary volume descriptor
    isoFile.seekg(16 * SECTOR_SIZE, std::ios::beg);
    int pvds = 16 * SECTOR_SIZE;
    // Read the primary volume descriptor
    char volumeDescriptor[2048];
    isoFile.read(volumeDescriptor, sizeof(volumeDescriptor));

    // Check if it's a primary volume descriptor
    if (volumeDescriptor[0] != 1) {
        std::cerr << "Not a primary volume descriptor" << std::endl;
        return false;
    }

    // Get the root directory record
    int rootDirRecordOffset = 156;
    // Compute start root sector from offset 156 (a word in little endian)
    int rootDirRecordLocation = *reinterpret_cast<int*>(volumeDescriptor + rootDirRecordOffset + 2);
    int rootDirRecordLength = *reinterpret_cast<int*>(volumeDescriptor + rootDirRecordOffset + 10);
    if (rootDirRecordLength == 0) {
        rootDirRecordLength = *reinterpret_cast<int*>(volumeDescriptor + 170);
    }

    // Seek to the root directory record
    isoFile.seekg(rootDirRecordLocation * SECTOR_SIZE, std::ios::beg);

    // Read the root directory record
    std::vector<char> rootDirRecord(rootDirRecordLength);
    isoFile.read(rootDirRecord.data(), rootDirRecordLength);

    // Extract the file index from the root directory record
    int offset = 5;
    while (offset < rootDirRecordLength) {
        int recordLength = static_cast<unsigned char>(rootDirRecord[offset]);
        if (recordLength == 0) {
            break;
        }

        int fileLocation = *reinterpret_cast<int*>(rootDirRecord.data() + offset + 2);
        int fileSize = *reinterpret_cast<int*>(rootDirRecord.data() + offset + 10);
        int fileFlags = static_cast<unsigned char>(rootDirRecord[offset + 25]);
        int fileNameLength = static_cast<unsigned char>(rootDirRecord[offset + 32]);
        std::string fileName(rootDirRecord.data() + offset + 33, fileNameLength);

        FileEntry entry;
        entry.isDirectory = (fileFlags & 0x02) != 0;
        if (!entry.isDirectory) {
            entry.data.resize(fileSize);
            isoFile.seekg(fileLocation * SECTOR_SIZE, std::ios::beg);
            isoFile.read(entry.data.data(), fileSize);
        }

        fileContents[fileName] = entry;

        offset += recordLength;
    }

    return true;
}

bool AssetManager::ReadFileFromISO(const std::string& fileName, std::vector<char>& fileData) {
    auto it = fileContents.find(fileName);
    if (it == fileContents.end() || it->second.isDirectory) {
        std::cerr << "File not found in ISO or is a directory: " << fileName << std::endl;
        return false;
    }

    fileData = it->second.data;
    return true;
}

void AssetManager::ListFilesInDirectory(const std::string& directory, std::vector<std::string>& files) {
    for (const auto& entry : fileContents) {
        if (entry.first.find(directory) == 0 && entry.first != directory) {
            files.push_back(entry.first);
        }
    }
}


