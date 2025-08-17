#include "RSPlaque.h"

void RSPlaque::parsePLAQ(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["FONT"] = std::bind(&RSPlaque::parsePLAQ_FONT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SHAP"] = std::bind(&RSPlaque::parsePLAQ_SHAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["INFO"] = std::bind(&RSPlaque::parsePLAQ_INFO, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSPlaque::parsePLAQ_FONT(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    this->tmp_font = new PLaqueFont();
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["WIDE"] = std::bind(&RSPlaque::parsePLAQ_FONT_WIDE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TINY"] = std::bind(&RSPlaque::parsePLAQ_FONT_TINY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SHAP"] = std::bind(&RSPlaque::parsePLAQ_FONT_SHAP, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
    this->fonts.push_back(*this->tmp_font);
}

void RSPlaque::parsePLAQ_FONT_WIDE(uint8_t *data, size_t size) {
    this->tmp_font->size = PlaqueFontSize::wide;
}

void RSPlaque::parsePLAQ_FONT_TINY(uint8_t *data, size_t size) {
    this->tmp_font->size = PlaqueFontSize::tiny;
}

void RSPlaque::parsePLAQ_FONT_SHAP(uint8_t *data, size_t size) {
    if (this->tmp_font) {
        uint8_t *archive_data = (uint8_t *)malloc(size);
        memcpy(archive_data, data, size);
        this->tmp_font->font = new RSFont();
        PakArchive *fontArchive = new PakArchive();
        fontArchive->InitFromRAM("PLAQUE_FONT", archive_data, size);
        if (fontArchive->GetNumEntries() == 0) {
            delete fontArchive;
            fontArchive = new PakArchive();
            size = size -1;
            fontArchive->InitFromRAM("PLAQUE_FONT", archive_data, size);
        }
        this->tmp_font->font->InitFromPAK(fontArchive);
    }
}

void RSPlaque::parsePLAQ_SHAP(uint8_t *data, size_t size) {
    uint8_t *archive_data = (uint8_t *)malloc(size);
    memcpy(archive_data, data, size);
    PakArchive *shapesArchive = new PakArchive();
    shapesArchive->InitFromRAM("PLAQUE_SHAPES", archive_data, size-1);
    for (int i = 0; i < shapesArchive->GetNumEntries(); i++) {
        PakEntry *entry = shapesArchive->GetEntry(i);
        RLEShape *shape = new RLEShape();
        shape->init(entry->data, entry->size);
        this->shapes.Add(shape);
    }
}

void RSPlaque::parsePLAQ_INFO(uint8_t *data, size_t size) {

}

void RSPlaque::InitFromRAM(uint8_t *data, size_t size) {
    
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["PLAQ"] = std::bind(&RSPlaque::parsePLAQ, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
