//
//  RSProf.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 27/09/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
#pragma once

#include "IFFSaxLexer.h"

struct RADI_INFO {
    uint16_t id;
    std::string name;
    std::string callsign;
};
struct RADI {
    uint16_t spch;
    RADI_INFO info;
    std::map<std::uint8_t, std::string> msgs;
    std::map<std::string, std::string> asks;
    std::vector<std::string> asks_vector;
    std::string opts;
};
struct AI_ATTR {
    uint8_t TH{0};
    uint8_t CN{0};
    uint8_t VB{0};
    uint8_t LY{0};
    uint8_t FL{0};
    uint8_t AG{0};
    uint8_t AA{0};
    uint8_t SM{0};
    uint8_t AR{0};
};
struct AI_STATE {
    uint8_t node_id;
    uint8_t value;
};
struct AI {
    std::vector <AI_STATE> mvrs;
    std::vector <uint8_t> goal;
    AI_ATTR atrb;
    bool isAI{false};
};

class RSProf {
private:
    void parsePROF(uint8_t *data, size_t size);
    void parsePROF_VERS(uint8_t *data, size_t size);
    void parsePROF_RADI(uint8_t *data, size_t size);
    void parsePROF_RADI_INFO(uint8_t *data, size_t size);
    void parsePROF_RADI_OPTS(uint8_t *data, size_t size);
    void parsePROF_RADI_MSGS(uint8_t *data, size_t size);
    void parsePROF_RADI_ASKS(uint8_t *data, size_t size);
    void parsePROF_RADI_SPCH(uint8_t *data, size_t size);
    void parsePROF__AI_(uint8_t *data, size_t size);
    void parsePROF__AI_AI(uint8_t *data, size_t size);
    void parsePROF__AI_MVRS(uint8_t *data, size_t size);
    void parsePROF__AI_GOAL(uint8_t *data, size_t size);
    void parsePROF__AI_ATRB(uint8_t *data, size_t size);


public:

    uint16_t version{0};
    RADI radi;
    AI ai;

    RSProf();
    ~RSProf();
    void InitFromRAM(uint8_t *data, size_t size);
};