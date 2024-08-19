//
//  File.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCGameFlow.h"
#include "precomp.h"


#define EFECT_OPT_CONV 0
#define EFECT_OPT_SCEN 1
#define EFECT_OPT_MISS 22
#define EFECT_OPT_MIS2 13
#define EFECT_OPT_SHOT 8
#define EFECT_OPT_FLYM 12

SCGameFlow::SCGameFlow() {
    this->current_miss = 0;
    this->current_scen = 0;
    this->efect = nullptr;
    this->currentOptCode = 0;
    this->fps = SDL_GetTicks() / 100;
}

SCGameFlow::~SCGameFlow() {
}

/**
 * Handles a click event on a sprite.
 *
 * @param id The ID of the sprite that was clicked.
 *
 * @return None
 */
void SCGameFlow::clicked(uint8_t id) {
    if (this->sprites[id]->efect != nullptr) {
        printf("clicked on %d\n", id);
        this->efect = this->sprites[id]->efect;
        this->currentOptCode = 0;
    }
    
}
/**
 * Runs the current effect based on the efect vector and currentOptCode.
 *
 * Iterates through the efect vector, executing different actions based on the opcode.
 * Actions include playing conversations, scenes, and missions, as well as handling unknown opcodes.
 *
 * @return None
 */
void SCGameFlow::runEffect() {
    uint8_t i = this->currentOptCode;
    if (this->efect == nullptr) {
        return;
    }
    if (i >= this->efect->size()) {
        this->efect = new std::vector<uint8_t>();
        this->currentOptCode = 0;
        return;
    }
    if (i + 1 < this->efect->size()) {
        this->currentOptCode += 2;
        switch (this->efect->at(i)) {
        case EFECT_OPT_CONV:
        {
            printf("PLAYING CONV %d\n", this->efect->at(i + 1));
            SCConvPlayer* conv = new SCConvPlayer();
            conv->Init();
            conv->SetID(this->efect->at(i + 1));
            Game.AddActivity(conv);
        }
        break;
        case EFECT_OPT_SCEN:
            for (int j = 0; j < this->gameFlowParser.game.game[this->current_miss]->scen.size(); j++) {
                if (this->gameFlowParser.game.game[this->current_miss]->scen.at(j)->info.ID == this->efect->at(i + 1)) {
                    this->current_scen = j;
                    printf("PLAYING SCEN %d\n", this->current_scen);
                    this->createMiss();
                    return;
                }
            }
            break;
        case EFECT_OPT_MISS:
        {
            this->current_miss = this->efect->at(i + 1);
            this->current_scen = 0;
            this->efect = nullptr;
            printf("PLAYING MISS %d\n", this->current_miss);
            this->createMiss();
            return;
        }
            break;
        case EFECT_OPT_MIS2:
            printf("PLAYING MIS2 %d\n", this->current_miss);
            printf("MIS2 NOT IMPLEMENTED\n");
            break;
        case EFECT_OPT_SHOT:
            printf("PLAYING SHOT %d\n", this->efect->at(i + 1));
            printf("SHOT NOT IMPLEMENTED\n");
            break;
        case EFECT_OPT_FLYM:
        {
            uint8_t flymID = this->efect->at(i + 1);
            printf("PLAYING FLYM %d\n", flymID);
            printf("Mission Name %s\n", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            printf("FLYM NOT IMPLEMENTED\n");
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
/**
 * Checks for keyboard events and updates the game state accordingly.
 *
 * This function peeks at the SDL event queue to check for keyboard events.
 * If a key is pressed, it updates the game state based on the key pressed.
 * The supported keys are escape, space, and return.
 *
 * @return None
 */
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
            this->efect = nullptr;
            this->currentOptCode = 0;
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
            return;
            break;
        default:
            break;
        }
    }
}

/**
 * Initializes the SCGameFlow object by loading the necessary IFF files and initializing the optionParser and gameFlowParser.
 * It also initializes the optShps and optPals objects by loading the OPTSHPS.PAK and OPTPALS.PAK files respectively.
 * Finally, it sets the efect pointer to nullptr and calls the createMiss() function.
 *
 * @throws None
 */
void SCGameFlow::Init() {

    TreEntry* gameflowIFF = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\GAMEFLOW.IFF");
    TreEntry* optionIFF = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTIONS.IFF");
    this->optionParser.InitFromRam(optionIFF->data, optionIFF->size);
    this->gameFlowParser.InitFromRam(gameflowIFF->data, gameflowIFF->size);

    TreEntry* optShapEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry* optPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    this->efect = nullptr;
    this->createMiss();
}

/**
 * Creates a new miss in the game flow.
 *
 * This function initializes the efect, sprites, and zones for the current miss.
 * It also sets up the layer, raw palette, and foreground palette.
 *
 * @return None
 *
 * @throws None
 */
void SCGameFlow::createMiss() {
    printf("current miss : %d, current_scen %d\n", this->current_miss, this->current_scen);
    if (this->efect == nullptr && this->gameFlowParser.game.game[this->current_miss]->efct.size() > 0) {
        this->efect = &this->gameFlowParser.game.game[this->current_miss]->efct;
    }
    printf("efect size %zd\n", this->efect->size());
    if (this->gameFlowParser.game.game[this->current_miss]->scen.size() > 0) {
        uint8_t optionScenID = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->info.ID;
        // note pour plus tard, une scene peu être composé de plusieur background
        // donc il faut boucler.
        uint8_t shapeID = this->optionParser.opts[optionScenID]->background->images[0]->ID;
        uint8_t paltID = this->optionParser.opts[optionScenID]->background->palette->ID;
        uint8_t forPalTID = this->optionParser.opts[optionScenID]->foreground->palette->ID;
        
        
        this->sprites.clear();
        this->zones.clear();
        for (auto sprite : this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->sprt) {
            uint8_t sprtId = sprite->info.ID;
            // le clck dans sprite semble indiquer qu'il faut jouer l'animation après avoir cliquer et donc executer le efect 
            // à la fin de l'animation.
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
                
                sprt->efect = &sprite->efct;
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
        // le layer de fonds peut être une animation, il faudra changer en RSimageSet
        this->layer = this->getShape(shapeID)->GetShape(0);
        this->rawPalette = this->optPals.GetEntry(paltID)->data;
        this->forPalette = this->optPals.GetEntry(forPalTID)->data;
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

/**
 * Runs a single frame of the game, updating the game state and rendering the graphics.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
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
            sprit.second->frameCounter = (sprit.second->frameCounter + fpsupdate) % static_cast<uint8_t>(sprit.second->frames->size());

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