#include "RSSmokeSet.h"

RSSmokeSet::RSSmokeSet(){

}

RSSmokeSet::~RSSmokeSet(){
}
void RSSmokeSet::parseSMOK(uint8_t* data, size_t size){
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSSmokeSet::parseSMOK_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TYPE"] = std::bind(&RSSmokeSet::parseSMOK_TYPE, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSSmokeSet::parseSMOK_TYPE(uint8_t* data, size_t size){
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["SMOK"] = std::bind(&RSSmokeSet::parseSMOK_TYPE_SMOK, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SHAP"] = std::bind(&RSSmokeSet::parseSMOK_TYPE_SHAP, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSSmokeSet::parseSMOK_TYPE_SMOK(uint8_t* data, size_t size){
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSSmokeSet::parseSMOK_TYPE_SMOK, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ANIM"] = std::bind(&RSSmokeSet::parseSMOK_TYPE_SMOK, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSSmokeSet::parseSMOK_TYPE_SHAP(uint8_t* data, size_t size){
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["SHAP"] = std::bind(&RSSmokeSet::parseSMOK_TYPE_SHAP_SHAP, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSSmokeSet::parseSMOK_TYPE_SHAP_SHAP(uint8_t* data, size_t size){
    RSImageSet *img_set = new RSImageSet();
    PakArchive pak;
    uint8_t* data2 = (uint8_t*) malloc(size);
    memcpy(data2, data, size);
    pak.InitFromRAM("SHAPE", data2, size-1);
    img_set->InitFromSubPakEntry(&pak);
    this->images.push_back(img_set);
}
void RSSmokeSet::parseSMOK_INFO(uint8_t* data, size_t size){
    
}
void RSSmokeSet::parseSMOK_TYPE_SMOK_INFO(uint8_t* data, size_t size){
    
}
void RSSmokeSet::parseSMOK_TYPE_SMOK_ANIM(uint8_t* data, size_t size){
    
}
void RSSmokeSet::InitFromRam(uint8_t* data, size_t size){
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["SMOK"] = std::bind(&RSSmokeSet::parseSMOK, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}