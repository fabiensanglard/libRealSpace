#include "RSGameflow.h"
#include "IFFSaxLexer.h"
#include <map>
#include <string>
#include <functional>

RSGameFlow::RSGameFlow() {
}

RSGameFlow::~RSGameFlow() {
}

void RSGameFlow::InitFromRam(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["GAME"] = std::bind(&RSGameFlow::parseGAME, this, std::placeholders::_1, std::placeholders::_2);
	handlers["WRLD"] = std::bind(&RSGameFlow::parseWRLD, this, std::placeholders::_1, std::placeholders::_2);
	handlers["LOAD"] = std::bind(&RSGameFlow::parseLOAD, this, std::placeholders::_1, std::placeholders::_2);
	handlers["MLST"] = std::bind(&RSGameFlow::parseMLST, this, std::placeholders::_1, std::placeholders::_2);
	handlers["WNGS"] = std::bind(&RSGameFlow::parseWNGS, this, std::placeholders::_1, std::placeholders::_2);
	handlers["STAT"] = std::bind(&RSGameFlow::parseSTAT, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseGAME(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["MISS"] = std::bind(&RSGameFlow::parseMISS, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);

}

void RSGameFlow::parseMISS(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseMISS_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["EFCT"] = std::bind(&RSGameFlow::parseMISS_EFCT, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SCEN"] = std::bind(&RSGameFlow::parseMISS_SCEN, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseMISS_INFO(uint8_t* data, size_t size) {

}

void RSGameFlow::parseMISS_EFCT(uint8_t* data, size_t size) {

}

void RSGameFlow::parseMISS_SCEN(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseMISS_SCEN_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SPRT"] = std::bind(&RSGameFlow::parseMISS_SCEN_SPRT, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseMISS_SCEN_INFO(uint8_t* data, size_t size) {

}

void RSGameFlow::parseMISS_SCEN_SPRT(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseMISS_SCEN_SPRT_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["EFCT"] = std::bind(&RSGameFlow::parseMISS_SCEN_SPRT_EFCT, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseMISS_SCEN_SPRT_INFO(uint8_t* data, size_t size) {

}

void RSGameFlow::parseMISS_SCEN_SPRT_EFCT(uint8_t* data, size_t size) {

}

void RSGameFlow::parseWRLD(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["MAPS"] = std::bind(&RSGameFlow::parseWRLD_MAPS, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseWRLD_MAPS(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseWRLD_MAPS_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SPED"] = std::bind(&RSGameFlow::parseWRLD_MAPS_SPED, this, std::placeholders::_1, std::placeholders::_2);
	handlers["DATA"] = std::bind(&RSGameFlow::parseWRLD_MAPS_DATA, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseWRLD_MAPS_INFO(uint8_t* data, size_t size) {

}

void RSGameFlow::parseWRLD_MAPS_SPED(uint8_t* data, size_t size) {

}

void RSGameFlow::parseWRLD_MAPS_DATA(uint8_t* data, size_t size) {

}

void RSGameFlow::parseLOAD(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["LOAD"] = std::bind(&RSGameFlow::parseLOAD_LOAD, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseLOAD_LOAD(uint8_t* data, size_t size) {

}

void RSGameFlow::parseMLST(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["DATA"] = std::bind(&RSGameFlow::parseMLST_DATA, this, std::placeholders::_1, std::placeholders::_2);
	handlers["PRTL"] = std::bind(&RSGameFlow::parseMLST_PRTL, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseMLST_DATA(uint8_t* data, size_t size) {

}

void RSGameFlow::parseMLST_PRTL(uint8_t* data, size_t size) {

}

void RSGameFlow::parseWNGS(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["WING"] = std::bind(&RSGameFlow::parseWNGS_WING, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseWNGS_WING(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseWNGS_WING_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["PILT"] = std::bind(&RSGameFlow::parseWNGS_WING_PILT, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseWNGS_WING_INFO(uint8_t* data, size_t size) {

}

void RSGameFlow::parseWNGS_WING_PILT(uint8_t* data, size_t size) {

}

void RSGameFlow::parseSTAT(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["CHNG"] = std::bind(&RSGameFlow::parseSTAT_CHNG, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseSTAT_CHNG(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseSTAT_CHNG_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["PILT"] = std::bind(&RSGameFlow::parseSTAT_CHNG_PILT, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseSTAT_CHNG_INFO(uint8_t* data, size_t size) {

}

void RSGameFlow::parseSTAT_CHNG_PILT(uint8_t* data, size_t size) {

}
