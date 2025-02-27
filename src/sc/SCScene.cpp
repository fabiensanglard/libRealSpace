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

    GameState.aircraftHooks.clear();
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
    if (this->sceneOpts->tune != nullptr) {
        if (Mixer.GetMusicID() != this->sceneOpts->tune->ID) {
            Mixer.SwitchBank(1);
            Mixer.PlayMusic(this->sceneOpts->tune->ID);
        }
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
    // Réinitialiser les zones supplémentaires
    this->extra_zones.clear();
    this->extra_zones.shrink_to_fit();

    // Déterminer la quantité maximale en fonction du type d'arme
    int max = 0;
    switch (id) {
        case AIM9J:
        case AIM9M:
            max = 6;
            break;
        case AIM120:
            max = 6;
            break;
        case DURANDAL:
            max = 12;
            break;
        case MK82:
            max = 24;
            break;
        case AGM65D:
        case MK20:
            max = 12;
            break;
        case GBU15:
            max = 4;
            break;
        case LAU3:
            max = 10;
            break;
        default:
            break;
    }
    
    if (GameState.weapon_load_out[id] < max) {
        GameState.weapon_load_out[id] += 2;
    }

    this->updateWeaponDisplay();
}

void WeaponLoadoutScene::removeWeapon(std::vector<EFCT *> *script, uint8_t id) {
    this->extra_zones.clear();
    this->extra_zones.shrink_to_fit();
    // L'opérateur [] crée l'entrée si elle n'existe pas et retourne 0 par défaut.
    GameState.weapon_load_out[id] = (std::max)(0, GameState.weapon_load_out[id] - 2);
    this->updateWeaponDisplay();
}

void WeaponLoadoutScene::updateWeaponDisplay() {

    // Réinitialiser la liste des zones
    this->zones.clear();
    this->zones.shrink_to_fit();

    // Lambda pour créer et ajouter une zone fixe
    auto createAndAddZone = [this](uint8_t spriteId, uint8_t zoneId, auto callback) {
        SCZone *z = createZone(this->sceneOpts->foreground->sprites[spriteId], zoneId);
        z->active = true;
        z->onclick = callback;
        this->zones.push_back(z);
    };

    // Ajout des zones fixes
    createAndAddZone(this->gameflow_scene->info.ID, this->gameflow_scene->info.ID, this->onclick);
    createAndAddZone(12, 12, this->onclick);
    createAndAddZone(AIM9J, AIM9J, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(AIM9M, AIM9M, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(AIM120, AIM120, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(DURANDAL, DURANDAL, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(MK82, MK82, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(AGM65D, AGM65D, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(MK20, MK20, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(GBU15, GBU15, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));
    createAndAddZone(LAU3, LAU3, std::bind(&WeaponLoadoutScene::addWeapon, this, std::placeholders::_1, std::placeholders::_2));

    // Lambda pour ajouter les zones de load-out (extra zones et zones "NoGfX")
    auto addWeaponZones = [this](uint8_t weaponType, const std::string &wlabel,
                                 const std::vector<uint8_t> &extraIds,
                                 const std::vector<uint8_t> &noGfxIds) {
        for (auto eid : extraIds)
            this->extra_zones.push_back(this->addExtraZone(eid));
        for (auto nid : noGfxIds)
            this->zones.push_back(this->addNoGfXZone(nid, wlabel, weaponType));
    };

    std::string wlabel;
    // Pour chaque arme présente dans le load-out, ajouter les extra zones correspondantes
    for (auto wpl : GameState.weapon_load_out) {
        switch (wpl.first) {
            case AIM9J:
                wlabel = string_format("AIM9J(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(AIM9J, wlabel, {4}, {117, 118});
                } else if (wpl.second == 4) {
                    addWeaponZones(AIM9J, wlabel, {4, 5}, {117, 118, 119, 120});
                } else if (wpl.second == 6) {
                    addWeaponZones(AIM9J, wlabel, {4, 5, 40}, {117, 118, 119, 120, 121, 122});
                }
            break;
            case AIM9M:
                wlabel = string_format("AIM9M(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(AIM9M, wlabel, {6}, {117, 118});
                } else if (wpl.second == 4) {
                    addWeaponZones(AIM9M, wlabel, {6, 7}, {117, 118, 119, 120});
                } else if (wpl.second == 6) {
                    addWeaponZones(AIM9M, wlabel, {6, 7, 41}, {117, 118, 119, 120, 121, 122});
                }
            break;
            case AIM120:
                wlabel = string_format("AIM120(%d)", wpl.second);
                if (wpl.second == 2) {
                    // Conserver les constantes : ici on ajoute directement les NoGfX zones
                    this->extra_zones.push_back(this->addExtraZone(1));
                    this->zones.push_back(this->addNoGfXZone(119, wlabel, AIM120));
                    this->zones.push_back(this->addNoGfXZone(120, wlabel, AIM120));
                } else if (wpl.second == 4) {
                    addWeaponZones(AIM120, wlabel, {1, 2}, {119, 120, 121, 122});
                } else if (wpl.second == 6) {
                    addWeaponZones(AIM120, wlabel, {1, 2, 3}, {119, 120, 121, 122});
                }
            break;
            case DURANDAL:
                wlabel = string_format("DURANDAL(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(DURANDAL, wlabel, {14}, {121, 122});
                } else if (wpl.second == 4) {
                    addWeaponZones(DURANDAL, wlabel, {15}, {121, 122});
                } else if (wpl.second == 6) {
                    addWeaponZones(DURANDAL, wlabel, {16}, {121, 122});
                } else if (wpl.second == 8) {
                    addWeaponZones(DURANDAL, wlabel, {16, 17}, {121, 122, 123, 124});
                } else if (wpl.second == 10) {
                    addWeaponZones(DURANDAL, wlabel, {16, 18}, {121, 122, 123, 124});
                } else if (wpl.second == 12) {
                    addWeaponZones(DURANDAL, wlabel, {16, 19}, {121, 122, 123, 124});
                }
            break;
            case MK20:
                wlabel = string_format("MK20(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(MK20, wlabel, {20}, {121, 122});
                } else if (wpl.second == 4) {
                    addWeaponZones(MK20, wlabel, {21}, {121, 122});
                } else if (wpl.second == 6) {
                    addWeaponZones(MK20, wlabel, {22}, {121, 122});
                } else if (wpl.second == 8) {
                    addWeaponZones(MK20, wlabel, {22, 23}, {121, 122, 123, 124});
                } else if (wpl.second == 10) {
                    addWeaponZones(MK20, wlabel, {22, 24}, {121, 122, 123, 124});
                } else if (wpl.second == 12) {
                    addWeaponZones(MK20, wlabel, {22, 25}, {121, 122, 123, 124});
                }
            break;
            case AGM65D:
                wlabel = string_format("AGM65D(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(AGM65D, wlabel, {8}, {121, 122});
                } else if (wpl.second == 4) {
                    addWeaponZones(AGM65D, wlabel, {9}, {121, 122});
                } else if (wpl.second == 6) {
                    addWeaponZones(AGM65D, wlabel, {10}, {121, 122});
                } else if (wpl.second == 8) {
                    addWeaponZones(AGM65D, wlabel, {10, 11}, {121, 122, 123, 124});
                } else if (wpl.second == 10) {
                    addWeaponZones(AGM65D, wlabel, {10, 12}, {121, 122, 123, 124});
                } else if (wpl.second == 12) {
                    addWeaponZones(AGM65D, wlabel, {10, 13}, {121, 122, 123, 124});
                }
            break;
            case MK82:
                wlabel = string_format("MK82(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(MK82, wlabel, {26}, {121, 122});
                } else if (wpl.second == 4) {
                    addWeaponZones(MK82, wlabel, {27}, {121, 122});
                } else if (wpl.second == 6) {
                    addWeaponZones(MK82, wlabel, {28}, {121, 122});
                } else if (wpl.second == 8) {
                    addWeaponZones(MK82, wlabel, {28, 29}, {121, 122, 123, 124});
                } else if (wpl.second == 10) {
                    addWeaponZones(MK82, wlabel, {28, 30}, {121, 122, 123, 124});
                } else if (wpl.second >= 12 && wpl.second <= 24) {
                    addWeaponZones(MK82, wlabel, {28, 31}, {121, 122, 123, 124});
                }
            break;
            case GBU15:
                wlabel = string_format("GBU15(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(GBU15, wlabel, {32}, {121, 122});
                } else if (wpl.second == 4) {
                    addWeaponZones(GBU15, wlabel, {32, 33}, {121, 122, 123, 124});
                }
            break;
            case LAU3:
                wlabel = string_format("LAU3(%d)", wpl.second);
                if (wpl.second == 2) {
                    addWeaponZones(LAU3, wlabel, {34}, {121, 122});
                } else if (wpl.second == 4) {
                    addWeaponZones(LAU3, wlabel, {35}, {121, 122});
                } else if (wpl.second == 6) {
                    addWeaponZones(LAU3, wlabel, {36}, {121, 122});
                } else if (wpl.second == 8) {
                    addWeaponZones(LAU3, wlabel, {36, 37}, {121, 122, 123, 124});
                } else if (wpl.second == 10) {
                    addWeaponZones(LAU3, wlabel, {36, 38}, {121, 122, 123, 124});
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
    if (this->sceneOpts->tune != nullptr) {
        if (Mixer.GetMusicID() != this->sceneOpts->tune->ID) {
            Mixer.SwitchBank(1);
            Mixer.PlayMusic(this->sceneOpts->tune->ID);
        }
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
