#include "DebugGameFlow.h"
#include "DebugStrike.h"
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

DebugGameFlow::DebugGameFlow() {}

DebugGameFlow::~DebugGameFlow() {}

void DebugGameFlow::flyMission() {
    SCStrike *fly = new DebugStrike();
    fly->init();
    fly->setMission(this->missionToFly);
    this->missionToFly = nullptr;
    fly_mission.push(fly);
    this->next_miss = GameState.mission_id;
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
void DebugGameFlow::renderMenu() {
    static bool show_scene_window = false;
    static bool show_load_miss = false;
    static bool show_gamestate = false;
    static bool show_shots = false;
    static bool load_sprites = false;
    static bool conv_player = false;
    static bool show_music_player = false;
    static int miss_selected = 0;
    if (ImGui::BeginMenu("GameFlow")) {
        ImGui::MenuItem("Load Miss", NULL, &show_load_miss);
        ImGui::MenuItem("Load shots", NULL, &show_shots);
        ImGui::MenuItem("Convert player", NULL, &conv_player);
        ImGui::MenuItem("load sprites from current scene", NULL, &load_sprites);
        ImGui::MenuItem("Info", NULL, &show_scene_window);
        ImGui::MenuItem("GameState", NULL, &show_gamestate);
        ImGui::MenuItem("Music Player", NULL, &show_music_player);
        ImGui::EndMenu();
    }
    int sceneid = -1;
    if (this->gameFlowParser.game.game[this->current_miss]->scen.size() > 0) {
        sceneid = this->gameFlowParser.game.game[this->current_miss]->scen[this->current_scen]->info.ID;
    }
    ImGui::Text("Current Miss %d, Current Scen %d", this->current_miss, sceneid);
    if (show_music_player) {
        ImGui::Begin("Music Player");
        if (ImGui::BeginCombo("Music list Bank 1", 0, 0)) {
            for (int i = 0; i < Mixer.music->musics[1].size(); i++) {
                if (ImGui::Selectable(std::to_string(i).c_str(), false)) {
                    this->scen->stopMusic();
                    Mixer.SwitchBank(1);
                    Mixer.PlayMusic(i);
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("Music list Bank 0", 0, 0)) {
            for (int i = 0; i < Mixer.music->musics[0].size(); i++) {
                if (ImGui::Selectable(std::to_string(i).c_str(), false)) {
                    Mixer.SwitchBank(0);
                    Mixer.PlayMusic(i);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::End();
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
        this->renderGameState();
        ImGui::End();
    }
    if (show_scene_window) {
        ImGui::Begin("Infos");
        this->renderMissionInfos();
        ImGui::End();
    }
    if (show_shots) {
        ImGui::Begin("Shots");
        static ImGuiComboFlags shtsflags = 0;
        if (ImGui::BeginCombo("List des shots", nullptr, shtsflags)) {
            for (auto shot : this->optionParser.estb) {
                if (ImGui::Selectable(std::to_string(shot.first).c_str(), false)) {
                    SCShot *sht = new SCShot();
                    sht->init();
                    sht->SetShotId(shot.first);
                    this->cutsenes.push(sht);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    }
    if (conv_player) {
        ImGui::Begin("Convert Player");
        static ImGuiComboFlags flags = 0;
        if (ImGui::BeginCombo("List des conversations", nullptr, flags)) {
            for (int i = 0; i < 256; i++) {
                if (ImGui::Selectable(std::to_string(i).c_str(), false)) {
                    SCConvPlayer *conv = new SCConvPlayer();
                    conv->init();
                    conv->SetID(i);
                    this->convs.push(conv);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    }
    if (load_sprites) {
        ImGui::Begin("Load Sprites");
        static ImGuiComboFlags flags = 0;
        if (ImGui::BeginCombo("List des extras", nullptr, flags)) {
            int cpt_sprite = 0;
            for (auto sprite : this->scen->sceneOpts->extr) {
                if (ImGui::Selectable(std::to_string(cpt_sprite).c_str(), false)) {
                    RSImageSet *shp;
                    shp = this->getShape(sprite->SHAPE_ID);
                    this->test_shape = shp->GetShape(0);
                }
                cpt_sprite++;
            }
            ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("List des sprites", nullptr, flags)) {
            int cpt_sprite = 0;
            for (auto sprite : this->scen->sceneOpts->foreground->sprites) {
                if (ImGui::Selectable(std::to_string(cpt_sprite).c_str(), false)) {
                    RSImageSet *shp;
                    shp = this->getShape(sprite.second->sprite.SHP_ID);
                    this->test_shape = shp->GetShape(0);
                }
                cpt_sprite++;
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    }
}

void DebugGameFlow::renderUI() {
    if (ImGui::BeginTabBar("GameFlowTabs")) {
        if (ImGui::BeginTabItem("Mission")) {
            this->renderMissionInfos();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("GameState")) {
            this->renderGameState();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void DebugGameFlow::renderGameState() {
    int nb_flags = (int) GameState.requierd_flags.size();
    int nb_rows = nb_flags / 8;
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("Flags", 8, flags)) {    
        auto it = GameState.requierd_flags.begin();
        for (int i = 0; i < nb_rows; i++) {
            ImGui::TableNextRow();
            for (int j = 0; j < 8; j++) {
                if (it != GameState.requierd_flags.end()) {
                    ImGui::TableSetColumnIndex(j);
                    ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                    if (it->second) {
                        cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f));
                    } else {
                        cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                    }
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                    ImGui::Text("[%03d]=%d", it->first, it->second);
                    ++it;
                }
            }
        }
        ImGui::EndTable();
    }
    
    ImGui::Text("Mission ID %d", GameState.mission_id);
    ImGui::Text("Mission Flyed %d", GameState.mission_flyed);
    ImGui::Text("Mission Accepted %d", GameState.mission_accepted);
    ImGui::Text("Proj Cash %d", GameState.proj_cash);
    ImGui::Text("Proj Overhead %d", GameState.over_head);
    for (auto killboard : GameState.kill_board) {
        ImGui::Text("Pilote %d, ground[%d] air[%d]", killboard.first, killboard.second[1], killboard.second[0]);
    }
    for (auto weapon : GameState.weapon_inventory) {
        ImGui::Text("Weapon %d, count %d", weapon.first, weapon.second);
    }
}

void DebugGameFlow::renderMissionInfos() {
    ImGui::Text("Current Miss %d, Current Scen %d", this->current_miss, this->current_scen);
    ImGui::Text("Nb Miss %d", this->gameFlowParser.game.game.size());
    ImGui::Text("Nb Layers %d", this->layers.size());
    ImGui::Text("Nb Scenes %d", this->gameFlowParser.game.game[this->current_miss]->scen.size());
    if (this->gameFlowParser.game.game[this->current_miss]->efct != nullptr) {
        ImGui::Text("Miss has efct");
        if (ImGui::TreeNode("Miss Efect")) {
            for (auto effect : *this->gameFlowParser.game.game[this->current_miss]->efct) {
                ImGui::Text("OPC: %s\tVAL: %03d", game_flow_op_name[GameFlowOpCode(effect->opcode)].c_str(), effect->value);
            }
            ImGui::TreePop();
        }
    }
    if (ImGui::TreeNode("Required Flags")) {
        int nb_flags = (int) GameState.requierd_flags.size();
        int nb_rows = nb_flags / 8;
        if (ImGui::BeginTable("Flags", 8)) {    
            auto it = GameState.requierd_flags.begin();
            for (int i = 0; i < nb_rows; i++) {
                ImGui::TableNextRow();
                for (int j = 0; j < 8; j++) {
                    if (it != GameState.requierd_flags.end()) {
                        ImGui::TableSetColumnIndex(j);
                        ImGui::Text("[%03d]=%d", it->first, it->second);
                        ++it;
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    ImGui::Text("Nb Zones %d", this->zones->size());
    if (this->zones->size() > 1) {
        if (ImGui::TreeNode("Zones")) {
            for (auto zone : *this->zones) {
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
                                    ImGui::Text("OPC: %s\tVAL: %03d", game_flow_op_name[GameFlowOpCode(efct->opcode)].c_str(), efct->value);
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
    if (ImGui::TreeNode("Scene Info")) {
        this->sceneOpts = this->scen->sceneOpts;
        if (this->sceneOpts->foreground != nullptr) {
            if (ImGui::TreeNode("Forground Sprites")) {
                for (auto sprt : this->sceneOpts->foreground->sprites) {
                    ImGui::Text("Sprite %d", sprt.second->sprite.SHP_ID);
                }
                ImGui::TreePop();
            }
        }
        if (ImGui::TreeNode("Background")) {
            for (auto bg : this->sceneOpts->background->images) {
                ImGui::Text("Image %d", bg->ID);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Extras")) {
            for (auto extra : this->sceneOpts->extr) {
                ImGui::Text("Extra %d", extra->SHAPE_ID);
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}