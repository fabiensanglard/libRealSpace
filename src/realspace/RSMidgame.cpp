#include "RSMidgame.h"

RSMidgame::RSMidgame() {
}

RSMidgame::~RSMidgame() {
}

void RSMidgame::parseMG_GEN(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DATA"] = std::bind(&RSMidgame::parseMG_DATA, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SPOK"] = std::bind(&RSMidgame::parseMG_SPOK, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSMidgame::InitFromRAM(uint8_t *data, size_t size) {
    this->tmp_key = "";
    this->parseMIDG(data, size);
}

void RSMidgame::parseMIDG(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["MG01"] = std::bind(&RSMidgame::parseMG01, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG02"] = std::bind(&RSMidgame::parseMG02, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG05"] = std::bind(&RSMidgame::parseMG05, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG12"] = std::bind(&RSMidgame::parseMG12, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG15"] = std::bind(&RSMidgame::parseMG15, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG16"] = std::bind(&RSMidgame::parseMG16, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG17"] = std::bind(&RSMidgame::parseMG17, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG20"] = std::bind(&RSMidgame::parseMG20, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG33"] = std::bind(&RSMidgame::parseMG33, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG34"] = std::bind(&RSMidgame::parseMG34, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MG36"] = std::bind(&RSMidgame::parseMG36, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSMidgame::parseMG01(uint8_t *data, size_t size){
    this->tmp_key = "MG01";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG02(uint8_t *data, size_t size){
    this->tmp_key = "MG02";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG05(uint8_t *data, size_t size){
    this->tmp_key = "MG05";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG12(uint8_t *data, size_t size){
    this->tmp_key = "MG12";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG15(uint8_t *data, size_t size){
    this->tmp_key = "MG15";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG16(uint8_t *data, size_t size){
    this->tmp_key = "MG16";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG17(uint8_t *data, size_t size){
    this->tmp_key = "MG17";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG20(uint8_t *data, size_t size){
    this->tmp_key = "MG20";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG33(uint8_t *data, size_t size){
    this->tmp_key = "MG33";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG34(uint8_t *data, size_t size){
    this->tmp_key = "MG34";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG36(uint8_t *data, size_t size){
    this->tmp_key = "MG36";
    this->parseMG_GEN(data, size);
}

void RSMidgame::parseMG_DATA(uint8_t *data, size_t size){
    IFFSaxLexer lexer;
    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["TEXT"] = std::bind(&RSMidgame::parseMG_DATA_TEXT, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSMidgame::parseMG_SPOK(uint8_t *data, size_t size){
    IFFSaxLexer lexer;
    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["TEXT"] = std::bind(&RSMidgame::parseMG_SPOK_TEXT, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSMidgame::parseMG_DATA_TEXT(uint8_t *data, size_t size){
    ByteStream stream(data, size);
    std::string text = stream.ReadStringNoSize(size);
    this->midgame_texts[this->tmp_key].DATA.push_back(text);
}

void RSMidgame::parseMG_SPOK_TEXT(uint8_t *data, size_t size){
    ByteStream stream(data, size);
    std::string text = stream.ReadStringNoSize(size);
    this->midgame_texts[this->tmp_key].DATA.push_back(text);
    this->midgame_texts[this->tmp_key].SPOK.push_back(text);
}
