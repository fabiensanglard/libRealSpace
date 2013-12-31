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

void testJet(void){
    
    const char* trePath = "OBJECTS.TRE";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\A-10.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\EJECSEAT.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-16DES.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\MIRAGE.IFF";
    const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-22.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\F-15.IFF";
    //const char* jetPath = "..\\..\\DATA\\OBJECTS\\YF23.IFF";
    
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
    //txmTextureSet.List(stdout);
    
    //Show all textures
    for(size_t i=0 ; i < txmTextureSet.GetNumTextures() ; i++ ){
        printf("Drawing %lu.\n",i);
        Texture* texture = txmTextureSet.GetTextureById(i);
        renderer.DrawImage(texture->data,texture->width, texture->height,Renderer::USE_DEFAULT_PALETTE,2);
    }
    
    
    const char* accPakName = "..\\..\\DATA\\TXM\\ACCPACK.PAK";
    treArchive.GetEntryByName(accPakName);
    
    PakArchive accPakArchive;
    accPakArchive.InitFromRAM(accPakName,treEntry->data, treEntry->size);
    //accPakArchive.List(stdout);
    
    //Show all textures
    RSMapTextureSet accTextureSet ;
    accTextureSet.InitFromPAK(&txmPakArchive);
    //Show all textures
    for(size_t i=0 ; i < accTextureSet.GetNumTextures() ; i++ ){
        printf("Drawing %lu.\n",i);
        Texture* texture = accTextureSet.GetTextureById(i);
        renderer.DrawImage(texture->data,texture->width, texture->height,Renderer::USE_DEFAULT_PALETTE,2);
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


int main( int argc,char** argv){
    
    SetBase("/Users/fabiensanglard/Desktop/SC/strikecommander/SC");
    
    renderer.Init(1280,800);
    testJet();
    
    //renderer.Init(320,200);
    //testShowAllTexturesPAK();
}