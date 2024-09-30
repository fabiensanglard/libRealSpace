
#include "RSProf.h"

RSProf::RSProf() {

}
RSProf::~RSProf() {

}
void RSProf::InitFromRAM(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["PROF"] = std::bind(&RSProf::parsePROF, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSProf::parsePROF(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["VERS"] = std::bind(&RSProf::parsePROF_VERS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["RADI"] = std::bind(&RSProf::parsePROF_RADI, this, std::placeholders::_1, std::placeholders::_2);
    handlers["_AI_"] = std::bind(&RSProf::parsePROF__AI_, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSProf::parsePROF_VERS(uint8_t *data, size_t size) {
    this->version = *(uint16_t*)data;
}
void RSProf::parsePROF_RADI(uint8_t *data, size_t size){
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSProf::parsePROF_RADI_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SPCH"] = std::bind(&RSProf::parsePROF_RADI_SPCH, this, std::placeholders::_1, std::placeholders::_2);
    handlers["OPTS"] = std::bind(&RSProf::parsePROF_RADI_OPTS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MSGS"] = std::bind(&RSProf::parsePROF_RADI_MSGS, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSProf::parsePROF_RADI_INFO(uint8_t *data, size_t size){
    ByteStream stream;
    if (data == nullptr) {
        return;
    }
    stream.Set(data);
    this->radi.info.id = stream.ReadShort();
    this->radi.info.name = stream.ReadStringNoSize(size);
    this->radi.info.callsign = stream.ReadStringNoSize(size);
}
void RSProf::parsePROF_RADI_OPTS(uint8_t *data, size_t size){
    ByteStream stream;
    if (data == nullptr) {
        return;
    }
    stream.Set(data);
    this->radi.opts = stream.ReadStringNoSize(size);
}
void RSProf::parsePROF_RADI_MSGS(uint8_t *data, size_t size){
    ByteStream stream;
    if (data == nullptr) {
        return;
    }
    stream.Set(data);
    while (stream.GetPosition() < data + size) {
        uint8_t key = stream.ReadByte();
        std::string value = stream.ReadStringNoSize(size);
        this->radi.msgs[key] = value;
    }
}
void RSProf::parsePROF_RADI_SPCH(uint8_t *data, size_t size){
    ByteStream stream;
    if (data == nullptr) {
        return;
    }
    stream.Set(data);
    this->radi.spch = stream.ReadShort();
}
void RSProf::parsePROF__AI_(uint8_t *data, size_t size){
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["AI\0_"] = std::bind(&RSProf::parsePROF__AI_AI, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MVRS"] = std::bind(&RSProf::parsePROF__AI_MVRS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["GOAL"] = std::bind(&RSProf::parsePROF__AI_GOAL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ATRB"] = std::bind(&RSProf::parsePROF__AI_ATRB, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSProf::parsePROF__AI_AI(uint8_t *data, size_t size) {
    this->ai.isAI = true;
}
void RSProf::parsePROF__AI_MVRS(uint8_t *data, size_t size) {
    ByteStream stream;
    if (data == nullptr) {
        return;
    }
    stream.Set(data);
    while (stream.GetPosition() < data + size) {
        this->ai.mvrs.push_back(stream.ReadByte());
    }
}
void RSProf::parsePROF__AI_GOAL(uint8_t *data, size_t size) {
    ByteStream stream;
    if (data == nullptr) {
        return;
    }
    stream.Set(data);
    while (stream.GetPosition() < data + size) {
        this->ai.goal.push_back(stream.ReadByte());
    }
}
void RSProf::parsePROF__AI_ATRB(uint8_t *data, size_t size) {
    ByteStream stream;
    if (data == nullptr) {
        return;
    }
    stream.Set(data);
    while (stream.GetPosition() < data + size) {
        this->ai.atrb.push_back(stream.ReadByte());
    }
}