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

#define EFECT_OPT_CONV 0
#define EFECT_OPT_SCEN 1
#define EFECT_OPT_FLYM2 3
#define EFECT_OPT_SETFLAG_TRUE 6
#define EFECT_OPT_SETFLAG_FALSE 7
#define EFECT_OPT_SHOT 8
#define EFECT_OPT_IF_NOT_FLAG 9
#define EFECT_OPT_IF_FLAG 10
#define EFECT_OPT_FLYM 12
#define EFECT_OPT_MIS2 13
#define EFECT_OPT_GO 15
#define EFECT_OPT_MISS_ACCEPTED 20
#define EFECT_OPT_MISS_REJECTED 21
#define EFECT_OPT_END_MISS 22
#define EFECT_OPT_MISS_ELSE 30
#define EFECT_OPT_MISS_ENDIF 31
#define EFECT_OPT_TEST_CURRENT_MISS 32

// identified but unknown effect optcodes
#define EFECT_OPT_U1 2
#define EFECT_OPT_U2 4
#define EFECT_OPT_U3 16
#define EFECT_OPT_U4 17
#define EFECT_OPT_U5 18
#define EFECT_OPT_U6 19
#define EFECT_OPT_U7 23
#define EFECT_OPT_U8 24
#define EFECT_OPT_U9 25
#define EFECT_OPT_U10 26
#define EFECT_OPT_U11 27
#define EFECT_OPT_U12 29
#define EFECT_OPT_U13 34
#define EFECT_OPT_U14 36
#define EFECT_OPT_U15 37
#define EFECT_OPT_U16 38

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
    this->runEffect();
}
SCZone *SCGameFlow::CheckZones(void) {
    for (size_t i = 0; i < zones.size(); i++) {
        SCZone *zone = zones[i];
        if (zone->IsActive(&GameState.requierd_flags)) {
            if (zone->quad != nullptr) {
                if (isPointInQuad(Mouse.GetPosition(), zone->quad)) {
                    Mouse.SetMode(SCMouse::VISOR);

                    // If the mouse button has just been released: trigger action.
                    if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
                        zone->OnAction();
                    Point2D p = {160 - static_cast<int32_t>(zone->label->length() / 2) * 8, 180};
                    VGA.PrintText(FontManager.GetFont(""), &p, (char *)zone->label->c_str(), 64, 0,
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
                VGA.PrintText(FontManager.GetFont(""), &p, (char *)zone->label->c_str(), 64, 0,
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
    std::stack<uint8_t> ifStack;

    for (int i = this->currentOptCode; i < this->efect->size(); i++) {
        if (ifStack.size() > 0) {
            if (ifStack.top() == false) {
                switch (this->efect->at(i)->opcode) {
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
                        printf("Opcode %d ignored cause flag is false\n", this->efect->at(i)->opcode);
                        break;
                }
                continue;
            }
        }
        switch (this->efect->at(i)->opcode) {
        case EFECT_OPT_CONV: {
            printf("PLAYING CONV %d\n", this->efect->at(i)->value);
            SCConvPlayer *conv = new SCConvPlayer();
            conv->Init();
            conv->SetID(this->efect->at(i)->value);
            this->convs.push(conv);
        } break;
        case EFECT_OPT_SCEN:
            for (int j = 0; j < this->gameFlowParser.game.game[this->current_miss]->scen.size(); j++) {
                if (this->gameFlowParser.game.game[this->current_miss]->scen.at(j)->info.ID ==
                    this->efect->at(i)->value) {
                    this->current_scen = j;
                    this->currentSpriteId = 0;
                    printf("PLAYING SCEN %d\n", this->current_scen);
                    this->createScen();
                }
            }
            break;
        case EFECT_OPT_END_MISS:{
            // @TODO Show weapons selection screen.
            bool direct = false;
            for (auto z: this->zones) {
                if (z->id == this->currentSpriteId) {
                    direct = z->sprite->requ == nullptr;
                    if (!direct) {
                        for (auto r: *z->sprite->requ) {
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
        }            
        break;
        case EFECT_OPT_MIS2:
            if (this->efect->at(i)->value != this->current_miss) {
                GameState.mission_id = this->efect->at(i)->value;
                printf("PLAYING MIS2 %d\n", this->efect->at(i)->value);
            }
            break;
        case EFECT_OPT_SHOT: {
            printf("PLAYING SHOT %d\n", this->efect->at(i)->value);
            SCShot *sht = new SCShot();
            sht->Init();
            sht->SetShotId(this->efect->at(i)->value);
            this->cutsenes.push(sht);
        } break;
        case EFECT_OPT_FLYM: {
            uint8_t flymID = this->efect->at(i)->value;
            GameState.mission_flyed = flymID;
            printf("PLAYING FLYM %d\n", flymID);
            printf("Mission Name %s\n", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            this->missionToFly = (char *)malloc(13);
            sprintf(this->missionToFly, "%s.IFF", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            strtoupper(this->missionToFly, this->missionToFly);
            SCStrike *fly = new SCStrike();
            fly->Init();
            fly->SetMission(this->missionToFly);
            this->missionToFly = nullptr;
            fly_mission.push(fly);
        } break;
        case EFECT_OPT_FLYM2: {
            uint8_t flymID = this->efect->at(i)->value;
            GameState.mission_flyed = flymID;
            printf("PLAYING FLYM %d\n", flymID);
            printf("Mission Name %s\n", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            this->missionToFly = (char *)malloc(13);
            sprintf(this->missionToFly, "%s.IFF", this->gameFlowParser.game.mlst->data[flymID]->c_str());
            strtoupper(this->missionToFly, this->missionToFly);
            SCStrike *fly = new SCStrike();
            fly->Init();
            fly->SetMission(this->missionToFly);
            this->missionToFly = nullptr;
            fly_mission.push(fly);
            this->next_miss = GameState.mission_id;
        } break;
        case EFECT_OPT_SETFLAG_TRUE:
            GameState.requierd_flags[this->efect->at(i)->value] = true;
            break;
        case EFECT_OPT_SETFLAG_FALSE:
            GameState.requierd_flags[this->efect->at(i)->value] = false;
            break;
        case EFECT_OPT_MISS_ACCEPTED:
            if (this->convs.size() > 0) {
                SCConvPlayer *conv = this->convs.front();
                this->convs.pop();
                Game.AddActivity(conv);
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
                Game.AddActivity(conv);
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
            if (GameState.requierd_flags[this->efect->at(i)->value] == true) {
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
            if (GameState.requierd_flags[this->efect->at(i)->value] == false) {
                ifStack.push(true);
            } else {
                ifStack.push(false);
            }
            break;
        case EFECT_OPT_TEST_CURRENT_MISS:
            if (GameState.mission_flyed == this->efect->at(i)->value) {
                ifStack.push(true);
            } else {
                ifStack.push(false);
            }
            break;
        case EFECT_OPT_GO:
            this->next_miss = GameState.mission_id;
            break;
        case EFECT_OPT_MISS_ENDIF:
            if (ifStack.size() > 0) {
                ifStack.pop();
            }
            break;
        default:
            printf("Unkown opcode :%d, %d\n", this->efect->at(i)->opcode, this->efect->at(i)->value);
            break;
        };
    }
    this->efect = nullptr;
    this->currentOptCode = 0;
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
    // Keyboard
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

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
 * Initializes the SCGameFlow object by loading the necessary IFF files and initializing the optionParser and
 * gameFlowParser. It also initializes the optShps and optPals objects by loading the OPTSHPS.PAK and OPTPALS.PAK files
 * respectively. Finally, it sets the efect pointer to nullptr and calls the createMiss() function.
 *
 * @throws None
 */
void SCGameFlow::Init() {

    TreEntry *gameflowIFF =
        Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\GAMEFLOW.IFF");
    TreEntry *optionIFF =
        Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTIONS.IFF");
    this->optionParser.InitFromRam(optionIFF->data, optionIFF->size);
    this->gameFlowParser.InitFromRam(gameflowIFF->data, gameflowIFF->size);

    TreEntry *optShapEntry =
        Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry *optPalettesEntry =
        Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
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
    this->missionToFly = nullptr;
    this->next_miss = 0;
    this->zones.clear();
    this->layers.clear();
    this->convs = std::queue<SCConvPlayer *>();
    this->cutsenes = std::queue<SCShot *>();
    this->fly_mission = std::queue<SCStrike *>();
    printf("current miss : %d, current_scen %d\n", this->current_miss, this->current_scen);
    if (this->gameFlowParser.game.game[this->current_miss]->efct != nullptr) {
        this->efect = this->gameFlowParser.game.game[this->current_miss]->efct;
    }

    printf("efect size %zd\n", this->efect->size());
    this->createScen();
    this->runEffect();
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
    this->zones.clear();
    if (this->gameFlowParser.game.game[this->current_miss]->scen.size() > 0) {
        uint8_t optionScenID = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->info.ID;
        SCEN *sceneOpts = this->loadScene(optionScenID);
        for (auto sprite : this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->sprt) {
        uint8_t sprtId = sprite->info.ID;
        // le clck dans sprite semble indiquer qu'il faut jouer l'animation après avoir cliquer et donc executer le
        // efect à la fin de l'animation.
        uint8_t zone_id = 0;
        if (sceneOpts->foreground->sprites.count(sprtId) > 0) {
            uint8_t optsprtId = sceneOpts->foreground->sprites[sprtId]->sprite.SHP_ID;
            animatedSprites *sprt = new animatedSprites();
            SCZone *z = new SCZone();
            if (sceneOpts->foreground->sprites[sprtId]->zone != nullptr) {
                sprt->rect = new sprtRect();
                sprt->rect->x1 = sceneOpts->foreground->sprites[sprtId]->zone->X1;
                sprt->rect->y1 = sceneOpts->foreground->sprites[sprtId]->zone->Y1;
                sprt->rect->x2 = sceneOpts->foreground->sprites[sprtId]->zone->X2;
                sprt->rect->y2 = sceneOpts->foreground->sprites[sprtId]->zone->Y2;

                z->id = sprtId;
                z->position.x = sprt->rect->x1;
                z->position.y = sprt->rect->y1;
                z->dimension.x = sprt->rect->x2 - sprt->rect->x1;
                z->dimension.y = sprt->rect->y2 - sprt->rect->y1;
                z->label = sceneOpts->foreground->sprites[sprtId]->label;
                z->onclick = std::bind(&SCGameFlow::clicked, this, std::placeholders::_1, std::placeholders::_2);
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
                z->onclick = std::bind(&SCGameFlow::clicked, this, std::placeholders::_1, std::placeholders::_2);
            }

            sprt->id = sprite->info.ID;
            sprt->shapid = optsprtId;
            sprt->unkown = sprite->info.UNKOWN;
            sprt->efect = sprite->efct;
            sprt->img = this->getShape(optsprtId);
            sprt->frameCounter = 0;
            if (sceneOpts->foreground->sprites[sprtId]->CLCK == 1) {
                sprt->cliked = true;
                sprt->frameCounter = 1;
            }
            if (sceneOpts->foreground->sprites[sprtId]->SEQU != nullptr) {
                sprt->frames = sceneOpts->foreground->sprites[sprtId]->SEQU;
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
    }
}
SCEN* SCGameFlow::loadScene(uint8_t scene_id) {
    // note pour plus tard, une scene peu être composé de plusieur background
        // donc il faut boucler.
    SCEN *sceneOpts = this->optionParser.opts[scene_id];
    if (sceneOpts->tune != nullptr) {
        Mixer.SwitchBank(1);
        Mixer.StopMusic();
        Mixer.PlayMusic(sceneOpts->tune->ID);
    }
    for (auto bg : sceneOpts->background->images) {
        background *tmpbg = new background();
        tmpbg->img = this->getShape(bg->ID);
        this->layers.push_back(tmpbg);
    }
    uint8_t paltID = sceneOpts->background->palette->ID;
    uint8_t forPalTID = sceneOpts->foreground->palette->ID;
    
    

    this->rawPalette = this->optPals.GetEntry(paltID)->data;
    this->forPalette = this->optPals.GetEntry(forPalTID)->data;
    return sceneOpts;
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
void SCGameFlow::RunFrame(void) {

    if (this->cutsenes.size() > 0) {
        SCShot *c = this->cutsenes.front();
        this->cutsenes.pop();
        Game.AddActivity(c);
        return;
    }
    if (this->convs.size() > 0) {
        SCConvPlayer *c = this->convs.front();
        this->convs.pop();
        Game.AddActivity(c);
        return;
    }
    if (this->fly_mission.size() > 0) {
        SCStrike *c = this->fly_mission.front();
        this->fly_mission.pop();
        Game.AddActivity(c);
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
    CheckKeyboard();
    VGA.Activate();
    VGA.FillWithColor(0);
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

    this->CheckZones();

    for (int zi = 0; zi < this->zones.size(); zi++) {
        if (this->zones.at(zi)->IsActive(&GameState.requierd_flags)) {
            this->zones.at(zi)->Draw();
        }
    }

    VGA.VSync();

    this->RenderMenu();
    VGA.SwithBuffers();
    VGA.Activate();
    VGA.Clear();
    Mouse.Draw();
    VGA.VSync();
    VGA.SwithBuffers();
}

/**
 * This method renders the game flow menu using ImGui.
 *
 * It draws a menu bar with the following menu items:
 *   - Load Miss: Opens a modal window to select a mission to load.
 *   - Info: Opens a modal window with information about the current mission and scene.
 *   - Quit: Quits the game flow.
 *
 * It also renders a window with information about the current mission and scene.
 * If the current mission has an efect, it renders a subtree with the efects.
 * If the current mission has zones, it renders a subtree with the zones.
 * If a zone has a sprite, it renders a subtree with the sprite information.
 */
void SCGameFlow::RenderMenu() {
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    static bool show_scene_window = false;
    static bool quit_gameflow = false;
    static bool show_load_miss = false;
    static bool show_load_scene = false;
    static bool show_gamestate = false;
    static int miss_selected = 0;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("GameFlow")) {
            ImGui::MenuItem("Load Miss", NULL, &show_load_miss);
            ImGui::MenuItem("Load Scene", NULL, &show_load_scene);
            ImGui::MenuItem("Info", NULL, &show_scene_window);
            ImGui::MenuItem("GameState", NULL, &show_gamestate);
            ImGui::MenuItem("Quit", NULL, &quit_gameflow);

            ImGui::EndMenu();
        }
        int sceneid = -1;
        if (this->gameFlowParser.game.game[this->current_miss]->scen.size() > 0) {
            sceneid = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->info.ID;
        }
        ImGui::Text("Current Miss %d, Current Scen %d", this->current_miss, sceneid);
        ImGui::EndMainMenuBar();
    }

    if (show_load_miss) {
        ImGui::Begin("Load Miss");
        static ImGuiComboFlags flags = 0;
        static char **miss = new char *[this->gameFlowParser.game.game.size()];
        for (int i = 0; i < this->gameFlowParser.game.game.size(); i++) {
            miss[i] = new char[10];
            sprintf(miss[i], "%d", i);
        }
        if (ImGui::BeginCombo("List des miss", miss[miss_selected], flags)) {
            for (int i = 0; i < this->gameFlowParser.game.game.size(); i++) {
                const bool is_selected = (miss_selected == i);
                if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
                    miss_selected = i;

                // Set the initial focus when opening the combo (scrolling +
                // keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Load Mission")) {
            this->current_miss = miss_selected;
            this->current_scen = 0;
            this->currentSpriteId = 0;
            this->efect = nullptr;
            this->createMiss();
        }
        ImGui::End();
    }
    if (show_gamestate) {
        ImGui::Begin("GameState");
        for (auto flag : GameState.requierd_flags) {
            ImGui::Text("Flag %d, Value %d", flag.first, flag.second);
        }
        ImGui::Text("Mission ID %d", GameState.mission_id);
        ImGui::Text("Mission Flyed %d", GameState.mission_flyed);
        ImGui::Text("Mission Accepted %d", GameState.mission_accepted);
        ImGui::End();
    }
    if (show_scene_window) {
        ImGui::Begin("Infos");
        ImGui::Text("Current Miss %d, Current Scen %d", this->current_miss, this->current_scen);
        ImGui::Text("Nb Miss %d", this->gameFlowParser.game.game.size());
        ImGui::Text("Nb Layers %d", this->layers.size());
        ImGui::Text("Nb Scenes %d", this->gameFlowParser.game.game[this->current_miss]->scen.size());
        if (this->gameFlowParser.game.game[this->current_miss]->efct != nullptr) {
            ImGui::Text("Miss has efct");
            if (ImGui::TreeNode("Miss Efect")) {
                for (auto effect : *this->gameFlowParser.game.game[this->current_miss]->efct) {
                    ImGui::Text("OPC: %03d\tVAL: %03d", effect->opcode, effect->value);
                }
                ImGui::TreePop();
            }
        }
        if (ImGui::TreeNode("Required Flags")) {
            for (auto req_flag : GameState.requierd_flags) {
                ImGui::Text("FLAG %03d\tVALUE %03d", req_flag.first, req_flag.second);
            }
            ImGui::TreePop();
        }
        ImGui::Text("Nb Zones %d", this->zones.size());
        if (this->zones.size() > 1) {
            if (ImGui::TreeNode("Zones")) {
                for (auto zone : this->zones) {
                    if (ImGui::TreeNode((void *)(intptr_t)zone->id, "Zone %d", zone->id)) {
                        ImGui::Text(zone->label->c_str());
                        animatedSprites *sprite = zone->sprite;
                        if (zone->IsActive(&GameState.requierd_flags)) {
                            ImGui::Text("Active");
                        }
                        if (ImGui::TreeNode((void *)(intptr_t)sprite->shapid, "Sprite, Frame %d, %d %d",
                                            sprite->frameCounter, sprite->shapid, sprite->unkown)) {
                            if (sprite->frames != nullptr) {
                                ImGui::Text("Frames %d", sprite->frames->size());
                            }
                            if (sprite->quad != nullptr) {
                                ImGui::Text("Quad selection area");
                            }
                            if (sprite->rect != nullptr) {
                                ImGui::Text("Rect selection area");
                            }

                            if (sprite->efect->size() > 0) {
                                if (ImGui::TreeNode("Efect")) {
                                    for (auto efct : *sprite->efect) {
                                        ImGui::Text("OPC: %03d\tVAL: %03d", efct->opcode, efct->value);
                                    }
                                    ImGui::TreePop();
                                }
                            }
                            if (sprite->requ != nullptr && sprite->requ->size() > 0) {
                                if (ImGui::TreeNode("Required Flags")) {
                                    for (auto requ : *sprite->requ) {
                                        ImGui::Text("OPC: %03d\tVAL: %03d", requ->op, requ->value);
                                    }
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }
    if (show_load_scene) {
        ImGui::Begin("Load Scene");
        static ImGuiComboFlags flags = 0;
        
        if (ImGui::BeginCombo("List des scenes", nullptr, flags)) {
            for (auto scene: this->optionParser.opts) {
                const bool is_selected = (this->current_scen == scene.first);
                if (ImGui::Selectable(std::to_string(scene.first).c_str(), is_selected)) {
                    this->loadScene(scene.first);
                    this->zones.clear();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    
    }
    if (quit_gameflow) {
        Game.StopTopActivity();
    }

    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}