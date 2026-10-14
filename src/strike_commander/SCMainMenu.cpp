//
//  SCMainMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include <boost/filesystem.hpp>


std::string SCMainMenu::findLatestSave() {
    std::string best;
    std::time_t bestTime = 0;
    boost::system::error_code ec;
    for (boost::filesystem::directory_iterator it(".", ec), end; !ec && it != end; it.increment(ec)) {
        const boost::filesystem::path &p = it->path();
        if (!boost::filesystem::is_regular_file(p, ec)) {
            continue;
        }
        std::string ext = p.extension().string();
        if (ext != ".SAV" && ext != ".sav") {
            continue;
        }
        std::time_t t = boost::filesystem::last_write_time(p, ec);
        if (ec) {
            continue;
        }
        if (best.empty() || t > bestTime) {
            best = p.filename().string();
            bestTime = t;
        }
    }
    return best;
}

void SCMainMenu::OnContinue(void) {
    std::string latest = findLatestSave();
    if (latest.empty()) {
        return;
    }
    this->LoadGame(latest);
}

void SCMainMenu::OnLoadGame() { 
    if (this->frequest) {
        delete this->frequest;
        this->frequest = nullptr;
    }
    this->frequest = new SCFileRequester(std::bind(&SCMainMenu::LoadGame, this, std::placeholders::_1),42);
    this->frequest->opened = true;
    this->frequest->loadFiles();
}

void SCMainMenu::OnStartNewGame() {
    SCGameFlow *gfl = new SCGameFlow();
    GameState.Reset();
    gfl->init();
    SCRegister *registerActivity = new SCRegister();
    registerActivity->init();
    Game->addActivity(gfl);
    Game->addActivity(registerActivity);
}

void SCMainMenu::OnTrainingMission() {

    SCTrainingMenu *trainingActivity = new SCTrainingMenu();
    trainingActivity->init();
    Game->addActivity(trainingActivity);
}

void SCMainMenu::OnViewObject() {
    SCObjectViewer *objViewer = new SCObjectViewer();
    objViewer->init();
    Game->addActivity(objViewer);
}

SCMainMenu::SCMainMenu() { 
    this->boardPosition = {44, 25}; 
    this->music_playing = false;
    if (Game == nullptr) {
        Game = &GameEngine::instance();
    }
}

SCMainMenu::~SCMainMenu() {}

void SCMainMenu::focus(void) {
    this->focused = true;
    if (!this->music_playing) {
        Mixer.switchBank(0);
        Mixer.playMusic(23);
        this->music_playing = true;
    }
}
void SCMainMenu::unFocus(void) {
    this->focused = false;
    if (this->music_playing) {
        Mixer.stopMusic();
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

    setTitle("Neo Strike Commander");
    timer = 4200;
}
void SCMainMenu::LoadGame(std::string filename) {
    GameState.Load(filename);
    SCGameFlow *gfl = new SCGameFlow();
    gfl->InitFromGameState();
    Game->addActivity(gfl);
    if (this->frequest) {
        this->frequest->requested_file = "";
    }
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

    button->initBehavior(std::bind(&SCMainMenu::OnContinue, this), continuePosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(0)->data, subPak.GetEntry(0)->size,
                                                          &continuePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(5)->data, subPak.GetEntry(5)->size,
                                                            &continuePosition);
    button->setEnable(!findLatestSave().empty());
    buttons.push_back(button);

    button = new SCButton();
    Point2D loadGamePosition = {boardPosition.x + 11, continuePosition.y + buttonDimension.y + 2};
    button->initBehavior(std::bind(&SCMainMenu::OnLoadGame, this), loadGamePosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(1)->data, subPak.GetEntry(1)->size,
                                                          &loadGamePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(6)->data, subPak.GetEntry(6)->size,
                                                            &loadGamePosition);
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    Point2D startNewGamePosition = {boardPosition.x + 11, loadGamePosition.y + buttonDimension.y + 2};
    button->initBehavior(std::bind(&SCMainMenu::OnStartNewGame, this), startNewGamePosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(2)->data, subPak.GetEntry(2)->size,
                                                          &startNewGamePosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(7)->data, subPak.GetEntry(7)->size,
                                                            &startNewGamePosition);
    buttons.push_back(button);

    button = new SCButton();
    Point2D trainingPosition = {boardPosition.x + 11, startNewGamePosition.y + buttonDimension.y + 2};
    button->initBehavior(std::bind(&SCMainMenu::OnTrainingMission, this), trainingPosition, buttonDimension);
    button->appearance[SCButton::APR_UP].InitWithPosition(subPak.GetEntry(3)->data, subPak.GetEntry(3)->size,
                                                          &trainingPosition);
    button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(8)->data, subPak.GetEntry(8)->size,
                                                            &trainingPosition);
    buttons.push_back(button);

    button = new SCButton();
    Point2D viewObjectPosition = {boardPosition.x + 11, trainingPosition.y + buttonDimension.y + 2};
    button->initBehavior(std::bind(&SCMainMenu::OnViewObject, this), viewObjectPosition, buttonDimension);
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

    VGAPalette *rendererPalette = VGA.getPalette();
    this->palette = *rendererPalette;
    // Load the default palette

    TreEntry *palettesEntry = Assets.GetEntryByName(OPTPALS_PAK_PATH);
    PakArchive palettesPak;
    palettesPak.InitFromRAM("OPTSHPS.PAK", palettesEntry->data, palettesEntry->size);
    // palettesPak.List(stdout);

    paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_MOUTAIN_PALETTE_PATCH_ID)->data, palettesPak.GetEntry(OPTPALS_PAK_MOUTAIN_PALETTE_PATCH_ID)->size); // mountains Good but not sky
    this->palette.ReadPatch(&paletteReader);
    paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_SKY_PALETTE_PATCH_ID)->data, palettesPak.GetEntry(OPTPALS_PAK_SKY_PALETTE_PATCH_ID)->size); // Sky Good but not mountains
    this->palette.ReadPatch(&paletteReader);

    // Third palette patch (for silver board and buttons)
    PakEntry *palettePatchEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_PALETTE);
    paletteReader.Set(palettePatchEntry->data, palettePatchEntry->size);
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
    if (this->frequest !=nullptr && this->frequest->opened) {
        this->frequest->checkevents();
    } else {
        checkKeyboard();
        checkButtons();
        timer --;
    }
    if (timer <= 0) {
        SCAnimationPlayer *intro = new SCAnimationPlayer();
        intro->init();
        timer = 4200;
        Mixer.stopMusic();
        Game->addActivity(intro);
        return;
    }
    
    
    VGA.activate();
    VGA.getFrameBuffer()->clear();

    VGA.setPalette(&this->palette);

    VGA.getFrameBuffer()->drawShape(&sky);
    VGA.getFrameBuffer()->drawShape(&mountain);
    VGA.getFrameBuffer()->drawShape(&cloud);

    VGA.getFrameBuffer()->drawShape(&board);

    drawButtons();
    if (this->frequest!= nullptr && this->frequest->opened) {
        this->frequest->draw(VGA.getFrameBuffer());
    }
    Mouse.draw();
    VGA.vSync();
}