//
//  File.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"




SCTrainingMenu::SCTrainingMenu(){
}

SCTrainingMenu::~SCTrainingMenu(){
}

void OnDogFight(){
    
}

void OnSearchAndDestroy(){
    SCSelectWeaponF16* select = new SCSelectWeaponF16();
    select->Init();
    Game.AddActivity(select);
}

static void OnExitTraining(void){
    Game.StopTopActivity();
}


void SCTrainingMenu::Init( ){

    
    
    TreEntry* objViewPAK = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.PAK");
    PakArchive assets;
    assets.InitFromRAM("OBJVIEW.PAK",objViewPAK->data, objViewPAK->size);
    assets.List(stdout);
    
    //Load palette
    ByteStream paletteReader;
    paletteReader.Set(assets.GetEntry(7)->data);
    this->palette.ReadPatch(&paletteReader);
    
    
    
    PakEntry* backgroundPakEntry = assets.GetEntry(6);
    //Identified as Dodge Fight background
    PakArchive bgPack;
    bgPack.InitFromRAM("OBJVIEW.PAK: file 6",backgroundPakEntry->data, backgroundPakEntry->size);
    background.Init(bgPack.GetEntry(0)->data, bgPack.GetEntry(0)->size);

    
    
    PakEntry* titlePackEntry = assets.GetEntry(1);
    PakArchive titlePack;
    titlePack.InitFromRAM("",titlePackEntry->data, titlePackEntry->size);
    title.Init(titlePack.GetEntry(0)->data, titlePack.GetEntry(0)->size);
    Point2D positionTitle = {4,0};
    title.SetPosition(&positionTitle);
    
    /*
    PakEntry* ue = assets.GetEntry(8);
    PakArchive up;
    up.InitFromRAM("OBJVIEW.PAK: file 5",ue->data, ue->size);
    up.List(stdout);
    
    title.Init(up.GetEntry(0)->data, up.GetEntry(0)->size);
     */
    
    TreEntry* trButtonsEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\TM.SHP");
    PakArchive trButtonsPack;
    trButtonsPack.InitFromRAM("TM.SHP",trButtonsEntry->data, trButtonsEntry->size);
    
    Point2D positionBoard = {6,150};
    board.Init(trButtonsPack.GetEntry(0)->data, trButtonsPack.GetEntry(0)->size);
    board.SetPosition(&positionBoard);
    
    //Search and destroy button
    SCButton* button;
    button = new SCButton();
    Point2D sandDDimension = {130, 15} ;
    Point2D sanDPosition = {positionBoard.x+16,positionBoard.y+9};
    button->InitBehavior(OnSearchAndDestroy, sanDPosition,sandDDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(trButtonsPack.GetEntry(1)->data, trButtonsPack.GetEntry(1)->size,&sanDPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(2)->data, trButtonsPack.GetEntry(2)->size,&sanDPosition);
    buttons.push_back(button);
    
    button = new SCButton();
    Point2D dogDDimension = {130, 15} ;
    Point2D dogDPosition = {positionBoard.x+155,positionBoard.y+9};
    button->InitBehavior(OnDogFight, dogDPosition,dogDDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(trButtonsPack.GetEntry(3)->data, trButtonsPack.GetEntry(3)->size,&dogDPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(4)->data, trButtonsPack.GetEntry(4)->size,&dogDPosition);
    button->SetEnable(false);
    buttons.push_back(button);
    
    button = new SCButton();
    Point2D exitDDimension = {60, 15} ;
    Point2D exitDPosition = {positionBoard.x+155,positionBoard.y+23};
    button->InitBehavior(OnExitTraining, exitDPosition,exitDDimension);
    button->appearance[SCButton::APR_UP]  .InitWithPosition(trButtonsPack.GetEntry(5)->data, trButtonsPack.GetEntry(5)->size,&exitDPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(6)->data, trButtonsPack.GetEntry(6)->size,&exitDPosition);
    buttons.push_back(button);
}

void SCTrainingMenu::RunFrame(void){
    CheckButtons();
    
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(&background);
    VGA.DrawShape(&title);
    VGA.DrawShape(&board);
    
    DrawButtons();
    
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();

}