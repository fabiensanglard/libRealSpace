#pragma once
#include "precomp.h"

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