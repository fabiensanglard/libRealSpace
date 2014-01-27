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


/*
 3 eagle
 9 letters
 */



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
    
    
    
    
    
    
    /*
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
    
    */
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

enum ButtonIDS {
    BUTTON_CONTINUE,
    BUTTON_LOADGAME,
    BUTTON_STARTNEWGAME,
    BUTTON_TRAINING,
    BUTTON_OBJVIEWER}   ;

void SCMainMenu::LoadButtons(void){

    
    PakEntry* boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BUTTONS_INDICE);
    
    //The buttons are within an other pak within MAINMENU.PAK !!!!
    PakArchive subPak;
    subPak.InitFromRAM("subPak Buttons",boardEntry->data ,boardEntry->size);

    Point2D buttonDimension = {211, 15} ;
    
    
    Point2D continuePosition = {44+11,20+10};
    this->buttons[0].InitBehavior(OnContinue, continuePosition,buttonDimension);
    this->buttons[0].appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(0)->data, subPak.GetEntry(0)->size,&continuePosition);
    this->buttons[0].appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(5)->data, subPak.GetEntry(5)->size,&continuePosition);

    Point2D loadGamePosition = {44+11,continuePosition.y+buttonDimension.y+2};
    this->buttons[1].InitBehavior(OnLoadGame, loadGamePosition,buttonDimension);
    this->buttons[1].appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(1)->data, subPak.GetEntry(1)->size,&loadGamePosition);
    this->buttons[1].appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(6)->data, subPak.GetEntry(6)->size,&loadGamePosition);

    Point2D startNewGamePosition = {44+11,loadGamePosition.y+buttonDimension.y+2};
    this->buttons[2].InitBehavior(OnStartNewGame, startNewGamePosition,buttonDimension);
    this->buttons[2].appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(2)->data, subPak.GetEntry(2)->size,&startNewGamePosition);
    this->buttons[2].appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(7)->data, subPak.GetEntry(7)->size,&startNewGamePosition);
    
    Point2D trainingPosition = {44+11,startNewGamePosition.y+buttonDimension.y+2};
    this->buttons[3].InitBehavior(OnTrainingMission, trainingPosition,buttonDimension);
    this->buttons[3].appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(3)->data, subPak.GetEntry(3)->size,&trainingPosition);
    this->buttons[3].appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(8)->data, subPak.GetEntry(8)->size,&trainingPosition);

    Point2D viewObjectPosition = {44+11,trainingPosition.y+buttonDimension.y+2};
    this->buttons[4].InitBehavior(OnViewObject, viewObjectPosition,buttonDimension);
    this->buttons[4].appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(4)->data, subPak.GetEntry(4)->size,&viewObjectPosition);
    this->buttons[4].appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(9)->data, subPak.GetEntry(9)->size,&viewObjectPosition);

}

void SCMainMenu::LoadBoard(void){
    
    PakEntry* boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_INDICE);
    
    //The board is within an other pak within MAINMENU.PAK !!!!
    PakArchive subPak;
    subPak.InitFromRAM("subPak board",boardEntry->data ,boardEntry->size);
    boardEntry = subPak.GetEntry(0);
    
    Point2D position = {44,20};
    board.InitWithPosition(boardEntry->data, boardEntry->size, &position);

}

void SCMainMenu::LoadPalette(void){
    
    VGAPalette* rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
    
    
    ByteStream paletteReader;
    
    
    
    TreEntry* mid1Entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\MIDGAMES\\MID1.PAK");
    PakArchive mid1Pak;
    mid1Pak.InitFromRAM("MID1.PAK",mid1Entry->data,mid1Entry->size);
    mid1Pak.List(stdout);
    
    IffLexer lexer;
    lexer.InitFromRAM(mid1Pak.GetEntry(9)->data-772-0x14, 772+0x14);
    RSPalette letterPalette;
    letterPalette.InitFromIFF(&lexer);
    //this->palette = *letterPalette.GetColorPalette();
    
    
    
    
    //Second palette patch
    PakEntry* paletteEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_PALETTE);
    paletteReader.Set(paletteEntry->data);
    this->palette.ReadPatch(&paletteReader);
    
}

void SCMainMenu::LoadBackgrounds(void){
    
    
    TreEntry* entryMountain = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    PakArchive pak;
    pak.InitFromRAM("",entryMountain->data,entryMountain->size);
    
    
    //The board is within an other pak within MAINMENU.PAK !!!!
    PakArchive mountainPak;
    mountainPak.InitFromRAM("subPak board",pak.GetEntry(44)->data ,pak.GetEntry(44)->size);
    mountain.Init(mountainPak.GetEntry(0)->data, mountainPak.GetEntry(0)->size);

    
    PakArchive skyPak;
    skyPak.InitFromRAM("subPak sky",pak.GetEntry(116)->data ,pak.GetEntry(116)->size);
    sky.Init(skyPak.GetEntry(0)->data, skyPak.GetEntry(0)->size);

    //background 116 ?
    
    TreEntry* entryCloud = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\MIDGAMES\\MIDGAMES.PAK");
    PakArchive subcloudPak;
    subcloudPak.InitFromRAM("cloud oak entry", entryCloud->data, entryCloud->size);
    PakArchive cloudPak;
    cloudPak.InitFromRAM("subPak cloud",subcloudPak.GetEntry(20)->data ,subcloudPak.GetEntry(20)->size);
    cloud.Init(cloudPak.GetEntry(0)->data, cloudPak.GetEntry(0)->size);

}

void SCMainMenu::CheckHit(void){
    
}

void SCMainMenu::DrawMenu(void){
    VGA.DrawShape(&board);
    VGA.DrawShape(&buttons[BUTTON_OBJVIEWER].appearance[SCButton::Appearance::APR_UP]);
    VGA.DrawShape(&buttons[BUTTON_STARTNEWGAME].appearance[SCButton::Appearance::APR_UP]);
    VGA.DrawShape(&buttons[BUTTON_TRAINING].appearance[SCButton::Appearance::APR_UP]);
    
}

void SCMainMenu::Run(void){
    
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(&sky);
    VGA.DrawShape(&mountain);
    VGA.DrawShape(&cloud);
    
    DrawMenu();
    
    //Check Mouse position. If clickable update to visor instead of cursor
    CheckHit();
    
    //Draw Mouse
    
    //Check Mouse state.
    
    VGA.VSync();
    
}