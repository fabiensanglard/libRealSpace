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

    this->weapon_inventory = {
        {AIM9J, buffer[0x16F]},
        {AIM9M, buffer[0x171]},
        {AIM120, buffer[0x173]},
        {AGM65D, buffer[0x175]},
        {DURANDAL, buffer[0x177]},
        {MK20, buffer[0x179]},
        {MK82, buffer[0x17B]},
        {GBU15, buffer[0x17D]}
    };
    this->current_mission = buffer[0x08];
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
