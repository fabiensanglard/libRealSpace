//
//  RSMap.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


RSArea::RSArea(){
    
    
}

RSArea::~RSArea(){
    delete archive;
    
    while(!textures.empty()){
        RSMapTextureSet* set = textures.back();
        textures.pop_back();
        delete set;
    }
}


void RSArea::ParseMetadata(){
    
    //Meta are in the first PAK file.
    
    PakEntry* entry = archive->GetEntry(0);
    
    IffLexer lexer;
    lexer.InitFromRAM(entry->data, entry->size);
    lexer.List(stdout);
    
    IffChunk* tera = lexer.GetChunkByID('TERA');
    if (tera == NULL) {
        printf("Error while parsing Metadata: Cannot find a TERA chunk in first file: Is this really an AREA PAK ?!?!?\n");
        return;
    }

    
    //Parse BLOX containing the Elevation, Triangles and OBJS.
    IffChunk* blox = lexer.GetChunkByID('BLOX');
    if (blox == NULL) {
        printf("Error while parsing Metadata: Cannot find a BLOX chunk.\n");
        return;
    }
    
    IffChunk* elev = lexer.GetChunkByID('ELEV');
    
    IffChunk* atri = lexer.GetChunkByID('ATRI');
    
    IffChunk* objs = lexer.GetChunkByID('OBJS');
    /*
     OBJS format:
      4 bytes unknown :
      4 bytes unknown :
      4 bytes unknown :
      14 bytes OBJ file name
     */

    
    
    
    IffChunk* txms = lexer.GetChunkByID('TXMS');
    if (txms == NULL) {
        printf("Error while parsing Metadata: Cannot find a TXMS chunk.\n");
        return;
    }
    
    /*
        TXMS format:
            One INFO chunk
            X MAPS textures entries
                    2 byte  : fast lookup ID (from TXM pack);
                    8 bytes : Set Name (from TXM pack);
                    1 byte  : unknown
                    1 byte  : num textures in that set
     
     */
     
    
    
    
}

void RSArea::ParseObjects(){
    /*
         The OBJ file seems to have a pattern: 
     
     It is a PAK archive
     
     For each entry in the PAK
     
        short: num records
           X records of length 0x46
     
               Record format :
               7 bytes for name
               1 bytes unknown (sometimes 0x00 sometimes 0xC3
               4 bytes unknown
               12 bytes for coordinates ?
     
     
    */
    PakEntry* objectsFilesLocation = archive->GetEntry(5);
    
    PakArchive objectFiles;
    objectFiles.InitFromRAM("PAK Objects from RAM",objectsFilesLocation->data, objectsFilesLocation->size);
    
    printf("This PAK features %lu OBJ files.\n",objectFiles.GetNumEntries());
    for(size_t i = 0 ; i < objectFiles.GetNumEntries() ; i++){
        PakEntry* entry = objectFiles.GetEntry(i);
        
        if (entry->size != 0){
            ByteStream reader(entry->data);
            uint16_t numObjs = reader.ReadUShort();
            printf("OBJ files %lu features %d objects.\n",i,numObjs);
        }
    }
    
}

void RSArea::ParseTrigo(){
    
}

void RSArea::InitFromPAKFileName(const char* pakFilename){
    
    
    //Check the PAK has 5 entries
    this->archive = new PakArchive();
    this->archive->InitFromFile(pakFilename);
    
    //Check that we have 6 entries.
    if (archive->GetNumEntries() != 7){
        printf("***Error: An area PAK file should have 7 files:\n");
        printf("        - IFF file.\n");
        printf("        - 1 file containing records of size 2400.\n");
        printf("        - 1 file containing records of size 600 (mipmaps of the 2400 records ?)\n");
        printf("        - 1  file containing records of size 150 (mipmaps of the 600  records ?)\n");
        printf("        - 1 file containing the map 3D data (MAURITAN.TRI).\n");
        printf("        - 1 file containing the objects locations on the map (MAURITAN.OBJ).\n");
        printf("        - 1 file containing MAURITAN.AVG (I assume Average of something ?)\n");
        return;
    }
    
    //Parse the meta datas.
    ParseMetadata();
    ParseObjects();
    ParseTrigo();
    
    
    //Load the textures from the PAKs (TXMPACK.PAK and ACCPACK.PAK) within TEXTURES.TRE.
    /*
          Note: This is the bruteforce approach and not very good:
                I feel like the right way would be to be able to parse the AREA info
                (the first IFF entry which seems to contain textures references.)
                And load only those from the TXM PACK. This is probably how they did 
                it on a machine with only 4MB of RAM.
     
    */
    const char* trePath = "TEXTURES.TRE";
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    
   
    
    
    
    
    //Find the texture PAKS.
    TreEntry* treEntry = NULL;
    RSMapTextureSet* set;
    
    
    
    
    
    const char* txmPakName = "..\\..\\DATA\\TXM\\TXMPACK.PAK";
    treEntry = treArchive.GetEntryByName(txmPakName);
    PakArchive txmPakArchive;
    txmPakArchive.InitFromRAM(txmPakName,treEntry->data, treEntry->size);
    set = new RSMapTextureSet();
    set->InitFromPAK(&txmPakArchive);
    textures.push_back(set);
    
    
    
    //ACCPACK.PAK seems to contain runway textures
    const char* accPakName = "..\\..\\DATA\\TXM\\ACCPACK.PAK";
    treEntry = treArchive.GetEntryByName(accPakName);
    PakArchive accPakArchive;
    accPakArchive.InitFromRAM(accPakName,treEntry->data, treEntry->size);
    set = new RSMapTextureSet();
    set->InitFromPAK(&accPakArchive);
    textures.push_back(set);


}
