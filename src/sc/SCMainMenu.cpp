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
    printf("OnContinue\n");
}

static void OnLoadGame(){
    printf("OnLoadGame\n");
}


static void OnStartNewGame(){
    printf("OnStartNewGame\n");
}


static void OnTrainingMission(){
    printf("OnTrainingMission\n");
    return;
    
    SCDogFightMenu* trainingActivity = new SCDogFightMenu();
    trainingActivity->Init();
    Game.AddActivity(trainingActivity);
}


static void OnViewObject(){
    SCObjectViewer* objViewer = new SCObjectViewer();
    objViewer->Init();
    Game.AddActivity(objViewer);
}







SCMainMenu::SCMainMenu(){
    
}

SCMainMenu::~SCMainMenu(){
    
}

void SCMainMenu::Init(void){
    
    
    TreArchive* gameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
    TreEntry* entry = gameFlow->GetEntryByName(MAINMENU_PAK_PATH);
    mainMenupak.InitFromRAM("MAINMENU.PAK",entry->data,entry->size);
    
    LoadPalette();
    LoadButtons();
    LoadBoard();
    LoadBackgrounds();
    
    
    SetTitle("Neo Strike Commander");
}

/*
void SCMainMenu::LoadButton(const char* name, PakArchive* subPak, size_t upIndice, size_t downIndice, Point2D dimensions, ActionFunction onClick){
    
}
 */


Point2D boardPosition = {44,25};

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

    
    
    SCButton* button;
    
    Point2D buttonDimension = {211, 15} ;
    

    button = new SCButton();
    Point2D continuePosition = {boardPosition.x+11,boardPosition.y+10};
    button->InitBehavior(OnContinue, continuePosition,buttonDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(0)->data, subPak.GetEntry(0)->size,&continuePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(5)->data, subPak.GetEntry(5)->size,&continuePosition);
    button->SetEnable(false);
    buttons.push_back(button);
    
    button = new SCButton();
    Point2D loadGamePosition = {boardPosition.x+11,continuePosition.y+buttonDimension.y+2};
    button->InitBehavior(OnLoadGame, loadGamePosition,buttonDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(1)->data, subPak.GetEntry(1)->size,&loadGamePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(6)->data, subPak.GetEntry(6)->size,&loadGamePosition);
    button->SetEnable(false);
    buttons.push_back(button);

    button = new SCButton();
    Point2D startNewGamePosition = {boardPosition.x+11,loadGamePosition.y+buttonDimension.y+2};
    button->InitBehavior(OnStartNewGame, startNewGamePosition,buttonDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(2)->data, subPak.GetEntry(2)->size,&startNewGamePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(7)->data, subPak.GetEntry(7)->size,&startNewGamePosition);
    buttons.push_back(button);
    
    button = new SCButton();
    Point2D trainingPosition = {boardPosition.x+11,startNewGamePosition.y+buttonDimension.y+2};
    button->InitBehavior(OnTrainingMission, trainingPosition,buttonDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(3)->data, subPak.GetEntry(3)->size,&trainingPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(8)->data, subPak.GetEntry(8)->size,&trainingPosition);
    buttons.push_back(button);
    
    button = new SCButton();
    Point2D viewObjectPosition = {boardPosition.x+11,trainingPosition.y+buttonDimension.y+2};
    button->InitBehavior(OnViewObject, viewObjectPosition,buttonDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(4)->data, subPak.GetEntry(4)->size,&viewObjectPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(9)->data, subPak.GetEntry(9)->size,&viewObjectPosition);
    buttons.push_back(button);
}

void SCMainMenu::LoadBoard(void){
    
    PakEntry* boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_INDICE);
    
    //The board is within an other pak within MAINMENU.PAK !!!!
    PakArchive subPak;
    subPak.InitFromRAM("subPak board",boardEntry->data ,boardEntry->size);
    boardEntry = subPak.GetEntry(0);
    
    
    board.InitWithPosition(boardEntry->data, boardEntry->size, &boardPosition);

}

void SCMainMenu::LoadPalette(void){
    
    VGAPalette* rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
    
    
    ByteStream paletteReader;
    
    
    /*
    TreEntry* mid1Entry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\MIDGAMES\\MID1.PAK");
    PakArchive mid1Pak;
    mid1Pak.InitFromRAM("MID1.PAK",mid1Entry->data,mid1Entry->size);
    mid1Pak.List(stdout);
    
    IffLexer lexer;
    lexer.InitFromRAM(mid1Pak.GetEntry(9)->data-772-0x14, 772+0x14);
    RSPalette letterPalette;
    letterPalette.InitFromIFF(&lexer);
    //this->palette = *letterPalette.GetColorPalette();
    */
    
    
    
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



void SCMainMenu::RunFrame(void){
    
    CheckButtons();
    
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(&sky);
    VGA.DrawShape(&mountain);
    VGA.DrawShape(&cloud);
    
    
    VGA.DrawShape(&board);
    
    DrawButtons();
    
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();
    
}