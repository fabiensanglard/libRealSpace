//
//  SCShot.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

SCShot::SCShot() {
    this->fps = SDL_GetTicks() / 100;
}

SCShot::~SCShot() {
}


/**
 * Checks for keyboard events and updates the game state accordingly.
 *
 * This function peeks at the SDL event queue to check for keyboard events.
 * If a key is pressed, it updates the game state based on the key pressed.
 * The supported keys are escape, space, and return.
 *
 * @return None
 */
void SCShot::CheckKeyboard(void) {
    SDL_Event mouseEvents[5];
    int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP);
    for (int i = 0; i < numMouseEvents; i++) {
        SDL_Event *event = &mouseEvents[i];

        switch (event->type) {
        case SDL_MOUSEBUTTONUP:
            Game.StopTopActivity();
            break;
        default:
            break;
        }
    }
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event* event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        default:
            break;
        }
    }
}

/**
 * Initializes the SCShot object by loading the necessary IFF files and initializing the optionParser and gameFlowParser.
 * It also initializes the optShps and optPals objects by loading the OPTSHPS.PAK and OPTPALS.PAK files respectively.
 * Finally, it sets the efect pointer to nullptr and calls the createMiss() function.
 *
 * @throws None
 */
void SCShot::Init() {

    TreEntry* optionIFF = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTIONS.IFF");
    this->optionParser.InitFromRam(optionIFF->data, optionIFF->size);
    TreEntry* optShapEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry* optPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);

}

/**
 * Sets the shot ID of the SCShot object and updates the layers and palette accordingly.
 *
 * @param shotid The shot ID to set.
 *
 * @throws None
 */
void SCShot::SetShotId(uint8_t shotid) {
    for (auto s: this->optionParser.estb[shotid]->images) {
        shotBackground *bg = new shotBackground();
        bg->img = this->getShape(s->OptshapeID);
        this->layers.push_back(bg);
    }
    uint8_t paltID = this->optionParser.estb[shotid]->palettes[0]->ID;
    this->rawPalette = this->optPals.GetEntry(paltID)->data;
    for (auto p: this->optionParser.estb[shotid]->palettes) {
        this->palettes.push_back(this->optPals.GetEntry(p->ID)->data);
    }
}

/**
 * Retrieves an RSImageSet object for the specified shape ID.
 *
 * @param shpid The ID of the shape to retrieve.
 *
 * @return A pointer to the RSImageSet object containing the shape data.
 *
 * @throws None.
 */
RSImageSet* SCShot::getShape(uint8_t shpid) {
    PakEntry* shapeEntry = this->optShps.GetEntry(shpid);
    PakArchive subPAK;

    subPAK.InitFromRAM("", shapeEntry->data, shapeEntry->size);
    RSImageSet* img = new RSImageSet();
    if (!subPAK.IsReady()) {
        img->InitFromPakEntry(this->optShps.GetEntry(shpid));
    } else {
        img->InitFromSubPakEntry(&subPAK);
    }
    return (img);
}

/**
 * Runs a single frame of the game, updating the game state and rendering the graphics.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
void SCShot::RunFrame(void) {
    CheckKeyboard();
    VGA.Activate();
    VGA.FillWithColor(0);
    ByteStream paletteReader;
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() /10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks()/10);
    }
    
    bool ended = false;
    int cpt=0;
    for (auto layer: this->layers) {
        if (cpt<this->palettes.size()) {
            paletteReader.Set((this->palettes[cpt]));
            this->palette.ReadPatch(&paletteReader);
            VGA.SetPalette(&this->palette);
        }
        
        if (layer->img->palettes.size()>=1) {
            layer->img->palettes[0]->copyFrom(&this->palette);
            VGA.SetPalette(layer->img->palettes[0]->GetColorPalette());
        } else {
            VGA.SetPalette(&this->palette);
        }
        VGA.DrawShape(layer->img->GetShape(layer->img->sequence[layer->frameCounter]));
        if (layer->img->sequence.size()>1) {
            layer->frameCounter = layer->frameCounter + fpsupdate;
            if (layer->frameCounter>=layer->img->sequence.size()) {
                ended = true;
            }
        }
        cpt++;
    }
    for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT; i++)
            VGA.FillLineColor(i, 0x00);

        for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
            VGA.FillLineColor(199 - i, 0x00);
    Mouse.Draw();

    VGA.VSync();
    if (ended) {
        Game.StopTopActivity();
    }
}

void EndMissionScene::Init() {
    // opt shp id 143
    // opt shp id 141
    SCShot::Init();
    this->layers.clear();
    shotBackground *tmpbg = new shotBackground();
    tmpbg->img = this->getShape(143);
    this->layers.push_back(tmpbg);
    tmpbg = new shotBackground();
    tmpbg->img = this->getShape(141);
    this->layers.push_back(tmpbg);
    this->rawPalette = this->optPals.GetEntry(20)->data;
}

void EndMissionScene::RunFrame() {
    CheckKeyboard();
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks() / 10);
    }
    Mixer.SwitchBank(0);
    Mixer.PlayMusic(27,1);
    ByteStream paletteReader;
    paletteReader.Set((this->rawPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.Activate();
    VGA.FillWithColor(0);
    VGA.SetPalette(&this->palette);
    shotBackground *layer = this->layers[this->part];
    VGA.DrawShape(layer->img->GetShape(layer->img->sequence[0]));
    VGA.DrawShape(layer->img->GetShape(layer->img->sequence[layer->frameCounter]));
    if (layer->img->sequence.size() > 1) {
        layer->frameCounter = (uint8_t)(layer->frameCounter + fpsupdate) % layer->img->sequence.size();
    }
    Mouse.Draw();
    VGA.VSync();
}

void EndMissionScene::CheckKeyboard(void) {
    SDL_Event mouseEvents[5];
    int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP);
    for (int i = 0; i < numMouseEvents; i++) {
        SDL_Event *event = &mouseEvents[i];

        switch (event->type) {
        case SDL_MOUSEBUTTONUP:
            if (this->part == 0) {
                this->part = 1;
                this->rawPalette = this->optPals.GetEntry(19)->data;
            } else {
                Game.StopTopActivity();
            }
            break;
        default:
            break;
        }
    }
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event* event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        default:
            break;
        }
    }
}