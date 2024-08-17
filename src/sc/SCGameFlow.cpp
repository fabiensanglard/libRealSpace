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
    this->fps = SDL_GetTicks() / 100;
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
                    z->position.x = sprt->rect->x1;
                    z->position.y = sprt->rect->y1;
                    z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
                    z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
                    z->label = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->label;
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
                    z->quad = sprt->quad;
                    z->label = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->label;
                    this->zones.push_back(z);
                }
                
                sprt->img = this->getShape(optsprtId);
                sprt->frameCounter = 0;
                if (this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->SEQU != nullptr) {
                    sprt->frames = this->optionParser.opts[optionScenID]->foreground->sprites[sprtId]->SEQU;
                    sprt->frameCounter = 0;
                }
                this->sprites.push_back(sprt);
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
    int fpsupdate = 0;
    fpsupdate = SDL_GetTicks() - this->fps > 80;
    if (fpsupdate) {
        this->fps = SDL_GetTicks();
    }
    for (int i = 0; i < this->sprites.size(); i++) {
        VGA.DrawShape(this->sprites[i]->img->GetShape(0));
        if (this->sprites[i]->img->GetNumImages() > 1 && this->sprites[i]->frames != nullptr) {
            VGA.DrawShape(this->sprites[i]->img->GetShape(this->sprites[i]->frames->at(this->sprites[i]->frameCounter)));
            this->sprites[i]->frameCounter = (this->sprites[i]->frameCounter + fpsupdate) % this->sprites[i]->frames->size();
        
        } else if (this->sprites[i]->img->GetNumImages() > 1 && this->sprites[i]->frames == nullptr) {
            VGA.DrawShape(this->sprites[i]->img->GetShape(this->sprites[i]->frameCounter));
            
            if (this->sprites[i]->frameCounter >= this->sprites[i]->img->GetNumImages()-1) {
                this->sprites[i]->frameCounter = 1;
            } else {
                this->sprites[i]->frameCounter += fpsupdate;
            }
            //this->sprites[i]->frameCounter = (this->sprites[i]->frameCounter + 1) % this->sprites[i]->img->GetNumImages()-1;
        }
        if (this->sprites[i]->rect != nullptr) {
            VGA.rect_slow(
                this->sprites[i]->rect->x1,
                this->sprites[i]->rect->y1,
                this->sprites[i]->rect->x2,
                this->sprites[i]->rect->y2,
                10
            );
        }
        if (this->sprites[i]->quad != nullptr) {
            /**/
            VGA.line(
                this->sprites[i]->quad->at(0)->x,
                this->sprites[i]->quad->at(0)->y,
                this->sprites[i]->quad->at(1)->x,
                this->sprites[i]->quad->at(1)->y,
                8
                );
            VGA.line(
                this->sprites[i]->quad->at(1)->x,
                this->sprites[i]->quad->at(1)->y,
                this->sprites[i]->quad->at(2)->x,
                this->sprites[i]->quad->at(2)->y,
                8
                );
            VGA.line(
                this->sprites[i]->quad->at(2)->x,
                this->sprites[i]->quad->at(2)->y,
                this->sprites[i]->quad->at(3)->x,
                this->sprites[i]->quad->at(3)->y,
                8
                );

            VGA.line(
                this->sprites[i]->quad->at(3)->x,
                this->sprites[i]->quad->at(3)->y,
                this->sprites[i]->quad->at(0)->x,
                this->sprites[i]->quad->at(0)->y,
                25
            );
            /**/
        }
    }
    CheckZones();
    //DrawButtons();

    //Draw Mouse
    Mouse.Draw();

    //Check Mouse state.

    VGA.VSync();
}