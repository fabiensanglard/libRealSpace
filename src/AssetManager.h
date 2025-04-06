//
//  AssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#pragma once
#ifndef __libRealSpace__AssetManager__
#define __libRealSpace__AssetManager__
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "TreArchive.h"
// Structure pour stocker le Root Directory Record (selon ISO9660)
struct RootDirectoryRecord {
    uint8_t recordLength;         // Longueur du record
    uint8_t extAttrRecordLength;  // Longueur de l'enregistrement des attributs étendus
    uint32_t extentLocation;      // LBA de début (LSB, on ignore le MSB ici)
    uint32_t dataLength;          // Taille des données (LSB)
    uint8_t recordingDate[7];     // Date et heure d'enregistrement
    uint8_t fileFlags;            // Attributs (bit 1 = répertoire, etc.)
    uint8_t fileUnitSize;         // Taille d'un bloc (pour fichiers intercalés)
    uint8_t interleaveGapSize;    // Taille du gap
    uint16_t volumeSequenceNumber;// Numéro de séquence du volume
    uint8_t fileIdentifierLength; // Longueur du nom de l'entrée
    std::string fileIdentifier;   // Nom du fichier/dossier
};
// Structure pour stocker les informations du Primary Volume Descriptor ISO9660
struct PrimaryVolumeDescriptor {
    uint8_t volumeDescriptorType;         // Offset 0 (doit être 1)
    std::string standardIdentifier;         // Offset 1-5 (doit être "CD001")
    uint8_t volumeDescriptorVersion;        // Offset 6
    std::string systemIdentifier;           // Offset 8-39 (32 octets)
    std::string volumeIdentifier;           // Offset 40-71 (32 octets)
    uint32_t volumeSpaceSize;               // Offset 80-83 (LSB)
    uint16_t volumeSetSize;                 // Offset 88-89 (LE)
    uint16_t volumeSequenceNumber;          // Offset 92-93 (LE)
    uint16_t logicalBlockSize;              // Offset 96-97 (LE)
    uint32_t pathTableSize;                 // Offset 100-103 (LE)
    uint32_t LPathTableLocation;            // Offset 108-111
    RootDirectoryRecord rootDirectoryRecord;// Root Directory Record (début à l'offset 156)
    std::string volumeSetIdentifier;        // Offset 190-317 (128 octets)
    std::string publisherIdentifier;        // Offset 318-445 (128 octets)
    std::string dataPreparerIdentifier;     // Offset 446-573 (128 octets)
    std::string applicationIdentifier;      // Offset 574-701 (128 octets)
    // D'autres champs (copyright, dates, etc.) peuvent être ajoutés au besoin.
};

// Structure de stockage d'une entrée de fichier (ou répertoire)
struct FileEntry {
    bool isDirectory;
    int position;   // LBA de début dans l'image (en secteurs)
    int size;       // Taille en octets
    // Vous pouvez ajouter d'autres champs si besoin (dates, attributs, etc.)
};

struct FileData {
    uint8_t* data;
    size_t size;
};

class AssetManager{
    
public:
    void SetBase(const char* base);
    void Init(void);
    void Init(std::vector<std::string> nameIds);
    enum TreID {TRE_GAMEFLOW, TRE_OBJECTS, TRE_MISC, TRE_SOUND, TRE_MISSIONS,TRE_TEXTURES } ;
    
    std::vector<TreArchive*> tres;
    TreEntry *GetEntryByName(std::string name);
    AssetManager();
    ~AssetManager();
    bool ReadISOImage(const std::string& isoPath);
    FileData *GetFileData(const std::string fileName);

private:
    bool isIsoImage{false};
    std::string *basePath;
    std::map<std::string, FileData*> cacheFileData;
    std::map<std::string, TreEntry *> treEntries;
    std::map<std::string, FileEntry> fileContents;
    bool ExtractPrimaryVolumeDescriptor(std::ifstream &isoFile, PrimaryVolumeDescriptor &pvd);
    bool ExtractFileListFromRootDirectory(std::ifstream &isoFile, const PrimaryVolumeDescriptor &pvd);
    FileData *ReadFileEntry(const FileEntry &entry, const std::string &isoPath);
};
#endif /* defined(__libRealSpace__AssetManager__) */
