#include "precomp.h"

RSCockpit::RSCockpit(){

}
RSCockpit::~RSCockpit(){

}

void RSCockpit::InitFromRam(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["CKPT"] = std::bind(&RSCockpit::parseCKPT, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSCockpit::parseCKPT(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSCockpit::parseINFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["ARTP"] = std::bind(&RSCockpit::parseARTP, this, std::placeholders::_1, std::placeholders::_2);
	handlers["VTMP"] = std::bind(&RSCockpit::parseVTMP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["EJEC"] = std::bind(&RSCockpit::parseEJEC, this, std::placeholders::_1, std::placeholders::_2);
	handlers["GUNF"] = std::bind(&RSCockpit::parseGUNF, this, std::placeholders::_1, std::placeholders::_2);
	handlers["GHUD"] = std::bind(&RSCockpit::parseGHUD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["REAL"] = std::bind(&RSCockpit::parseREAL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CHUD"] = std::bind(&RSCockpit::parseCHUD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MONI"] = std::bind(&RSCockpit::parseMONI, this, std::placeholders::_1, std::placeholders::_2);
    handlers["FADE"] = std::bind(&RSCockpit::parseFADE, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSCockpit::parseINFO(uint8_t* data, size_t size) {
    this->INFO = std::vector<uint8_t>(data, data + size);
}
void RSCockpit::parseARTP(uint8_t* data, size_t size) {
    PakArchive* pak = new PakArchive();
    pak->InitFromRAM("ARTP", data, size);
    this->ARTP.InitFromSubPakEntry(pak);
}
void RSCockpit::parseVTMP(uint8_t* data, size_t size) {
    PakArchive* pak = new PakArchive();
    pak->InitFromRAM("VTMP", data, size);
    this->VTMP.InitFromSubPakEntry(pak);
}
void RSCockpit::parseEJEC(uint8_t* data, size_t size) {
    PakArchive* pak = new PakArchive();
    pak->InitFromRAM("EJEC", data, size);
    this->EJEC.InitFromSubPakEntry(pak);
}
void RSCockpit::parseGUNF(uint8_t* data, size_t size) {
    PakArchive* pak = new PakArchive();
    pak->InitFromRAM("GUNF", data, size);
    this->GUNF.InitFromSubPakEntry(pak);
}
void RSCockpit::parseGHUD(uint8_t* data, size_t size) {
    PakArchive* pak = new PakArchive();
    pak->InitFromRAM("GHUD", data, size);
    this->GHUD.InitFromSubPakEntry(pak);
}
void RSCockpit::parseREAL(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;

    handlers["INFO"] = std::bind(&RSCockpit::parseREAL_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["OBJS"] = std::bind(&RSCockpit::parseREAL_OBJS, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSCockpit::parseREAL_INFO(uint8_t* data, size_t size) {
    ByteStream* reader = new ByteStream(data);
    REAL.INFO = std::vector<uint8_t>(data, data + size);
}
void RSCockpit::parseREAL_OBJS(uint8_t* data, size_t size) {
    ByteStream* reader = new ByteStream(data);
    std::string name = reader->ReadString(size);
}
void RSCockpit::parseCHUD(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
    handlers["FILE"] = std::bind(&RSCockpit::parseCHUD_FILE, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSCockpit::parseCHUD_FILE(uint8_t* data, size_t size) {
    ByteStream* reader = new ByteStream(data);
    CHUD.FILE = reader->ReadString(size);
}
void RSCockpit::parseMONI(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;

    handlers["INFO"] = std::bind(&RSCockpit::parseMONI_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SPOT"] = std::bind(&RSCockpit::parseMONI_SPOT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SHAP"] = std::bind(&RSCockpit::parseMONI_SHAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] = std::bind(&RSCockpit::parseMONI_DAMG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MFDS"] = std::bind(&RSCockpit::parseMONI_MFDS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["INST"] = std::bind(&RSCockpit::parseMONI_INST, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSCockpit::parseMONI_INFO(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_SPOT(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_SHAP(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_DAMG(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_MFDS(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;

    handlers["COMM"] = std::bind(&RSCockpit::parseMONI_MFDS_COMM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["AARD"] = std::bind(&RSCockpit::parseMONI_MFDS_AARD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ARGD"] = std::bind(&RSCockpit::parseMONI_MFDS_ARGD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["GCAM"] = std::bind(&RSCockpit::parseMONI_MFDS_GCAM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WEAP"] = std::bind(&RSCockpit::parseMONI_MFDS_WEAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] = std::bind(&RSCockpit::parseMONI_MFDS_DAMG, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSCockpit::parseMONI_MFDS_COMM(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_MFDS_AARD(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_MFDS_ARGD(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_MFDS_GCAM(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_MFDS_WEAP(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_MFDS_DAMG(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_INST(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;

    handlers["RAWS"] = std::bind(&RSCockpit::parseMONI_INST_RAWS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ALTI"] = std::bind(&RSCockpit::parseMONI_INST_ALTI, this, std::placeholders::_1, std::placeholders::_2);
    handlers["AIRS"] = std::bind(&RSCockpit::parseMONI_INST_AIRS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MWRN"] = std::bind(&RSCockpit::parseMONI_INST_MWRN, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSCockpit::parseMONI_INST_RAWS(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_INST_ALTI(uint8_t* data, size_t size) {
    
}
void RSCockpit::parseMONI_INST_AIRS(uint8_t* data, size_t size) {

}
void RSCockpit::parseMONI_INST_MWRN(uint8_t* data, size_t size) {
   
}

void RSCockpit::parseFADE(uint8_t* data, size_t size) {

}