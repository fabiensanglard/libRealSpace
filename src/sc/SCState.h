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
    bool mission_accepted{false};
    uint8_t mission_id{0};
};