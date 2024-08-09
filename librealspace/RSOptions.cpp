#include "RSOption.h"
#include "IFFSaxLexer.h"
#include <map>
#include <string>
#include <functional>

RSOption::RSOption() {
	this->mark.DATA = nullptr;
	this->mark.unkown_1.UNKOWN_1 = 0;
}

RSOption::~RSOption() {
	for (auto& it : this->opts) {
		delete it.second;
	}
	for (auto& it : this->estb) {
		delete it.second;
	}
	if (this->mark.DATA != nullptr) {
		delete[] this->mark.DATA;
	}
}

void RSOption::InitFromRam(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["OPTS"] = std::bind(&RSOption::parseOPTS, this, std::placeholders::_1, std::placeholders::_2);
	lexer.InitFromRAM(data, size, handlers);

}

void RSOption::parseOPTS(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["SCEN"] = std::bind(&RSOption::parseSCEN, this, std::placeholders::_1, std::placeholders::_2);
	handlers["ETSB"] = std::bind(&RSOption::parseETSB, this, std::placeholders::_1, std::placeholders::_2);
	handlers["MARK"] = std::bind(&RSOption::parseMARK, this, std::placeholders::_1, std::placeholders::_2);
	lexer.InitFromRAM(data, size, handlers);
}

void RSOption::parseSCEN(uint8_t* data, size_t size) {
	printf("Parsing SCEN %llu\n", size);
}

void RSOption::parseETSB(uint8_t* data, size_t size) {
	printf("Parsing ETSB %llu\n", size);
}

void RSOption::parseMARK(uint8_t* data, size_t size) {
	printf("Parsing MARK %llu\n", size);
}