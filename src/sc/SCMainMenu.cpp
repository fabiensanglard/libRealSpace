//
//  SCMainMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#define MAINMENU_PAK_PATH "..\\..\\DATA\\GAMEFLOW\\MAINMENU.PAK"

#define MAINMENU_PAK_BUTTONS_INDICE 0
#define MAINMENU_PAK_BOARD_INDICE   1
#define MAINMENU_PAK_BOARD_PALETTE  2

#define GAMEFLOW_PALETTE_PAK_PATH "..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK"


static void OnContinue(void){
    
}

static void OnLoadGame(){
    
}


static void OnStartNewGame(){
    
}


static void OnTrainingMission(){
    
}


static void OnViewObject(){
    
}







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

            renderer.DrawImage(&screen);
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
    TreEntry* entry = gameFlow->GetEntryByName(MAINMENU_PAK_PATH);
    mainMenupak.InitFromRAM("MAINMENU.PAK",entry->data,entry->size);
    
    //mainMenupak.List(stdout);
    
    
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
    
    /*
    PakArchive buttons;
    buttons.InitFromRAM("1",mainMenupak.GetEntry(0)->data,mainMenupak.GetEntry(0)->size);
    buttons.List(stdout);
    showAllImage(&buttons);
    */
    
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
    //PakArchive buttonsGroup;
    //buttonsGroup.InitFromRAM("2",mainMenupak.GetEntry(1)->data,mainMenupak.GetEntry(1)->size);
    //buttonsGroup.List(stdout);
    //showAllImage(&buttonsGroup);
    
    //What is in the third entry ?!??!?!
    /*
    PakArchive pak3;
    pak3.InitFromRAM("3",pak.GetEntry(2)->data,pak.GetEntry(2)->size);
    pak3.List(stdout);
    showAllImage(&pak3);
    */
    
    LoadPalette();
    LoadButtons();
    LoadBoard();
    LoadBackgrounds();
    
    
    SetTitle("Main Menu");
    
}

/*
void SCMainMenu::LoadButton(const char* name, PakArchive* subPak, size_t upIndice, size_t downIndice, Point2D dimensions, ActionFunction onClick){
    
}
 */

void SCMainMenu::LoadButtons(void){
    
    RLECodex codec;
    size_t byteRead;
    
    PakEntry* boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BUTTONS_INDICE);
    
    //The buttons are within an other pak within MAINMENU.PAK !!!!
    PakArchive subPak;
    subPak.InitFromRAM("subPak Buttons",boardEntry->data ,boardEntry->size);


    this->buttons[0].appearance[SCButton::APR_UP].Create("CONTINUE_UP", 211, 15);
    codec.ReadImage(subPak.GetEntry(0)->data,& this->buttons[0].appearance[SCButton::APR_UP], &byteRead);
    this->buttons[0].appearance[SCButton::APR_DOWN].Create("CONTINUE_DOWN", 211, 15);
    codec.ReadImage(subPak.GetEntry(0)->data,& this->buttons[0].appearance[SCButton::APR_DOWN], &byteRead);
    Point2D continuePosition = {0,0};
    this->buttons[0].InitBehavior(OnContinue, continuePosition);
    

    this->buttons[1].appearance[SCButton::APR_UP].Create("LOADGAME_UP", 211, 15);
    codec.ReadImage(subPak.GetEntry(1)->data,& this->buttons[1].appearance[SCButton::APR_UP], &byteRead);
    this->buttons[1].appearance[SCButton::APR_DOWN].Create("LOADGAME_DOWN", 211, 15);
    codec.ReadImage(subPak.GetEntry(1)->data,& this->buttons[1].appearance[SCButton::APR_DOWN], &byteRead);
    Point2D loadGamePosition = {0,0};
    this->buttons[0].InitBehavior(OnLoadGame, loadGamePosition);
    
    this->buttons[2].appearance[SCButton::APR_UP].Create("STARTNEWGAME_UP", 211, 15);
    codec.ReadImage(subPak.GetEntry(2)->data,& this->buttons[2].appearance[SCButton::APR_UP], &byteRead);
    this->buttons[2].appearance[SCButton::APR_DOWN].Create("STARTNEWGAME_DOWN", 211, 15);
    codec.ReadImage(subPak.GetEntry(2)->data,& this->buttons[2].appearance[SCButton::APR_DOWN], &byteRead);
    Point2D startNewGamePosition = {0,0};
    this->buttons[0].InitBehavior(OnStartNewGame, startNewGamePosition);
    
    this->buttons[3].appearance[SCButton::APR_UP].Create("TRAINING MISSIONS_UP", 211, 15);
    codec.ReadImage(subPak.GetEntry(3)->data,& this->buttons[3].appearance[SCButton::APR_UP], &byteRead);
    this->buttons[3].appearance[SCButton::APR_DOWN].Create("TRAINING MISSIONS_DOWN", 211, 15);
    codec.ReadImage(subPak.GetEntry(3)->data,& this->buttons[3].appearance[SCButton::APR_DOWN], &byteRead);
    Point2D trainMissionPosition = {0,0};
    this->buttons[0].InitBehavior(OnTrainingMission, trainMissionPosition);
    
    this->buttons[4].appearance[SCButton::APR_UP].Create("VIEWOBJECTS_UP", 211, 15);
    codec.ReadImage(subPak.GetEntry(4)->data,& this->buttons[4].appearance[SCButton::APR_UP], &byteRead);
    this->buttons[4].appearance[SCButton::APR_DOWN].Create("VIEWOBJECTS_DOWN", 211, 15);
    codec.ReadImage(subPak.GetEntry(4)->data,& this->buttons[4].appearance[SCButton::APR_DOWN], &byteRead);
    Point2D viewObjPosition = {0,0};
    this->buttons[0].InitBehavior(OnViewObject, viewObjPosition);
}

void SCMainMenu::LoadBoard(void){
    
    this->board = new RSImage();
    board->Create("MainMenu Board", 233, 104);
    Point2D position = {44.0f,48.0f};
    board->SetPosition(position);
    
    
    
    
    PakEntry* boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_INDICE);
    
    //The board is within an other pak within MAINMENU.PAK !!!!
    PakArchive subPak;
    subPak.InitFromRAM("subPak board",boardEntry->data ,boardEntry->size);
    boardEntry = subPak.GetEntry(0);
    
    size_t byteRead;
    RLECodex codec;
    codec.ReadImage(boardEntry->data, this->board, &byteRead);
    
}

void SCMainMenu::LoadPalette(void){
    
    VGAPalette* rendererPalette = renderer.GetPalette();
    this->palette = *rendererPalette;
    
    
    ByteStream paletteReader;
    
    //First patch:
    TreEntry* firstPaletteUpdate = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(GAMEFLOW_PALETTE_PAK_PATH);
    PakArchive firstPaletteUpdatePak ;
    firstPaletteUpdatePak.InitFromRAM("",firstPaletteUpdate->data,firstPaletteUpdate->size);
//    0-40
    paletteReader.Set(firstPaletteUpdatePak.GetEntry(24)->data);
    this->palette.ReadPatch(&paletteReader);
    
    
    //Second palette patch
    PakEntry* paletteEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_PALETTE);
    paletteReader.Set(paletteEntry->data);
    this->palette.ReadPatch(&paletteReader);
    
    Texel transp;
    transp.r = 255;
    transp.g = 0;
    transp.b = 255;
    transp.a = 0;
    this->palette.SetColor(255,&transp);
    
}

void SCMainMenu::LoadBackgrounds(void){
    
    size_t byteRead;
    RLECodex codec;
    /*
    Pak entry 44
    Exploring 0XC50FA56
    Pak Found 0XC50FA56
    Pak entry 0
     */
    
    
    TreEntry* entryMountain = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    PakArchive pak;
    pak.InitFromRAM("",entryMountain->data,entryMountain->size);
    
    
    //The board is within an other pak within MAINMENU.PAK !!!!
    PakArchive mountainPak;
    mountainPak.InitFromRAM("subPak board",pak.GetEntry(44)->data ,pak.GetEntry(44)->size);
    mountain = new RSImage();
    mountain->Create("Mountain", 320, 200);
    codec.ReadImage(mountainPak.GetEntry(0)->data, this->mountain, &byteRead);
    
    PakArchive skyPak;
    skyPak.InitFromRAM("subPak board",pak.GetEntry(116)->data ,pak.GetEntry(116)->size);
    sky = new RSImage();
    sky->Create("Sky", 320, 200);
    codec.ReadImage(skyPak.GetEntry(0)->data, this->sky, &byteRead);
    //background 116 ?
}

void SCMainMenu::CheckHit(void){
    
}

void SCMainMenu::Run(void){
    
    renderer.SetPalette(&this->palette);
    
    //Draw static
    renderer.DrawImage(sky);
    renderer.DrawImage(mountain);
    renderer.DrawImage(board);
    
    //Check Mouse position. If clickable update to visor instead of cursor
    CheckHit();
    
    //Draw Mouse
    
    //Check Mouse state.
}