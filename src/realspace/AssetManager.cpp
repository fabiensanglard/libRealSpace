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
#include <string>
#include "AssetManager.h"

#ifndef _WIN32
size_t fread_s(void *buffer, size_t bufferSize, size_t elementSize, size_t elementCount, FILE *stream) {
    if (buffer == NULL || stream == NULL) {
        fprintf(stderr, "Invalid buffer or stream.\n");
        return 0;
    }

    if (bufferSize < elementSize * elementCount) {
        fprintf(stderr, "Buffer size is too small.\n");
        return 0;
    }

    return fread(buffer, elementSize, elementCount, stream);
}
int fopen_s(FILE **file, const char *filename, const char *mode) {
    if (file == NULL || filename == NULL || mode == NULL) {
        fprintf(stderr, "Invalid arguments to fopen_s.\n");
        return EINVAL; // Invalid argument error code
    }

    *file = fopen(filename, mode);
    if (*file == NULL) {
        perror("Failed to open file");
        return errno; // Return the error code from errno
    }

    return 0; // Success
}
#endif


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
    this->basePath = new std::string(newBase);
}

void AssetManager::init(void){
    
    //Load all TRE in RAM and store them.
    for (size_t i =0 ; i < NUM_TRES; i++) {
        TreArchive* tre = new TreArchive();
        tre->InitFromFile(nameIds[i].filename);
        
        if (tre->IsValid()) {
            this->tres.push_back(tre);
            for (auto treEntry : tre->entries) {
                treEntries[treEntry->name] = treEntry;
            }
        } else {
            printf("Unable to load asset '%s' (Did you set the SC base folder ?).",nameIds[i].filename);
            exit(-1);
        }
    }
}
void AssetManager::init(std::vector<std::string> nameIds){
    this->tres.clear();
    this->tres.shrink_to_fit();
    this->treEntries.clear();
    this->fileContents.clear();

    //Load all TRE in RAM and store them.
    for (auto nameId : nameIds) {
        TreArchive* tre = new TreArchive();
        FileData *fileData = GetFileData(nameId);
        if (fileData == nullptr) {
            printf("Unable to load asset '%s' (Did you set the SC base folder ?).\n",nameId.c_str());
            exit(-1);
        }
        tre->InitFromRAM(nameId.c_str(), fileData->data, fileData->size);
        
        if (tre->IsValid()) {
            this->tres.push_back(tre);
            for (auto treEntry : tre->entries) {
                std::string *name = new std::string(treEntry->name);
                if (treEntries.find(*name) != treEntries.end()) {
                    printf("Duplicate entry '%s' in TRE '%s'.\n",name->c_str(),nameId.c_str());
                    //exit(-1);
                }
                auto res = treEntries.insert(std::pair<std::string,TreEntry*>(*name,treEntry));
                if (!res.second) {
                    printf("ERROR INSERTING name %s", name->c_str());
                }
            }

        } else {
            printf("Unable to load asset '%s' (Did you set the SC base folder ?).\n",nameId.c_str());
            exit(-1);
        }
    }
}
TreEntry * AssetManager::GetEntryByName(std::string name){
    if (treEntries.find(name) == treEntries.end()) {
        return nullptr;
    }
    return treEntries[name];
}
AssetManager::AssetManager(){
    
}

AssetManager::~AssetManager(){
    for (auto file_cache: this->cacheFileData) {
        delete file_cache.second;
        file_cache.second = nullptr;
    }
    this->cacheFileData.clear();
    this->treEntries.clear();
    //this->tres.clear();

}

bool AssetManager::ReadISOImage(const std::string& isoPath) {
    this->cacheFileData.clear();
    std::ifstream isoFile(isoPath, std::ios::binary);
    if (!isoFile.is_open()) {
        std::cerr << "Failed to open ISO file: " << isoPath << std::endl;
        return false;
    }
    PrimaryVolumeDescriptor pvd; 
    if (!ExtractPrimaryVolumeDescriptor(isoFile, pvd)) {
        std::cerr << "Failed to extract file index from ISO file: " << isoPath << std::endl;
        return false;
    }
    
    if (!ExtractFileListFromRootDirectory(isoFile, pvd)) {
        std::cerr << "Failed to extract file list from ISO file: " << isoPath << std::endl;
        return false;
    }
    isoFile.close();
    for (auto entry : this->fileContents) {
        if (!entry.second.isDirectory) {
            FileData *fileData = ReadFileEntry(entry.second, isoPath);
            if (fileData != nullptr) {
                this->cacheFileData[entry.first] = fileData;
            }
        }
    }
    return true;
}


// Structure pour stocker les informations du Primary Volume Descriptor ISO9660

bool AssetManager::ExtractPrimaryVolumeDescriptor(std::ifstream &isoFile, PrimaryVolumeDescriptor &pvd) {
    const int rawSectorSize = 2352;
    const int headerSize = 16;
    const int userDataSize = 2048; // Taille logique ISO9660
    
    // Positionner le flux sur le secteur 16 (16 * rawSectorSize)
    isoFile.seekg(16 * rawSectorSize, std::ios::beg);
    
    // Lire le secteur RAW complet (2352 octets)
    char rawSector[2352];
    isoFile.read(rawSector, rawSectorSize);
    if (isoFile.gcount() != rawSectorSize) {
        std::cerr << "Erreur de lecture du secteur RAW du volume descriptor." << std::endl;
        return false;
    }
    
    // Extraire la zone user data de 2048 octets (après les 16 octets d'en-tête)
    char volumeData[2048];
    memcpy(volumeData, rawSector + headerSize, userDataSize);
    
    // Remplissage des champs du Primary Volume Descriptor
    pvd.volumeDescriptorType = static_cast<uint8_t>(volumeData[0]);
    pvd.standardIdentifier = std::string(volumeData + 1, 5);
    pvd.volumeDescriptorVersion = static_cast<uint8_t>(volumeData[6]);
    
    if (pvd.volumeDescriptorType != 1 || pvd.standardIdentifier != "CD001") {
        std::cerr << "Ce n'est pas un Primary Volume Descriptor valide." << std::endl;
        return false;
    }
    
    // Fonction lambda pour supprimer les espaces en début/fin de chaîne
    auto trim = [](const std::string &s) -> std::string {
        size_t start = s.find_first_not_of(' ');
        size_t end = s.find_last_not_of(' ');
        return (start == std::string::npos ? "" : s.substr(start, end - start + 1));
    };
    
    pvd.systemIdentifier = trim(std::string(volumeData + 8, 32));
    pvd.volumeIdentifier = trim(std::string(volumeData + 40, 32));
    
    memcpy(&pvd.volumeSpaceSize, volumeData + 80, 4);
    memcpy(&pvd.volumeSetSize, volumeData + 88, 2);
    memcpy(&pvd.volumeSequenceNumber, volumeData + 92, 2);
    memcpy(&pvd.logicalBlockSize, volumeData + 96, 2);
    memcpy(&pvd.pathTableSize, volumeData + 100, 4);
    memcpy(&pvd.LPathTableLocation, volumeData + 108, 4);
    
    // Parsing du Root Directory Record situé à l'offset 156 dans volumeData
    {
        const char* rdr = volumeData + 156;
        RootDirectoryRecord root;
        root.recordLength = static_cast<uint8_t>(rdr[0]);
        root.extAttrRecordLength = static_cast<uint8_t>(rdr[1]);
        memcpy(&root.extentLocation, rdr + 2, 4);  // Position d'extent (LSB)
        memcpy(&root.dataLength, rdr + 10, 4);       // Taille des données (LSB)
        memcpy(root.recordingDate, rdr + 18, 7);
        root.fileFlags = static_cast<uint8_t>(rdr[25]);
        root.fileUnitSize = static_cast<uint8_t>(rdr[26]);
        root.interleaveGapSize = static_cast<uint8_t>(rdr[27]);
        memcpy(&root.volumeSequenceNumber, rdr + 28, 2);
        root.fileIdentifierLength = static_cast<uint8_t>(rdr[32]);
        root.fileIdentifier = std::string(rdr + 33, root.fileIdentifierLength);
        
        pvd.rootDirectoryRecord = root;
    }
    
    pvd.volumeSetIdentifier = trim(std::string(volumeData + 190, 128));
    pvd.publisherIdentifier = trim(std::string(volumeData + 318, 128));
    pvd.dataPreparerIdentifier = trim(std::string(volumeData + 446, 128));
    pvd.applicationIdentifier = trim(std::string(volumeData + 574, 128));
    
    return true;
}

bool AssetManager::ExtractFileListFromRootDirectory(std::ifstream &isoFile, const PrimaryVolumeDescriptor &pvd) {
    // Constantes : secteur RAW de 2352 octets, header de 16 => données utiles de 2048 octets
    const int rawSectorSize = 2352;
    const int headerSize = 16;
    const int userDataSize = 2048; // taille logique ISO9660
    
    // Le Root Directory Record du PVD indique la position (en blocs logiques) et la taille totale en octets de la zone user data
    int rootSector = pvd.rootDirectoryRecord.extentLocation;
    int dataLength = pvd.rootDirectoryRecord.dataLength;
    
    // Calcul du nombre de secteurs RAW à lire afin d'obtenir dataLength octets de données utiles
    int numSectors = (dataLength + userDataSize - 1) / userDataSize;
    
    // Lire et assembler les données utilisateur de chacun des secteurs RAW
    std::vector<char> rootDirData;
    rootDirData.reserve(numSectors * userDataSize);
    
    // Positionner le flux sur le début du répertoire racine dans l'image RAW.
    // Chaque secteur RAW est de 2352 octets.
    isoFile.seekg(rootSector * rawSectorSize, std::ios::beg);
    
    for (int i = 0; i < numSectors; i++) {
        char rawSector[2352];
        isoFile.read(rawSector, rawSectorSize);
        if (isoFile.gcount() != rawSectorSize) {
            std::cerr << "Erreur lors de la lecture du secteur " << i << " du répertoire racine." << std::endl;
            return false;
        }
        // Ajouter la portion de données utiles (les 2048 octets après les 16 octets d'en-tête)
        rootDirData.insert(rootDirData.end(), rawSector + headerSize, rawSector + headerSize + userDataSize);
    }
    
    // Tronquer le buffer s'il dépasse dataLength octets
    if (static_cast<int>(rootDirData.size()) > dataLength) {
        rootDirData.resize(dataLength);
    }
    
    // Parcours des enregistrements ISO9660 dans le buffer assemblé.
    int offset = 0;
    while (offset < dataLength) {
        uint8_t recordLen = static_cast<uint8_t>(rootDirData[offset]);
        if (recordLen == 0) {
            // Un record de longueur nulle : passer à l'alignement du prochain bloc logique
            offset = ((offset / userDataSize) + 1) * userDataSize;
            continue;
        }
        if (offset + recordLen > static_cast<int>(rootDirData.size()))
            break;
        
        // Extraction des informations essentielles selon ISO9660 :
        // - Octets 2 à 5 : Extent Location (LBA) en little-endian
        // - Octets 10 à 13 : Data Length (taille du fichier, little-endian)
        // - Octet 25 : File Flags (bit 1 = répertoire)
        // - Octet 32 : File Identifier Length
        // - À partir de l'octet 33 : File Identifier (nom)
        int fileLocation = *reinterpret_cast<const uint32_t*>(rootDirData.data() + offset + 2);
        int fileSize = *reinterpret_cast<const uint32_t*>(rootDirData.data() + offset + 10);
        uint8_t fileFlags = static_cast<uint8_t>(rootDirData[offset + 25]);
        uint8_t fileNameLen = static_cast<uint8_t>(rootDirData[offset + 32]);
        std::string fileName(rootDirData.data() + offset + 33, fileNameLen);
        
        FileEntry entry;
        entry.isDirectory = (fileFlags & 0x02) != 0;
        // La position est donnée en blocs logiques (2048 octets chacun)
        entry.position = fileLocation;
        entry.size = fileSize;
        
        fileContents[fileName] = entry;
        offset += recordLen;
    }
    
    return true;
}

FileData* AssetManager::ReadFileEntry(const FileEntry &entry, const std::string &isoPath) {
    FileData* fileData = new FileData();
    fileData->size = entry.size;
    fileData->data = new uint8_t[fileData->size];

    std::ifstream isoFile(isoPath, std::ios::binary);
    if (!isoFile.is_open()) {
        std::cerr << "Failed to open ISO file: " << isoPath << std::endl;
        delete[] fileData->data;
        delete fileData;
        return nullptr;
    }

    const int sectorSize = 2352;
    const int headerSize = 16;
    const int userDataSize = 2048;

    // Compute the number of sectors needed to cover the file data.
    int numSectors = ((int)fileData->size + userDataSize - 1) / userDataSize;
    int bytesCopied = 0;

    for (int i = 0; i < numSectors; i++) {
        // The file position in the ISO is given in logical sectors.
        // For a raw image, each logical sector occupies sectorSize bytes.
        int sectorOffset = (entry.position + i) * sectorSize;
        isoFile.seekg(sectorOffset, std::ios::beg);

        char sectorBuffer[2352];
        isoFile.read(sectorBuffer, sectorSize);
        if (isoFile.gcount() != sectorSize) {
            std::cerr << "Failed to read sector " << (entry.position + i) << " from ISO." << std::endl;
            delete[] fileData->data;
            delete fileData;
            return nullptr;
        }

        // Determine how many bytes to copy from this sector.
        int bytesToCopy = userDataSize;
        if (bytesCopied + bytesToCopy > fileData->size) {
            bytesToCopy = (int)fileData->size - bytesCopied;
        }
        memcpy(fileData->data + bytesCopied, sectorBuffer + headerSize, bytesToCopy);
        bytesCopied += bytesToCopy;
    }

    isoFile.close();
    return fileData;
}
void AssetManager::writeFileData(const std::string fileName, FileData *fileData) {
    std::ofstream out(fileName, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << fileName << std::endl;
        return;
    }
    out.write(reinterpret_cast<const char*>(fileData->data), fileData->size);
    out.close();
}

FileData * AssetManager::GetFileData(std::string filename) {
    FileData *fileData;
    if (cacheFileData.find(filename) == cacheFileData.end()) {
        fileData = new FileData();
        fileData->data = nullptr;
        fileData->size = 0;
        FILE *file_descriptor;
        std::string final_path_name = *this->basePath +"/"+ filename;
        fopen_s(&file_descriptor, final_path_name.c_str(), "rb");
        if (file_descriptor == nullptr) {
            std::cerr << "Failed to open file: " << final_path_name << std::endl;
            return nullptr;
        }
        fseek(file_descriptor, 0, SEEK_END);
        fileData->size = ftell(file_descriptor);
        fseek(file_descriptor, 0, SEEK_SET);
        fileData->data = new uint8_t[fileData->size];
        fread_s(fileData->data, fileData->size, 1, fileData->size, file_descriptor);
        fclose(file_descriptor);
        cacheFileData[filename] = fileData;
    } else {
        fileData = cacheFileData[filename];
    }
    return fileData;
}

