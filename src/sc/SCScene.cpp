#include "precomp.h"
#include "SCScene.h"

SCScene::SCScene(PakArchive *optShps, PakArchive *optPals) {
    this->optShps = optShps;
    this->optPals = optPals;
}

SCScene::~SCScene() {}

std::vector<SCZone *>  SCScene::Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick) {
    this->gameflow_scene = gf;
    this->sceneOpts = sc_opts;
    this->zones.clear();
    this->zones.shrink_to_fit();
    for (auto bg : sceneOpts->background->images) {
        background *tmpbg = new background();
        tmpbg->img = this->getShape(bg->ID);
        this->layers.push_back(tmpbg);
    }
    uint8_t paltID = sceneOpts->background->palette->ID;
    uint8_t forPalTID = sceneOpts->foreground->palette->ID;

    this->rawPalette = this->optPals->GetEntry(paltID)->data;
    this->forPalette = this->optPals->GetEntry(forPalTID)->data;

    uint8_t optionScenID = gameflow_scene->info.ID;
    for (auto sprite : this->gameflow_scene->sprt) {
        uint8_t sprtId = sprite->info.ID;
        // le clck dans sprite semble indiquer qu'il faut jouer l'animation après avoir cliquer et donc executer le
        // efect à la fin de l'animation.
        uint8_t zone_id = 0;
        if (this->sceneOpts->foreground->sprites.count(sprtId) > 0) {
            uint8_t optsprtId = this->sceneOpts->foreground->sprites[sprtId]->sprite.SHP_ID;
            animatedSprites *sprt = new animatedSprites();
            SCZone *z = new SCZone();
            if (this->sceneOpts->foreground->sprites[sprtId]->zone != nullptr) {
                sprt->rect = new sprtRect();
                sprt->rect->x1 = this->sceneOpts->foreground->sprites[sprtId]->zone->X1;
                sprt->rect->y1 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y1;
                sprt->rect->x2 = this->sceneOpts->foreground->sprites[sprtId]->zone->X2;
                sprt->rect->y2 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y2;

                z->id = sprtId;
                z->position.x = sprt->rect->x1;
                z->position.y = sprt->rect->y1;
                z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
                z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
                z->label = this->sceneOpts->foreground->sprites[sprtId]->label;
                z->onclick = onclick;
                z->quad = nullptr;
            }
            if (sceneOpts->foreground->sprites[sprtId]->quad != nullptr) {
                sprt->quad = new std::vector<Point2D *>();

                Point2D *p;

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya2;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb2;
                sprt->quad->push_back(p);

                z->id = sprtId;
                z->quad = sprt->quad;
                z->label = sceneOpts->foreground->sprites[sprtId]->label;
                z->onclick = onclick;
            }

            sprt->id = sprite->info.ID;
            sprt->shapid = optsprtId;
            sprt->unkown = sprite->info.UNKOWN;
            sprt->efect = sprite->efct;
            sprt->img = this->getShape(optsprtId);
            sprt->frameCounter = 0;
            if (this->sceneOpts->foreground->sprites[sprtId]->CLCK == 1) {
                sprt->cliked = true;
                sprt->frameCounter = 1;
            }
            if (this->sceneOpts->foreground->sprites[sprtId]->SEQU != nullptr) {
                sprt->frames = this->sceneOpts->foreground->sprites[sprtId]->SEQU;
                sprt->frameCounter = 0;
            }
            sprt->requ = sprite->requ;
            z->sprite = sprt;
            z->id = zone_id;
            zone_id++;
            this->zones.push_back(z);
        } else {
            printf("%d, ID Sprite not found !!\n", sprtId);
        }
    }
    for (auto zn: this->zones) {
        zn->IsActive(&GameState.requierd_flags);
    }
    return (this->zones);
}

void SCScene::Render() {
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks() / 10);
    }
    ByteStream paletteReader;
    paletteReader.Set((this->rawPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.SetPalette(&this->palette);
    for (auto layer : this->layers) {
        VGA.DrawShape(layer->img->GetShape(layer->img->sequence[layer->frameCounter]));
        if (layer->img->sequence.size() > 1) {
            layer->frameCounter = (uint8_t)(layer->frameCounter + fpsupdate) % layer->img->sequence.size();
        }
    }

    paletteReader.Set((this->forPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.SetPalette(&this->palette);
    if (this->zones.size() > 0) {
        for (auto zn: this->zones) {
            if (zn->active) {
                zn->Draw();   
            }
        }    
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
RSImageSet *SCScene::getShape(uint8_t shpid) {
    PakEntry *shapeEntry = this->optShps->GetEntry(shpid);
    PakArchive subPAK;

    subPAK.InitFromRAM("", shapeEntry->data, shapeEntry->size);
    RSImageSet *img = new RSImageSet();
    if (!subPAK.IsReady()) {
        img->InitFromPakEntry(this->optShps->GetEntry(shpid));
    } else {
        img->InitFromSubPakEntry(&subPAK);
    }
    return (img);
}

std::vector<SCZone *> WeaponLoadoutScene::Init(
    GAMEFLOW_SCEN *gf,
    SCEN *sc_opts,
    std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick
) {
    this->gameflow_scene = gf;
    this->sceneOpts = sc_opts;
    this->zones.clear();
    for (auto bg : sceneOpts->background->images) {
        background *tmpbg = new background();
        tmpbg->img = this->getShape(bg->ID);
        this->layers.push_back(tmpbg);
    }
    uint8_t paltID = sceneOpts->background->palette->ID;
    uint8_t forPalTID = sceneOpts->foreground->palette->ID;

    this->rawPalette = this->optPals->GetEntry(paltID)->data;
    this->forPalette = this->optPals->GetEntry(forPalTID)->data;

    uint8_t optionScenID = gameflow_scene->info.ID;
    if (gameflow_scene->info.ID == 20) {
        SCZone *z = new SCZone();
        z->id = 255;
        animatedSprites *sprt = new animatedSprites();
        sprt->img = this->getShape(this->sceneOpts->extr[0]->SHAPE_ID);
        sprt->id = this->sceneOpts->extr[0]->EXTR_ID;
        z->sprite = sprt;
        this->zones.push_back(z);
    }
    for (auto sprts : this->sceneOpts->foreground->sprites) {
        uint8_t sprtId = sprts.first;
        SPRT *sprite = sprts.second;
        // le clck dans sprite semble indiquer qu'il faut jouer l'animation après avoir cliquer et donc executer le
        // efect à la fin de l'animation.
        if (this->sceneOpts->foreground->sprites.count(sprtId) > 0) {
            uint8_t optsprtId = this->sceneOpts->foreground->sprites[sprtId]->sprite.SHP_ID;
            animatedSprites *sprt = new animatedSprites();
            SCZone *z = new SCZone();
            if (this->sceneOpts->foreground->sprites[sprtId]->zone != nullptr) {
                sprt->rect = new sprtRect();
                sprt->rect->x1 = this->sceneOpts->foreground->sprites[sprtId]->zone->X1;
                sprt->rect->y1 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y1;
                sprt->rect->x2 = this->sceneOpts->foreground->sprites[sprtId]->zone->X2;
                sprt->rect->y2 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y2;

                z->id = sprtId;
                z->position.x = sprt->rect->x1;
                z->position.y = sprt->rect->y1;
                z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
                z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
                z->label = this->sceneOpts->foreground->sprites[sprtId]->label;
                z->quad = nullptr;
            }
            if (sceneOpts->foreground->sprites[sprtId]->quad != nullptr) {
                sprt->quad = new std::vector<Point2D *>();

                Point2D *p;

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya2;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb2;
                sprt->quad->push_back(p);

                z->id = sprtId;
                z->quad = sprt->quad;
                z->label = sceneOpts->foreground->sprites[sprtId]->label;
            }

            sprt->id = sprtId;
            sprt->shapid = optsprtId;
            sprt->img = this->getShape(optsprtId);
            sprt->frameCounter = 0;
            if (this->sceneOpts->foreground->sprites[sprtId]->CLCK == 1) {
                sprt->cliked = true;
                sprt->frameCounter = 1;
            }
            if (this->sceneOpts->foreground->sprites[sprtId]->SEQU != nullptr) {
                sprt->frames = this->sceneOpts->foreground->sprites[sprtId]->SEQU;
                sprt->frameCounter = 0;
            }
            z->sprite = sprt;
            if (this->gameflow_scene->info.ID == sprtId || sprtId == 12) {
                z->onclick = onclick;
                z->active = true;
                this->zones.push_back(z);
            }
            
        } else {
            printf("%d, ID Sprite not found !!\n", sprtId);
        }
    }
    return (this->zones);
}

std::vector<SCZone *> LedgerScene::Init(
    GAMEFLOW_SCEN *gf,
    SCEN *sc_opts,
    std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick
) {
    this->gameflow_scene = gf;
    this->sceneOpts = sc_opts;
    this->zones.clear();
    for (auto bg : sceneOpts->background->images) {
        background *tmpbg = new background();
        tmpbg->img = this->getShape(bg->ID);
        this->layers.push_back(tmpbg);
    }
    uint8_t paltID = sceneOpts->background->palette->ID;
    uint8_t forPalTID = sceneOpts->foreground->palette->ID;

    this->rawPalette = this->optPals->GetEntry(paltID)->data;
    this->forPalette = this->optPals->GetEntry(forPalTID)->data;

    uint8_t optionScenID = gameflow_scene->info.ID;
    
    for (auto sprts : this->sceneOpts->foreground->sprites) {
        uint8_t sprtId = sprts.first;
        SPRT *sprite = sprts.second;
        // le clck dans sprite semble indiquer qu'il faut jouer l'animation après avoir cliquer et donc executer le
        // efect à la fin de l'animation.
        uint8_t zone_id = 0;
        if (this->sceneOpts->foreground->sprites.count(sprtId) > 0) {
            uint8_t optsprtId = this->sceneOpts->foreground->sprites[sprtId]->sprite.SHP_ID;
            animatedSprites *sprt = new animatedSprites();
            SCZone *z = new SCZone();
            if (this->sceneOpts->foreground->sprites[sprtId]->zone != nullptr) {
                sprt->rect = new sprtRect();
                sprt->rect->x1 = this->sceneOpts->foreground->sprites[sprtId]->zone->X1;
                sprt->rect->y1 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y1;
                sprt->rect->x2 = this->sceneOpts->foreground->sprites[sprtId]->zone->X2;
                sprt->rect->y2 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y2;

                z->id = sprtId;
                z->position.x = sprt->rect->x1;
                z->position.y = sprt->rect->y1;
                z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
                z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
                z->label = this->sceneOpts->foreground->sprites[sprtId]->label;
                z->quad = nullptr;
            }
            if (sceneOpts->foreground->sprites[sprtId]->quad != nullptr) {
                sprt->quad = new std::vector<Point2D *>();

                Point2D *p;

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya2;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb2;
                sprt->quad->push_back(p);

                z->id = sprtId;
                z->quad = sprt->quad;
                z->label = sceneOpts->foreground->sprites[sprtId]->label;
            }

            sprt->id = sprtId;
            sprt->shapid = optsprtId;
            sprt->img = this->getShape(optsprtId);
            sprt->frameCounter = 0;
            if (this->sceneOpts->foreground->sprites[sprtId]->CLCK == 1) {
                sprt->cliked = true;
                sprt->frameCounter = 1;
            }
            if (this->sceneOpts->foreground->sprites[sprtId]->SEQU != nullptr) {
                sprt->frames = this->sceneOpts->foreground->sprites[sprtId]->SEQU;
                sprt->frameCounter = 0;
            }
            z->sprite = sprt;
            z->id = zone_id;
            zone_id++;
            if (this->gameflow_scene->info.ID == sprtId) {
                z->onclick = onclick;
                z->active = true;
                this->zones.push_back(z);
            }
            
        } else {
            printf("%d, ID Sprite not found !!\n", sprtId);
        }
    }
    return (this->zones);
}

std::vector<SCZone *> CatalogueScene::Init(
    GAMEFLOW_SCEN *gf,
    SCEN *sc_opts,
    std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick
) {
    this->gameflow_scene = gf;
    this->sceneOpts = sc_opts;
    this->zones.clear();
    for (auto bg : sceneOpts->background->images) {
        background *tmpbg = new background();
        tmpbg->img = this->getShape(bg->ID);
        this->layers.push_back(tmpbg);
    }
    uint8_t paltID = sceneOpts->background->palette->ID;
    uint8_t forPalTID = sceneOpts->foreground->palette->ID;

    this->rawPalette = this->optPals->GetEntry(paltID)->data;
    this->forPalette = this->optPals->GetEntry(forPalTID)->data;

    uint8_t optionScenID = gameflow_scene->info.ID;
    
    for (auto sprts : this->sceneOpts->foreground->sprites) {
        uint8_t sprtId = sprts.first;
        SPRT *sprite = sprts.second;
        // le clck dans sprite semble indiquer qu'il faut jouer l'animation après avoir cliquer et donc executer le
        // efect à la fin de l'animation.
        uint8_t zone_id = 0;
        if (this->sceneOpts->foreground->sprites.count(sprtId) > 0) {
            uint8_t optsprtId = this->sceneOpts->foreground->sprites[sprtId]->sprite.SHP_ID;
            animatedSprites *sprt = new animatedSprites();
            SCZone *z = new SCZone();
            if (this->sceneOpts->foreground->sprites[sprtId]->zone != nullptr) {
                sprt->rect = new sprtRect();
                sprt->rect->x1 = this->sceneOpts->foreground->sprites[sprtId]->zone->X1;
                sprt->rect->y1 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y1;
                sprt->rect->x2 = this->sceneOpts->foreground->sprites[sprtId]->zone->X2;
                sprt->rect->y2 = this->sceneOpts->foreground->sprites[sprtId]->zone->Y2;

                z->id = sprtId;
                z->position.x = sprt->rect->x1;
                z->position.y = sprt->rect->y1;
                z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
                z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
                z->label = this->sceneOpts->foreground->sprites[sprtId]->label;
                z->quad = nullptr;
            }
            if (sceneOpts->foreground->sprites[sprtId]->quad != nullptr) {
                sprt->quad = new std::vector<Point2D *>();

                Point2D *p;

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xa2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->ya2;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb1;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb1;
                sprt->quad->push_back(p);

                p = new Point2D();
                p->x = sceneOpts->foreground->sprites[sprtId]->quad->xb2;
                p->y = sceneOpts->foreground->sprites[sprtId]->quad->yb2;
                sprt->quad->push_back(p);

                z->id = sprtId;
                z->quad = sprt->quad;
                z->label = sceneOpts->foreground->sprites[sprtId]->label;
            }

            sprt->id = sprtId;
            sprt->shapid = optsprtId;
            sprt->img = this->getShape(optsprtId);
            sprt->frameCounter = 0;
            if (this->sceneOpts->foreground->sprites[sprtId]->CLCK == 1) {
                sprt->cliked = true;
                sprt->frameCounter = 1;
            }
            if (this->sceneOpts->foreground->sprites[sprtId]->SEQU != nullptr) {
                sprt->frames = this->sceneOpts->foreground->sprites[sprtId]->SEQU;
                sprt->frameCounter = 0;
            }
            z->sprite = sprt;
            z->id = zone_id;
            zone_id++;
            if (this->gameflow_scene->info.ID == sprtId) {
                z->onclick = onclick;
                z->active = true;
                this->zones.push_back(z);
            }
            
        } else {
            printf("%d, ID Sprite not found !!\n", sprtId);
        }
    }
    return (this->zones);
}
