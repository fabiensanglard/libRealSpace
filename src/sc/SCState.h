//
//  SCState.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 09/09/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "precomp.h"

class SCState {
public:
    std::map<uint8_t, bool> requierd_flags;
    std::map<uint8_t, bool> mission_flyed_success;
    std::vector<int64_t> missions_flags;
    std::map<uint8_t, uint8_t> weapon_load_out;
    bool mission_accepted{false};
    uint8_t mission_id{0};
    uint8_t mission_flyed{0};
};