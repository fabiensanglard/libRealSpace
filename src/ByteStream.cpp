//
//  Stream.cpp
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "ByteStream.h"


ByteStream::ByteStream(uint8_t* cursor) {
	this->cursor = cursor;
}

ByteStream::ByteStream(ByteStream& stream) {
	this->cursor = stream.cursor;
}

ByteStream::ByteStream() :cursor(NULL)
{

}

ByteStream::~ByteStream() {

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