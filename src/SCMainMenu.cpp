//
//  SCMainMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCMainMenu.h"



SCMainMenu::SCMainMenu(){
    
}

SCMainMenu::~SCMainMenu(){
    
}

//DELETE ME
static void showAllImage(PakArchive* archive){
    
    
    
    
    
    
    
    for(size_t i = 0 ; i < archive->GetNumEntries() ; i ++){
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

void SCMainMenu::Init(void){
    
    TreArchive archive ;
    archive.InitFromFile("GAMEFLOW.TRE");
    
    TreEntry* entry = archive.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\MAINMENU.PAK");
    
    PakArchive pak;
    pak.InitFromRAM("MAINMENU.PAK",entry->data,entry->size);
    pak.List(stdout);
    
    renderer.Init(640, 400);
    
    
    PakArchive pak1;
    pak1.InitFromRAM("1",pak.GetEntry(0)->data,pak.GetEntry(0)->size);
    pak1.List(stdout);
    showAllImage(&pak1);
    
    PakArchive pak2;
    pak2.InitFromRAM("2",pak.GetEntry(1)->data,pak.GetEntry(1)->size);
    pak2.List(stdout);
    showAllImage(&pak2);
    
    PakArchive pak3;
    pak3.InitFromRAM("3",pak.GetEntry(2)->data,pak.GetEntry(2)->size);
    pak3.List(stdout);
    showAllImage(&pak3);
}

void SCMainMenu::Run(void){
    renderer.Init(640, 400);
}