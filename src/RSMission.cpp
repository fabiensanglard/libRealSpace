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
void printArea(AREA* a) {
	printf("NEW AREA :%s\n", a->AreaName);
	printf("TYPE : %c\n", a->AreaType);
	printf("AT : [%d,%d,%d]\n", a->XAxis, a->YAxis, a->ZAxis);
	printf("SIZE : {%d,%d}\n", a->AreaWidth, a->AreaHeight);
}

void printPart(PART p) {
	printf("NEW PART :%s\n", p.MemberName);
	printf("NUMBER : %d\n", p.MemberNumber);
	printf("LOAD: %s\n", p.WeaponLoad);
	printf("AT : [%d,%d,%d]\n", p.XAxisRelative, p.YAxisRelative, p.ZAxisRelative);
	printf("U0 = %d\n", p.Unknown0);
	printf("U1 = %d\n", p.Unknown1);
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
	
	ByteStream stream(chunk->data);
	size_t fsize = chunk->size;
	
	uint8_t buffer;
	while (fsize >0) {
		buffer = stream.ReadByte();
		AREA area;
		switch (buffer) {
		case 'S':
			area.AreaType = 'S';
			for (int k = 0; k < 33; k++) {
				area.AreaName[k] = stream.ReadByte();
			}
			area.XAxis = stream.ReadInt24LE();
			area.YAxis = stream.ReadInt24LE();
			area.ZAxis = stream.ReadInt24LE();
			area.AreaWidth = stream.ReadUShort();
			stream.ReadByte();
			fsize -= 49;
			break;
		case 'C':
			area.AreaType = 'C';
			for (int k = 0; k < 33; k++) {
				area.AreaName[k] = stream.ReadByte();
			}
			area.XAxis = stream.ReadInt24LE();
			area.YAxis = stream.ReadInt24LE();
			area.ZAxis = stream.ReadInt24LE();
			area.AreaWidth = stream.ReadUShort();

			//unsigned int Blank0; // off 48-49
			stream.ReadByte();
			stream.ReadByte();
			area.AreaHeight = stream.ReadUShort();
			//unsigned char Blank1; // off 52
			stream.ReadByte();
			fsize -= 52;
			break;
		case 'B':
			area.AreaType = 'B';
			for (int k = 0; k < 33; k++) {
				area.AreaName[k] = stream.ReadByte();
			}
			area.XAxis = stream.ReadInt24LE();
			area.YAxis = stream.ReadInt24LE();
			area.ZAxis = stream.ReadInt24LE();
			area.AreaWidth = stream.ReadUShort();

			//unsigned char Blank0[10]; // off 48-59
			for (int k = 0; k < 10; k++) {
				stream.ReadByte();
			}
			//unsigned int AreaHeight; // off 60-61
			area.AreaHeight = stream.ReadUShort();

			//unsigned char Unknown[5]; // off 62-67
			for (int k = 0; k < 5; k++) {
				stream.ReadByte();
			}

			fsize -= 67;
			break;
		}
 		printArea(&area);
	}
	

}

void RSMission::parsePART(IffChunk* chunk) {
	if (chunk == NULL) {
		return ;
	}
	printf("PARSING PART\n");
	printf("Number of entries %d\n", chunk->size / 62);
	ByteStream stream(chunk->data);
	size_t numParts = chunk->size / 62;
	for (int i = 0; i < numParts; i++) {
		PART prt;
		prt.MemberNumber = 0;
		prt.MemberNumber |= stream.ReadByte() << 0;
		prt.MemberNumber |= stream.ReadByte() << 8;
		for (int k = 0; k < 16; k++) {
			prt.MemberName[k] = stream.ReadByte();
		}
		for (int k = 0; k < 8; k++) {
			prt.WeaponLoad[k] = stream.ReadByte();
		}
		prt.Unknown0 = 0;
		prt.Unknown0 |= stream.ReadByte() << 0;
		prt.Unknown0 |= stream.ReadByte() << 8;

		prt.Unknown1 = 0;
		prt.Unknown1 |= stream.ReadByte() << 0;
		prt.Unknown1 |= stream.ReadByte() << 8;

		prt.XAxisRelative = stream.ReadInt24LE();
		prt.YAxisRelative = stream.ReadInt24LE();
		prt.ZAxisRelative = 0;
		prt.ZAxisRelative |= stream.ReadByte() << 0;
		prt.ZAxisRelative |= stream.ReadByte() << 8;

		for (int k = 0; k < 22; k++) {
			prt.Controls[k] = stream.ReadByte();
		}
		printPart(prt);
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