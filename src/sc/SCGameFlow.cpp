//
//  File.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCGameFlow.h"
#include "precomp.h"




SCGameFlow::SCGameFlow() {
    this->current_miss = 0;
    this->current_scen = 0;
}

SCGameFlow::~SCGameFlow() {
}


void SCGameFlow::CheckKeyboard(void) {
    //Keyboard
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event* event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        case SDLK_SPACE:
            this->current_miss++;
            this->current_scen = 0;
            if (this->current_miss >= this->gameFlowParser.game.game.size()) {
                this->current_miss = 0;
            }
            this->createMiss();
            break;
        case SDLK_RETURN:
            this->current_scen++;
            if (this->current_scen >= this->gameFlowParser.game.game[this->current_miss]->scen.size()) {
                this->current_scen = 0;
                this->current_miss++;
                if (this->current_miss >= this->gameFlowParser.game.game.size()) {
                    this->current_miss = 0;
                }
            }
            this->createMiss();
            break;
        default:
            break;
        }
    }
}

void SCGameFlow::Init() {

    TreEntry* gameflowIFF = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\GAMEFLOW.IFF");
    TreEntry* optionIFF = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTIONS.IFF");
    this->optionParser.InitFromRam(optionIFF->data, optionIFF->size);
    this->gameFlowParser.InitFromRam(gameflowIFF->data, gameflowIFF->size);

    TreEntry* optShapEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry* optPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    this->createMiss();
}

void SCGameFlow::createMiss() {
    printf("current miss : %d, current_scen %d\n", this->current_miss, this->current_scen);
    if (this->gameFlowParser.game.game[this->current_miss]->scen.size() > 0) {
        uint8_t optionScenID = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->info.ID;
        //printf("%d, ID in optpak for miss 0\n", this->optionParser.opts[optionScenID]->background->images[0]->ID);
        uint8_t shapeID = this->optionParser.opts[optionScenID]->background->images[0]->ID;
        uint8_t paltID = this->optionParser.opts[optionScenID]->background->palette->ID;
        uint8_t forPalTID = this->optionParser.opts[optionScenID]->foreground->palette->ID;
        this->sprites.clear();

        for (int i=0; i < this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->sprt.size(); i++) {
            uint8_t sprtId = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->sprt[i]->info.ID;
            if (this->optionParser.opts[optionScenID]->foreground->sprites.count(sprtId) > 0) {
                uint8_t optsprtId = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->sprite.SHP_ID;
                
                RLEShape* sprite = this->getShape(optsprtId);
                this->sprites.push_back(sprite);
            } else {
                printf("%d, ID Sprite not found !!\n", sprtId);
            }
            
        }
        this->layer = this->getShape(shapeID);
        this->rawPalette = this->optPals.GetEntry(paltID)->data;
        this->forPalette = this->optPals.GetEntry(forPalTID)->data;
    }
}
RLEShape* SCGameFlow::getShape(uint8_t shpid) {
    PakEntry* shapeEntry = this->optShps.GetEntry(shpid);
    PakArchive subPAK;
    RLEShape* s = new RLEShape();

    subPAK.InitFromRAM("", shapeEntry->data, shapeEntry->size);

    if (!subPAK.IsReady()) {

        //Sometimes the image is not in a PAK but as raw data.
        Game.Log("Error on Pak %d  => Using dummy instead\n", shpid);

        //Using an empty shape for now...
        *s = *RLEShape::GetEmptyShape();
    } else {
        s->Init(subPAK.GetEntry(0)->data, subPAK.GetEntry(0)->size);
        Point2D pos = { 0, 0 };
        s->SetPosition(&pos);
    }
    return (s);
}
void SCGameFlow::RunFrame(void) {
    CheckButtons();
    CheckKeyboard();
    VGA.Activate();
    VGA.Clear();
    ByteStream paletteReader;
    paletteReader.Set((this->rawPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.SetPalette(&this->palette);

    VGA.DrawShape(this->layer);
    paletteReader.Set((this->forPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.SetPalette(&this->palette);
    for (int i = 0; i < this->sprites.size(); i++) {
        VGA.DrawShape(this->sprites[i]);
    }

    DrawButtons();

    //Draw Mouse
    Mouse.Draw();

    //Check Mouse state.

    VGA.VSync();

}