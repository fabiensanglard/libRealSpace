//
//  RSMap.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSArea.h"
#include "SCRenderer.h"

extern SCRenderer Renderer;

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
    
    
    
    
    
    
    //IffChunk* objs = lexer.GetChunkByID('OBJS');
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
        /*uint16_t fastID =*/ textureRefStrean.ReadUShort();
        char setName[8];
        for(int n=0; n < 8 ; n++){
            setName[n] = textureRefStrean.ReadByte();
        }
        /*uint8_t unknown =*/ textureRefStrean.ReadByte();
        /*uint8_t numImages =*/ textureRefStrean.ReadByte();
        
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

#define OBJ_ENTRY_SIZE 0x46
#define OBJ_ENTRY_NUM_OBJECTS_FIELD 0x2
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
    
    printf("This .OBJ features %lu entries.\n",objectFiles.GetNumEntries());
    
    
    for(size_t i = 0 ; i < objectFiles.GetNumEntries() ; i++){
        PakEntry* entry = objectFiles.GetEntry(i);
  
        if (entry->size == 0)
            continue;
        
        ByteStream sizeGetter(entry->data);
        uint16_t numObjs = sizeGetter.ReadUShort();
        printf("OBJ files %lu features %d objects.\n",i,numObjs);
        
        //if (i != 97)
        //    continue;
        
        for(int j=0 ; j < numObjs ; j++){
            
            
            
            ByteStream reader(entry->data+OBJ_ENTRY_NUM_OBJECTS_FIELD+OBJ_ENTRY_SIZE*j);
            
            MapObject mapObject;
            
            
            for(int k=0 ; k <8 ; k++)
                mapObject.name[k] = reader.ReadByte();
            mapObject.name[8] = 0;
                
            uint8_t unknown09 = reader.ReadByte();
            uint8_t unknown10 = reader.ReadByte();
            uint8_t unknown11 = reader.ReadByte();
            uint8_t unknown12 = reader.ReadByte();
            uint8_t unknown13 = reader.ReadByte();
                
                
                
            
            for(int k=0 ; k <8 ; k++)
                mapObject.destroyedName[k] = reader.ReadByte();
            mapObject.destroyedName[8] = 0;
            
            int32_t coo[12];
            coo[0] = reader.ReadByte();
            coo[1] = reader.ReadByte();
            coo[2] = reader.ReadByte();
            coo[3] = reader.ReadByte();
            mapObject.position[0] =  (coo[3] << 8) | coo[2];
            //coo[3] = (uint32_t)reader.ReadByte();

            coo[4] = reader.ReadByte();
            coo[5] = reader.ReadByte();
            coo[6] = reader.ReadByte();
            coo[7] = reader.ReadByte();
            mapObject.position[2] = (coo[7] << 8) | coo[6];

            
            coo[8] = reader.ReadByte();
            coo[9] = reader.ReadByte();
            coo[10] = reader.ReadByte();
            coo[11] = reader.ReadByte();
            mapObject.position[1] = (coo[11] << 8) | coo[10];
           

            uint8_t unknowns[0x31-12];

            for(int k=0 ; k <0x31-12; k++)
                unknowns[k] = reader.ReadByte();
                
            printf("object set [%3lu] obj [%2d] - '%-8s' %2X %2X %2X %2X %2X '%-8s'",i,j,mapObject.name,
                       unknown09,
                       unknown10,
                       unknown11,
                       unknown12,
                       unknown13,mapObject.destroyedName);
            
            for(int k=0 ; k < 12 ; k++)
                printf("%2X ",coo[k]);
            
            for(int k=0 ; k <0x31-12 ; k++)
                printf("%2X ",unknowns[k]);
            
            printf("\n");
                
            
            objects[i].push_back(mapObject);
            
        }
        
    }
    
    
}


void RSArea::ParseTriFile(PakEntry* entry){
    
    Point3D* vertices = new Point3D[300];
    
    ByteStream stream(entry->data);
    
    stream.ReadInt32LE();
    stream.ReadInt32LE();
    
    for (int i=0 ; i < 300; i++) {
        Point3D* v = &vertices[i];
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
    Renderer.RenderVerticeField(vertices,300);
    
    delete[] vertices;
}


void RSArea::ParseTrigo(){
    
    PakEntry* entry ;
    
    Renderer.Init(2);
    
    entry = archive->GetEntry(4);
    
    printf(".TRI file is %lu bytes.\n",entry->size);
    // .TRI is a PAK
    PakArchive triFiles;
    triFiles.InitFromRAM(".TRI",entry->data, entry->size);
    triFiles.List(stdout);
    //triFiles.Decompress("/Users/fabiensanglard/Desktop/MAURITAN.TRIS/","TRI");
    
    printf("Found %zu .TRI files.\n",triFiles.GetNumEntries());
    
    for(size_t i=0 ; i < triFiles.GetNumEntries() ; i++){
        
        PakEntry* entry  = triFiles.GetEntry(i);
        if (entry->size != 0)
            ParseTriFile(entry);
    }
}


#define LAND_TYPE_SEA      0
#define LAND_TYPE_DESERT   1
#define LAND_TYPE_GROUND   2
#define LAND_TYPE_SAVANNAH 3
#define LAND_TYPE_TAIGA    4
#define LAND_TYPE_TUNDRA   5
#define LAND_TYPE_SNOW     6

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
(byte & 0x80 ? 1 : 0), \
(byte & 0x40 ? 1 : 0), \
(byte & 0x20 ? 1 : 0), \
(byte & 0x10 ? 1 : 0), \
(byte & 0x08 ? 1 : 0), \
(byte & 0x04 ? 1 : 0), \
(byte & 0x02 ? 1 : 0), \
(byte & 0x01 ? 1 : 0)



//A lod features
//A block features either 25, 100 or 400 vertex
void RSArea::ParseBlocks(size_t lod,PakEntry* entry, size_t blockDim){
    
    PakArchive blocksPAL;
    blocksPAL.InitFromRAM("BLOCKS",entry->data, entry->size);
    
    for (size_t i=0; i < blocksPAL.GetNumEntries(); i++) { // Iterate over the BLOCKS_PER_MAP block entries.
        
        //SRC Asset Block
        PakEntry* blockEntry = blocksPAL.GetEntry(i);
        
        //DST custom block
        AreaBlock* block = &blocks[lod][i];
        
        block->sideSize = static_cast<int32_t>(blockDim);
        
        ByteStream vertStream(blockEntry->data);
        for(size_t vertexID=0 ; vertexID < blockDim*blockDim ; vertexID++){
            
            MapVertex* vertex = &block->vertice[vertexID];
            
            
            
            
            int16_t height ;
            height = vertStream.ReadShort();
			//height = height * 50000;
            //height /= HEIGHT_DIVIDER;
          
            vertex->flag = vertStream.ReadByte();
            vertex->type = vertStream.ReadByte();
            
            uint8_t paletteColor =0;
            
            
            // Hardcoding the values since I have no idea where those
            // are coming from. Maybe it was hard-coded in STRIKE.EXE ?
            switch (vertex->type) {
                case LAND_TYPE_SEA:
                    paletteColor = 0xA;
                break;
                case LAND_TYPE_DESERT:
                    paletteColor = 0x3;
                break;
                case LAND_TYPE_GROUND:
                   paletteColor = 0x7;
                    break;
                case LAND_TYPE_SAVANNAH:
                   paletteColor = 0x5;
                    break;
                case LAND_TYPE_TAIGA:
                    paletteColor = 0x9;
                    break;
                case LAND_TYPE_TUNDRA:
                     paletteColor = 0x1;
                    break;
                case LAND_TYPE_SNOW:
                    paletteColor = 0xC;
                    break;
                default:
                    printf("No color for type %d\n",vertex->type);
            }

            uint8_t shade =  (vertex->flag & 0x0F)  ;
            shade = shade >> 1;
            
            /*
            if (shade & 1)
                shade = 0;
            */
            
            int16_t unknown = (vertex->flag & 0xF0)  ;
            unknown = unknown >> 8;
            
            
            
            
           
            
            
            
            vertex->upperImageID    = vertStream.ReadByte();
            vertex->lowerImageID = vertStream.ReadByte();
            
            /*
            //City block
            if (blockDim == 20 && i==97){
                printf("%2X (%2X) ",vertex->lowerImageID,vertex->upperImageID );
                
                if (vertexID % 20 == 19)
                    printf("\n");
            }
            */
            
     
            Texel* t = Renderer.GetPalette()->GetRGBColor(paletteColor*16+shade);
            
            //Texel* t = renderer.GetDefaultPalette()->GetRGBColor(vertex->text);
            /*
              TODO: Figure out what are:
                    - flag high 4 bits.
                    - flag low  1 bits.
                    - textSet
                    - text
            */
            
            vertex->v.y = height*3;//-vertex->text * 10;//height ;
            
#define BLOCK_WIDTH (1000000/18)
            vertex->v.x = (i % 18 * BLOCK_WIDTH + (vertexID % blockDim ) / (float)(blockDim) * BLOCK_WIDTH) - 500000;
            vertex->v.z = (i / 18 * BLOCK_WIDTH + (vertexID / blockDim ) / (float)(blockDim) *BLOCK_WIDTH) - 500000;
           
            
            vertex->color[0] = t->r/255.0f;//*1-(vertex->z/(float)(BLOCK_WIDTH*blockDim))/2;
            vertex->color[1] = t->g/255.0f;;//*1-(vertex->z/(float)(BLOCK_WIDTH*blockDim))/2;
            vertex->color[2] = t->b/255.0f;;//*1-(vertex->z/(float)(BLOCK_WIDTH*blockDim))/2;
            vertex->color[3] = 255;
        }
        
    }

    
}


void RSArea::ParseElevations(void){
    
    PakEntry* entry ;
    
    entry = archive->GetEntry(6);
    
    ByteStream stream(entry->data);
    
    for (size_t i = 0 ; i < BLOCKS_PER_MAP; i++) {
        elevation[i]=stream.ReadUShort() ;
    }
}

void RSArea::ParseHeightMap(void){
    
    //char title[512];
    
    
    
    PakEntry* entry ;
    
    entry = archive->GetEntry(1);
    PakArchive fullPak;
    fullPak.InitFromRAM("FULLSIZE",entry->data,entry->size);
   // fullPak.List(stdout);
    ParseBlocks(BLOCK_LOD_MAX,entry,20);
    
    //renderer.RenderWorldPoints(this,BLOCK_LOD_MAX,400);

    
    
    entry = archive->GetEntry(2);
    PakArchive medPak;
    medPak.InitFromRAM("MED SIZE",entry->data,entry->size);
  //  medPak.List(stdout);
    ParseBlocks(BLOCK_LOD_MED,entry,10);
    
    
    //renderer.RenderWorldSolid(this,BLOCK_LOD_MED,100);
    
    
    entry = archive->GetEntry(3);
    PakArchive smallPak;
    smallPak.InitFromRAM("SMALSIZE",entry->data,entry->size);
 //   smallPak.List(stdout);
    ParseBlocks(BLOCK_LOD_MIN,entry,5);
    
    
    //renderer.RenderWorldSolid(this,BLOCK_LOD_MIN,25);
    
}


RSImage* RSArea::GetImageByID(size_t ID){
    
    return textures[0]->GetImageById(ID);
}

void RSArea::AddJet(TreArchive* tre, const char* name, Quaternion* orientation, Point3D* position){
    
    TreEntry* jetEntry = tre->GetEntryByName(name);
    RSEntity* entity = new RSEntity();
    IffLexer lexer;
    lexer.InitFromRAM(jetEntry->data, jetEntry->size);
    entity->InitFromIFF(&lexer);
    
    entity->orientation = *orientation;
    entity->position = *position;
    
    jets.push_back(entity);
}

void RSArea::AddJets(void){
    
    TreArchive tre;
    tre.InitFromFile("OBJECTS.TRE");
    
    Quaternion rot;
    Matrix f16m;
    f16m.Identity();
    f16m.SetRotationX(0.5f);
    
    rot.FromMatrix(&f16m);
    Point3D pos ;
    pos = {4066,95,2980};
    AddJet(&tre,"..\\..\\DATA\\OBJECTS\\F-16DES.IFF",&rot,&pos);
    
    
    f16m.SetRotationX(-0.5f);
    rot.FromMatrix(&f16m);
    pos = {4010,100,2990};
    AddJet(&tre,"..\\..\\DATA\\OBJECTS\\F-22.IFF",&rot,&pos);
    
    //pos = {3886,300,2886};
    //AddJet(&tre,"..\\..\\DATA\\OBJECTS\\MIG29.IFF",&rot,&pos);
    
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-22.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-15.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\YF23.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\MIG21.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\MIG29.IFF";

}

void RSArea::InitFromPAKFileName(const char* pakFilename){
    
    
    
    strcpy(name,pakFilename);
    
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

    //Parse the meta datas.
    ParseElevations();
    ParseMetadata();
    ParseObjects();
   // ParseTrigo();
    
    
    ParseHeightMap();
    
    
    AddJets();
    
    
}

size_t RSArea::GetNumJets(void){
    return jets.size();
}

RSEntity* RSArea::GetJet(size_t jetID){
    return jets[jetID];
}
