//
//  RSProf.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 27/09/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
#pragma once

#include "IFFSaxLexer.h"



class RSProf {
private:
    void parsePROF(uint8_t *data, size_t size);
    void parsePROF_VERS(uint8_t *data, size_t size);
    void parsePROF_RADI(uint8_t *data, size_t size);
    void parsePROF_RADI_INFO(uint8_t *data, size_t size);
    void parsePROF_RADI_OPTS(uint8_t *data, size_t size);
    void parsePROF_RADI_MSGS(uint8_t *data, size_t size);
    void parsePROF_RADI_SPCH(uint8_t *data, size_t size);
    void parsePROF__AI_(uint8_t *data, size_t size);
    void parsePROF__AI_AI(uint8_t *data, size_t size);
    void parsePROF__AI_MVRS(uint8_t *data, size_t size);
    void parsePROF__AI_GOAL(uint8_t *data, size_t size);
    void parsePROF__AI_ATRB(uint8_t *data, size_t size);

public:
    RSProf();
    ~RSProf();
    void InitFromRAM(uint8_t *data, size_t size);
};