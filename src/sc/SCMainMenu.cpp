//
//  SCMainMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

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
    
    TreArchive* gameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
    
    TreEntry* entry = gameFlow->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\MAINMENU.PAK");
    PakArchive pak;
    pak.InitFromRAM("MAINMENU.PAK",entry->data,entry->size);
    pak.List(stdout);
    
    //Buttons are X 211 * 15 texels
    /*
     
       CONTINUE UP
       LOADGAME UP
       STARTNEWGAME UP
       TRAINING MISSIONS UP
       VIEWOBJECTS UP
     
       CONTINUE DOWN
       LOADGAME DOWN
       STARTNEWGAME DOWN
       TRAINING MISSIONS DOWN
       VIEWOBJECTS DOWN
     
    */
    PakArchive buttons;
    buttons.InitFromRAM("1",pak.GetEntry(0)->data,pak.GetEntry(0)->size);
    buttons.List(stdout);
    showAllImage(&buttons);
    
    /*
          Full board is 233 * 104
          First button at 11 * 10
          Then space between buttons is 2 texels high
     */
    /*
     CONTINUE DOWN
     LOADGAME DOWN
     STARTNEWGAME DOWN
     TRAINING MISSIONS DOWN
     VIEWOBJECTS DOWN
     */
    PakArchive buttonsGroup;
    buttonsGroup.InitFromRAM("2",pak.GetEntry(1)->data,pak.GetEntry(1)->size);
    buttonsGroup.List(stdout);
    showAllImage(&buttonsGroup);
    
    //What is in the third entry ?!??!?!
    /*
    PakArchive pak3;
    pak3.InitFromRAM("3",pak.GetEntry(2)->data,pak.GetEntry(2)->size);
    pak3.List(stdout);
    showAllImage(&pak3);
    */
    
    LoadButtons();
    LoadBoard();
    LoadBackground();
    
    SetTitle("Main Menu");
    
}

void SCMainMenu::LoadButtons(void){
    
}

void SCMainMenu::LoadBoard(void){
    
}

void SCMainMenu::LoadBackground(void){
    
}

void SCMainMenu::Run(void){
    
    //Draw static
    
    //Check Mouse position. If clickable update to visor instead of cursor
    
    //Draw Mouse
    
    //Check Mouse state.
}