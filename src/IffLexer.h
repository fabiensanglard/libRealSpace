//
//  IffLexer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <map>

#include "ByteStream.h"
#include "ByteSlice.h"

class IffChunk {

public:

	IffChunk();
	~IffChunk();

	uint32_t id;
	uint8_t* data;
	size_t size;

	//In the case of FORM,CAT  and LIST
	uint32_t subId;
	std::vector<IffChunk*> childs;

	char name[5];
	char* GetName(void) {
		name[0] = (id & 0xFF000000) >> 24;
		name[1] = (id & 0x00FF0000) >> 16;
		name[2] = (id & 0x0000FF00) >> 8;
		name[3] = (id & 0x000000FF) >> 0;
		name[4] = 0;

		return name;
	};
};

class IffLexer {

public:

	IffLexer();
	~IffLexer();

	bool InitFromFile(const char* filepath);
	bool InitFromRAM(uint8_t* data, size_t size);

	void List(FILE* output);

	IffChunk* GetChunkByID(uint32_t id);

	inline const char* GetName(void) { return this->path; }

private:

	size_t ParseChunk(IffChunk* child);
	size_t ParseFORM(IffChunk* child);
	size_t ParseCAT(IffChunk* child);
	size_t ParseLIST(IffChunk* child);

	void Parse(void);
	std::map<uint32_t, IffChunk*> chunksHashTable;

	ByteStream stream;
	uint8_t* data;
	size_t size;

	IffChunk topChunk;

	char path[512];
};