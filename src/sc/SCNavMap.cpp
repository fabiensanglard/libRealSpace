//
//  SCNavMap.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 31/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"

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
void SCNavMap::CheckKeyboard(void) {
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
        case SDLK_c: 
            this->color = (this->color + 1) % 255;
        break;
        case SDLK_LEFT:
            *this->current_nav_point = *this->current_nav_point-1;
            if (*this->current_nav_point == 255) {
                *this->current_nav_point = 0;
            }
        break;
        case SDLK_RIGHT:
            *this->current_nav_point = *this->current_nav_point+1;
            if (*this->current_nav_point > this->mission->waypoints.size()-1) {
                *this->current_nav_point = (uint8_t) this->mission->waypoints.size()-1;
            }
        break;
        default:
            break;
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
void SCNavMap::Init(){
    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();
    for (uint8_t i=0; i<255; i++) {
        Texel *c = new Texel({i,0,0,255});
        this->objpal.SetColor(i, c);
    }
    
    TreEntry *nav_map = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName("..\\..\\DATA\\COCKPITS\\NAVMAP.IFF");
    this->navMap = new RSNavMap();
    this->navMap->InitFromRam(nav_map->data, nav_map->size);
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
void SCNavMap::RunFrame(void) {
    this->CheckKeyboard();
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(this->navMap->background);
    float center = BLOCK_WIDTH * 9.0f;
    float map_width = BLOCK_WIDTH * 18.0f;
    if (this->navMap->maps.count(*this->name)>0) {
        VGA.DrawShape(this->navMap->maps[*this->name]);
        Point2D pos = this->navMap->maps[*this->name]->position;
        int w = 238;
        int h = 155;
        int t = 18;
        int l = 6;
        
        std::string leader = "solo";
        if (this->mission->friendlies.size() > 0) {
            leader = this->mission->friendlies[0]->actor_name;
        }
        std::string mission_name = this->mission->mission->mission_data.info;
        std::transform(leader.begin(), leader.end(), leader.begin(), ::tolower);
        std::transform(mission_name.begin(), mission_name.end(), mission_name.begin(), ::tolower);
        int msg_newx = 257;
        int msg_newy = 10;
        Point2D mission_name_pos{252, 40};
        Point2D leader_pos{252, 50};

        VGA.PrintText(
            this->navMap->font,
            &mission_name_pos,
            "mission",
            0,
            0,
            7,1,this->navMap->font->GetShapeForChar('A')->GetWidth());
        
        mission_name_pos.x = 260;
        mission_name_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();

        VGA.PrintText(
            this->navMap->font,
            &mission_name_pos,
            (char*) mission_name.c_str(),
            0,
            0,
            (int32_t)mission_name.size(),1,this->navMap->font->GetShapeForChar('A')->GetWidth());
        leader_pos.y = mission_name_pos.y + this->navMap->font->GetShapeForChar('A')->GetHeight();
        VGA.PrintText(
            this->navMap->font,
            &leader_pos,
            "leader",
            0,
            0,
            6,1,this->navMap->font->GetShapeForChar('A')->GetWidth());
        leader_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();
        leader_pos.x = 260;
        VGA.PrintText(
            this->navMap->font,
            &leader_pos,
            (char*) leader.c_str(),
            0,
            0,
            (int32_t)leader.size(),1,this->navMap->font->GetShapeForChar('A')->GetWidth());
        if (this->mission->friendlies.size() > 1) {
            leader = this->mission->friendlies[1]->actor_name;
            std::transform(leader.begin(), leader.end(), leader.begin(), ::tolower);
            leader_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();
            leader_pos.x = 252;
            VGA.PrintText(
                this->navMap->font,
                &leader_pos,
                "wingman",
                0,
                0,
                6,1,this->navMap->font->GetShapeForChar('A')->GetWidth());
            leader_pos.y += this->navMap->font->GetShapeForChar('A')->GetHeight();
            leader_pos.x = 260;
            VGA.PrintText(
                this->navMap->font,
                &leader_pos,
                (char*) leader.c_str(),
                0,
                0,
                (int32_t)leader.size(),1,this->navMap->font->GetShapeForChar('A')->GetWidth());
        }
        if (show_area) {
            for (auto area: this->missionObj->mission_data.areas) {
                this->showArea(area, center, map_width, w, h, t, l, 134);
            }

        }
        
        if (show_obj) {
            for (auto ob: this->missionObj->mission_data.parts) {
                int newx = (int) (((ob->position.x+center)/map_width)*w)+l;
                int newy = newy = (int) (((ob->position.z+center)/map_width)*h)+t;
                if (newx>0 && newx<320 && newy>0 && newy<200) {
                    VGA.plot_pixel(newx, newy, ob->id);
                    VGA.line(newx-5, newy-5, newx+5, newy-5, ob->id);
                    VGA.line(newx-5, newy+5, newx+5, newy+5, ob->id);
                    VGA.line(newx-5, newy-5, newx-5, newy+5, ob->id);
                    VGA.line(newx+5, newy-5, newx+5, newy+5, ob->id);
                    Point2D *obj_pos = new Point2D({newx, newy});
                    
                    VGA.PrintText(
                        this->navMap->font,
                        obj_pos,
                        (char*) ob->member_name.c_str(),
                        this->color,
                        0,
                        (int32_t)ob->member_name.size(),1,this->navMap->font->GetShapeForChar('A')->GetWidth());
                }
            }
        }
        
        if (show_waypoint) {
            int cpt = 0;
            for (auto wp: this->mission->waypoints) {
                int newx = (int) (((wp->spot->position.x+center)/map_width)*w)+l;
                int newy = (int) (((wp->spot->position.z+center)/map_width)*h)+t;
                int c = 134;
                if (cpt == *this->current_nav_point) {
                    
                }
                if (wp->spot->area_id != 255) {
                    newx = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.x+center)/map_width)*w)+l;
                    newy = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.z+center)/map_width)*h)+t;
                    this->showArea(this->missionObj->mission_data.areas[wp->spot->area_id], center, map_width, w, h, t, l, c);
                }
                if (newx>0 && newx<320 && newy>0 && newy<200) {
                    VGA.plot_pixel(newx, newy, 128);
                    VGA.line(newx-5, newy-5, newx+5, newy-5, 128);
                    VGA.line(newx-5, newy+5, newx+5, newy+5, 128);
                    VGA.line(newx-5, newy-5, newx-5, newy+5, 128);
                    VGA.line(newx+5, newy-5, newx+5, newy+5, 128);
                }
                if (cpt == *this->current_nav_point) {
                    int msg_newx = 257;
                    int msg_newy = 90;
                    Point2D *msg_p1 = new Point2D({msg_newx, msg_newy});
                    VGA.PrintText(
                        this->navMap->font,
                        msg_p1,
                        "objective",
                        0,
                        0,
                        9,
                        1,
                        this->navMap->font->GetShapeForChar('A')->GetWidth()
                    );
                    msg_p1->y += this->navMap->font->GetShapeForChar('A')->GetHeight();
                    msg_p1->x = msg_newx;
                    if (wp->objective != nullptr) {
                        std::transform(wp->objective->begin(), wp->objective->end(), wp->objective->begin(), ::tolower);
                        VGA.PrintText(
                            this->navMap->font,
                            msg_p1,
                            (char*) wp->objective->c_str(),
                            0,
                            0,
                            (int32_t)wp->objective->size(),
                            1,
                            this->navMap->font->GetShapeForChar('A')->GetWidth()
                        );
                    }

                    msg_newx = 257;
                    msg_newy = 143;
                    msg_p1 = new Point2D({msg_newx, msg_newy});
                    VGA.PrintText(
                        this->navMap->font,
                        msg_p1,
                        "notes",
                        0,
                        0,
                        5,
                        1,
                        this->navMap->font->GetShapeForChar('A')->GetWidth()
                    );
                    msg_p1->y += this->navMap->font->GetShapeForChar('A')->GetHeight();
                    msg_p1->x = msg_newx;
                    if (wp->message != nullptr) {
                        std::transform(wp->message->begin(), wp->message->end(), wp->message->begin(), ::tolower);
                        VGA.PrintText(
                            this->navMap->font,
                            msg_p1,
                            (char*) wp->message->c_str(),
                            0,
                            0,
                            (int32_t)wp->message->size(),
                            1,
                            this->navMap->font->GetShapeForChar('A')->GetWidth()
                        );
                    }
                    c = 255;
                    newx = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.x+center)/map_width)*w)+l;
                    newy = (int) (((this->missionObj->mission_data.areas[wp->spot->area_id]->position.z+center)/map_width)*h)+t;
                    this->showArea(this->missionObj->mission_data.areas[wp->spot->area_id], center, map_width, w, h, t, l, c);
                }
                cpt++;
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
                VGA.line(newx-neww, newy-neww, newx+neww, newy-neww, 1);
                VGA.line(newx-neww, newy+neww, newx+neww, newy+neww, 1);
                VGA.line(newx-neww, newy-neww, newx-neww, newy+neww, 1);
                VGA.line(newx+neww, newy-neww, newx+neww, newy+neww, 1);
                break;
            case 'C':
                VGA.circle_slow(newx, newy, neww, 128);
                break;
            default:
                VGA.plot_pixel(newx, newy, 128);
                break;
        }
        VGA.PrintText(
            this->navMap->font,
            p1,
            area->AreaName,
            c,
            0,
            txtl,1,this->navMap->font->GetShapeForChar('A')->GetWidth()
        );
    }
}
