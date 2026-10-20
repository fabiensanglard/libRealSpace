//
//  Stream.cpp
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "ByteStream.h"


ByteStream::ByteStream(uint8_t* cursor, size_t size) {
	this->size = size;
	this->cursor = cursor;
	this->position = 0;
}

ByteStream::ByteStream(ByteStream& stream) {
	this->cursor = stream.cursor;
	this->size = stream.size;
	this->position = stream.position;
}
ByteStream::ByteStream(const ByteStream& stream) {
    this->cursor   = stream.cursor;
    this->size     = stream.size;
    this->position = stream.position;
}

ByteStream& ByteStream::operator=(const ByteStream& other) {
    if (this == &other) return *this;
    this->cursor   = other.cursor;
    this->size     = other.size;
    this->position = other.position;
    return *this;
}

ByteStream::ByteStream() {
	this->cursor = nullptr;
}

ByteStream::~ByteStream() {
	this->cursor = nullptr;
}
void ByteStream::dump(size_t lenght, int hexonly) {
	uint8_t byte;
	int cl = 0;
	for (int read = 0; read < lenght; read++) {
		byte = this->ReadByte();
		if (byte >= 40 && byte <= 90 && !hexonly) {
			printf("[%c]", char(byte));
		}
		else if (byte >= 97 && byte <= 122 && !hexonly) {
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
void ByteStream::Set(uint8_t *cursor, size_t size) { this->cursor = cursor; this->size = size; this->position = 0; }
std::string ByteStream::ReadString(size_t lenght) {
	std::string str;
	if (this->position >= this->size) {
		return str;
	}
	if (lenght == 0) {
		return str;
	}
	if (lenght > this->size - this->position) {
		lenght = this->size - this->position;
	}
	uint8_t c = ReadByte();
	for (size_t i = 1; i < lenght; i++) {
		if (c != 0) {
			str += c;
		}
		c = ReadByte();
	}
	if (c != 0) {
		str += c;
	}
	return str;
}
std::string ByteStream::ReadStringNoSize(size_t maxLenght) {
	std::string str;
	if (this->position >= this->size) {
		return str;
	}
	if (maxLenght == 0) {
		return str;
	}
	if (maxLenght > this->size - this->position) {
		maxLenght = this->size - this->position;
	}
	uint8_t c = ReadByte();
	size_t i = 1;
	for (i = 1; i <= maxLenght && c != 0; i++) {
		str += c;
		c = ReadByte();
	}
	return str;
}
void ByteStream::MoveForward(size_t bytes) {
	if (this->position >= this->size) {
		return;
	}
	if (bytes > this->size - this->position) {
		bytes = this->size - this->position - 1;
	}
	this->cursor += bytes;
	this->position += bytes;
}
void ByteStream::MoveBackward(int bytes) {
	if (this->position == 0) {
		return;
	}
	if ((int)this->position - bytes < 0) {
		this->cursor = this->cursor - this->position;
		this->position = 0;
		return;
	}
	this->cursor -= bytes;
	this->position -= bytes;
}

uint8_t ByteStream::ReadByte(void) { 
	if (this->position > this->size) {
		printf("ByteStream: Attempt to read past end of stream (%zu >= %zu)\n", this->position, this->size);
		return 0;
	}
	if (this->position == this->size) {
		return *this->cursor;
	}
	this->position++;
	return *this->cursor++; 
}

uint8_t ByteStream::PeekByte(void) { 
	return (this->PeekByte(1));
}
uint8_t ByteStream::PeekByte(int offset) { 
	if (this->position+offset > this->size) {
		printf("ByteStream: Attempt to peek past end of stream (%zu >= %zu)\n", this->position, this->size);
		return 0;
	}
	return *(this->cursor + offset);
}
uint16_t ByteStream::PeekUShort(void) { 
	if (this->position+1 > this->size) {
		printf("ByteStream: Attempt to peek past end of stream (%zu >= %zu)\n", this->position, this->size);
		return 0;
	}
	uint16_t *ushortP = (uint16_t *)this->cursor;
	return *ushortP;
}
uint8_t ByteStream::CurrentByte(void) { 
	return (this->PeekByte(0));
}
uint16_t ByteStream::ReadUShort(void) {
	if (this->position + 1 >= this->size) {
		printf("ByteStream: Attempt to read ushort past end of stream (%zu >= %zu)\n", this->position + 1, this->size);
		return 0;
	}
	this->position += 2;
	uint16_t *ushortP = (uint16_t *)this->cursor;
	this->cursor += 2;
	return *ushortP;
}


int16_t ByteStream::ReadShort(void) {
	if (this->position + 1 >= this->size) {
		printf("ByteStream: Attempt to read short past end of stream (%zu >= %zu)\n", this->position + 1, this->size);
		return 0;
	}
	this->position += 2;
	int16_t *shortP = (int16_t *)this->cursor;
	this->cursor += 2;
	return *shortP;
}

uint8_t *ByteStream::GetPosition(void) { return this->cursor; }

uint32_t ByteStream::ReadUInt32LE(void) {
	if (this->position + 3 >= this->size) {
		printf("ByteStream: Attempt to read uint32 past end of stream (%zu >= %zu)\n", this->position + 3, this->size);
		return 0;
	}
	this->position += 4;
	uint32_t *i = (uint32_t *)cursor;
	cursor += 4;
	return *i;
}

int32_t ByteStream::ReadInt32LE(void) {
	if (this->position + 3 >= this->size) {
		printf("ByteStream: Attempt to read int32 past end of stream (%zu >= %zu)\n", this->position + 3, this->size);
		return 0;
	}
	this->position += 4;
	int32_t *i = (int32_t *)cursor;
	cursor += 4;
	return *i;
}
float ByteStream::ReadFixedFloatLE(void) {
	int32_t integer_value = this->ReadInt32LE();
	float float_value = (integer_value >> 8) + static_cast<float>(integer_value & 0x000000FF) / 255.0f;
	return float_value;
}
float ByteStream::ReadFixedFloatBE(void) {
	uint32_t integer_value = this->ReadUInt32BE();
	float float_value = (integer_value >> 8) + static_cast<float>(integer_value & 0x000000FF) / 255.0f;
	return float_value;
}
float ByteStream::ReadFixedFloat16LE(void) {
	uint16_t integer_value = this->ReadUShort();
	float float_value = (integer_value >> 8) + static_cast<float>(integer_value & 0x00FF) / 255.0f;
	return float_value;
}
int32_t ByteStream::ReadInt24LE(void) {
	int32_t i = 0;
	uint8_t buffer[4];
	if (this->position + 3 > this->size) {
		printf("ByteStream: Attempt to read int24 past end of stream (%zu >= %zu)\n", this->position + 2, this->size);
		return 0;
	}
	this->position += 4;
	buffer[0] = *(cursor++);
	buffer[1] = *(cursor++);
	buffer[2] = *(cursor++);
	buffer[3] = *(cursor++);
	i = (buffer[2] << 16) | (buffer[1] << 8) | (buffer[0] << 0);
	if (buffer[2] & 0x80) {
		i = (0xff << 24) | i;
	}
	return i;
}

int32_t ByteStream::ReadInt24LEByte3(void) {
	int32_t i = 0;
	uint8_t buffer[3];
	if (this->position + 2 >= this->size) {
		printf("ByteStream: Attempt to read int24 past end of stream (%zu >= %zu)\n", this->position + 2, this->size);
		return 0;
	}
	this->position += 3;
	buffer[0] = *(cursor++);
	buffer[1] = *(cursor++);
	buffer[2] = *(cursor++);
	i = (buffer[2] << 16) | (buffer[1] << 8) | (buffer[0] << 0);
	if (buffer[2] & 0x80) {
		i = (0xff << 24) | i;
	}
	return i;
}

uint32_t ByteStream::ReadUInt32BE(void) {

	uint32_t toLittleEndian = 0;
	if (this->position + 3 >= this->size) {
		printf("ByteStream: Attempt to read uint32 BE past end of stream (%zu >= %zu)\n", this->position + 3, this->size);
		return 0;
	}
	this->position += 4;
	toLittleEndian |= *(cursor++) << 24;
	toLittleEndian |= *(cursor++) << 16;
	toLittleEndian |= *(cursor++) << 8;
	toLittleEndian |= *(cursor++) << 0;

	return toLittleEndian;
}

uint16_t ByteStream::ReadUShortBE(void) {
	if (this->position + 1 >= this->size) {
		printf("ByteStream: Attempt to read ushort BE past end of stream (%zu >= %zu)\n", this->position + 1, this->size);
		return 0;
	}
	this->position += 2;
	uint16_t toLittleEndian = 0;
	toLittleEndian |= *(cursor++) << 8;
	toLittleEndian |= *(cursor++) << 0;

	return toLittleEndian;
}

std::vector<uint8_t> ByteStream::ReadBytes(size_t count) {
	if (count == 0) {
		return std::vector<uint8_t>();
	}
	if (this->position >= this->size) {
		printf("ByteStream: Attempt to read bytes past end of stream (%zu >= %zu)\n", this->position, this->size);
		return std::vector<uint8_t>();
	}
	
	if (count > this->size - this->position) {
		count = this->size - this->position;
	}
	std::vector<uint8_t> bytes;
	bytes.reserve(count);
	for (size_t i = 0; i < count; i++) {
		bytes.push_back(ReadByte());
	}
	return bytes;
}
void ByteStream::ReadBytes(uint8_t *buffer, size_t count) {
	if (this->position > this->size) {
		printf("ByteStream: Attempt to read bytes past end of stream (%zu >= %zu)\n", this->position, this->size);
		return;
	}
	if (count == 0) {
		return;
	}
	if (count > this->size - this->position) {
		printf("ByteStream: Attempt to read bytes past end of stream (%zu >= %zu)\n", this->position + count - 1, this->size);
		count = this->size - this->position;
	}
	this->position += count;
	memcpy(buffer, this->cursor, count);
	this->cursor += count;
}