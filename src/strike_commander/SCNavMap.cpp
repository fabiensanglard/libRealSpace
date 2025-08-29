//
//  SCNavMap.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 31/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"

namespace {
    struct LabelBox {
        int x,y,w,h;
    };
    static std::vector<LabelBox> g_labelBoxes;

    inline bool intersects(const LabelBox &a, const LabelBox &b) {
        return !(a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y || b.y + b.h <= a.y);
    }

    Point2D placeLabel(int anchorX, int anchorY, int w, int h) {
        // Positions candidates (dx, dy appliqués au point d’ancrage)
        static const std::vector<std::pair<int,int>> candidates = {
            { 0, 0},
            { 0,-h },
            { 0, h },
            { 0, h},
            { w, 0 },
            { w,-h },
            { w, h },
            { -w, 0},
            { -w,-h },
            { -w, h }
        };
        for (auto &c : candidates) {
            LabelBox box { anchorX + c.first, anchorY + c.second, w, h };
            bool clash = false;
            for (auto &b : g_labelBoxes) {
                if (intersects(box, b)) { clash = true; break; }
            }
            if (!clash) {
                g_labelBoxes.push_back(box);
                return Point2D{ box.x, box.y };
            }
        }
        // Fallback : empiler vers le bas
        int yOff = 6;
        while (yOff < 60) {
            LabelBox box { anchorX - w/2, anchorY + yOff, w, h };
            bool clash = false;
            for (auto &b : g_labelBoxes) {
                if (intersects(box, b)) { clash = true; break; }
            }
            if (!clash) {
                g_labelBoxes.push_back(box);
                return Point2D{ box.x, box.y };
            }
            yOff += h + 2;
        }
        // Dernier recours : ne pas enregistrer (risque chevauchement)
        return Point2D{ anchorX - w/2, anchorY + 4 };
    }

    inline void ResetLabelBoxes() {
        g_labelBoxes.clear();
    }
}

SCNavMap::SCNavMap(){
    this->navMap = nullptr;
}
SCNavMap::~SCNavMap(){

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
void SCNavMap::checkKeyboard(void) {
    if (this->m_keyboard == nullptr) {
        return;
    }
    if (this->m_keyboard->isActionJustPressed(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_ESCAPE))) {
        Game->stopTopActivity();
    }
    if (this->m_keyboard->isActionJustPressed(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_ESCAPE2))) {
        Game->stopTopActivity();
    }
    if (this->m_keyboard->isActionJustPressed(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_NEXT_WP))) {
        *this->current_nav_point = *this->current_nav_point+1;
        if (*this->current_nav_point > this->mission->waypoints.size()-1) {
            *this->current_nav_point = (uint8_t) this->mission->waypoints.size()-1;
        }
    }
    if (this->m_keyboard->isActionJustPressed(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_PREV_WP))) {
        *this->current_nav_point = *this->current_nav_point-1;
        if (*this->current_nav_point == 255) {
            *this->current_nav_point = 0;
        }
    }
}
/**
 * Initializes the SCNavMap object.
 *
 * This function reads in the PALETTE.IFF file and sets the object's color palette.
 * It also reads in the NAMAP.IFF file and sets the object's navMap property.
 *
 * @return None
 */
void SCNavMap::init(){
    RSPalette palette;
    palette.initFromFileData(Assets.GetFileData("PALETTE.IFF"));
    this->palette = *palette.GetColorPalette();
    for (uint8_t i=0; i<255; i++) {
        Texel *c = new Texel({i,0,0,255});
        this->objpal.SetColor(i, c);
    }
    
    TreEntry *nav_map = Assets.GetEntryByName(Assets.navmap_filename);
    this->navMap = new RSNavMap();
    this->navMap->InitFromRam(nav_map->data, nav_map->size);

    if (Assets.navmap_add_filename != "") {
        TreEntry *nav_map_add = Assets.GetEntryByName(Assets.navmap_add_filename);
        if (nav_map_add != nullptr) {
            RSNavMap *nav_map_add_obj = new RSNavMap();
            nav_map_add_obj->InitFromRam(nav_map_add->data, nav_map_add->size);
            for (auto &map: nav_map_add_obj->maps) {
                this->navMap->maps[map.first] = map.second;
            }
        }
    }
    m_keyboard = Game->getKeyboard();
    m_keyboard->registerAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_ESCAPE));
    m_keyboard->registerAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_NEXT_WP));
    m_keyboard->registerAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_PREV_WP));
    m_keyboard->registerAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_ESCAPE2));


    m_keyboard->bindKeyToAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_ESCAPE), SDL_SCANCODE_ESCAPE);
    m_keyboard->bindKeyToAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_PREV_WP), SDL_SCANCODE_LEFT);
    m_keyboard->bindKeyToAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_NEXT_WP), SDL_SCANCODE_RIGHT);
    m_keyboard->bindKeyToAction(CreateAction(InputAction::NAVMAP_START, NavActionOfst::NAV_ESCAPE2), SDL_SCANCODE_N);
    
}
/**
 * Sets the name of the current mission.
 *
 * This function takes a C-style string as its argument and sets the object's
 * name to that string.
 *
 * @param name the name of the current mission
 */
void SCNavMap::SetName(char *name) {
    this->name = new std::string(name);
    std::transform(this->name->begin(), this->name->end(), this->name->begin(), ::toupper);
}
/**
 * SCNavMap::RunFrame
 *
 * This function runs the frame of the navigation map.
 *
 * It clears the screen, sets the palette, draws the background and the shape
 * corresponding to the current mission, and draws the mouse.
 *
 * @return None
 */
void SCNavMap::runFrame(void) {
    this->checkKeyboard();
    VGA.Activate();
    VGA.GetFrameBuffer()->Clear();
    ResetLabelBoxes();
    VGA.SetPalette(&this->palette);
    
    //Draw static
    used_areas.clear();
    float center = BLOCK_WIDTH * 9.0f;
    float map_width = BLOCK_WIDTH * 18.0f;
    if (this->navMap->maps.count(*this->name)>0) {
        VGA.GetFrameBuffer()->DrawShape(this->navMap->maps[*this->name]);
        VGA.GetFrameBuffer()->DrawShape(this->navMap->background);
        Point2D pos = this->navMap->maps[*this->name]->position;
        int w = 238;
        int h = 155;
        int t = 18;
        int l = 6;
        
        std::string leader = "solo";
        if (this->mission->friendlies.size() > 0) {
            leader = this->mission->friendlies[0]->profile->radi.info.callsign;
        }
        std::string mission_name = this->mission->mission->mission_data.world_filename;
        std::transform(leader.begin(), leader.end(), leader.begin(), ::tolower);
        std::transform(mission_name.begin(), mission_name.end(), mission_name.begin(), ::tolower);
        int msg_newx = 257;
        int msg_newy = 10;
        Point2D mission_name_pos{248, 30};
        Point2D leader_pos{248, 50};

        VGA.GetFrameBuffer()->PrintText_SM(
            this->navMap->font,
            &mission_name_pos,
            const_cast<char*>("mission"),
            0,
            0,
            7,1,this->navMap->font->GetShapeForChar('A')->GetWidth(), false, false);
        
        mission_name_pos.x = 260;
        mission_name_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();

        VGA.GetFrameBuffer()->PrintText_SM(
            this->navMap->font,
            &mission_name_pos,
            (char*) mission_name.c_str(),
            0,
            0,
            (int32_t)mission_name.size(),1,this->navMap->font->GetShapeForChar('A')->GetWidth(), false, false);
        leader_pos.y = mission_name_pos.y + this->navMap->font->GetShapeForChar('A')->GetHeight();
        VGA.GetFrameBuffer()->PrintText_SM(
            this->navMap->font,
            &leader_pos,
            const_cast<char*>("leader"),
            0,
            0,
            6,1,this->navMap->font->GetShapeForChar('A')->GetWidth(), false, false);
        leader_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();
        leader_pos.x = 260;
        VGA.GetFrameBuffer()->PrintText_SM(
            this->navMap->font,
            &leader_pos,
            (char*) leader.c_str(),
            0,
            0,
            (int32_t)leader.size(),1,this->navMap->font->GetShapeForChar('A')->GetWidth(), false, false);
        if (this->mission->friendlies.size() > 1) {
            if (this->mission->friendlies[1]->profile != nullptr) {
                leader = this->mission->friendlies[1]->profile->radi.info.callsign;
            } else {
                leader = "wingman";
            }
            std::transform(leader.begin(), leader.end(), leader.begin(), ::tolower);
            leader_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();
            leader_pos.x = 252;
            VGA.GetFrameBuffer()->PrintText_SM(
                this->navMap->font,
                &leader_pos,
                const_cast<char*>("wingman"),
                0,
                0,
                7,1,this->navMap->font->GetShapeForChar('A')->GetWidth(), false, false);
            leader_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();
            leader_pos.x = 260;
            VGA.GetFrameBuffer()->PrintText_SM(
                this->navMap->font,
                &leader_pos,
                (char*) leader.c_str(),
                0,
                0,
                (int32_t)leader.size(),1,this->navMap->font->GetShapeForChar('A')->GetWidth(), false, false);
        }
        
        if (show_waypoint) {
            int cpt = 0;
            
            for (auto wp: this->mission->waypoints) {
                int newx = (int) (((wp->spot->position.x+center)/map_width)*w)+l;
                int newy = (int) (((wp->spot->position.z+center)/map_width)*h)+t;
                int c = 134;
                if (cpt == *this->current_nav_point) {
                    int msg_newx = 252;
                    int msg_newy = 80;
                    Point2D *msg_p1 = new Point2D({msg_newx, msg_newy});
                    
                    VGA.GetFrameBuffer()->PrintText_SM(
                        this->navMap->font,
                        msg_p1,
                        const_cast<char*>("objective"),
                        0,
                        0,
                        9,
                        1,
                        this->navMap->font->GetShapeForChar('A')->GetWidth(),
                        false, false
                    );
                    
                    msg_p1->y += this->navMap->font->GetShapeForChar('A')->GetHeight();
                    msg_p1->x = msg_newx+5;
                    if (wp->objective != nullptr) {
                        std::transform(wp->objective->begin(), wp->objective->end(), wp->objective->begin(), ::tolower);
                        VGA.GetFrameBuffer()->PrintText_SM(
                            this->navMap->font,
                            msg_p1,
                            (char*) wp->objective->c_str(),
                            0,
                            0,
                            (int32_t)wp->objective->size(),
                            1,
                            this->navMap->font->GetShapeForChar('A')->GetWidth(),
                            false, false
                        );
                    }

                    msg_newx = 255;
                    msg_newy = 135;
                    msg_p1 = new Point2D({msg_newx, msg_newy});
                    
                    VGA.GetFrameBuffer()->PrintText_SM(
                        this->navMap->font,
                        msg_p1,
                        const_cast<char*>("notes"),
                        0,
                        0,
                        5,
                        1,
                        this->navMap->font->GetShapeForChar('A')->GetWidth(),
                        false, false
                    );
                    msg_p1->y += this->navMap->font->GetShapeForChar('A')->GetHeight();
                    msg_p1->x = msg_newx+3;
                    if (wp->message != nullptr) {
                        std::transform(wp->message->begin(), wp->message->end(), wp->message->begin(), ::tolower);
                        VGA.GetFrameBuffer()->PrintText_SM(
                            this->navMap->font,
                            msg_p1,
                            (char*) wp->message->c_str(),
                            0,
                            0,
                            (int32_t)wp->message->size(),
                            1,
                            this->navMap->font->GetShapeForChar('A')->GetWidth(),
                            false, false
                        );
                    }
                    c = 255;
                    newx = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.x+center)/map_width)*w)+l;
                    newy = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.z+center)/map_width)*h)+t;
                    
                    if (newx>0 && newx<320 && newy>0 && newy<200) {
                        VGA.GetFrameBuffer()->plot_pixel(newx, newy, 128);
                        VGA.GetFrameBuffer()->circle_slow(newx, newy, 3, 1);
                    }
                    this->showArea(this->missionObj->mission_data.areas[wp->spot->area_id], center, map_width, w, h, t, l, c);
                } else {
                    if (wp->spot->area_id != 255) {
                        newx = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.x+center)/map_width)*w)+l;
                        newy = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.z+center)/map_width)*h)+t;
                        if (used_areas.find(wp->spot->area_id) == used_areas.end()) {
                            
                            if (newx>0 && newx<320 && newy>0 && newy<200) {
                                VGA.GetFrameBuffer()->plot_pixel(newx, newy, 128);
                                VGA.GetFrameBuffer()->circle_slow(newx, newy, 3, 1);
                            }
                            this->showArea(this->missionObj->mission_data.areas[wp->spot->area_id], center, map_width, w, h, t, l, c);
                        }                    
                    }
                    
                }
                cpt++;
            }
        }
        if (show_obj) {
            for (auto friends : this->mission->friendlies) {
                if (friends->is_active == 0 && friends->actor_name != "PLAYER") continue;
                int newx = (int) (((friends->object->position.x+center)/map_width)*w)+l;
                int newy = (int) (((friends->object->position.z+center)/map_width)*h)+t;
                if (newx>0 && newx<320 && newy>0 && newy<200) {
                    

                    std::string name = (friends->profile != nullptr) ? friends->profile->radi.info.callsign : "FRIEND";
                    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

                    int glyphW = this->navMap->font->GetShapeForChar('A')->GetWidth();
                    int glyphH = this->navMap->font->GetShapeForChar('A')->GetHeight();
                    int labelW = (int)name.size() * (glyphW + 1);
                    int labelH = glyphH;

                    Point2D obj_pos = placeLabel(newx-labelW, newy+glyphH, labelW, labelH);
                    VGA.GetFrameBuffer()->PrintText_SM(
                        this->navMap->font,
                        &obj_pos,
                        (char*) name.c_str(),
                        34,
                        0,
                        (int32_t)name.size(),1,glyphW,false,false
                    );
                    VGA.GetFrameBuffer()->plot_pixel(newx, newy, 10);
                    VGA.GetFrameBuffer()->circle_slow(newx, newy, 2, 1);
                }
            }
        }
    }
    
    Mouse.Draw();
    VGA.VSync();
}

void SCNavMap::showArea(AREA *area, float center, float map_width, int w, int h, int t, int l, int c) {
    int newx = (int) (((area->position.x+center)/map_width)*w)+l;
    int newy = (int) (((area->position.z+center)/map_width)*h)+t;
    int neww = (int) ((area->AreaWidth / map_width) * w);
    int newh = neww;
    if (area->AreaHeight != 0) {
        newh = (int) ((area->AreaHeight / map_width) * h);
    }
    if (newx>0 && newx<320 && newy>0 && newy<200) {
        int txtw = (int) strlen(area->AreaName) * (this->navMap->font->GetShapeForChar('A')->GetWidth()+1);
        int txtl = (int) strlen(area->AreaName);
        Point2D *p1 = new Point2D({newx-(txtw/2)<0?newx:newx-(txtw/2), newy});
        switch (area->AreaType) {
            case 'S':
                VGA.GetFrameBuffer()->circle_slow(newx, newy, neww, 1);
                /*VGA.GetFrameBuffer()->line(newx-neww, newy-neww, newx+neww, newy-neww, 1);
                VGA.GetFrameBuffer()->line(newx-neww, newy+neww, newx+neww, newy+neww, 1);
                VGA.GetFrameBuffer()->line(newx-neww, newy-neww, newx-neww, newy+neww, 1);
                VGA.GetFrameBuffer()->line(newx+neww, newy-neww, newx+neww, newy+neww, 1);*/
                break;
            case 'C':
                VGA.GetFrameBuffer()->circle_slow(newx, newy, neww, 1);
                break;
            default:
                VGA.GetFrameBuffer()->plot_pixel(newx, newy, 1);
                VGA.GetFrameBuffer()->line(newx-neww, newy-neww, newx+neww, newy-neww, 1);
                VGA.GetFrameBuffer()->line(newx-neww, newy+neww, newx+neww, newy+neww, 1);
                VGA.GetFrameBuffer()->line(newx-neww, newy-neww, newx-neww, newy+neww, 1);
                VGA.GetFrameBuffer()->line(newx+neww, newy-neww, newx+neww, newy+neww, 1);
                break;
        }
        int glyphW = this->navMap->font->GetShapeForChar('A')->GetWidth();
        int glyphH = this->navMap->font->GetShapeForChar('A')->GetHeight();
        int labelW = (int)strlen(area->AreaName) * (glyphW + 1);
        int labelH = glyphH;

        Point2D area_pos;
        if (used_areas.find(area->id-1) != used_areas.end()) {
            area_pos = used_areas[area->id-1];
        } else {
            area_pos = placeLabel(p1->x, p1->y, labelW, labelH);
            used_areas[area->id-1]=area_pos;
        }
        VGA.GetFrameBuffer()->PrintText(
            this->navMap->font,
            &area_pos,
            area->AreaName,
            c,
            0,
            txtl,1,this->navMap->font->GetShapeForChar('A')->GetWidth()
        );
    }
}
