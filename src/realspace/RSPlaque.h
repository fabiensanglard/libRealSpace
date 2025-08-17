#pragma once

#include "../commons/IFFSaxLexer.h"
#include "RSFont.h"
#include "RSImageSet.h"

enum PlaqueFontSize {
    tiny,
    wide
};

typedef struct PLaqueFont {
    PlaqueFontSize size;
    RSFont *font;
} PLaqueFont;

class RSPlaque {
private:
    PLaqueFont *tmp_font{nullptr};
    
    void parsePLAQ(uint8_t *data, size_t size);
    void parsePLAQ_FONT(uint8_t *data, size_t size);
    void parsePLAQ_FONT_WIDE(uint8_t *data, size_t size);
    void parsePLAQ_FONT_TINY(uint8_t *data, size_t size);
    void parsePLAQ_FONT_SHAP(uint8_t *data, size_t size);
    void parsePLAQ_SHAP(uint8_t *data, size_t size);
    void parsePLAQ_INFO(uint8_t *data, size_t size);
    
public:
    std::vector<PLaqueFont> fonts;
    RSImageSet shapes;
    void InitFromRAM(uint8_t *data, size_t size);
};