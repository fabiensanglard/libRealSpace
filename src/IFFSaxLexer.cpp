#include "IFFSaxLexer.h"
#include <cstring>
#include <string>

#include "../src/Base.h"
IFFSaxLexer::IFFSaxLexer() {
	this->stream = NULL;
	this->data = NULL;
	this->size = 0;
	this->path[0] = '\0';
}

IFFSaxLexer::~IFFSaxLexer() {

}
bool IFFSaxLexer::InitFromFile(const char* filepath, std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events) {
	char fullPath[512];
	fullPath[0] = '\0';

	strcat(fullPath, GetBase());
	strcat(fullPath, filepath);


	FILE* file = fopen(fullPath, "r+b");
	printf("IFF SAX:opening %s\n", fullPath);
	

	if (!file) {
		printf("IFF SAX:Unable to open IFF archive: '%s'.\n", filepath);
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
	return InitFromRAM(fileData, fileSize, events);
}

bool IFFSaxLexer::InitFromRAM(uint8_t* data, size_t size, std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events) {
	this->data = data;
	this->size = size;
	this->stream = new ByteStream();
	this->stream->Set(this->data);
	Parse(events);
	return false;
}

void IFFSaxLexer::Parse(std::map<std::string, std::function<void(uint8_t* data, size_t size)>> events) {
	size_t read = 0;
	while (read < this->size) {
		std::vector<uint8_t> bname = this->stream->ReadBytes(4);
		read += 4;
		std::string chunk_stype;
		chunk_stype.assign(bname.begin(), bname.end());

		if (chunk_stype == "FORM") {
			size_t chunk_size = this->stream->ReadUInt32BE();
			chunk_size += chunk_size % 2;
			std::vector<uint8_t> bname = this->stream->ReadBytes(4);
			chunk_stype.assign(bname.begin(), bname.end());
			read += 8;
			if (events.count(chunk_stype) > 0) {
				events.at(chunk_stype)(this->stream->ReadBytes(chunk_size-4).data(), chunk_size-4);
				read += (chunk_size-4);
			} else {
				printf("%s not handled\n", chunk_stype.c_str());
				std::vector<uint8_t> dump = this->stream->ReadBytes(chunk_size-4);
				read += (chunk_size-4);
			}
		} else {
			size_t chunk_size = this->stream->ReadUInt32BE();
			chunk_size += chunk_size % 2;
			read += 4;
			if (events.count(chunk_stype) > 0) {
				events.at(chunk_stype)(this->stream->ReadBytes(chunk_size).data(), chunk_size);
				read += chunk_size;
			} else {
				printf("%s not handled\n", chunk_stype.c_str());
				std::vector<uint8_t> dump = this->stream->ReadBytes(chunk_size);
				read += chunk_size;
			}
		}
	}
}