#include "precomp.h"
#include "SCScene.h"
#include <memory>
#include <string>
#include <stdexcept>

#pragma warning(disable : 4996)
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args ) {
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
    if (this->sceneOpts->tune != nullptr && this->music) {
        if (GameState.tune_modifier != 0 && (this->sceneOpts->infos.ID == 20 || this->sceneOpts->infos.ID == 29)) {
            if (Mixer.GetMusicID() != this->sceneOpts->tune->ID+GameState.tune_modifier) {
                Mixer.SwitchBank(1);
                Mixer.StopMusic();
                Mixer.PlayMusic(this->sceneOpts->tune->ID+GameState.tune_modifier);
            }
        } else {
            if (Mixer.GetMusicID() != this->sceneOpts->tune->ID) {
                Mixer.SwitchBank(1);
                Mixer.StopMusic();
                Mixer.PlayMusic(this->sceneOpts->tune->ID);
            }
        }
    }
    ByteStream paletteReader;
    paletteReader.Set((this->rawPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.SetPalette(&this->palette);
    for (auto layer : this->layers) {
        VGA.GetFrameBuffer()->DrawShape(layer->img->GetShape(layer->img->sequence[layer->frameCounter]));
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

 SCZone *WeaponLoadoutScene::addNoGfXZone(uint8_t id, std::string label, uint8_t weapon_type_shp_id) { 
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
    z->id = weapon_type_shp_id;
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
            if (GameState.weapon_load_out[1] == 0) {
                GameState.weapon_load_out[1] = AIM9J;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[2] == 0) {
                GameState.weapon_load_out[2] = AIM9J;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] == 0){
                GameState.weapon_load_out[3] = AIM9M;
                GameState.weapon_load_out[id] += 2;
            }
            break;
        case AIM9M:
            if (GameState.weapon_load_out[1] == 0) {
                GameState.weapon_load_out[1] = AIM9M;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[2] == 0) {
                GameState.weapon_load_out[2] = AIM9M;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] == 0){
                GameState.weapon_load_out[3] = AIM9M;
                GameState.weapon_load_out[id] += 2;
            }
            break;
        case AIM120:
            if (GameState.weapon_load_out[2] == 0) {
                GameState.weapon_load_out[2] = AIM120;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] == 0) {
                GameState.weapon_load_out[3] = AIM120;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] == AIM120) {
                if (GameState.weapon_load_out[2] == AIM120 && GameState.weapon_load_out[id] < 6) {
                    GameState.weapon_load_out[id] += 2;
                } else if (GameState.weapon_load_out[id] < 4) {
                    GameState.weapon_load_out[id] += 2;
                }
            } 
            break;
        case AGM65D:
        case MK20:
        case DURANDAL:
            if (GameState.weapon_load_out[3] == 0) {
                GameState.weapon_load_out[3] = id;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] == id && GameState.weapon_load_out[id] < 6) {
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] !=  id && GameState.weapon_load_out[4] == 0 && GameState.weapon_load_out[id] < 6) {
                GameState.weapon_load_out[4] = id;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] !=  id && GameState.weapon_load_out[4] == id && GameState.weapon_load_out[id] < 6) {
                GameState.weapon_load_out[4] = id;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] == id && GameState.weapon_load_out[4] == 0) {
                GameState.weapon_load_out[4] = id;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[3] == id && GameState.weapon_load_out[4] == id && GameState.weapon_load_out[id] < 12) {
                GameState.weapon_load_out[id] += 2;
            } 
            break;
        case MK82:
            if (GameState.weapon_load_out[3] == 0) {
                GameState.weapon_load_out[3] = MK82;
                if (GameState.weapon_load_out[id] < 12) {
                    GameState.weapon_load_out[id] += 2;
                }
            } else if (GameState.weapon_load_out[4] == 0) {
                GameState.weapon_load_out[4] = MK82;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[4] == MK82) {
                if (GameState.weapon_load_out[4] == MK82 && GameState.weapon_load_out[id] < 24) {
                    GameState.weapon_load_out[id] += 2;
                } else if (GameState.weapon_load_out[id] < 12) {
                    GameState.weapon_load_out[id] += 2;
                }
            } 
            break;
        case GBU15:
            if (GameState.weapon_load_out[3] == 0) {
                GameState.weapon_load_out[3] = GBU15;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[4] == 0) {
                GameState.weapon_load_out[4] = GBU15;
                GameState.weapon_load_out[id] += 2;
            break;
        case LAU3:
            if (GameState.weapon_load_out[3] == 0) {
                GameState.weapon_load_out[3] = LAU3;
                if (GameState.weapon_load_out[id] < 3) {
                    GameState.weapon_load_out[id] += 2;
                }
            } else if (GameState.weapon_load_out[4] == 0) {
                GameState.weapon_load_out[4] = LAU3;
                GameState.weapon_load_out[id] += 2;
            } else if (GameState.weapon_load_out[4] == LAU3) {
                if (GameState.weapon_load_out[4] == LAU3 && GameState.weapon_load_out[id] < 10) {
                    GameState.weapon_load_out[id] += 2;
                } else if (GameState.weapon_load_out[id] < 4) {
                    GameState.weapon_load_out[id] += 2;
                }
            } 
            break;
        default:
            break;
        }
    }

    this->updateWeaponDisplay();
}

void WeaponLoadoutScene::removeWeapon(std::vector<EFCT *> *script, uint8_t id) {
    this->extra_zones.clear();
    this->extra_zones.shrink_to_fit();
    // L'opérateur [] crée l'entrée si elle n'existe pas et retourne 0 par défaut.
    int nbweap = GameState.weapon_load_out[id];
    std::map<weapon_type_shp_id, uint8_t> max_weapons = {
        {AIM9J,6},
        {AIM9M,6},
        {AIM120,6},
        {AGM65D,12},
        {DURANDAL,12},
        {MK20,12},
        {MK82,24},
        {GBU15,2},
        {LAU3,10}
    };
    GameState.weapon_load_out[id] = (std::max)(0, GameState.weapon_load_out[id] - 2);
    int last_id = 0;
    int nb_pylone = 0;
    for (int i=1; i<5; i++) {
        if (GameState.weapon_load_out[i] == id) {
            last_id = i;
            nb_pylone++;
        }
    }
    if ((GameState.weapon_load_out[id]<= max_weapons[weapon_type_shp_id(id)]/nb_pylone && nb_pylone>1) || GameState.weapon_load_out[id] == 0) {
        GameState.weapon_load_out[last_id] = 0;
    }
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
                                 uint8_t &extraIds,
                                 const std::vector<uint8_t> &noGfxIds) {
        this->extra_zones.push_back(this->addExtraZone(extraIds));
        for (auto nid : noGfxIds)
            this->zones.push_back(this->addNoGfXZone(nid, wlabel, weaponType));
    };

    std::string wlabel;
    // Pour chaque arme présente dans le load-out, ajouter les extra zones correspondantes
    
    for (int i=1; i<5; i++) {
        switch (GameState.weapon_load_out[i]) {
            case AIM9J:
                wlabel = string_format("AIM9J(%d)", GameState.weapon_load_out[AIM9J]);
                addWeaponZones(AIM9J, wlabel, weapon_pylones[i-1][AIM9J][0][0], weapon_pylones[i-1][AIM9J][1]);
            break;
            case AIM9M:
                wlabel = string_format("AIM9M(%d)", GameState.weapon_load_out[AIM9M]);
                addWeaponZones(AIM9M, wlabel, weapon_pylones[i-1][AIM9M][0][0], weapon_pylones[i-1][AIM9M][1]);
            break;
            case AIM120:
                wlabel = string_format("AIM120(%d)", GameState.weapon_load_out[AIM120]);
                if (i==3) {
                    if (GameState.weapon_load_out[AIM120] > 4 && GameState.weapon_load_out[i-1] == AIM120) {
                        addWeaponZones(AIM120, wlabel, weapon_pylones[i-1][AIM120][0][1], weapon_pylones[i-1][AIM120][1]);
                    } else {
                        addWeaponZones(AIM120, wlabel, weapon_pylones[i-1][AIM120][0][0], weapon_pylones[i-1][AIM120][1]);
                    }
                } else {
                    addWeaponZones(AIM120, wlabel, weapon_pylones[i-1][AIM120][0][0], weapon_pylones[i-1][AIM120][1]);
                }
            break;
            case DURANDAL:
                {
                    wlabel = string_format("DURANDAL(%d)", GameState.weapon_load_out[DURANDAL]);
                    int nbweap=0;
                    weapon_type_shp_id pylid = weapon_type_shp_id(GameState.weapon_load_out[i]);
                    if (i == 3) {
                        if (GameState.weapon_load_out[pylid] >= 6) {
                            nbweap = 2;
                        } else {
                            nbweap = GameState.weapon_load_out[pylid] / 2;
                            nbweap = nbweap -1;
                        }
                    } else if (i == 4) {
                        nbweap = GameState.weapon_load_out[pylid];
                        if (GameState.weapon_load_out[pylid] > 6) {
                            nbweap = GameState.weapon_load_out[pylid] - 6;
                        }
                        nbweap = nbweap / 2;                   
                        nbweap = nbweap -1;

                    }
                    addWeaponZones(pylid, wlabel, weapon_pylones[i-1][pylid][0][nbweap], weapon_pylones[i-1][pylid][1]);
                }
            break;
            case MK20:
                {
                    wlabel = string_format("MK20(%d)", GameState.weapon_load_out[MK20]);
                    int nbweap=0;
                    weapon_type_shp_id pylid = weapon_type_shp_id(GameState.weapon_load_out[i]);
                    if (i == 3) {
                        if (GameState.weapon_load_out[pylid] >= 6) {
                            nbweap = 2;
                        } else {
                            nbweap = GameState.weapon_load_out[pylid] / 2;
                            nbweap = nbweap -1;
                        }
                    } else if (i == 4) {
                        nbweap = GameState.weapon_load_out[pylid];
                        if (GameState.weapon_load_out[pylid] > 6) {
                            nbweap = GameState.weapon_load_out[pylid] - 6;
                        }
                        nbweap = nbweap / 2;                   
                        nbweap = nbweap -1;

                    }
                    addWeaponZones(pylid, wlabel, weapon_pylones[i-1][pylid][0][nbweap], weapon_pylones[i-1][pylid][1]);
                }
            break;
            case AGM65D:
                {
                    wlabel = string_format("AGM65D(%d)", GameState.weapon_load_out[AGM65D]);
                    int nbweap=0;
                    weapon_type_shp_id pylid = weapon_type_shp_id(GameState.weapon_load_out[i]);
                    if (i == 3) {
                        if (GameState.weapon_load_out[pylid] >= 6) {
                            nbweap = 2;
                        } else {
                            nbweap = GameState.weapon_load_out[pylid] / 2;
                            nbweap = nbweap -1;
                        }
                    } else if (i == 4) {
                        nbweap = GameState.weapon_load_out[pylid];
                        if (GameState.weapon_load_out[pylid] > 6) {
                            nbweap = GameState.weapon_load_out[pylid] - 6;
                        }
                        nbweap = nbweap / 2;                   
                        nbweap = nbweap -1;

                    }
                    addWeaponZones(pylid, wlabel, weapon_pylones[i-1][pylid][0][nbweap], weapon_pylones[i-1][pylid][1]);
                }
            break;
            case MK82:
                wlabel = string_format("MK82(%d)", GameState.weapon_load_out[MK82]);
                {
                    int nbweap=0;
                    weapon_type_shp_id pylid = weapon_type_shp_id(GameState.weapon_load_out[i]);
                    if (GameState.weapon_load_out[pylid]-6 > 0) {
                        nbweap = GameState.weapon_load_out[pylid]-6;
                    } else {
                        nbweap = GameState.weapon_load_out[pylid];
                    }
                    nbweap = nbweap / 2;
                    nbweap = nbweap -1;
                    addWeaponZones(pylid, wlabel, weapon_pylones[i-1][pylid][0][nbweap], weapon_pylones[i-1][pylid][1]);
                }
            break;
            case GBU15:
                wlabel = string_format("GBU15(%d)", GameState.weapon_load_out[GBU15]);
                addWeaponZones(GBU15, wlabel, weapon_pylones[i-1][GBU15][0][0], weapon_pylones[i-1][GBU15][1]);
            break;
            case LAU3:
                wlabel = string_format("LAU3(%d)", GameState.weapon_load_out[LAU3]);
                if (i==4) {
                    if (GameState.weapon_load_out[LAU3] > 4 && GameState.weapon_load_out[i-1] == LAU3) {
                        addWeaponZones(LAU3, wlabel, weapon_pylones[i-1][LAU3][0][1], weapon_pylones[i-1][LAU3][1]);
                    } else {
                        addWeaponZones(LAU3, wlabel, weapon_pylones[i-1][LAU3][0][0], weapon_pylones[i-1][LAU3][1]);
                    }
                } else {
                    if (GameState.weapon_load_out[LAU3]-4 > 0) {
                        int nbweap = GameState.weapon_load_out[LAU3]-4 / 3;
                        addWeaponZones(LAU3, wlabel, weapon_pylones[i-1][LAU3][0][0], weapon_pylones[i-1][LAU3][1]);
                    } else {
                        int nbweap = GameState.weapon_load_out[LAU3] / 3;
                        addWeaponZones(LAU3, wlabel, weapon_pylones[i-1][LAU3][0][0], weapon_pylones[i-1][LAU3][1]);
                    }
                    
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
    GameState.weapon_load_out[1] = 0;
    GameState.weapon_load_out[2] = 0;
    GameState.weapon_load_out[3] = 0;
    GameState.weapon_load_out[4] = 0;
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
        VGA.GetFrameBuffer()->DrawShape(layer->img->GetShape(layer->img->sequence[layer->frameCounter]));
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
    this->font = FontManager.GetFont("..\\..\\DATA\\FONTS\\LEDGFONT.SHP");
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
    this->onclick = onclick;
    this->CreateZones();
    return (&this->zones);
}
void LedgerScene::CreateZones() {
    this->zones.clear();
    this->zones.shrink_to_fit();
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
                z->onclick = this->onclick;
                z->active = true;
                this->zones.push_back(z);
            } else {
                if (page == 0 && sprtId == 126) {
                    z->active = true;
                    z->onclick = std::bind(&LedgerScene::TurnPage, this, std::placeholders::_1, std::placeholders::_2);
                    this->zones.push_back(z);
                } else if (page == 1 && sprtId == 127) {
                    z->active = true;
                    z->onclick = std::bind(&LedgerScene::TurnPage, this, std::placeholders::_1, std::placeholders::_2);
                    this->zones.push_back(z);
                }
            }
            
        } else {
            printf("%d, ID Sprite not found !!\n", sprtId);
        }
    }
}
void LedgerScene::Render() {
    SCScene::Render();
    FrameBuffer *fb = VGA.GetFrameBuffer();
    std::map<weapon_ids, std::string> weapon_label = {
        {ID_AIM9J, "AIM-9J"},
        {ID_AIM9M, "AIM-9M"},
        {ID_AGM65D, "AGM-65D"},
        {ID_LAU3, "LAU-3"},
        {ID_MK20, "MK-20"},
        {ID_MK82, "MK-82"},
        {ID_DURANDAL, "DURANDAL"},
        {ID_GBU15, "GBU-15"},
        {ID_AIM120, "AIM-120"},
        {ID_20MM, "20MM"},
    };
    int color = 0;
    if (page == 0) {
        fb->PrintText(this->font, {67,64}, std::string("PREVIOUS CASH"), color);
        fb->PrintText(this->font, {177,64}, std::to_string(GameState.proj_cash), color);   
        fb->PrintText(this->font, {67,70}, std::string("PROJECT OVERHEAD"), color);
        fb->PrintText(this->font, {177,70}, std::to_string(GameState.over_head), color);    
    } else if (page == 1) {
        int y = 64;
        for (auto weap: GameState.weapon_inventory) {
            fb->PrintText(this->font, {67,y}, weapon_label[weapon_ids(weap.first+1)], color);
            fb->PrintText(this->font, {203,y}, std::to_string(weap.second), color);
            y += 5+(y%2);
        }
    }
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

