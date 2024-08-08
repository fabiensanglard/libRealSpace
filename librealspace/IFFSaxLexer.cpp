#include "IFFSaxLexer.h"
#include <cstring>
#include "../src/Base.h"
IFFSaxLexer::IFFSaxLexer() {
	this->data = NULL;
	this->size = 0;
	this->path[0] = '\0';
}

IFFSaxLexer::~IFFSaxLexer() {
	free(this->data);

}
bool IFFSaxLexer::InitFromFile(const char* filepath, std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events) {
	char fullPath[512];
	fullPath[0] = '\0';

	strcat(fullPath, GetBase());
	strcat(fullPath, filepath);


	FILE* file = fopen(fullPath, "r+b");

	if (!file) {
		printf("Unable to open IFF archive: '%s'.\n", filepath);
		return false;
	}

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t* fileData = new uint8_t[fileSize];
	size_t t = fread(fileData, 1, fileSize, file);
	printf("file %s read %llu bytes should be %llu\n", fullPath, t, fileSize);
	strcpy(this->path, filepath);

	return InitFromRAM(fileData, fileSize, events);
}

bool IFFSaxLexer::InitFromRAM(uint8_t* data, size_t size, std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events) {
	this->data = data;
	this->size = size;

	this->stream.Set(this->data);
	Parse(events);
	return false;
}

void IFFSaxLexer::Parse(std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events) {

}
