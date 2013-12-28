//
//  main.cpp
//  pak
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

using namespace std;




Entry::Entry(){
    
}

Entry::~Entry(){
    
}

void LoadPAK(const char* path,PakArchive* archive){
    
    archive->file = fopen(path,"r");
    
    if (!archive->file){
        printf("Unable to open file '%s'.\n",path);
        return ;
    }
    
    strcpy(archive->path,path);
    
    fseek(archive->file, 0L, SEEK_END);
    archive->size = (uint32_t)ftell(archive->file);
    fseek(archive->file, 0L, SEEK_SET);
    
    archive->data = (uint8_t*)malloc(archive->size);
    fread(archive->data,1, archive->size, archive->file);
    
}

/*
void AnalyseImageBlock(void){//SCBlock* block){
    
    ByteStream stream(block->data);
    
    stream.ReadUInt32LE();
    
    uint32_t size = stream.ReadUInt32LE(); //This is the offset to the next block. We don't care about this.
    
    //...but let's check that it is equals to what we calculated.
    if (size !=) block->size ){
        //Hey, this is not what we calculated earlier.
        block->size = size;
        printf("The size of this block is not what we expected.");
    }
 
    
    uint32_t nextOffset  = stream.ReadUInt32LE();
    uint32_t numElements = (nextOffset-4) / 4;
    

    for (int i=0; i<numElements; i++) {
        Element* element = new Element();
        element->data = block->data + nextOffset;
        element->offset = nextOffset;
        
        block->elements.push_back(element);
        
       
        
        nextOffset  = stream.ReadUInt32LE();
        
        if (i == (numElements-1)){
            if (numElements == 1)
                element->size = block->size-8;
            else
                element->size = block->size - block->elements[i-1]->offset ;
        }
        else
            element->size = nextOffset-element->offset;
        
        
        printf("            Element[%d] size %d.\n",i,element->size);
    }
}

void AnalyseBlock(SCBlock* block, ssize_t blockID){
    
    ByteStream stream(block->data);
    
    
    //What about Animation blocks ?
}
*/

void ParseAnimatiom(Entry* entry){
    //printf("ParseAnimatiom: Not implemented.\n");
}

void TestBlockOffets(Entry* entry){
    
    ByteStream stream = ByteStream(entry->data);
    
    
    uint32_t lastOffset = 0;
    uint32_t offset = stream.ReadUInt32LE();
    offset &= 0x00FFFFFF;
    
    uint32_t blockCounter = 0;
    while(offset < entry->size){
        
        uint32_t blockSize = offset - lastOffset;
        
        printf("        Block %u (%u bytes)\n",blockCounter,blockSize);
        
        
        //Move forward
        lastOffset = offset;
        stream = ByteStream(entry->data+offset);
        offset = stream.ReadUInt32LE();
        offset &= 0x00FFFFFF;
        
        
        blockCounter++;
    }
    
    if (lastOffset < entry->size)
        printf("        Block %u (%u bytes)\n",blockCounter,entry->size-lastOffset);
    
    //Does it ?
}

void TestAnimation(Entry* entry){
    
    //If this block is an animation, it is followed by the IFF palette
    
    ByteStream stream(entry->data);
    
    uint32_t offsetToNext = stream.ReadUInt32LE();
   
    if (offsetToNext+4 < entry->size){
        ByteStream iffStream(entry->data+offsetToNext);
        //uint8_t* header = iffStream.GetPosition();
        
        if ('F' == iffStream.ReadByte() &&
            'O' == iffStream.ReadByte() &&
            'R' == iffStream.ReadByte() &&
            'M' == iffStream.ReadByte()
            ){
            entry->type = Entry::ANIMATION;
            ParseAnimatiom(entry);
        }
    }
    
   
    
}

void TestImage(Entry* entry){
    
    ByteStream stream(entry->data);
    
    uint32_t offsetToNext = stream.ReadUInt32LE();
    
    
    
    Texture texture;
    
    texture.CreateEmpty("TEST",320, 200);
    
    bool error;
    
    size_t byteRead=0;
    error = ReadImage(stream.GetPosition(),&texture,&byteRead);
    
    if (!error){
        
        entry->type = Entry::IMAGE;
    
    
    }
    
}


void TestPAK(Entry* entry){
    
    ByteStream stream(entry->data);
    
    uint32_t size = stream.ReadUInt32LE();
    
    if (size == entry->size)
        entry->type = Entry::PAK;
}

void TestTexturesLib(Entry* entry){
    
    ByteStream stream(entry->data);
    
    uint32_t size = stream.ReadUInt32LE();
    
    if (size != entry->size)
        return;
    
    uint32_t offset = stream.ReadUInt32LE();
    offset &= 0x00FFFFFF;
    
    if (offset < entry->size)
        printf("            %d textures entries.\n",(offset-4)/4);
    
    uint32_t textureCounter = 0;
    uint32_t lastOffset = 0;
    while (offset < entry->size && offset > lastOffset) {
        
        printf("                texture[%u] size %u offset=0x%X.\n",textureCounter,offset-lastOffset,entry->offset+offset);
        lastOffset = offset;
        offset = stream.ReadUInt32LE();
        offset &= 0x00FFFFFF;
        textureCounter++;
    }
}

void TestMapTexture(Entry* entry){
    
    ByteStream stream(entry->data);
    
    uint16_t width = stream.ReadUShort();
    uint16_t height = stream.ReadUShort();
    
    uint32_t size = width*height;
    
    if (entry->size-4 == size){
        //That does look like a map texture !
        entry->type = Entry::TEXTURE ;
        printf("Entry may be a texture width=%u height=%u.\n",width,height);
    }
}

void AnalyseEntry(PakArchive* archive, uint32_t entryID){
    
    Entry* entry = &archive->entries[entryID];
    
    
    
   // uint32_t offsetsAccumulator = 0;
    
    entry->data = archive->data + entry->offset;
    
    uint32_t entryBytesRemaining = entry->size;
    
    entry->type = Entry::UNKNOWN;
    
    TestImage(entry);
    TestAnimation(entry);
    
    TestPAK(entry);
    
    if (entry->data[0] == 'F' && entry->data[1] == 'O' && entry->data[2] == 'R' && entry->data[3] == 'M' ){
        entry->type = Entry::IFF;
            printf("    Entry[%d] size: %u bytes (IFF ).\n",entryID,entry->size);
    }
    
    if (entry->data[0] == 'L' && entry->data[1] == 'I' && entry->data[2] == 'S' && entry->data[3] == 'T' ){
        entry->type = Entry::IFF;
        printf("    Entry[%d] size: %u bytes (IFF ).\n",entryID,entry->size);
    }
    
    if (entry->data[0] == 'C' && entry->data[1] == 'A' && entry->data[2] == 'T' && entry->data[3] == ' ' ){
        entry->type = Entry::IFF;
        printf("    Entry[%d] size: %u bytes (IFF ).\n",entryID,entry->size);
    }
    if(entry->data[0] == 'C' && entry->data[1] == 'r' && entry->data[2] == 'e' && entry->data[3] == 'a') {
        entry->type = Entry::VOC;
        printf("    Entry[%d] size: %u bytes (VOC ).\n",entryID,entry->size);
    }
    
    if(entry->data[0] == 'C' && entry->data[1] == 'r' && entry->data[2] == 'e' && entry->data[3] == 'a') {
        entry->type = Entry::VOC;
        printf("    Entry[%d] size: %u bytes (VOC ).\n",entryID,entry->size);
    }
    
    if (entry->type == Entry::IMAGE)
        printf("    Entry[%d] size: %u bytes (IMAGE maybe ?).\n",entryID,entry->size);
    
    if (entry->type == Entry::ANIMATION)
        printf("    Entry[%d] size: %u bytes (ANIMATION ).\n",entryID,entry->size);
    
    if (entry->type == Entry::PAK){
        printf("    Entry[%d] size: %u bytes (PAK Header or single block entry).\n",entryID,entry->size);
          TestTexturesLib(entry);
    }
    
    TestMapTexture(entry);
    if (entry->type == Entry::TEXTURE){
        printf("    Entry[%d] size: %u bytes (TEXTURE).\n",entryID,entry->size);
     //   TestTexturesLib(entry);
    }
    
    if (entry->type == Entry::UNKNOWN){
        printf("    Entry[%d] size: %u bytes (UNKNOWN ) offset 0X%X.\n",entryID,entry->size,entry->offset);
        //We have no idea what is in this entry....but could it follow the block/offset structure ?
        
        
    }
    TestBlockOffets(entry);
    
    
    //    AnalyseBlock(block);
    
    /*
    uint32_t lastBlockOffset = 0;
    uint32_t nextBlockOffset=0;
    while (entryBytesRemaining > 0) {
        
        
        SCBlock* block = new SCBlock;
        block->offset = offsetsAccumulator;
        block->data = entry->data+offsetsAccumulator;
        
        ByteStream stream(block->data);
        
        lastBlockOffset = nextBlockOffset;
        nextBlockOffset = stream.ReadUInt32LE();
        
        if (nextBlockOffset-lastBlockOffset < entryBytesRemaining){
            block->size = entryBytesRemaining;
            entry->blocks.push_back(block);
            return;
        }
        else
            block->size = nextBlockOffset - offsetsAccumulator;

        
        offsetsAccumulator+=nextBlockOffset;
        entryBytesRemaining-=nextBlockOffset;
        
        
        
        
        AnalyseBlock(block);
        
        entry->blocks.push_back(block);
        
    }
    
    // If the last block in the entry is an IFF and is a palette and size 768/769, that entry is likely
    // an animation
    if (entry->blocks.size() == 2 && entry->blocks[1]->type == SCBlock::IFF){
        
        entry->type = Entry::ANIMATION;
        
        entry->blocks[0]->type = SCBlock::ANIMATION_IMAGE;
        entry->blocks[1]->type = SCBlock::ANIMATION_PALETTE;
        
        AnalyseImageBlock(entry->blocks[0]);
    }
     */
}

void ParsePAK(PakArchive* archive){
    
    ByteStream stream(archive->data);
    
    printf("Archive name: '%s'.\n",archive->path);
    
    printf("    Size: %u bytes.\n",archive->size);
    
    uint32_t magicNumber = stream.ReadUInt32LE();
    
    if (magicNumber == archive->size)
        printf("    This is indeed a PAK archive (magic number == PAK size).\n");
    else
        printf("    #####Not Valid.\n");
    
    uint32_t firstOffset = stream.ReadUInt32LE();
    firstOffset &= 0x00FFFFFF ; //Remove the leading 0xE0
    
    archive->numEntries = (firstOffset-4)/4;
    
   
    
    archive->entries = new Entry[archive->numEntries];
    
    archive->entries[0].offset = firstOffset;
    
    for (int i =1 ; i < archive->numEntries; i++) {
        uint32_t offset = stream.ReadUInt32LE();
        offset &= 0x00FFFFFF ; //Remove the leading 0xE0
        
        archive->entries[i].offset = offset;
    }
    
    //Calculate sizes and numFiles
    archive->numDifferentOffsets = 0;
    for(int i=0 ; i < archive->numEntries-1 ; i++){
        
        archive->entries[i].size = archive->entries[i+1].offset - archive->entries[i].offset;
        if (archive->entries[i].size != 0)
            archive->numDifferentOffsets++;
    }
    
    archive->entries[archive->numEntries-1].size = archive->size - archive->entries[archive->numEntries-1].offset;
    if (archive->entries[archive->numEntries-1].size != 0)
        archive->numDifferentOffsets++;
    
    printf("    %d entries.\n",archive->numEntries);
    printf("    %d unique entries (%d duplicates) .\n",archive->numDifferentOffsets,
           archive->numEntries-archive->numDifferentOffsets);
    
    
    
    
    
    
    
}

void PAK_GuessContent(PakArchive* archive){
    for(int i=0 ; i < archive->numEntries ; i++){
        if (archive->entries[i].size != 0)
            AnalyseEntry(archive,i);
        else
            printf("    Entry[%d] DUPLICATED.\n",i);
    }
}



const char* pakList[] = {
    "/Users/fabiensanglard/Desktop/DATA/COCKPITS/BETTY.PAK",
    "/Users/fabiensanglard/Desktop/DATA/GAMEFLOW/CONV.PAK",
    "/Users/fabiensanglard/Desktop/DATA/GAMEFLOW/CONVPALS.PAK",
    "/Users/fabiensanglard/Desktop/DATA/GAMEFLOW/CONVSHPS.PAK",
    "/Users/fabiensanglard/Desktop/DATA/GAMEFLOW/MAINMENU.PAK",
    "/Users/fabiensanglard/Desktop/DATA/GAMEFLOW/OBJVIEW.PAK",
    "/Users/fabiensanglard/Desktop/DATA/GAMEFLOW/OPTPALS.PAK",
    "/Users/fabiensanglard/Desktop/DATA/GAMEFLOW/OPTSHPS.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/AMUSIC.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/ASOUNDFX.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID1.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID12.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID14.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID15.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID16.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID17.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID1VOC.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID2.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID20.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID3.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID36.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID5.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MIDGAMES.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/RMUSIC.PAK",
    "/Users/fabiensanglard/Desktop/DATA/MIDGAMES/RSOUNDFX.PAK",
    "/Users/fabiensanglard/Desktop/DATA/OBJECTS/EJECT.PAK",
    "/Users/fabiensanglard/Desktop/DATA/TXM/ACCPACK.PAK",
    "/Users/fabiensanglard/Desktop/DATA/TXM/TXMPACK.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/ALASKA.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/ANDMAL1.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/ANDMAL2.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/ARENA.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/CANYON.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/EGYPT.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/EUROPE.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/MAPDATA.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/MAURITAN.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/MSFILES.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/QUEBEC.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/RHODEI.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/SANFRAN.PAK",
    "/Users/fabiensanglard/Desktop/SC/strikecommander/SC/TURKEY.PAK"
} ;

int main(int argc, const char * argv[])
{
    
    
    /*
    for (int i=0 ; i < sizeof(pakList)/sizeof(char*) ; i++){
        PakArchive betty ;
        LoadPAK(pakList[i], &betty);
        ParsePAK(&betty);
        printf("\n");
    }
    */
    
    /*
    PakArchive betty ;
    LoadPAK("/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID1.PAK", &betty);
    ParsePAK(&betty);
    printf("\n");
    
    
    PakArchive MAURITAN ;
    LoadPAK("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/MAURITAN.PAK", &MAURITAN);
    ParsePAK(&MAURITAN);
    */
    
    
    /*
    IFFReader iffReader ;
    iffReader.InitWithFile("/Users/fabiensanglard/Desktop/DATA/TXM/TXM_LIST.IFF");
    iffReader.Parse();
    
    
    IFFPalettetReader defaultPaletteReader;
    defaultPaletteReader.InitWithFile("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/PALETTE.IFF");
    defaultPaletteReader.Parse();
    Palette* defaultPalette = defaultPaletteReader.GetObject()->GetColorPalette();
    */
    
    Palette* defaultPalette = NULL;
    
    IMG_Init();
    
    PakArchive MID1 ;
    //LoadPAK("/Users/fabiensanglard/Desktop/DATA/MIDGAMES/MID1.PAK", &MID1);
    LoadPAK("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/MAURITAN.PAK", &MID1);
    //LoadPAK("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/SANFRAN.PAK", &MID1);
    //LoadPAK("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/QUEBEC.PAK", &MID1);
    //LoadPAK("/Users/fabiensanglard/Desktop/DATA/TXM/TXMPACK.PAK", &MID1);
    //LoadPAK("/Users/fabiensanglard/Desktop/DATA/TXM/ACCPACK.PAK", &MID1);
    //LoadPAK("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/MAPDATA.PAK", &MID1);
    //LoadPAK("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/MAURITAN.TRI", &MID1);
    
    
    ParsePAK(&MID1);
    
    PAK_GuessContent(&MID1);
    
    Texture screen;
    screen.CreateEmpty("VGA",320,200);
    
   // for(int i = 5 ; i < MID1.numEntries; i++){
    for(int i = 0 ; i < MID1.numEntries; i++){
    //for(int i = 4 ; i < 6; i++){
        screen.Clear();
        Entry* entry = &MID1.entries[i];
        
        if (entry->size != 0 && (entry->type == Entry::TEXTURE )){
            
            
            ByteStream stream(entry->data);
            
            uint16_t width = stream.ReadUShort();
            uint16_t height = stream.ReadUShort();
            
            Texture texture ;
            
            texture.Set("X", width, height, stream.GetPosition());
            
            //ReadImage(entry->data, &screen,&byteRead);
            IMG_ShowImage(texture.data, texture.width, texture.height, defaultPalette , 2,true);
            // usleep(45000);

        }
        
        if (entry->size != 0 && (entry->type == Entry::IMAGE )){

            size_t byteRead;
            ReadImage(entry->data, &screen,&byteRead);
            IMG_ShowImage(screen.data, screen.width, screen.height, defaultPalette , 2,true);
            // usleep(45000);

            
           /*
            Palette* palette = NULL;
            //Find the IFF block that contains the palette
            for(int j=0; j < entry->blocks.size() ; j++){
                if (entry->blocks[j]->type == SCBlock::IFF){
                    IFFPalettetReader paletteReader;
                    paletteReader.InitWithRAM(entry->blocks[j]->data,entry->blocks[j]->size);
                    paletteReader.Parse();
                    palette = paletteReader.GetObject()->GetColorPalette();
                }
            }
            
            if (palette == NULL)
                palette = defaultPalette;
                    
            
           //
           // IMG_ShowPalette(palette, 16);
            
            //For each Element in the IMAGE block,
            for(int j=0; j < entry->blocks.size() ; j++)
            {
                
                
                SCBlock* block = entry->blocks[j];
                if (block->type == SCBlock::ANIMATION_IMAGE){
                    
                    
                    for (int k=0 ; k < block->elements.size() ; k++){
                        Element* el = block->elements[k];
                        screen.Clear();
                        printf("Playing entry %d block %d element %d.\n",i,j,k);
                        ReadImage(el->data, &screen);
                        
                        
                        IMG_ShowImage(screen.data, screen.width, screen.height, palette , 2,true);
                      // usleep(45000);
                    }
                }
                else
                    printf("NOT Playing entry %d block %d.\n",i,j);
            }
            */
        }
    }
    
    return 0;
}
