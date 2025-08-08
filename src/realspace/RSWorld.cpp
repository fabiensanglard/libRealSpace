#include "RSWorld.h"

RSWorld::RSWorld() {

}
RSWorld::~RSWorld() {

}
void RSWorld::InitFromRAM(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["WRLD"] = std::bind(&RSWorld::parseWRLD, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSWorld::parseWRLD(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSWorld::parseWRLD_INFO, this, std::placeholders::_1, std::placeholders::_2);

    handlers["HORZ"] = std::bind(&RSWorld::parseWRLD_HORZ, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WTCH"] = std::bind(&RSWorld::parseWRLD_WTCH, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PALT"] = std::bind(&RSWorld::parseWRLD_PALT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TERA"] = std::bind(&RSWorld::parseWRLD_TERA, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SKYS"] = std::bind(&RSWorld::parseWRLD_SKYS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["GLNT"] = std::bind(&RSWorld::parseWRLD_GLNT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SMOK"] = std::bind(&RSWorld::parseWRLD_SMOK, this, std::placeholders::_1, std::placeholders::_2);
    handlers["LGHT"] = std::bind(&RSWorld::parseWRLD_LGHT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CAMR"] = std::bind(&RSWorld::parseWRLD_CAMR, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSWorld::parseWRLD_INFO(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_HORZ(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_WTCH(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_PALT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_TERA(uint8_t *data, size_t size) {IFFSaxLexer lexer;

    ByteStream stream(data);
    this->tera = stream.ReadStringNoSize(size+1);
}

void RSWorld::parseWRLD_SKYS(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_GLNT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_SMOK(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_LGHT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_STRT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_CHAS(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_CKPT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_VICT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_TARG(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_WEAP(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_ROTA(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_CAMR_COMP(uint8_t *data, size_t size) {}
