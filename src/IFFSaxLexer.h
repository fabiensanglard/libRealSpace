//
//  IffSaxLexer.h
//  libRealSpace
//
//  Created by R�mi LEONARD on 08/08/2024.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <functional>
#include <string>
#include <map>

#include "../src/ByteStream.h"


class IFFSaxLexer {

public:

	IFFSaxLexer();
	~IFFSaxLexer();

	bool InitFromFile(const char* filepath, std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events);
	bool InitFromRAM(uint8_t* data, size_t size, std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events);


private:

	void Parse(std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events);

	ByteStream *stream;
	uint8_t* data;
	size_t size;

	char path[512];
};