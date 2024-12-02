//
//  RSWorld.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/12/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
#pragma once

#include "IFFSaxLexer.h"

class RSWorld {
private:
    void parseWRLD(uint8_t *data, size_t size);
    void parseWRLD_INFO(uint8_t *data, size_t size);
    void parseWRLD_HORZ(uint8_t *data, size_t size);
    void parseWRLD_WTCH(uint8_t *data, size_t size);
    void parseWRLD_PALT(uint8_t *data, size_t size);
    void parseWRLD_TERA(uint8_t *data, size_t size);
    void parseWRLD_TERA_FILE(uint8_t *data, size_t size);
    void parseWRLD_SKYS(uint8_t *data, size_t size);
    void parseWRLD_GLNT(uint8_t *data, size_t size);
    void parseWRLD_SMOK(uint8_t *data, size_t size);
    void parseWRLD_LGHT(uint8_t *data, size_t size);
    void parseWRLD_CAMR(uint8_t *data, size_t size);
    void parseWRLD_CAMR_STRT(uint8_t *data, size_t size);
    void parseWRLD_CAMR_CHAS(uint8_t *data, size_t size);
    void parseWRLD_CAMR_CKPT(uint8_t *data, size_t size);
    void parseWRLD_CAMR_VICT(uint8_t *data, size_t size);
    void parseWRLD_CAMR_TARG(uint8_t *data, size_t size);
    void parseWRLD_CAMR_WEAP(uint8_t *data, size_t size);
    void parseWRLD_CAMR_ROTA(uint8_t *data, size_t size);
    void parseWRLD_CAMR_COMP(uint8_t *data, size_t size);

public:
    std::string tera;
    RSWorld();
    ~RSWorld();
    void InitFromRAM(uint8_t *data, size_t size);
};