#include "precomp.h"
#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>
//
//  SCState.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 09/09/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
void SCState::Load(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (buffer.size() < 0x24E) {
        std::cerr << "File size is too small: " << filename << std::endl;
        return;
    }
    for (int i=0; i<120; i++) {
        this->requierd_flags[i] = buffer[0x0B + i];
    }
    for (int i=0; i<46; i++) {
        this->mission_flyed_success[i] = buffer[0x10B + i];
    }
    /*
        018D - 018E - Amount of money lost
        0189 - 018A - Bank balance
    */
    this->proj_cash = (0 | (buffer[0x18A] << 8) | buffer[0x189]) * 1000;
    this->cash = this->proj_cash;
    this->over_head = (0 | (buffer[0x18E] << 8) | buffer[0x18D]) * 1000;
    
    /*
        024D - 024E - Current Score
    */
    this->score = 0 | (buffer[0x24E] << 8) | buffer[0x24D];
    /*
        019B - # of ground kills
        0199 - # of Air Kills
    */
    /*
        01C9 - 01DC - Player Last Name
    */
    this->player_name = std::string(buffer.begin() + 0x1C9, buffer.begin() + 0x1DC);
    /*
        01DD - 01F0 - Player First Name
    */
    this->player_firstname = std::string(buffer.begin() + 0x1DD, buffer.begin() + 0x1F0);
    /*
        01F1 - 0204 - Player Callsign
    */
    this->player_callsign = std::string(buffer.begin() + 0x1F1, buffer.begin() + 0x204);
    /*
        208 - 21B - WingMan
    */
    this->wingman = std::string(buffer.begin() + 0x208, buffer.begin() + 0x21B);
    this->ground_kills = buffer[0x199];
    this->air_kills = buffer[0x19B];
    /* 19D - 1C5 killboard */
    for (int i=0; i<6; i++) {
        int alive = buffer[0x19D + i*0x06];
        int ground_kills = (buffer[0x19D + i*0x06+3] << 8) | buffer[0x19D + i*0x06+2];
        int air_kills = (buffer[0x19D + i*0x06+5] << 8) | buffer[0x19D + i*0x06+4];
        this->kill_board[i+1][0] = ground_kills;
        this->kill_board[i+1][1] = air_kills;
    }
    this->kill_board[0][0] = this->ground_kills;
    this->kill_board[0][1] = this->air_kills;
    
    this->weapon_inventory = {
        {ID_AIM9J, buffer[0x16F]},
        {ID_AIM9M, buffer[0x171]},
        {ID_AGM65D, buffer[0x173]},
        {ID_LAU3, buffer[0x17D]},
        {ID_MK20, buffer[0x177]},
        {ID_MK82, buffer[0x179]},
        {ID_DURANDAL, buffer[0x175]},
        {ID_GBU15, buffer[0x17B]},
        {ID_AIM120, buffer[0x17F]}
    };
    this->current_mission = buffer[0x08];
    this->mission_flyed = buffer[0x09];
    this->current_scene = buffer[0x0A];
}

void SCState::Save(std::string filename) {}

void SCState::Reset() {
    this->requierd_flags.clear();
    this->mission_flyed_success.clear();
    this->missions_flags.clear();
    this->weapon_load_out.clear();
    this->aircraftHooks.clear();
    this->kill_board.clear();
    this->weapon_inventory.clear();
    this->current_mission = 0;
    this->current_scene = 0;
    this->over_head = 0;
    this->proj_cash = 0;
    this->tune_modifier = 0;
}
