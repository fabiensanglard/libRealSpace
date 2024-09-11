//
//  IffLexer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "IffLexer.h"

#include <cstring>

#include "Base.h"

char textIDs[5];
char* GetChunkTextID(uint32_t id) {
	char* cursor = (char*)&id;
	for (int i = 3; i >= 0; i--)
		textIDs[i] = cursor[3 - i];


	return textIDs;
}

IffChunk::IffChunk() :
	subId(0) {
	this->data = NULL;
	this->size = 0;
	this->name[0] = '\0';
	this->id = 0;
}


IffChunk::~IffChunk() {
	/*while (!childs.empty()) {
		IffChunk* chunk = childs.back();
		childs.pop_back();
		delete chunk;
	}*/
}

//A CHUNK_HEADER_SIZE features a 4 bytes ID and a 4 bytes size;
#define CHUNK_HEADER_SIZE 8

IffLexer::IffLexer() {
	this->data = NULL;
	this->size = 0;
	this->path[0] = '\0';
}

IffLexer::~IffLexer() {

	/*
	for(std::vector<IffChunk*>::iterator i = topChunk.childs.begin();
		i != topChunk.childs.end();
		++i)
		delete *i;
	*/


	while (!topChunk.childs.empty()) {
		IffChunk* chunk = topChunk.childs.back();
		topChunk.childs.pop_back();
		delete chunk;
	}


	/*
	for(int i=0 ; i < this->topChunk.childs.size() ; i++){
		printf("~IffLexer Feeing topChunk childs  %d. (size=%lu)\n",i,this->topChunk.childs.size());
		IffChunk* chunk = topChunk.childs[i];
		delete chunk;
	}
	 */

}

bool IffLexer::InitFromFile(const char* filepath) {
	char fullPath[512];
	fullPath[0] = '\0';

	strcat(fullPath, GetBase());
	strcat(fullPath, filepath);


	FILE* file = fopen(fullPath, "r+b");
	printf("IFF Lexer: opening %s\n", fullPath);

	

	if (!file) {
		printf("IFF Lexer: Unable to open IFF archive: '%s'.\n", fullPath);
		return false;
	}

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t* fileData = new uint8_t[fileSize];
	size_t t = fread(fileData, 1, fileSize, file);
	printf("file %s read %llu bytes should be %llu\n", fullPath, t, fileSize);
	strcpy(this->path, filepath);
	fclose(file);
	return InitFromRAM(fileData, fileSize);

}

bool IffLexer::InitFromRAM(uint8_t* data, size_t size) {

	this->data = data;
	this->size = size;

	stream.Set(this->data);

	if (this->path[0] == '\0')
		strcpy(this->path, "PALETTE FROM RAM");

	Parse();

	return true;
}




size_t IffLexer::ParseFORM(IffChunk* chunk) {


	//FORM id
	chunk->id = stream.ReadUInt32BE();

	chunk->size = stream.ReadUInt32BE();

	if (chunk->size % 2 != 0)
		chunk->size++;
	size_t bytesToParse = chunk->size;

	//Form subtype
	chunk->subId = stream.ReadUInt32BE();
	chunksHashTable[chunk->subId] = chunk;

	bytesToParse -= 4;

	while (bytesToParse > 0) {

		IffChunk* child = new IffChunk();
		size_t byteParsed = ParseChunk(child);

		chunk->childs.push_back(child);
		chunksHashTable[child->id] = child;
		if (byteParsed > bytesToParse) {

			bytesToParse = 0;	
		} else {
			bytesToParse -= byteParsed;
		}
		
	}



	return chunk->size + CHUNK_HEADER_SIZE;
}

size_t IffLexer::ParseChunk(IffChunk* chunk) {

	ByteStream peek(stream);


	chunk->id = peek.ReadUInt32BE();

	switch (chunk->id) {
	case 'FORM':
		return ParseFORM(chunk);
		break;
	case 'CAT ':
		return ParseFORM(chunk);
		break;
	case 'LIST':
		return ParseFORM(chunk);
		break;
	default:
	{

		chunk->id = stream.ReadUInt32BE();

		chunk->size = stream.ReadUInt32BE();

		if (chunk->size % 2 != 0)
			chunk->size++;

		//That this chunk
		chunk->data = stream.GetPosition();
		stream.MoveForward(chunk->size);

		chunksHashTable[chunk->id] = chunk;



		return chunk->size + CHUNK_HEADER_SIZE;
	}
	break;
	}

}

size_t IffLexer::ParseCAT(IffChunk* chunk) {
	return ParseFORM(chunk);
}

//Return how many bytes were moved forward
size_t IffLexer::ParseLIST(IffChunk* chunk) {
	return ParseFORM(chunk);
}

void IffLexer::Parse(void) {

	size_t bytesToParse = this->size;


	ByteStream peek(stream);
	uint32_t header = peek.ReadUInt32BE();

	switch (header) {
	case 'FORM':
	case 'CAR ':
	case 'LIST':
		break;
	default:
	{
		printf("ERROR, this is not an IFF file.\n");
		return;
		break;
	}
	}



	while (bytesToParse > 0) {


		IffChunk* child = new IffChunk();

		size_t byteParsed = ParseChunk(child);

		topChunk.childs.push_back(child);
		chunksHashTable[child->id] = child;

		bytesToParse -= byteParsed;

	}

}

IffChunk* IffLexer::GetChunkByID(uint32_t id) {
	return this->chunksHashTable[id];
}

void Tab(int tab) {
	for (int i = 0; i < tab * 2; i++)
		putchar(' ');
}

void dumpChunk(IffChunk* chunk, const char* name) {
	printf("PARSING %s\n", name);
	if (chunk == NULL) {
		printf("NO CHUNK\n");
		return;
	}
	if (chunk->data == NULL) {
		printf("%s : NO DATA\n", name);
		return;
	}
	ByteStream stream(chunk->data);
	size_t fsize = chunk->size;
	uint8_t byte;
	int cl = 0;
	for (int read = 0; read < fsize; read++) {
		byte = stream.ReadByte();
		if (byte >= 40 && byte <= 90) {
			printf("[%c]", char(byte));
		}
		else if (byte >= 97 && byte <= 122) {
			printf("[%c]", char(byte));
		}
		else {
			printf("[0x%X]", byte);
		}
		if (cl > 2) {
			printf("\n");
			cl = 0;
		}
		else {
			printf("\t");
			cl++;
		}

	}
	printf("\n");
}

void ListChunkContent(uint32_t level, IffChunk* chunk) {

	Tab(level);
	printf("%s\n", GetChunkTextID(chunk->id));

	Tab(level);
	printf("%llu\n", chunk->size);

	//dumpChunk(chunk, GetChunkTextID(chunk->id));
	if (chunk->subId != 0) {
		Tab(level);
		printf("%s\n", GetChunkTextID(chunk->subId));

	}

	for (size_t i = 0; i < chunk->childs.size(); i++) {
		IffChunk* child = chunk->childs[i];
		ListChunkContent(level + 1, child);
	}

}


void IffLexer::List(FILE* output) {

	uint32_t level = 0;

	for (size_t i = 0; i < topChunk.childs.size(); i++) {
		IffChunk* child = topChunk.childs[i];
		ListChunkContent(level, child);
	}
}