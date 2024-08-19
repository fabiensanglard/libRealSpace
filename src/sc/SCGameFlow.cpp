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
    this->efect = nullptr;
    this->currentOptCode = 0;
    this->fps = SDL_GetTicks() / 100;
}

SCGameFlow::~SCGameFlow() {
}

void SCGameFlow::clicked(uint8_t id) {
    if (this->sprites[id]->efect != nullptr) {
        printf("clicked on %d\n", id);
        this->efect = this->sprites[id]->efect;
        this->currentOptCode = 0;
    }
    
}
void SCGameFlow::runEffect() {
    uint8_t i = this->currentOptCode;
    if (this->efect == nullptr) {
        return;
    }
    if (i + 1 < this->efect->size()) {
        this->currentOptCode += 2;
        switch (this->efect->at(i)) {
        case 0:
        {
            SCConvPlayer* conv = new SCConvPlayer();
            conv->Init();
            conv->SetID(this->efect->at(i + 1));
            Game.AddActivity(conv);
        }
        break;
        case 1:
            for (int j = 0; j < this->gameFlowParser.game.game[this->current_miss]->scen.size(); j++) {
                if (this->gameFlowParser.game.game[this->current_miss]->scen.at(j)->info.ID == this->efect->at(i + 1)) {
                    this->current_scen = j;
                    this->createMiss();
                    return;
                }
            }
            break;
        case 22:
        {
            this->current_miss = this->efect->at(i + 1);
            this->current_scen = 0;
            this->createMiss();
            return;
        }
            break;
        default:
            printf("Unkown opcode :%d, %d\n", this->efect->at(i), this->efect->at(i + 1));
            break;
        };
        
    } else {
        this->efect = nullptr;
        this->currentOptCode = 0;
    }
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
        this->zones.clear();

        for (int i=0; i < this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->sprt.size(); i++) {
            uint8_t sprtId = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->sprt[i]->info.ID;
            if (this->optionParser.opts[optionScenID]->foreground->sprites.count(sprtId) > 0) {
                uint8_t optsprtId = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->sprite.SHP_ID;
                animatedSprites* sprt = new animatedSprites();
                if (this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->zone != nullptr) {
                    sprt->rect = new sprtRect();
                    sprt->rect->x1 = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->zone->X1;
                    sprt->rect->y1 = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->zone->Y1;
                    sprt->rect->x2 = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->zone->X2;
                    sprt->rect->y2 = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->zone->Y2;
                    SCZone* z = new SCZone();
                    z->id = sprtId;
                    z->position.x = sprt->rect->x1;
                    z->position.y = sprt->rect->y1;
                    z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
                    z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
                    z->label = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->label;
                    z->onclick = std::bind(&SCGameFlow::clicked, this, std::placeholders::_1);
                    z->quad = nullptr;
                    this->zones.push_back(z);
                }
                if (this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad != nullptr) {
                    sprt->quad = new std::vector<Point2D*>();

                    Point2D* p;

                    p = new Point2D();
                    p->x = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->xa1;
                    p->y = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->ya1;
                    sprt->quad->push_back(p);

                    p = new Point2D();
                    p->x = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->xa2;
                    p->y = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->ya2;
                    sprt->quad->push_back(p);

                    p = new Point2D();
                    p->x = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->xb1;
                    p->y = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->yb1;
                    sprt->quad->push_back(p);

                    p = new Point2D();
                    p->x = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->xb2;
                    p->y = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->quad->yb2;
                    sprt->quad->push_back(p);

                    SCZone* z = new SCZone();
                    z->id = sprtId;
                    z->quad = sprt->quad;
                    z->label = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->label;
                    z->onclick = std::bind(&SCGameFlow::clicked, this, std::placeholders::_1);
                    this->zones.push_back(z);
                }
                
                sprt->efect = &this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->sprt[i]->efct;
                sprt->img = this->getShape(optsprtId);
                sprt->frameCounter = 0;
                if (this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->SEQU != nullptr) {
                    sprt->frames = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->SEQU;
                    sprt->frameCounter = 0;
                }
                this->sprites[sprtId]=sprt;
            } else {
                printf("%d, ID Sprite not found !!\n", sprtId);
            }
            
        }
        this->layer = this->getShape(shapeID)->GetShape(0);
        this->rawPalette = this->optPals.GetEntry(paltID)->data;
        this->forPalette = this->optPals.GetEntry(forPalTID)->data;
    }
}
RSImageSet* SCGameFlow::getShape(uint8_t shpid) {
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

void SCGameFlow::RunFrame(void) {
    this->runEffect();   
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
    int fpsupdate = 0;
    fpsupdate = SDL_GetTicks() - this->fps > 80;
    if (fpsupdate) {
        this->fps = SDL_GetTicks();
    }

    for (const auto &sprit : this->sprites) {
        VGA.DrawShape(sprit.second->img->GetShape(0));
        if (sprit.second->img->GetNumImages() > 1 && sprit.second->frames != nullptr) {
            VGA.DrawShape(sprit.second->img->GetShape(sprit.second->frames->at(sprit.second->frameCounter)));
            sprit.second->frameCounter = (sprit.second->frameCounter + fpsupdate) % static_cast<size_t>(sprit.second->frames->size());

        } else if (sprit.second->img->GetNumImages() > 1 && sprit.second->frames == nullptr) {
            VGA.DrawShape(sprit.second->img->GetShape(sprit.second->frameCounter));

            if (sprit.second->frameCounter >= sprit.second->img->GetNumImages() - 1) {
                sprit.second->frameCounter = 1;
            } else {
                sprit.second->frameCounter += fpsupdate;
            }
        }
    }
    this->CheckZones();

    for (int f = 0; f < this->zones.size(); f++) {
        this->zones.at(f)->Draw();
    }

    Mouse.Draw();

    VGA.VSync();
}