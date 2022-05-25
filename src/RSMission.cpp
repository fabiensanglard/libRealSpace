#include "RSMission.h"
#include "PakArchive.h"

void printChunk(IffChunk* chunk, const char *name) {
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
		} else if (byte >= 97 && byte <= 122) {
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

void printBytes(ByteStream *stream, int lenght) {
	uint8_t byte;
	int cl = 0;
	for (int k = 0; k < lenght; k++) {
		byte = stream->ReadByte();
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
}

RSMission::RSMission() {
	printf("CREATE MISSION\n");
}

RSMission::~RSMission() {
	printf("DELETE MISSION\n");
}

void RSMission::InitFromIFF(IffLexer* lexer) {
	IffChunk* chunk;
	printf("PARSING MISSION DATA\n");

	chunk = lexer->GetChunkByID('WRLD');
	parseWORLD(chunk);

	chunk = lexer->GetChunkByID('INFO');
	parseINFO(chunk);

	chunk = lexer->GetChunkByID('PALT');
	parsePALT(chunk);

	chunk = lexer->GetChunkByID('TERA');
	parseTERA(chunk);

	chunk = lexer->GetChunkByID('SKYS');
	parseSKYS(chunk);

	chunk = lexer->GetChunkByID('GLNT');
	parseGLNT(chunk);

	chunk = lexer->GetChunkByID('SMOK');
	parseSMOK(chunk);

	chunk = lexer->GetChunkByID('LGHT');
	parseLGHT(chunk);

	chunk = lexer->GetChunkByID('VERS');
	printChunk(chunk, "VERS");

	chunk = lexer->GetChunkByID('NAME');
	printChunk(chunk, "NAME");

	chunk = lexer->GetChunkByID('FILE');
	printChunk(chunk, "FILE");

	chunk = lexer->GetChunkByID('AREA');
	parseAREA(chunk);

	chunk = lexer->GetChunkByID('SPOT');
	parseSPOT(chunk);

	chunk = lexer->GetChunkByID('NUMS');
	printChunk(chunk, "SPOT");

	chunk = lexer->GetChunkByID('MSGS');
	printChunk(chunk, "MSGS");
	
	chunk = lexer->GetChunkByID('FLAG');
	printChunk(chunk, "FLAG");

	chunk = lexer->GetChunkByID('CAST');
	printChunk(chunk, "CAST");

	chunk = lexer->GetChunkByID('PROG');
	parsePROG(chunk);
	
	chunk = lexer->GetChunkByID('PART');
	parsePART(chunk);

	chunk = lexer->GetChunkByID('TEAM');
	printChunk(chunk, "TEAM");

	chunk = lexer->GetChunkByID('LOAD');
	printChunk(chunk, "LOAD");

	chunk = lexer->GetChunkByID('SCNE');
	printChunk(chunk, "SCNE");
	
	
}



void RSMission::parseWORLD(IffChunk* chunk) {
	printChunk(chunk, "WRDL");
}

void RSMission::parseINFO(IffChunk* chunk) {
	printChunk(chunk, "INFO");
}

void RSMission::parsePALT(IffChunk* chunk) {
	printChunk(chunk, "PALT");
}

void RSMission::parseTERA(IffChunk* chunk) {
	printChunk(chunk, "TERA");
}

void RSMission::parseSKYS(IffChunk* chunk) {
	printChunk(chunk, "SKYS");
}

void RSMission::parseAREA(IffChunk* chunk) {
	printf("PARSING AREA\n");
	printChunk(chunk, "AREA");
}

void RSMission::parsePART(IffChunk* chunk) {
	printf("PARSING PART\n");
	printf("Number of entries %d\n", chunk->size / 62);
	ByteStream stream(chunk->data);
	size_t numParts = chunk->size / 62;
	for (int i = 0; i < numParts; i++) {
		short id = stream.ReadShort();
		char objName[9];
		for (int k = 0; k < 8; k++)
			objName[k] = stream.ReadByte();
		objName[8] = 0;
		uint8_t byte;
		printf("LOADIND obj id[%d], name [%s]\n", id, objName);
		printf("DATA:\n");
		int cl = 0;
		printf("UNKOWN :\n");
		printBytes(&stream, 8);
		printf("LOAD \n:");
		printBytes(&stream, 8);

		printf("UNKOWN :\n");
		printBytes(&stream, 2);

		int32_t i1, i2, i3, i4;
		int32_t px, py, pz, pu;

		int32_t coo[12];
		coo[0] = stream.ReadByte();
		coo[1] = stream.ReadByte();
		i1 = ((coo[1] << 8) | coo[0]);
		coo[2] = stream.ReadByte();
		coo[3] = stream.ReadByte();
		px = ((coo[3] << 8) | coo[2]);


		coo[4] = stream.ReadByte();
		coo[5] = stream.ReadByte();
		i2 = ((coo[5] << 8) | coo[4]);
		coo[6] = stream.ReadByte();
		coo[7] = stream.ReadByte();
		pz = ((coo[7] << 8) | coo[6]);


		coo[8] = stream.ReadByte();
		coo[9] = stream.ReadByte();
		i3 = ((coo[9] << 8) | coo[8]);
		coo[10] = stream.ReadByte();
		coo[11] = stream.ReadByte();
		py = ((coo[11] << 8) | coo[10]);


		printf("\nCOORD{%d,%d,%d} | SIDE [%d,%d,%d]\n", px, py, pz, i1, i2, i3);

		printf("UNKOWN : \n");
		printBytes(&stream, 22);
		printf("\nEND\n");
	}
}
void RSMission::parseGLNT(IffChunk* chunk) {
	printChunk(chunk, "GLNT");
}

void RSMission::parseSMOK(IffChunk* chunk) {
	printChunk(chunk, "SMOK");
}

void RSMission::parseLGHT(IffChunk* chunk) {
	printChunk(chunk, "LGHT");
}

void RSMission::parseSPOT(IffChunk* chunk) {
	printf("PARSING SPOT\n");
	if (chunk != NULL) {
		PakArchive p;
		p.InitFromRAM("SPOT.PAK", chunk->data, chunk->size);
		p.List(stdout);
	}
	printChunk(chunk, "SPOT");
}

void RSMission::parsePROG(IffChunk* chunk) {
	printf("PARSING PROG\n");
	if (chunk != NULL) {
		PakArchive p;
		p.InitFromRAM("PROG.PAK", chunk->data, chunk->size);
		p.List(stdout);
	}
	printChunk(chunk, "PROG");
}