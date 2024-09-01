#pragma once

#include <map>
#include <vector>
#include <cstring>

#include "IFFSaxLexer.h"
#include "RLEShape.h"
#include "ByteStream.h"

class RSNavMap {
    
    RLEShape *font;
    
    std::vector<uint8_t> texts;

    void parseNMAP(uint8_t* data, size_t size);
    void parseNMAP_MAPS(uint8_t* data, size_t size);
    void parseNMAP_FONT(uint8_t* data, size_t size);
    void parseNMAP_TEXT(uint8_t* data, size_t size);
    void parseNMAP_SHAP(uint8_t* data, size_t size);

public:
    std::map<std::string, RLEShape*> maps;
    RLEShape *background;
    RSNavMap();
	~RSNavMap();
	void InitFromRam(uint8_t* data, size_t size);
};