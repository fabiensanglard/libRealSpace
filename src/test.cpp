 //
//  test.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"



void testTRE(void){
    
    TreArchive treArchive;
    treArchive.InitFromFile("OBJECTS.TRE");
    treArchive.List(stdout);
    
}





//const char* trePath = "OBJECTS.TRE";
//const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-16DES.IFF";
void testShowAllJetTextures(const char* trePath,const char* jetPath)
{
    
    renderer.Init(640,400);
    
    // Let's open the TRE archive.
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    //treArchive.List(stdout);
    
    // Let's open the jet IFF file in that archive
    TreEntry* iffJet = treArchive.GetEntryByName(jetPath);
    
    // Oops !
    if (iffJet == NULL){
        printf("Unable to find jet '%s' in TRE archive '%s'.\n",jetPath,trePath);
        return;
    }
    
    
    
    
    
    IffLexer jetIffLexer;
    jetIffLexer.InitFromRAM(iffJet->data,iffJet->size);
    
    //Verify the object has an appearance
    if (jetIffLexer.GetChunkByID('APPR') != NULL){
        ;//printf("This object does have an appearance.\n");
    }
    else{
        printf("This object does NOT have an appearance !!\n");
        return;
    }
    
    //Render it !
    renderer.SetTitle(jetPath);
    
    
    RSEntity jet;
    jet.InitFromIFF(&jetIffLexer);
    
    printf("Model '%s' features %lu textures.\n",jetPath,jet.NumImages());
    for(size_t i = 0 ; i < jet.NumImages(); i++){
        RSImage* image = jet.images[i];
        renderer.DrawImage(image,2);
    }
        
}

void ShowAllJets(void){
    
    renderer.Init(1280,800);
    
    const char* trePath = "OBJECTS.TRE";
    
    // Let's open the TRE archive.
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    
    for (size_t i = 0 ; i < treArchive.GetNumEntries(); i++) {
        

        
        TreEntry* e = treArchive.GetEntryByID(i);

        printf("Rendering jet [%lu] '%s'.\n",i,e->name);
        
        IffLexer lexer;
        lexer.InitFromRAM(e->data,e->size);
        
        if (lexer.GetChunkByID('APPR') == NULL){
            continue;
            printf("Skipping '%s' (No APPR).\n",e->name);
        }
        
        //Render it !
        renderer.SetTitle(e->name);
        
        
        RSEntity jet;
        jet.InitFromIFF(&lexer);
        
        //Oops we don't have that number of level
        if (jet.NumLods() <= LOD_LEVEL_MAX ){
            printf("Skipping '%s' (No LOD_LEVEL_MAX).\n",e->name);
            continue;
        }

        
        
        renderer.DisplayModel(&jet,LOD_LEVEL_MAX);
    }
}

void testJet(void){
    
    const char* trePath = "OBJECTS.TRE";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\A-10.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\EJECSEAT.IFF";
    const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-16DES.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\MIRAGE.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-22.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-15.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\YF23.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\MIG21.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\MIG29.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\SU27.IFF";
    
    
    // Let's open the TRE archive.
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    treArchive.List(stdout);
    
    // Let's open the jet IFF file in that archive
    TreEntry* iffJet = treArchive.GetEntryByName(jetPath);
    
    // Oops !
    if (iffJet == NULL){
        printf("Unable to find jet '%s' in TRE archive '%s'.\n",jetPath,trePath);
        return;
    }
        
    
    
    

    IffLexer jetIffLexer;
    jetIffLexer.InitFromRAM(iffJet->data,iffJet->size);
    
    //Verify the object has an appearance
    if (jetIffLexer.GetChunkByID('APPR') != NULL){
        ;//printf("This object does have an appearance.\n");
    }
    else{
        printf("This object does NOT have an appearance !!\n");
        return;
    }
    
    //Render it !
    renderer.SetTitle(jetPath);
    
    
    RSEntity jet;
    jet.InitFromIFF(&jetIffLexer);
    renderer.DisplayModel(&jet,LOD_LEVEL_MAX);
     
}

void testPalette(void){
    
    IffLexer lexer ;
    lexer.InitFromFile("PALETTE.IFF");
    lexer.List(stdout);
    
    RSPalette palette;
    palette.InitFromIFF(&lexer);
}

void testTREDecompress(void){
    
    const char* trePath = "OBJECTS.TRE";
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    treArchive.Decompress(".");
}

void testPAKDecompress(void){
    
    const char* trePath = "GAMEFLOW.TRE";
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    
    treArchive.List(stdout);
    
    //Find the sounds PAKS.
    const char* pakName = "..\\..\\DATA\\MIDGAMES\\MID1VOC.PAK";
    TreEntry* treEntry = treArchive.GetEntryByName(pakName);
    
    
    PakArchive pakArchive;
    pakArchive.InitFromRAM(pakName,treEntry->data, treEntry->size);
    
    //Decompress it
    pakArchive.Decompress(".","VOC");
}

void testShowAllTexturesPAK(void){
    
    renderer.Init(640,400);
    
    const char* trePath = "TEXTURES.TRE";
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    
    //Find the texture PAKS.
    TreEntry* treEntry = NULL;
    const char* pakName = "..\\..\\DATA\\TXM\\TXMPACK.PAK";
    treEntry = treArchive.GetEntryByName(pakName);
    
    PakArchive txmPakArchive;
    txmPakArchive.InitFromRAM(pakName,treEntry->data, treEntry->size);
    
    RSMapTextureSet txmTextureSet ;
    txmTextureSet.InitFromPAK(&txmPakArchive);
    txmTextureSet.List(stdout);
    
    //Show all textures
    for(size_t i=0 ; i < txmTextureSet.GetNumImages() ; i++ ){
        printf("Drawing %lu.\n",i);
        RSImage* image = txmTextureSet.GetImageById(i);
        renderer.DrawImage(image,2);
    }
    
    
    const char* accPakName = "..\\..\\DATA\\TXM\\ACCPACK.PAK";
    treEntry = treArchive.GetEntryByName(accPakName);
    
    PakArchive accPakArchive;
    accPakArchive.InitFromRAM(accPakName,treEntry->data, treEntry->size);
    //accPakArchive.List(stdout);
    
    //Show all textures
    RSMapTextureSet accTextureSet ;
    accTextureSet.InitFromPAK(&accPakArchive);
    //Show all textures
    for(size_t i=0 ; i < accTextureSet.GetNumImages() ; i++ ){
        printf("Drawing %lu.\n",i);
        RSImage* image = accTextureSet.GetImageById(i);
        renderer.DrawImage(image,1);
    }
}

void testParsePAK(){
    PakArchive archive;
    archive.InitFromFile("MAURITAN.PAK");
    
    archive.List(stdout);
}

int testShowPalette(void)
{
    
    IffLexer lexer ;
    lexer.InitFromFile("PALETTE.IFF");
    lexer.List(stdout);
    
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    VGAPalette* vgaPalette = palette.GetColorPalette();
    
    
    renderer.SetTitle(lexer.GetName());
    
    renderer.ShowPalette(vgaPalette);
    
    return 0;
}


void ParseAndRenderVertices(PakEntry* entry){
    
    renderer.Init(1280,800);
    
    Vertex* vertices = new Vertex[512];
    
    ByteStream stream(entry->data);
    
    stream.ReadInt32LE();
    stream.ReadInt32LE();
    
    for (int i=0 ; i < 208; i++) {
        Vertex* v = &vertices[i];
        int32_t coo ;
        
        coo = stream.ReadInt32LE();
        v->x = (coo>>16) + (coo&0x000000FF)/65550.0;
        coo = stream.ReadInt32LE();
        v->z = (coo>>16) + (coo&0x000000FF)/65550.0;
        coo = stream.ReadInt32LE();
        v->y = (coo>>16) + (coo&0x000000FF)/65550.0;
    }
    
    //Render them
    renderer.RenderVerticeField(vertices,208);
}

void reverseEngineeMapTri(void){
    
    PakArchive archive ;
    archive.InitFromFile("MAURITAN.TRI");
    archive.List(stdout);
    //archive.Decompress("/Users/fabiensanglard/Desktop/","TXT");
    
    for (size_t i =0 ; i < archive.GetNumEntries();  i++){
        PakEntry* entry = archive.GetEntry(i);
        if (entry->size != 0){
            ParseAndRenderVertices(entry);
        }
    }
    
}

int main( int argc,char** argv){
    
    SetBase("/Users/fabiensanglard/Desktop/SC/strikecommander/SC");
    
    
   // reverseEngineeMapTri();
    
    //
    ShowAllJets();
    
    //renderer.Init(320,200);
    //testShowAllJetTextures("OBJECTS.TRE","..\\..\\DATA\\OBJECTS\\40MMAA.IFF");
    
    //renderer.Init(320,200);
    //testShowAllTexturesPAK();
}