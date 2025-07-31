//
//  SCMainMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

/*
 3 eagle
 9 letters
 */

void SCMainMenu::OnContinue(void) { printf("OnContinue\n"); }

void SCMainMenu::OnLoadGame() { 
    printf("OnLoadGame\n");
    this->frequest->opened = true;
    this->frequest->loadFiles();
}

void SCMainMenu::OnStartNewGame() {
    SCGameFlow *gfl = new SCGameFlow();
    GameState.Reset();
    gfl->init();
    SCRegister *registerActivity = new SCRegister();
    registerActivity->init();
    Game->AddActivity(gfl);
    Game->AddActivity(registerActivity);
}

void SCMainMenu::OnTrainingMission() {

    SCTrainingMenu *trainingActivity = new SCTrainingMenu();
    trainingActivity->init();
    Game->AddActivity(trainingActivity);
}

void SCMainMenu::OnViewObject() {
    SCObjectViewer *objViewer = new SCObjectViewer();
    objViewer->init();
    Game->AddActivity(objViewer);
}

SCMainMenu::SCMainMenu() { 
    this->boardPosition = {44, 25}; 
    this->music_playing = false;
}

SCMainMenu::~SCMainMenu() {}

void SCMainMenu::Focus(void) {
    this->focused = true;
    if (!this->music_playing) {
        Mixer.SwitchBank(0);
        Mixer.PlayMusic(23);
        this->music_playing = true;
    }
}
void SCMainMenu::UnFocus(void) {
    this->focused = false;
    if (this->music_playing) {
        Mixer.StopMusic();
        this->music_playing = false;
    }
}
void SCMainMenu::init(void) {

    TreEntry *entry = Assets.GetEntryByName(MAINMENU_PAK_PATH);
    mainMenupak.InitFromRAM("MAINMENU.PAK", entry->data, entry->size);
    
    LoadPalette();
    LoadButtons();
    LoadBoard();
    LoadBackgrounds();

    SetTitle("Neo Strike Commander");
    this->frequest = new SCFileRequester(std::bind(&SCMainMenu::LoadGame, this, std::placeholders::_1),42);
    timer = 4200;
}
void SCMainMenu::LoadGame(std::string filename) {
    GameState.Load(filename);
    SCGameFlow *gfl = new SCGameFlow();
    gfl->InitFromGameState();
    Game->AddActivity(gfl);
    this->frequest->requested_file = "";
}
    
void SCMainMenu::LoadButtons(void) {

    PakEntry *boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BUTTONS_INDICE);

    // The buttons are within an other pak within MAINMENU.PAK !!!!
    PakArchive subPak;
    subPak.InitFromRAM("subPak Buttons", boardEntry->data, boardEntry->size);

    SCButton *button = nullptr;

    Point2D buttonDimension = {211, 15};

    button = new SCButton();
    Point2D continuePosition = {boardPosition.x + 11, boardPosition.y + 10};

    button->InitBehavior(std::bind(&SCMainMenu::OnContinue, this), continuePosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(0)->data, subPak.GetEntry(0)->size,
                                                          &continuePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(5)->data, subPak.GetEntry(5)->size,
                                                            &continuePosition);
    button->SetEnable(false);
    buttons.push_back(button);

    button = new SCButton();
    Point2D loadGamePosition = {boardPosition.x + 11, continuePosition.y + buttonDimension.y + 2};
    button->InitBehavior(std::bind(&SCMainMenu::OnLoadGame, this), loadGamePosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(1)->data, subPak.GetEntry(1)->size,
                                                          &loadGamePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(6)->data, subPak.GetEntry(6)->size,
                                                            &loadGamePosition);
    button->SetEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    Point2D startNewGamePosition = {boardPosition.x + 11, loadGamePosition.y + buttonDimension.y + 2};
    button->InitBehavior(std::bind(&SCMainMenu::OnStartNewGame, this), startNewGamePosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(2)->data, subPak.GetEntry(2)->size,
                                                          &startNewGamePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(7)->data, subPak.GetEntry(7)->size,
                                                            &startNewGamePosition);
    buttons.push_back(button);

    button = new SCButton();
    Point2D trainingPosition = {boardPosition.x + 11, startNewGamePosition.y + buttonDimension.y + 2};
    button->InitBehavior(std::bind(&SCMainMenu::OnTrainingMission, this), trainingPosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(3)->data, subPak.GetEntry(3)->size,
                                                          &trainingPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(8)->data, subPak.GetEntry(8)->size,
                                                            &trainingPosition);
    buttons.push_back(button);

    button = new SCButton();
    Point2D viewObjectPosition = {boardPosition.x + 11, trainingPosition.y + buttonDimension.y + 2};
    button->InitBehavior(std::bind(&SCMainMenu::OnViewObject, this), viewObjectPosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(4)->data, subPak.GetEntry(4)->size,
                                                          &viewObjectPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(9)->data, subPak.GetEntry(9)->size,
                                                            &viewObjectPosition);
    buttons.push_back(button);
}

void SCMainMenu::LoadBoard(void) {

    PakEntry *boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_INDICE);

    // The board is within an other pak within MAINMENU.PAK !!!!
    PakArchive subPak;
    subPak.InitFromRAM("subPak board", boardEntry->data, boardEntry->size);
    boardEntry = subPak.GetEntry(0);

    board.InitWithPosition(boardEntry->data, boardEntry->size, &boardPosition);
}

void SCMainMenu::LoadPalette(void) {

    ByteStream paletteReader;

    VGAPalette *rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
    // Load the default palette

    TreEntry *palettesEntry = Assets.GetEntryByName(OPTPALS_PAK_PATH);
    PakArchive palettesPak;
    palettesPak.InitFromRAM("OPTSHPS.PAK", palettesEntry->data, palettesEntry->size);
    // palettesPak.List(stdout);

    paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_MOUTAIN_PALETTE_PATCH_ID)->data); // mountains Good but not sky
    this->palette.ReadPatch(&paletteReader);
    paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_SKY_PALETTE_PATCH_ID)->data); // Sky Good but not mountains
    this->palette.ReadPatch(&paletteReader);

    // Third palette patch (for silver board and buttons)
    PakEntry *palettePatchEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_PALETTE);
    paletteReader.Set(palettePatchEntry->data);
    this->palette.ReadPatch(&paletteReader);
}

void SCMainMenu::LoadBackgrounds(void) {

    TreEntry *entryMountain = Assets.GetEntryByName(Assets.optshps_filename);
    PakArchive pak;
    pak.InitFromRAM("", entryMountain->data, entryMountain->size);

    mountain.InitFromPakEntry(pak.GetEntry(OptionShapeID::MOUTAINS_BG));
    sky.InitFromPakEntry(pak.GetEntry(OptionShapeID::SKY));
    TreEntry *entryCloud = Assets.GetEntryByName("..\\..\\DATA\\MIDGAMES\\MIDGAMES.PAK");
    PakArchive subcloudPak;
    subcloudPak.InitFromRAM("cloud oak entry", entryCloud->data, entryCloud->size);
    cloud.InitFromPakEntry(subcloudPak.GetEntry(20));

}

void SCMainMenu::runFrame(void) {
    if (this->frequest->opened) {
        this->frequest->checkevents();
    } else {
        checkKeyboard();
        CheckButtons();
        timer --;
    }
    if (timer <= 0) {
        SCAnimationPlayer *intro = new SCAnimationPlayer();
        intro->init();
        timer = 4200;
        Mixer.StopMusic();
        Game->AddActivity(intro);
        return;
    }
    
    
    VGA.Activate();
    VGA.GetFrameBuffer()->Clear();

    VGA.SetPalette(&this->palette);

    VGA.GetFrameBuffer()->DrawShape(&sky);
    VGA.GetFrameBuffer()->DrawShape(&mountain);
    VGA.GetFrameBuffer()->DrawShape(&cloud);

    VGA.GetFrameBuffer()->DrawShape(&board);

    DrawButtons();
    if (this->frequest->opened) {
        this->frequest->draw(VGA.GetFrameBuffer());
    }
    Mouse.Draw();
    VGA.VSync();
}