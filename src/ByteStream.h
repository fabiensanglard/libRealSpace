//
//  Stream.h
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <array>
#include <vector>


/*

 This is the class used to parse all byte streams.
 It is just a class which encapsulate convenient operations
 and can read byte,short,int as Little or Big Endian.

*/
class ByteStream {
public:

	ByteStream(uint8_t* cursor);
	ByteStream(ByteStream& stream);
	ByteStream();
	~ByteStream();

	void dump(size_t lenght, int hexonly);

	inline void Set(uint8_t* cursor) {
		this->cursor = cursor;
	}
	inline std::string ReadString(void) {
		std::string str;
		while (PeekByte() != 0) {
			str += ReadByte();
		}
		return str;
	}
	inline void MoveForward(size_t bytes) {
		this->cursor += bytes;
	}

	inline uint8_t ReadByte(void) {
		return *this->cursor++;
	}

	inline uint8_t PeekByte(void) {
		return *(this->cursor + 1);
	}

	inline uint16_t ReadUShort(void) {
		uint16_t* ushortP = (uint16_t*)this->cursor;
		this->cursor += 2;
		return *ushortP;
	}

	inline int16_t ReadShort(void) {
		int16_t* shortP = (int16_t*)this->cursor;
		this->cursor += 2;
		return *shortP;
	}

	inline uint8_t* GetPosition(void) {
		return this->cursor;
	}

	inline uint32_t ReadUInt32LE(void) {
		uint32_t* i = (uint32_t*)cursor;
		cursor += 4;
		return *i;
	}

	inline int32_t ReadInt32LE(void) {
		int32_t* i = (int32_t*)cursor;
		cursor += 4;
		return *i;
	}

	inline int32_t ReadInt24LE(void) {
		int32_t i = 0;
		uint8_t buffer[4];
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

	inline uint32_t ReadUInt32BE(void) {

		uint32_t toLittleEndian = 0;
		toLittleEndian |= *(cursor++) << 24;
		toLittleEndian |= *(cursor++) << 16;
		toLittleEndian |= *(cursor++) << 8;
		toLittleEndian |= *(cursor++) << 0;

		return toLittleEndian;
	}

	inline uint16_t ReadUShortBE(void) {

		uint16_t toLittleEndian = 0;
		toLittleEndian |= *(cursor++) << 8;
		toLittleEndian |= *(cursor++) << 0;

		return toLittleEndian;
	}

	inline std::vector<uint8_t> ReadBytes(size_t count) {
		std::vector<uint8_t> bytes;
		bytes.reserve(count);
		for (size_t i = 0; i < count; i++) {
			bytes.push_back(ReadByte());
		}
		return bytes;
	}



private:
	uint8_t* cursor;
};
