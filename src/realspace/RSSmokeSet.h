#pragma once
#include <vector>
#include <cstdint>
#include "AssetManager.h"
#include "../commons/IFFSaxLexer.h"

class RSImageSet;  // Forward declaration

class RSSmokeSet {
    
    void parseSMOK(uint8_t* data, size_t size);
    void parseSMOK_INFO(uint8_t* data, size_t size);
    void parseSMOK_TYPE(uint8_t* data, size_t size);
    void parseSMOK_TYPE_SMOK(uint8_t* data, size_t size);
    void parseSMOK_TYPE_SMOK_INFO(uint8_t* data, size_t size);
    void parseSMOK_TYPE_SMOK_ANIM(uint8_t* data, size_t size);
    void parseSMOK_TYPE_SHAP(uint8_t* data, size_t size);
    void parseSMOK_TYPE_SHAP_SHAP(uint8_t* data, size_t size);

public:
    RSSmokeSet();
    ~RSSmokeSet();
    void InitFromRam(uint8_t* data, size_t size);
    std::vector<RSImageSet*> images;
};