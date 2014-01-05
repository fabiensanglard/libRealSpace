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
    
    printf("Parsing file[0] (Metadatas)\n");
    
    PakEntry* entry = archive->GetEntry(0);
    
    IffLexer lexer;
    lexer.InitFromRAM(entry->data, entry->size);
    //lexer.List(stdout);
    
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
    
    
    
    /*
     The content of the ELEV chunk is the same across ALL MAPS
     Only RHODEI.ELV    RHODEI.AVG
          RHODEI.MED    RHODEI.AVG 
          RHODEI.LOW    RHODEI.AVG
     
     At the end change. RHODEI / CANYON / QUEBEC / MAURITAN etc.....
     */
    
    //Elevation format entry is 46 bytes long:
    
    // 2 bytes 0F 00
    // 2 bytes
    // 4 bytes 08 00 00 00
    // 2 bytes
    // 4 bytes 00 00 12 00
    // 1 byte
    // 4 bytes 00 20 4e 00 00
    
    //13 bytes: A filename
    //13 bytes: An other filename
    
    IffChunk* elev = lexer.GetChunkByID('ELEV');
    printf("Content of elevation chunk:\n");
    size_t numEleRecords = elev->size / 46;
    ByteStream elevStream(elev->data);
    for(size_t e=0 ; e < numEleRecords ; e++)
    {
        
        printf("elev record [%zu] ",e);
        uint8_t unknownsElev[20];
        for(int i=0; i < 20 ; i++)
            unknownsElev[i] = elevStream.ReadByte();
    
        char elevName[14];
        for(int i=0; i < 13 ; i++)
            elevName[i] = elevStream.ReadByte();
        elevName[13]  = 0;
    
        char elevOtherName[14];
        for(int i=0; i < 13 ; i++)
            elevOtherName[i] = elevStream.ReadByte();
        elevOtherName[13]  = 0;
        
        for (int i=0; i<20 ; i++){
            printf("%2X ",unknownsElev[i]);
        }
        
        printf("%-13s %-13s \n",elevName,elevOtherName);
    }
    
   
    
    
    IffChunk* atri = lexer.GetChunkByID('ATRI');
    printf("Content of trigo chunk:\n");
    
    ByteStream triStream(atri->data);
    for (int i=0; i < 40; i++) {
        printf(" %2X",triStream.ReadByte());
    }
    char triFileName[13];
    for (int i=0; i < 13; i++)
        triFileName[i] = triStream.ReadByte();
    printf(" '%-13s' \n",triFileName);
    
    
    
    
    
    
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
    
    IffChunk* txmsInfo = txms->childs[0];
    if (txmsInfo->id != 'INFO'){
        printf("Error: First child in TXMS is not an INFO chunk ?!\n");
        return;
    }
    
    IffChunk* txmsMaps = txms->childs[1];
    if (txmsMaps->id != 'MAPS'){
        printf("Error: Second child in TXMS is not an MAP chunk ?!\n");
        return;
    }
    
    //Num texture sets
    size_t numTexturesSets = txmsMaps->size/12;
    //printf("This area features %lu textureSets references.\n",numTexturesSets);
    
    ByteStream textureRefStrean(txmsMaps->data);
        
    for (size_t i=0; i < numTexturesSets ; i++) {
        uint16_t fastID = textureRefStrean.ReadUShort();
        char setName[8];
        for(int n=0; n < 8 ; n++){
            setName[n] = textureRefStrean.ReadByte();
        }
        uint8_t unknown = textureRefStrean.ReadByte();
        uint8_t numImages = textureRefStrean.ReadByte();
        
       // printf("Texture Set Ref [%3lu] 0x%2X[%-8s] %02X (%2u files).\n",i,fastID,setName,unknown,numImages);
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
    
    printf("Parsing file[5] (Objects)\n");
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
    
    size_t numObjects=0;
    for(size_t i = 0 ; i < objectFiles.GetNumEntries() ; i++){
        PakEntry* entry = objectFiles.GetEntry(i);
        
        if (entry->size != 0){
            ByteStream reader(entry->data);
            uint16_t numObjs = reader.ReadUShort();
            printf("OBJ files %lu features %d objects.\n",i,numObjs);
            
            for(int j=0 ; j < numObjs ; j++){
                
                
                char setName[9];
                for(int k=0 ; k <8 ; k++)
                    setName[k] = reader.ReadByte();
                setName[8] = 0;
                
                uint8_t unknown09 = reader.ReadByte();
                uint8_t unknown10 = reader.ReadByte();
                uint8_t unknown11 = reader.ReadByte();
                uint8_t unknown12 = reader.ReadByte();
                uint8_t unknown13 = reader.ReadByte();
                
                
                
                char accessoryName[9];
                for(int k=0 ; k <8 ; k++)
                    accessoryName[k] = reader.ReadByte();
                accessoryName[8] = 0;
                
                
                uint8_t unknowns[0x31];

                for(int k=0 ; k <0x31 ; k++)
                    unknowns[k] = reader.ReadByte();
                
                printf("object set [%3lu] obj [%2d] - '%-8s' %2X %2X %2X %2X %2X '%-8s'",i,j,setName,
                       unknown09,
                       unknown10,
                       unknown11,
                       unknown12,
                       unknown13,accessoryName);
                
                for(int k=0 ; k <0x31 ; k++)
                    printf("%2X ",unknowns[k]);
            
                printf("\n");
                
            }
            
            numObjects+=numObjs;
        }
    }
    printf("This Area features %lu Objects.\n",numObjects);
    
}


void RSArea::ParseTriFile(PakEntry* entry){
    
    Vertex* vertices = new Vertex[832];
    
    ByteStream stream(entry->data);
    
    stream.ReadInt32LE();
    stream.ReadInt32LE();
    
    for (int i=0 ; i < 832; i++) {
        Vertex* v = &vertices[i];
        int32_t coo ;
        
        coo = stream.ReadInt32LE();
        v->x = (coo>>8) + (coo&0x000000FF)/255.0;
        v->x /= 2000;
        coo = stream.ReadInt32LE();
        v->z = (coo>>8) + (coo&0x000000FF)/255.0;
        v->z /= 2000;
        
        coo = stream.ReadInt32LE();
        v->y =   (coo>>8) + (coo&0x000000FF)/255.0;
        v->y /= 2000;
    }
    
    //Render them
    renderer.RenderVerticeField(vertices,832);
}


void RSArea::ParseTrigo(){
    
    PakEntry* entry ;
    
    renderer.Init(640, 400);
    
    entry = archive->GetEntry(4);
    
    printf(".TRI file is %lu bytes.\n",entry->size);
    // .TRI is a PAK
    PakArchive triFiles;
    triFiles.InitFromRAM(".TRI",entry->data, entry->size);
    triFiles.List(stdout);
    //triFiles.Decompress("/Users/fabiensanglard/Desktop/MAURITAN.TRIS/","TRI");
    
    printf("Found %d .TRI files.\n",triFiles.GetNumEntries());
    
    for(size_t i=0 ; i < triFiles.GetNumEntries() ; i++){
        
        PakEntry* entry  = triFiles.GetEntry(i);
        if (entry->size != 0)
            ParseTriFile(entry);
    }
}


void RSArea::ParseMystery(void){
    
    PakEntry* entry ;
    
    entry = archive->GetEntry(1);
    PakArchive fullPak;
    fullPak.InitFromRAM("FULLSIZE",entry->data,entry->size);
    fullPak.List(stdout);
    
    entry = archive->GetEntry(2);
    PakArchive medPak;
    medPak.InitFromRAM("MED SIZE",entry->data,entry->size);
    medPak.List(stdout);

    
    entry = archive->GetEntry(3);
    PakArchive smallPak;
    smallPak.InitFromRAM("SMALSIZE",entry->data,entry->size);
    smallPak.List(stdout);

    
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
        printf("        - 1 file containing the objects locations on the map (MAURITAN.OBJ).\n");      //COMPLETELY REVERSE ENGINEERED !!!!
        printf("        - 1 file containing MAURITAN.AVG (I assume Average of something ?)\n");        
        return;
    }
    
    //Parse the meta datas.
    //ParseMetadata();
    //ParseObjects();
    ParseTrigo();
    //ParseMystery();
    
    
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
