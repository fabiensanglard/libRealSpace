#include "precomp.h"
#include "SCScene.h"
#include <memory>
#include <string>
#include <stdexcept>
#pragma warning(disable : 4996)
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

SCScene::SCScene(PakArchive *optShps, PakArchive *optPals) {
    this->optShps = optShps;
    this->optPals = optPals;
}

SCScene::~SCScene() {}

std::vector<SCZone *>  *SCScene::Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick) {
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
    return (&this->zones);
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

SCZone *SCScene::createZone(SPRT *sprts, uint8_t sprtId) { 

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
    return (z);
}

SCZone *WeaponLoadoutScene::addExtraZone(uint8_t id) {
    SCZone *z{nullptr};
    animatedSprites *sprt{nullptr};
    EXTR_SHAP *extr{nullptr};
    extr = this->sceneOpts->extr[id];
    sprt = new animatedSprites();
    z = new SCZone();
    sprt->img = this->getShape(extr->SHAPE_ID);
    sprt->id = extr->EXTR_ID;
    z->sprite = sprt;
    return (z);
 }

 SCZone *WeaponLoadoutScene::addNoGfXZone(uint8_t id, std::string label, uint8_t weapon_type_id) { 
    SCZone *z{nullptr};
    animatedSprites *sprt{nullptr};
    sprt = new animatedSprites();
    z = new SCZone();

    if (this->sceneOpts->foreground->sprites[id]->zone != nullptr) {
        sprt->rect = new sprtRect();
        sprt->rect->x1 = this->sceneOpts->foreground->sprites[id]->zone->X1;
        sprt->rect->y1 = this->sceneOpts->foreground->sprites[id]->zone->Y1;
        sprt->rect->x2 = this->sceneOpts->foreground->sprites[id]->zone->X2;
        sprt->rect->y2 = this->sceneOpts->foreground->sprites[id]->zone->Y2;

        z->position.x = sprt->rect->x1;
        z->position.y = sprt->rect->y1;
        z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
        z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
        z->quad = nullptr;
    }
    if (sceneOpts->foreground->sprites[id]->quad != nullptr) {
        sprt->quad = new std::vector<Point2D *>();

        Point2D *p;

        p = new Point2D();
        p->x = sceneOpts->foreground->sprites[id]->quad->xa1;
        p->y = sceneOpts->foreground->sprites[id]->quad->ya1;
        sprt->quad->push_back(p);

        p = new Point2D();
        p->x = sceneOpts->foreground->sprites[id]->quad->xa2;
        p->y = sceneOpts->foreground->sprites[id]->quad->ya2;
        sprt->quad->push_back(p);

        p = new Point2D();
        p->x = sceneOpts->foreground->sprites[id]->quad->xb1;
        p->y = sceneOpts->foreground->sprites[id]->quad->yb1;
        sprt->quad->push_back(p);

        p = new Point2D();
        p->x = sceneOpts->foreground->sprites[id]->quad->xb2;
        p->y = sceneOpts->foreground->sprites[id]->quad->yb2;
        sprt->quad->push_back(p);
        z->quad = sprt->quad;
    }
    sprt->id = id;
    z->id = weapon_type_id;
    z->sprite = nullptr;
    z->label = new std::string(label);
    z->onclick = std::bind(&WeaponLoadoutScene::removeWeapon, this, std::placeholders::_1, std::placeholders::_2);
    return (z);
 }

 void WeaponLoadoutScene::addWeapon(std::vector<EFCT *> *script, uint8_t id) {

     this->extra_zones.clear();
     this->extra_zones.shrink_to_fit();
     switch (id) {
     case AIM9J:
         if (GameState.weapon_load_out.find(AIM9J) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[AIM9J] = 0;
         }
         if (GameState.weapon_load_out[AIM9J] < 6) {
             GameState.weapon_load_out[AIM9J] += 2;
         }
         break;
     case AIM9M:
         if (GameState.weapon_load_out.find(AIM9M) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[AIM9M] = 0;
         }
         if (GameState.weapon_load_out[AIM9M] < 6) {
             GameState.weapon_load_out[AIM9M] += 2;
         }
         break;
     case AIM120:
         if (GameState.weapon_load_out.find(AIM120) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[AIM120] = 0;
         }
         if (GameState.weapon_load_out[AIM120] < 6) {
             GameState.weapon_load_out[AIM120] += 2;
         }
         break;
     case DURANDAL:
         if (GameState.weapon_load_out.find(DURANDAL) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[DURANDAL] = 0;
         }
         if (GameState.weapon_load_out[DURANDAL] < 12) {
             GameState.weapon_load_out[DURANDAL] += 2;
         }
         break;
     case MK82:
         if (GameState.weapon_load_out.find(MK82) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[MK82] = 0;
         }
         if (GameState.weapon_load_out[MK82] < 24) {
             GameState.weapon_load_out[MK82] += 2;
         }
         break;
     case AGM65D:
         if (GameState.weapon_load_out.find(AGM65D) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[AGM65D] = 0;
         }
         if (GameState.weapon_load_out[AGM65D] < 12) {
             GameState.weapon_load_out[AGM65D] += 2;
         }
         break;
     case MK20:
         if (GameState.weapon_load_out.find(MK20) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[MK20] = 0;
         }
         if (GameState.weapon_load_out[MK20] < 12) {
             GameState.weapon_load_out[MK20] += 2;
         }
         break;
     case GBU15:
         if (GameState.weapon_load_out.find(GBU15) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[GBU15] = 0;
         }
         if (GameState.weapon_load_out[GBU15] < 4) {
             GameState.weapon_load_out[GBU15] += 2;
         }
         break;
     case LAU3:
         if (GameState.weapon_load_out.find(LAU3) == GameState.weapon_load_out.end()) {
             GameState.weapon_load_out[LAU3] = 0;
         }
         if (GameState.weapon_load_out[LAU3] < 10) {
             GameState.weapon_load_out[LAU3] += 2;
         }
         break;
     }
     this->updateWeaponDisplay();
}

void WeaponLoadoutScene::removeWeapon(std::vector<EFCT *> *script, uint8_t id) {
        
    this->extra_zones.clear();
    this->extra_zones.shrink_to_fit();
    switch (id) {
        case AIM9J:
            if (GameState.weapon_load_out.find(AIM9J) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[AIM9J]=0;    
            }
            if (GameState.weapon_load_out[AIM9J] > 0) {
                GameState.weapon_load_out[AIM9J]-=2;
            }
        break;
        case AIM9M:
            if (GameState.weapon_load_out.find(AIM9M) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[AIM9M]=0;    
            }
            if (GameState.weapon_load_out[AIM9M] > 0) {
                GameState.weapon_load_out[AIM9M]-=2;
            }
        break;
        case AIM120:
            if (GameState.weapon_load_out.find(AIM120) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[AIM120]=0;    
            }
            if (GameState.weapon_load_out[AIM120] > 0) {
                GameState.weapon_load_out[AIM120]-=2;
            }
        break;
        case DURANDAL:
            if (GameState.weapon_load_out.find(DURANDAL) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[DURANDAL]=0;    
            }
            if (GameState.weapon_load_out[DURANDAL] > 0) {
                GameState.weapon_load_out[DURANDAL]-=2;
            }
        break;
        case MK82:
            if (GameState.weapon_load_out.find(MK82) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[MK82]=0;    
            }
            if (GameState.weapon_load_out[MK82] > 0) {
                GameState.weapon_load_out[MK82]-=2;
            }
        break;
        case AGM65D:
            if (GameState.weapon_load_out.find(AGM65D) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[AGM65D]=0;    
            }
            if (GameState.weapon_load_out[AGM65D] > 0) {
                GameState.weapon_load_out[AGM65D]-=2;
            }
        break;
        case MK20:
            if (GameState.weapon_load_out.find(MK20) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[MK20]=0;    
            }
            if (GameState.weapon_load_out[MK20] > 0) {
                GameState.weapon_load_out[MK20]-=2;
            }
        break;
        case GBU15:
            if (GameState.weapon_load_out.find(GBU15) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[GBU15]=0;    
            }
            if (GameState.weapon_load_out[GBU15] > 0) {
                GameState.weapon_load_out[GBU15]-=2;
            }
        break;
        case LAU3:
            if (GameState.weapon_load_out.find(LAU3) == GameState.weapon_load_out.end()) {
                GameState.weapon_load_out[LAU3]=0;    
            }
            if (GameState.weapon_load_out[LAU3] > 0) {
                GameState.weapon_load_out[LAU3]-=2;
            }
        break;
    }
    this->updateWeaponDisplay();
}

void WeaponLoadoutScene::updateWeaponDisplay() {

    this->zones.clear();
    this->zones.shrink_to_fit();
    SCZone *z=createZone(this->sceneOpts->foreground->sprites[this->gameflow_scene->info.ID], this->gameflow_scene->info.ID);
    z->onclick = onclick;
    z->active = true;
    this->zones.push_back(z);
    
    // FLY MISSION
    z=createZone(this->sceneOpts->foreground->sprites[12], 12);
    z->onclick = onclick;
    z->active = true;
    this->zones.push_back(z);    
    
    z=createZone(this->sceneOpts->foreground->sprites[AIM9J], AIM9J);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[AIM9M], AIM9M);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[AIM120], AIM120);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[DURANDAL], DURANDAL);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[MK82], MK82);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[AGM65D], AGM65D);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[MK20], MK20);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[GBU15], GBU15);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);

    z=createZone(this->sceneOpts->foreground->sprites[LAU3], LAU3);
    z->active = true;
    z->onclick = std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2);
    this->zones.push_back(z);
    std::string wlabel;
    for (auto wpl : GameState.weapon_load_out) {
        switch (wpl.first) {
            case AIM9J:
                wlabel = string_format("AIM9J(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(4));
                    this->zones.push_back(this->addNoGfXZone(117, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(118, wlabel, AIM9J));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(4));
                    this->extra_zones.push_back(this->addExtraZone(5));
                    this->zones.push_back(this->addNoGfXZone(117, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(118, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM9J));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(4));
                    this->extra_zones.push_back(this->addExtraZone(5));
                    this->extra_zones.push_back(this->addExtraZone(40));
                    this->zones.push_back(this->addNoGfXZone(117, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(118, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AIM9J));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AIM9J));
                }
            break;
            case AIM9M:
                wlabel = string_format("AIM9M(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(6));
                    this->zones.push_back(this->addNoGfXZone(117, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(118, wlabel, AIM9M));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(6));
                    this->extra_zones.push_back(this->addExtraZone(7));
                    this->zones.push_back(this->addNoGfXZone(117, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(118, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM9M));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(6));
                    this->extra_zones.push_back(this->addExtraZone(7));
                    this->extra_zones.push_back(this->addExtraZone(41));
                    this->zones.push_back(this->addNoGfXZone(117, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(118, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AIM9M));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AIM9M));
                }
            break;
            case AIM120:
                wlabel = string_format("AIM120(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(1));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM120));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(1));
                    this->extra_zones.push_back(this->addExtraZone(2));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AIM120));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(1));
                    this->extra_zones.push_back(this->addExtraZone(2));
                    this->extra_zones.push_back(this->addExtraZone(3));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AIM120));
                }
            break;
            case DURANDAL:
                wlabel = string_format("DURANDAL(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(14));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, DURANDAL));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(15));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, DURANDAL));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(16));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, DURANDAL));
                } else if (wpl.second == 8) {
                    this->extra_zones.push_back(this->addExtraZone(16));
                    this->extra_zones.push_back(this->addExtraZone(17));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, DURANDAL));
                } else if (wpl.second == 10) {
                    this->extra_zones.push_back(this->addExtraZone(16));
                    this->extra_zones.push_back(this->addExtraZone(18));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, DURANDAL));
                } else if (wpl.second == 12) {
                    this->extra_zones.push_back(this->addExtraZone(16));
                    this->extra_zones.push_back(this->addExtraZone(19));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, DURANDAL));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, DURANDAL));
                }
            break;
            case MK20:
                wlabel = string_format("MK20(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(20));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK20));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(21));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK20));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(22));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK20));
                } else if (wpl.second == 8) {
                    this->extra_zones.push_back(this->addExtraZone(22));
                    this->extra_zones.push_back(this->addExtraZone(23));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, MK20));
                } else if (wpl.second == 10) {
                    this->extra_zones.push_back(this->addExtraZone(22));
                    this->extra_zones.push_back(this->addExtraZone(24));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, MK20));
                } else if (wpl.second == 12) {
                    this->extra_zones.push_back(this->addExtraZone(22));
                    this->extra_zones.push_back(this->addExtraZone(25));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, MK20));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, MK20));
                }
            break;
            case AGM65D:
                wlabel = string_format("AGM65D(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(8));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AGM65D));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(9));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AGM65D));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(10));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AGM65D));
                } else if (wpl.second == 8) {
                    this->extra_zones.push_back(this->addExtraZone(10));
                    this->extra_zones.push_back(this->addExtraZone(11));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, AGM65D));
                } else if (wpl.second == 10) {
                    this->extra_zones.push_back(this->addExtraZone(10));
                    this->extra_zones.push_back(this->addExtraZone(12));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, AGM65D));
                } else if (wpl.second == 12) {
                    this->extra_zones.push_back(this->addExtraZone(10));
                    this->extra_zones.push_back(this->addExtraZone(13));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, AGM65D));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, AGM65D));
                }
            break;
            case MK82:
                wlabel = string_format("MK82(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(26));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK82));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(27));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK82));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(28));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK82));
                } else if (wpl.second == 8) {
                    this->extra_zones.push_back(this->addExtraZone(28));
                    this->extra_zones.push_back(this->addExtraZone(29));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, MK82));
                } else if (wpl.second == 10) {
                    this->extra_zones.push_back(this->addExtraZone(28));
                    this->extra_zones.push_back(this->addExtraZone(30));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, MK82));
                } else if (wpl.second >= 12 && wpl.second <=24) {
                    this->extra_zones.push_back(this->addExtraZone(28));
                    this->extra_zones.push_back(this->addExtraZone(31));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, MK82));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, MK82));
                }
            break;
            case GBU15:
                wlabel = string_format("GBU15(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(32));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, GBU15));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, GBU15));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(32));
                    this->extra_zones.push_back(this->addExtraZone(33));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, GBU15));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, GBU15));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, GBU15));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, GBU15));
                }
            break;
            case LAU3:
                wlabel = string_format("LAU3(%d)", wpl.second);
                if (wpl.second == 2) {
                    this->extra_zones.push_back(this->addExtraZone(34));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, LAU3));
                } else if (wpl.second == 4) {
                    this->extra_zones.push_back(this->addExtraZone(35));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, LAU3));
                } else if (wpl.second == 6) {
                    this->extra_zones.push_back(this->addExtraZone(36));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, LAU3));
                } else if (wpl.second == 8) {
                    this->extra_zones.push_back(this->addExtraZone(36));
                    this->extra_zones.push_back(this->addExtraZone(37));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, LAU3));
                } else if (wpl.second == 10) {
                    this->extra_zones.push_back(this->addExtraZone(36));
                    this->extra_zones.push_back(this->addExtraZone(38));
                    this->zones.push_back(this->addNoGfXZone(121, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(122, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(123, wlabel, LAU3));
                    this->zones.push_back(this->addNoGfXZone(124, wlabel, LAU3));
                }
            break;
        }
    }
}

std::vector<SCZone *> *WeaponLoadoutScene::Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts,
                                               std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick) {
    this->gameflow_scene = gf;
    this->sceneOpts = sc_opts;
    this->zones.clear();
    GameState.weapon_load_out.clear();
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
        background *tmpbg = new background();
        tmpbg->img =  this->getShape(this->sceneOpts->extr[0]->SHAPE_ID);
        this->layers.push_back(tmpbg);
    }
    
    this->onclick = onclick;
    this->updateWeaponDisplay();
    return (&this->zones);
}

void WeaponLoadoutScene::Render() {
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
    for (auto ezn : this->extra_zones) {
        ezn->Draw();
    }
    if (this->zones.size() > 0) {
        for (auto zn: this->zones) {
            if (zn->active) {
                zn->Draw();
            }
        }    
    }
}

std::vector<SCZone *> *LedgerScene::Init(
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
    return (&this->zones);
}

std::vector<SCZone *> *CatalogueScene::Init(
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
    return (&this->zones);
}
