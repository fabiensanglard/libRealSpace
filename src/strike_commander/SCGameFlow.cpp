//
//  SCGameFlow.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 16/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>
#include "SCGameFlow.h"

/**
 * SCGameFlow constructor.
 *
 * Initializes the SCGameFlow object with default values.
 *
 * @return None
 *
 * @throws None
 */
SCGameFlow::SCGameFlow() {
    this->current_miss = 0;
    this->current_scen = 0;
    this->efect = nullptr;
    this->currentOptCode = 0;
    this->fps = SDL_GetTicks() / 10;
    this->zones = new std::vector<SCZone *>();
    this->frequest = nullptr;
}

SCGameFlow::~SCGameFlow() {}

/**
 * Handles a click event on a sprite.
 *
 * @param id The ID of the sprite that was clicked.
 *
 * @return None
 */
void SCGameFlow::clicked(std::vector<EFCT *> *script, uint8_t id) {
    this->efect = script;
    this->currentSpriteId = id;
    this->currentOptCode = 0;
    if (this->scen->sceneOpts->foreground->sprites[id]->tune != nullptr) {
        Mixer.SwitchBank(1);
        Mixer.StopMusic();
        if (GameState.tune_modifier != 0 && id == 18) {
            Mixer.PlayMusic(this->scen->sceneOpts->foreground->sprites[id]->tune->ID + GameState.tune_modifier, 1);
        } else {
            Mixer.PlayMusic(this->scen->sceneOpts->foreground->sprites[id]->tune->ID, 1);
        }
    }
    this->runEffect();
}
/**
 * Return from a scene and go back to the scenario.
 *
 * @param script The script of the scene.
 * @param id The ID of the scene.
 *
 * @return None
 *
 * @throws None
 */

void SCGameFlow::loadGame(std::string filename) {
    GameState.Load(filename);
    this->frequest->requested_file = "";
    this->efect = nullptr;
    this->current_miss = GameState.current_mission;
    this->next_miss = GameState.next_mission;
    int i=0;
    for (auto sc: this->gameFlowParser.game.game[this->current_miss]->scen) {
        if (sc->info.ID == GameState.current_scene) {
            this->current_scen = i;
            break;
        }
        i++;
    }
    if (this->gameFlowParser.game.game[this->current_miss]->efct != nullptr) {
        this->efect = this->gameFlowParser.game.game[this->current_miss]->efct;
    }
    this->runEffectAfterLoad();
    this->efect = nullptr;
    this->loadMiss();
    
}

void SCGameFlow::returnFromScene(std::vector<EFCT *> *script, uint8_t id) { this->createScen(); }
void SCGameFlow::flyOrReturnFromScene(std::vector<EFCT *> *script, uint8_t id) {
    switch (id) {
    case 20:
        this->createScen();
        break;
    default:
        this->flyMission();
        break;
    }
}
void SCGameFlow::flyMission() {
    SCStrike *fly = new SCStrike();
    fly->init();
    fly->setMission(this->missionToFly);
    this->missionToFly = nullptr;
    fly_mission.push(fly);
    this->next_miss = GameState.mission_id;
}
SCZone *SCGameFlow::CheckZones(void) {
    static uint8_t color = 0;
    color = 0;
    RSFont *fnt = FontManager.GetFont("..\\..\\DATA\\FONTS\\OPTFONT.SHP");
    for (auto zone : *this->zones) {
        if (zone->active) {
            if (zone->quad != nullptr) {
                if (isPointInQuad(Mouse.GetPosition(), zone->quad)) {
                    Mouse.SetMode(SCMouse::VISOR);

                    // If the mouse button has just been released: trigger action.
                    if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
                        zone->OnAction();
                    Point2D p = {160 - static_cast<int32_t>(zone->label->length() / 2) * 8, 180};
                    VGA.GetFrameBuffer()->PrintText(fnt, &p, (char *)zone->label->c_str(), color, 0,
                                                    static_cast<uint32_t>(zone->label->length()), 3, 5);
                    return zone;
                }
            }
            if (Mouse.GetPosition().x > zone->position.x &&
                Mouse.GetPosition().x < zone->position.x + zone->dimension.x &&
                Mouse.GetPosition().y > zone->position.y &&
                Mouse.GetPosition().y < zone->position.y + zone->dimension.y) {
                // HIT !
                Mouse.SetMode(SCMouse::VISOR);

                // If the mouse button has just been released: trigger action.
                if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
                    zone->OnAction();
                Point2D p = {160 - ((int32_t)(zone->label->length() / 2) * 8), 180};
                VGA.GetFrameBuffer()->PrintText(fnt, &p, (char *)zone->label->c_str(), color, 0,
                                                static_cast<uint32_t>(zone->label->length()), 3, 5);

                return zone;
            }
        }
    }

    Mouse.SetMode(SCMouse::CURSOR);
    return nullptr;
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
    if (this->efect == nullptr) {
        return;
    }
    bool flymission = false;
    std::stack<uint8_t> ifStack;
    int i = 0;
    for (auto instruction : *this->efect) {
        if (i < this->currentOptCode) {
            i++;
            continue;
        }
        if (ifStack.size() > 0) {
            if (ifStack.top() == false) {
                switch (instruction->opcode) {
                case EFECT_OPT_MISS_ELSE:
                    if (ifStack.size() > 0) {
                        uint8_t ifval = 0;
                        ifval = ifStack.top();
                        ifStack.pop();
                        ifStack.push(!ifval);
                    }
                    break;
                case EFECT_OPT_MISS_ENDIF:
                    if (ifStack.size() > 0) {
                        ifStack.pop();
                    }
                    break;
                default:
                    printf("Opcode %d ignored cause flag is false\n", instruction->opcode);
                    break;
                }
                continue;
            }
        }
        switch (instruction->opcode) {
        case EFFCT_OPT_SHOW_MAP: {
            printf("PLAYING MAP %d\n", instruction->value);
            MapShot *map = new MapShot();
            map->init();
            map->SetPoints(&this->gameFlowParser.game.wrld[instruction->value]->data->points);
            this->cutsenes.push(map);
        } break;
        case EFECT_OPT_CONV: {
            printf("PLAYING CONV %d\n", instruction->value);
            this->playConv(instruction->value);
        } break;
        case EFECT_OPT_SCEN:
            for (int j = 0; j < this->gameFlowParser.game.game[this->current_miss]->scen.size(); j++) {
                if (this->gameFlowParser.game.game[this->current_miss]->scen.at(j)->info.ID == instruction->value) {
                    this->current_scen = j;
                    this->currentSpriteId = 0;
                    printf("PLAYING SCEN %d\n", this->current_scen);

                    this->createScen();
                }
            }
            break;
        case EFECT_OPT_END_MISS: {
            // @TODO Show weapons selection screen.
            bool direct = false;
            for (auto z : *this->zones) {
                if (z->id == this->currentSpriteId) {
                    direct = z->sprite->requ == nullptr;
                    if (!direct) {
                        for (auto r : *z->sprite->requ) {
                            if (r->op == 1) {
                                direct = true;
                            }
                        }
                    }
                    break;
                }
            }
            if (direct) {
                this->next_miss = GameState.mission_id;
            }
        } break;
        case EFECT_OPT_MIS2:
            // if (instruction->value != this->current_miss) {
            GameState.mission_id = instruction->value;
            printf("PLAYING MIS2 %d\n", instruction->value);
            //}
            break;
        case EFECT_OPT_SHOT: {
            printf("PLAYING SHOT %d\n", instruction->value);
            this->playShot(instruction->value);
        } break;
        case EFECT_OPT_FLYM: {
            uint8_t flymID = instruction->value;
            GameState.mission_flyed = flymID;
            printf("PLAYING FLYM %d\n", flymID);
            printf("Mission Name %s\n", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            this->missionToFly = (char *)malloc(13);
            sprintf(this->missionToFly, "%s.IFF", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            strtoupper(this->missionToFly, this->missionToFly);
            flymission = true;
        } break;
        case EFECT_OPT_FLYM2: {
            uint8_t flymID = instruction->value;
            GameState.mission_flyed = flymID;
            printf("PLAYING FLYM %d\n", flymID);
            printf("Mission Name %s\n", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            this->missionToFly = (char *)malloc(13);
            sprintf(this->missionToFly, "%s.IFF", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            strtoupper(this->missionToFly, this->missionToFly);
            flymission = true;
        } break;
        case EFECT_OPT_SETFLAG_TRUE:
            GameState.requierd_flags[instruction->value] = true;
            break;
        case EFECT_OPT_SETFLAG_FALSE:
            GameState.requierd_flags[instruction->value] = false;
            break;
        case EFECT_OPT_MISS_ACCEPTED:
            if (this->convs.size() > 0) {
                SCConvPlayer *conv = this->convs.front();
                this->convs.pop();
                Game->AddActivity(conv);
                this->currentOptCode = i;
                return;
            }
            if (GameState.mission_accepted == true) {
                ifStack.push(true);
            } else {
                ifStack.push(false);
            }
            break;
        case EFECT_OPT_MISS_REJECTED:
            if (this->convs.size() > 0) {
                SCConvPlayer *conv = this->convs.front();
                this->convs.pop();
                Game->AddActivity(conv);
                this->currentOptCode = i;
                return;
            }
            if (GameState.mission_accepted == false) {
                ifStack.push(true);
            } else {
                ifStack.push(false);
            }
            break;
        case EFECT_OPT_IF_FLAG:
            if (GameState.requierd_flags[instruction->value] == true) {
                ifStack.push(true);
            } else {
                ifStack.push(false);
            }
            break;
        case EFECT_OPT_MISS_ELSE:
            if (ifStack.size() > 0) {
                uint8_t ifval = 0;
                ifval = ifStack.top();
                ifStack.pop();
                ifStack.push(!ifval);
            }
            break;
        case EFECT_OPT_IF_NOT_FLAG:
            if (GameState.requierd_flags[instruction->value] == false) {
                ifStack.push(true);
            } else {
                ifStack.push(false);
            }
            break;
        case EFFCT_OPT_IF_MISS_SUCCESS:
            if (GameState.mission_flyed_success[instruction->value]) {
                ifStack.push(true);
            } else {
                ifStack.push(false);
            }
            break;
        case EFECT_OPT_LOAD_GAME:
            if (this->frequest == nullptr) {
                this->frequest = new SCFileRequester(std::bind(&SCGameFlow::loadGame, this, std::placeholders::_1),0);
            }    
            this->frequest->opened = true;
            this->frequest->shape_id_offset = 0;
            this->frequest->loadFiles();
            
        break;
        case EFECT_OPT_LOOK_AT_LEDGER:
            this->zones->clear();
            if (this->scen != nullptr) {
                delete this->scen;
            }
            this->scen = new LedgerScene(&this->optShps, &this->optPals);

            this->zones = this->scen->init(
                this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen],
                this->optionParser.opts[128],
                std::bind(&SCGameFlow::returnFromScene, this, std::placeholders::_1, std::placeholders::_2));
            break;
        case EFECT_OPT_LOOK_AT_KILLBOARD:
            this->zones->clear();
            if (this->scen != nullptr) {
                delete this->scen;
            }
            this->scen = new KillBoardScene(&this->optShps, &this->optPals);

            this->zones = this->scen->init(
                this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen],
                this->optionParser.opts[30],
                std::bind(&SCGameFlow::returnFromScene, this, std::placeholders::_1, std::placeholders::_2));
            break;
        case EFECT_OPT_VIEW_CATALOG:
            this->zones->clear();
            if (this->scen != nullptr) {
                delete this->scen;
            }
            this->scen = new CatalogueScene(&this->optShps, &this->optPals);

            this->zones = this->scen->init(
                this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen],
                this->optionParser.opts[30],
                std::bind(&SCGameFlow::returnFromScene, this, std::placeholders::_1, std::placeholders::_2));
            break;
        case EFECT_OPT_GO:
            this->next_miss = GameState.mission_id;
            break;
        case EFECT_OPT_MISS_ENDIF:
            if (ifStack.size() > 0) {
                ifStack.pop();
            }
            break;
        case EFECT_OPT_TUNE_MODIFIER:
            GameState.tune_modifier = instruction->value;
            break;
        case EFECT_OPT_APPLY_CHANGE:
            if (this->gameFlowParser.game.stat[instruction->value] != nullptr) {
                CHNG *chng = this->gameFlowParser.game.stat[instruction->value];
                if (chng->cash != nullptr) {
                    switch (chng->cash->op) {
                    case 2:
                        GameState.proj_cash = chng->cash->value*1000;
                        break;
                    case 1:
                        GameState.proj_cash += chng->cash->value*1000;
                        break;
                    case 0:
                        GameState.proj_cash -= chng->cash->value*1000;
                        break;
                    }
                }
                if (chng->over != nullptr) {
                    switch (chng->over->op) {
                    case 2:
                        GameState.over_head = chng->over->value*1000;
                        break;
                    case 1:
                        GameState.over_head += chng->over->value*1000;
                        break;
                    case 0:
                        GameState.over_head -= chng->over->value*1000;
                        break;
                    }
                }
                if (chng->weap != nullptr) {
                    for (auto weapon : *chng->weap) {
                        switch (weapon->op) {
                        case 2:
                            GameState.weapon_inventory[weapon->weap_id+1] = weapon->value;
                            break;
                        case 1:
                            GameState.weapon_inventory[weapon->weap_id+1] += weapon->value;
                            break;
                        case 0:
                            GameState.weapon_inventory[weapon->weap_id+1] -= weapon->value;
                            break;
                        }
                    }
                }
                if (chng->pilt != nullptr) {
                    for (auto pil : *chng->pilt) {
                        switch (pil->op) {
                        case 2:
                            GameState.kill_board[pil->pilot_id][pil->air] = pil->value;
                            break;
                        case 1:
                            GameState.kill_board[pil->pilot_id][pil->air] += pil->value;
                            break;
                        case 0:
                            GameState.kill_board[pil->pilot_id][pil->air] -= pil->value;
                            break;
                        }
                    }
                }
            }
            break;
        default:
            printf("Unkown opcode :%d, %d\n", instruction->opcode, instruction->value);
            break;
        };
        i++;
    }
    if (flymission) {
        if (this->scen != nullptr) {
            delete this->scen;
        }
        this->next_miss = 0;
        this->scen = new WeaponLoadoutScene(&this->optShps, &this->optPals);
        this->zones = this->scen->init(
            this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen], this->optionParser.opts[15],
            std::bind(&SCGameFlow::flyOrReturnFromScene, this, std::placeholders::_1, std::placeholders::_2));
    }
    this->efect = nullptr;
    this->currentOptCode = 0;
}

/**
 * Runs the current effect based on the efect vector and currentOptCode.
 *
 * Iterates through the efect vector, executing different actions based on the opcode.
 * Actions include playing conversations, scenes, and missions, as well as handling unknown opcodes.
 *
 * @return None
 */
void SCGameFlow::runEffectAfterLoad() {
    for (auto instruction : *this->efect) {
        switch (instruction->opcode) {
        case EFECT_OPT_TUNE_MODIFIER:
            GameState.tune_modifier = instruction->value;
            break;
        default:
            break;
        }
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
void SCGameFlow::checkKeyboard(void) {
    // Keyboard
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game->StopTopActivity();
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
        case SDLK_a:
            this->extcpt++;
            if (this->extcpt > this->sceneOpts->extr.size() - 1) {
                this->extcpt--;
            }
            break;
        case SDLK_z:
            this->extcpt--;
            if (this->extcpt <= 0) {
                this->extcpt = 0;
            }
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
 * Initializes the SCGameFlow object by loading the necessary IFF files and initializing the optionParser and
 * gameFlowParser. It also initializes the optShps and optPals objects by loading the OPTSHPS.PAK and OPTPALS.PAK files
 * respectively. Finally, it sets the efect pointer to nullptr and calls the createMiss() function.
 *
 * @throws None
 */
void SCGameFlow::init() {

    TreEntry *gameflowIFF = Assets.GetEntryByName(Assets.gameflow_filename);
    TreEntry *optionIFF = Assets.GetEntryByName(Assets.option_filename);
    this->optionParser.InitFromRam(optionIFF->data, optionIFF->size);
    this->gameFlowParser.InitFromRam(gameflowIFF->data, gameflowIFF->size);

    TreEntry *optShapEntry = Assets.GetEntryByName(Assets.optshps_filename);
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry *optPalettesEntry = Assets.GetEntryByName(Assets.optpals_filename);
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    this->efect = nullptr;
    this->createMiss();
}

void SCGameFlow::InitFromGameState() {

    TreEntry *gameflowIFF = Assets.GetEntryByName(Assets.gameflow_filename);
    TreEntry *optionIFF = Assets.GetEntryByName(Assets.option_filename);
    this->optionParser.InitFromRam(optionIFF->data, optionIFF->size);
    this->gameFlowParser.InitFromRam(gameflowIFF->data, gameflowIFF->size);

    TreEntry *optShapEntry = Assets.GetEntryByName(Assets.optshps_filename);
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry *optPalettesEntry = Assets.GetEntryByName(Assets.optpals_filename);
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    this->efect = nullptr;
    this->current_miss = GameState.current_mission;
    this->next_miss = GameState.next_mission;
    int i=0;
    for (auto sc: this->gameFlowParser.game.game[this->current_miss]->scen) {
        if (sc->info.ID == GameState.current_scene) {
            this->current_scen = i;
            break;
        }
        i++;
    }
    if (this->gameFlowParser.game.game[this->current_miss]->efct != nullptr) {
        this->efect = this->gameFlowParser.game.game[this->current_miss]->efct;
    }
    this->runEffectAfterLoad();
    this->efect = nullptr;
    this->loadMiss();
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
    this->missionToFly = nullptr;
    this->next_miss = 0;
    if (this->zones != nullptr) {
        this->zones->clear();
    }
    this->layers.clear();
    this->convs = std::queue<SCConvPlayer *>();
    this->cutsenes = std::queue<SCShot *>();
    this->fly_mission = std::queue<SCStrike *>();
    printf("current miss : %d, current_scen %d\n", this->current_miss, this->current_scen);
    if (this->gameFlowParser.game.game[this->current_miss]->efct != nullptr) {
        this->efect = this->gameFlowParser.game.game[this->current_miss]->efct;
    }

    printf("efect size %zd\n", this->efect->size());
    this->runEffect();
    this->createScen();
}

void SCGameFlow::loadMiss() {
    this->missionToFly = nullptr;
    this->next_miss = 0;
    if (this->zones != nullptr) {
        this->zones->clear();
    }
    this->layers.clear();
    this->convs = std::queue<SCConvPlayer *>();
    this->cutsenes = std::queue<SCShot *>();
    this->fly_mission = std::queue<SCStrike *>();
    printf("current miss : %d, current_scen %d\n", this->current_miss, this->current_scen);
    this->createScen();
}

/**
 * Creates a new scene in the current miss.
 *
 * This function initializes the layers, sprites, and zones for the current scene.
 * It also sets up the raw palette and foreground palette.
 *
 * @return None
 *
 * @throws None
 */
void SCGameFlow::createScen() {
    this->zones->clear();
    if (this->gameFlowParser.game.game[this->current_miss]->scen.size() > 0) {
        uint8_t optionScenID = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->info.ID;
        if (this->scen != nullptr) {
            // delete this->scen;
        }
        this->scen = new SCScene(&this->optShps, &this->optPals);
        this->zones =
            this->scen->init(this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen],
                             this->optionParser.opts[optionScenID],
                             std::bind(&SCGameFlow::clicked, this, std::placeholders::_1, std::placeholders::_2));
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
RSImageSet *SCGameFlow::getShape(uint8_t shpid) {
    PakEntry *shapeEntry = this->optShps.GetEntry(shpid);
    PakArchive subPAK;

    subPAK.InitFromRAM("", shapeEntry->data, shapeEntry->size);
    RSImageSet *img = new RSImageSet();
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
void SCGameFlow::runFrame(void) {
    if (this->frequest != nullptr && !this->frequest->opened) {
        delete this->frequest;
        this->frequest = nullptr;
    }
    if (this->frequest != nullptr && this->frequest->opened) {
        this->frequest->checkevents();
    }
    if (this->cutsenes.size() > 0) {
        SCShot *c = this->cutsenes.front();
        this->cutsenes.pop();
        Game->AddActivity(c);
        return;
    }
    if (this->convs.size() > 0) {
        SCConvPlayer *c = this->convs.front();
        this->convs.pop();
        Game->AddActivity(c);
        return;
    }
    if (this->fly_mission.size() > 0) {
        SCStrike *c = this->fly_mission.front();
        this->fly_mission.pop();
        Game->AddActivity(c);
        EndMissionScene *end = new EndMissionScene(&this->optShps, &this->optPals);
        end->init();
        this->cutsenes.push(end);
        return;
    }
    if (this->next_miss > 0) {
        this->current_miss = this->next_miss;
        this->next_miss = 0;
        this->current_scen = 0;
        this->createMiss();
        return;
    }
    if (this->currentOptCode > 0 && this->efect != nullptr) {
        this->runEffect();
        return;
    }

    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks() / 10);
    }
    checkKeyboard();
    VGA.Activate();
    VGA.GetFrameBuffer()->FillWithColor(0);
    if (this->scen != nullptr) {
        this->scen->Render();
        if (this->test_shape != nullptr) {
            VGA.GetFrameBuffer()->DrawShape(this->test_shape);
        }
        this->CheckZones();
    }
    VGA.VSync();
    VGA.SwithBuffers();
    VGA.Activate();
    VGA.GetFrameBuffer()->Clear();
    if (this->frequest != nullptr && this->frequest->opened) {
        this->frequest->draw(VGA.GetFrameBuffer());
    }
    Mouse.Draw();
    VGA.VSync();
    VGA.SwithBuffers();
}
void SCGameFlow::playShot(uint8_t shotId) {
    SCShot *sht = new SCShot();
    sht->init();
    sht->SetShotId(shotId);
    this->cutsenes.push(sht);
}
void SCGameFlow::playConv(uint8_t convId) {
    SCConvPlayer *conv = new SCConvPlayer();
    conv->init();
    conv->SetID(convId);
    this->convs.push(conv);
}