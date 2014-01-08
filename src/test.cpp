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
        renderer.Swap();
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

void testTREDecompress(const char* dst){
    
   // const char* trePath = "GAMEFLOW.TRE";
   // const char* trePath = "MISC.TRE";
   // const char* trePath = "MISSIONS.TRE";
   // const char* trePath = "OBJECTS.TRE";
   // const char* trePath = "SOUND.TRE";
    const char* trePath = "TEXTURES.TRE";
   
    
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    treArchive.Decompress(dst);
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
    
    
    renderer.ShowWindow();
    renderer.SetTitle("TXMPACK.PAK");
    
    //Show all textures
    for(size_t i=0 ; i < txmTextureSet.GetNumImages() ; i++ ){
        printf("Drawing %lu.\n",i);
        RSImage* image = txmTextureSet.GetImageById(i);
        renderer.Clear();
        renderer.DrawImage(image,2);
        renderer.Swap();
        renderer.Pause();
        while (renderer.IsPaused()) {
            renderer.PumpEvents();
        }
    }
    
    
    
    
    const char* accPakName = "..\\..\\DATA\\TXM\\ACCPACK.PAK";
    treEntry = treArchive.GetEntryByName(accPakName);
    
    PakArchive accPakArchive;
    accPakArchive.InitFromRAM(accPakName,treEntry->data, treEntry->size);
    //accPakArchive.List(stdout);
    
        renderer.SetTitle("ACCPACK.PAK");
    
    //Show all textures
    RSMapTextureSet accTextureSet ;
    accTextureSet.InitFromPAK(&accPakArchive);
    //Show all textures
    for(size_t i=0 ; i < accTextureSet.GetNumImages() ; i++ ){
        printf("Drawing %lu.\n",i);
        RSImage* image = accTextureSet.GetImageById(i);
        renderer.Clear();
        renderer.DrawImage(image,1);
        renderer.Swap();
        renderer.Pause();
        while (renderer.IsPaused()) {
            renderer.PumpEvents();
        }
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


void DecompressAllTRE(void){
    
    TreArchive gameflow ;
    gameflow.InitFromFile("GAMEFLOW.TRE");
    gameflow.List(stdout);
    gameflow.Decompress("/Users/fabiensanglard/Desktop/");
    
    TreArchive misc ;
    misc.InitFromFile("MISC.TRE");
    misc.List(stdout);
    misc.Decompress("/Users/fabiensanglard/Desktop/");
    
    
    TreArchive missions ;
    missions.InitFromFile("MISSIONS.TRE");
    missions.List(stdout);
    missions.Decompress("/Users/fabiensanglard/Desktop/");
    
    
    TreArchive objects ;
    objects.InitFromFile("OBJECTS.TRE");
    objects.List(stdout);
    objects.Decompress("/Users/fabiensanglard/Desktop/");
    
    
    TreArchive sound ;
    sound.InitFromFile("SOUND.TRE");
    sound.List(stdout);
    sound.Decompress("/Users/fabiensanglard/Desktop/");

    TreArchive textures ;
    textures.InitFromFile("TEXTURES.TRE");
    textures.List(stdout);
    textures.Decompress("/Users/fabiensanglard/Desktop/");
}


//DELETE ME
static void showAllImageInArchive(PakArchive* archive){
    
    
    for(size_t i = 0 ; i < archive->GetNumEntries() ; i ++){
        
        printf("Show all images %lu.\n",i);
        PakEntry* entry = archive->GetEntry(i);
        
        
        
        RLECodex codex ;
        size_t byteRead;
        
        RSImage screen;
        screen.Create("SCREEN", 320, 200);
        bool errorFound = codex.ReadImage(entry->data, &screen, &byteRead);
        
        
        renderer.Clear();
        if (!errorFound){
            renderer.Pause();
            while(renderer.IsPaused()){
                
                renderer.DrawImage(&screen, 2);
                renderer.Swap();
                renderer.ShowWindow();
                renderer.PumpEvents();
            }
        }
        screen.ClearContent();
    }
    
    
}

int main( int argc,char** argv){
    
    SetBase("/Users/fabiensanglard/Desktop/SC/strikecommander/SC");

    //Render palette
    /*
    renderer.Init(512, 512);
    renderer.SetClearColor(255,0,255);
    renderer.Clear();
    renderer.ShowPalette(renderer.GetDefaultPalette());
    */
    
    
    /*
    SetBase("/Users/fabiensanglard/Desktop/ARENA.PAK.CONTENT/");
    PakArchive archiveMauritan ;
    archiveMauritan.InitFromFile("FILE1.TXT");
    archiveMauritan.Decompress("/Users/fabiensanglard/Desktop/ARENA.PAK.CONTENT/MIP_MAX/", "MIP_MAX");
    
    PakArchive f ;
    f.InitFromFile("FILE2.TXT");
    f.Decompress("/Users/fabiensanglard/Desktop/ARENA.PAK.CONTENT/MIP_MED/", "MIP_MED");
    
    PakArchive g ;
    g.InitFromFile("FILE3.TXT");
    g.Decompress("/Users/fabiensanglard/Desktop/ARENA.PAK.CONTENT/MIP_MIN/", "MIP_MIN");
    */
    
    /*
    PakArchive archiveMauritan ;
    archiveMauritan.InitFromFile("ARENA.PAK");
    archiveMauritan.Decompress("/Users/fabiensanglard/Desktop/", "TXT");
    */
    
    
    //testShowAllTexturesPAK();
    //return 0;
    
    //WIP
    /*
    renderer.Init(640, 400);
    TreArchive tre;
    tre.InitFromFile("GAMEFLOW.TRE");
    
    TreEntry* pakToExplore = tre.GetEntryByName("..\\..\\DATA\\MIDGAMES\\MID1.PAK");
    
    PakArchive arch;
    arch.InitFromRAM("MID1.PAK",pakToExplore->data,pakToExplore->size);
    
    printf("MID1.PAK" " contains %lu entries\n.",arch.GetNumEntries());
    
    for(size_t i =0 ;i < arch.GetNumEntries() ; i++){
        
        PakEntry* entry = arch.GetEntry(i);
        PakArchive a;
        a.InitFromRAM("", entry->data, entry->size);
        showAllImageInArchive(&a);
    }
    return 0;
    */
    
    
    /*
    SCMainMenu mainMenu;
    mainMenu.Init();
    mainMenu.Run();
    return 0;
    */

    /* 
    SCDogFightMenu dogFightMenu;
    dogFightMenu.Init();
    dogFightMenu.Run();
    return 0;
    */
    
    /*
    SCObjectViewer viewer;
    viewer.Init();
    viewer.Run();
    return 0;
    */
    
    /*
    PakArchive archiveCanyon ;
    archiveCanyon.InitFromFile("CANYON.PAK");
    archiveCanyon.Decompress("/Users/fabiensanglard/Desktop/", "TXT");

    PakArchive archiveMauritan ;
    archiveMauritan.InitFromFile("MAURITAN.PAK");
    archiveMauritan.Decompress("/Users/fabiensanglard/Desktop/", "TXT");

    return 0;
    */
    
    
    
    
    //testShowAllTexturesPAK();
    
    
    RSArea* area = new RSArea();

    area->InitFromPAKFileName("ARENA.PAK");
    area->InitFromPAKFileName("ALASKA.PAK");
    area->InitFromPAKFileName("EUROPE.PAK");
    // // NOT A MAP !area.InitFromPAKFileName("MSFILES.PAK");
    area->InitFromPAKFileName("SANFRAN.PAK");
    area->InitFromPAKFileName("ANDMAL1.PAK");
    area->InitFromPAKFileName("CANYON.PAK");
    // // NOT A MAP !area.InitFromPAKFileName("MAPDATA.PAK");
    area->InitFromPAKFileName("QUEBEC.PAK");
    area->InitFromPAKFileName("TURKEY.PAK");
    area->InitFromPAKFileName("ANDMAL2.PAK");
    area->InitFromPAKFileName("EGYPT.PAK");
    area->InitFromPAKFileName("MAURITAN.PAK");
    area->InitFromPAKFileName("RHODEI.PAK");
    
}