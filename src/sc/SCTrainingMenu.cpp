//
//  File.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

SCTrainingMenu::SCTrainingMenu() {
    this->music_playing = false;
}

SCTrainingMenu::~SCTrainingMenu() {}

void SCTrainingMenu::OnDogFight() {}

void SCTrainingMenu::Focus(void) {
    this->focused = true;
    if (!this->music_playing) {
        Mixer.SwitchBank(0);
        Mixer.PlayMusic(28);
        this->music_playing = true;
    }
}
void SCTrainingMenu::UnFocus(void) {
    this->focused = false;
    if (this->music_playing) {
        Mixer.StopMusic();
        this->music_playing = false;
    }
}

void SCTrainingMenu::OnSearchAndDestroy() {
    SCStrike *strike = new SCStrike();
    strike->Init();
    Game.AddActivity(strike);
}

void SCTrainingMenu::OnExitTraining(void) { Game.StopTopActivity(); }

void SCTrainingMenu::Init() {

    TreEntry *objViewPAK =
        Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.PAK");
    PakArchive assets;
    assets.InitFromRAM("OBJVIEW.PAK", objViewPAK->data, objViewPAK->size);
    assets.List(stdout);

    ByteStream paletteReader;
    paletteReader.Set(assets.GetEntry(7)->data);
    this->palette.ReadPatch(&paletteReader);

    PakEntry *backgroundPakEntry = assets.GetEntry(6);
    PakArchive bgPack;
    bgPack.InitFromRAM("OBJVIEW.PAK: file 6", backgroundPakEntry->data, backgroundPakEntry->size);
    background.Init(bgPack.GetEntry(0)->data, bgPack.GetEntry(0)->size);

    PakEntry *titlePackEntry = assets.GetEntry(1);
    PakArchive titlePack;
    titlePack.InitFromRAM("", titlePackEntry->data, titlePackEntry->size);
    title.Init(titlePack.GetEntry(0)->data, titlePack.GetEntry(0)->size);
    Point2D positionTitle = {4, 0};
    title.SetPosition(&positionTitle);

    TreEntry *trButtonsEntry =
        Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\TM.SHP");
    PakArchive trButtonsPack;
    trButtonsPack.InitFromRAM("TM.SHP", trButtonsEntry->data, trButtonsEntry->size);

    Point2D positionBoard = {6, 150};
    board.Init(trButtonsPack.GetEntry(0)->data, trButtonsPack.GetEntry(0)->size);
    board.SetPosition(&positionBoard);
    PakEntry *pak = nullptr;
    SCButton *button;
    button = new SCButton();
    Point2D sandDDimension = {130, 15};
    Point2D sanDPosition = {positionBoard.x + 16, positionBoard.y + 9};
    button->InitBehavior(std::bind(&SCTrainingMenu::OnSearchAndDestroy, this), sanDPosition, sandDDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(trButtonsPack.GetEntry(1)->data,
                                                          trButtonsPack.GetEntry(1)->size, &sanDPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(2)->data,
                                                            trButtonsPack.GetEntry(2)->size, &sanDPosition);
    buttons.push_back(button);

    button = new SCButton();
    Point2D dogDDimension = {130, 15};
    Point2D dogDPosition = {positionBoard.x + 155, positionBoard.y + 9};
    button->InitBehavior(std::bind(&SCTrainingMenu::OnDogFight, this), dogDPosition, dogDDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(trButtonsPack.GetEntry(3)->data,
                                                          trButtonsPack.GetEntry(3)->size, &dogDPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(4)->data,
                                                            trButtonsPack.GetEntry(4)->size, &dogDPosition);
    button->SetEnable(false);
    buttons.push_back(button);

    button = new SCButton();
    Point2D exitDDimension = {60, 15};
    Point2D exitDPosition = {positionBoard.x + 155, positionBoard.y + 23};
    button->InitBehavior(std::bind(&SCTrainingMenu::OnExitTraining, this), exitDPosition, exitDDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(trButtonsPack.GetEntry(5)->data,
                                                          trButtonsPack.GetEntry(5)->size, &exitDPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(6)->data,
                                                            trButtonsPack.GetEntry(6)->size, &exitDPosition);
    buttons.push_back(button);
}

void SCTrainingMenu::RunFrame(void) {
    CheckButtons();

    VGA.Activate();
    VGA.GetFrameBuffer()->Clear();

    VGA.SetPalette(&this->palette);

    VGA.GetFrameBuffer()->DrawShape(&background);
    VGA.GetFrameBuffer()->DrawShape(&title);
    VGA.GetFrameBuffer()->DrawShape(&board);

    DrawButtons();

    Mouse.Draw();

    VGA.VSync();
}