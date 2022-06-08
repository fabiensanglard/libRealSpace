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


RSMission::RSMission() {
	printf("CREATE MISSION\n");
}

RSMission::~RSMission() {
	printf("DELETE MISSION\n");
}
void RSMission::printArea(AREA* a) {
	printf("*************************************\n");
	printf("AREA[%d] :%s\n", a->id, a->AreaName);
	printf("TYPE : %c\n", a->AreaType);
	printf("AT : [%d,%d,%d]\n", a->XAxis, a->YAxis, a->ZAxis);
	printf("SIZE : {%d,%d}\n", a->AreaWidth, a->AreaHeight);
	printf("*************************************\n\n");
}

void RSMission::printPart(PART* p) {
	printf("--------------------------------------------------\n");
	printf("NEW PART :%s\n", p->MemberName);
	printf("NUMBER : %d\n", p->MemberNumber);
	printf("LOAD: %s\n", p->WeaponLoad);
	printf("AT : [%d,%d,%d]\n", p->XAxisRelative, p->YAxisRelative, p->ZAxisRelative);
	printf("U0 = %d\n", p->Unknown0);
	printf("U1 = %d\n", p->Unknown1);
	printf("--------------------------------------------------\n\n");
}
void RSMission::printBytes(ByteStream* stream, int lenght) {
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
void RSMission::PrintMissionInfos() {
	printf("MISSION : %s\n", missionName);
	printf("IN AREA : %s\n", missionAreaFile);
	printf("INFOS : %s\n", missionInfo);
	printf("MSGS : \n");
	for (int i = 0; i < missionMessages.size(); i++) {
		printf("\tM[%d]: %s\n",missionMessages[i]->id, missionMessages[i]->message);
	}
	printf("CASTING : \n");
	for (int i = 0; i < missionCasting.size(); i++) {
		printf("\tCAST[%d]: %s\n", missionCasting[i]->id, missionCasting[i]->actor);
	}
	printf("MISSION AREAS OF INTEREST : \n");
	for (int i = 0; i < missionAreas.size(); i++) {
		printArea(missionAreas[i]);
	}

	printf("MISSION OBJECTS / PARTS : \n");
	for (int i = 0; i < missionObjects.size(); i++) {
		printPart(missionObjects[i]);
	}

	printf("MISSION SPOTS : \n");
	for (int i = 0; i < missionSpots.size(); i++) {
		printf("SPOT[%d] at {%d,%d,%d}, UN(%d)\n",
			missionSpots[i]->id,
			missionSpots[i]->XAxis,
			missionSpots[i]->YAxis,
			missionSpots[i]->ZAxis,
			missionSpots[i]->unknown
		);
	}
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
	parseNAME(chunk);

	chunk = lexer->GetChunkByID('FILE');
	parseFILE(chunk);

	chunk = lexer->GetChunkByID('AREA');
	parseAREA(chunk);

	chunk = lexer->GetChunkByID('SPOT');
	parseSPOT(chunk);

	chunk = lexer->GetChunkByID('NUMS');
	printChunk(chunk, "SPOT");

	chunk = lexer->GetChunkByID('MSGS');
	parseMSGS(chunk);
	
	chunk = lexer->GetChunkByID('FLAG');
	printChunk(chunk, "FLAG");

	chunk = lexer->GetChunkByID('CAST');
	parseCAST(chunk);

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
void RSMission::parseLOAD(IffChunk* chunk) {

}
void RSMission::parseCAST(IffChunk* chunk) {
	CAST* cst;
	size_t fsize = chunk->size;
	size_t nbactor = fsize / 9;
	ByteStream stream(chunk->data);
	for (int i = 0; i < nbactor; i++) {
		cst = (CAST*)malloc(sizeof(CAST));
		if (cst != NULL) {
			cst->id = i;
			for (int k = 0; k < 8; k++) {
				cst->actor[k] = stream.ReadByte();
			}
			cst->actor[8] = '\0';
			stream.ReadByte();
			missionCasting.push_back(cst);
		}
	}
}
void RSMission::parseMSGS(IffChunk* chunk) {
	size_t fsize = chunk->size;
	size_t read = 0;
	size_t strc = 0;
	size_t msgc = 0;
	ByteStream stream(chunk->data);
	MSGS* scmsg=NULL;
	byte r;
	while (read < fsize) {
		r = stream.ReadByte();
		read++;
		if (r != '\0') {
			if (scmsg == NULL) {
				scmsg = (MSGS*)malloc(sizeof(MSGS));
				if (scmsg == NULL) {
					read = fsize + 1;
				}
				else {
					strc = 0;
					scmsg->id = msgc++;
				}
			}
			if (scmsg != NULL) {
				scmsg->message[strc++] = r;
			}
		}
		else {
			if (scmsg != NULL) {
				scmsg->message[strc++] = '\0';
				missionMessages.push_back(scmsg);
			}
			scmsg = NULL;
		}
	}
}

void RSMission::parseWORLD(IffChunk* chunk) {
	printChunk(chunk, "WRDL");
}

void RSMission::parseINFO(IffChunk* chunk) {
	size_t fsize = chunk->size;
	ByteStream stream(chunk->data);
	for (int i = 0; i < fsize; i++) {
		missionInfo[i] = stream.ReadByte();
	}
	missionInfo[fsize] = '\0';
}
void RSMission::parseNAME(IffChunk* chunk) {
	if (chunk != NULL) {
		size_t fsize = chunk->size;
		ByteStream stream(chunk->data);
		for (int i = 0; i < fsize; i++) {
			missionName[i] = stream.ReadByte();
		}
		missionName[fsize] = '\0';
	}
}
void RSMission::parseFILE(IffChunk* chunk) {
	size_t fsize = chunk->size;
	ByteStream stream(chunk->data);
	for (int i = 0; i < fsize; i++) {
		missionAreaFile[i] = stream.ReadByte();
	}
	missionAreaFile[fsize] = '\0';
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
	size_t read = 0;
	uint8_t buffer;
	int cpt = 0;
	while (read<fsize) {
		buffer = stream.ReadByte();
		AREA *area;
		area = (AREA *)malloc(sizeof(AREA));
		if (area != NULL) {
			area->id = ++cpt;
			switch (buffer) {
			case 'S':
				area->AreaType = 'S';
				for (int k = 0; k < 33; k++) {
					area->AreaName[k] = stream.ReadByte();
				}
				area->XAxis = stream.ReadInt24LE();
				area->YAxis = stream.ReadInt24LE();
				area->ZAxis = stream.ReadInt24LE();
				area->AreaWidth = stream.ReadUShort();
				stream.ReadByte();
				read += 49;
				break;
			case 'C':
				area->AreaType = 'C';
				for (int k = 0; k < 33; k++) {
					area->AreaName[k] = stream.ReadByte();
				}
				area->XAxis = stream.ReadInt24LE();
				area->YAxis = stream.ReadInt24LE();
				area->ZAxis = stream.ReadInt24LE();
				area->AreaWidth = stream.ReadUShort();

				//unsigned int Blank0; // off 48-49
				stream.ReadByte();
				stream.ReadByte();
				area->AreaHeight = stream.ReadUShort();
				//unsigned char Blank1; // off 52
				stream.ReadByte();
				read += 52;
				break;
			case 'B':
				area->AreaType = 'B';
				for (int k = 0; k < 33; k++) {
					area->AreaName[k] = stream.ReadByte();
				}
				area->XAxis = stream.ReadInt24LE();
				area->YAxis = stream.ReadInt24LE();
				area->ZAxis = stream.ReadInt24LE();
				area->AreaWidth = stream.ReadUShort();

				//unsigned char Blank0[10]; // off 48-59
				for (int k = 0; k < 10; k++) {
					stream.ReadByte();
				}
				//unsigned int AreaHeight; // off 60-61
				area->AreaHeight = stream.ReadUShort();

				//unsigned char Unknown[5]; // off 62-67
				for (int k = 0; k < 5; k++) {
					stream.ReadByte();
				}

				read += 67;
				break;
			}
			missionAreas.push_back(area);
		}
		else {
			read = fsize + 1;
			// exit the loop, memory error
		}
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
		PART *prt;
		prt = (PART *)malloc(sizeof(PART));
		if (prt != NULL) {
			prt->MemberNumber = 0;
			prt->MemberNumber |= stream.ReadByte() << 0;
			prt->MemberNumber |= stream.ReadByte() << 8;
			for (int k = 0; k < 16; k++) {
				prt->MemberName[k] = stream.ReadByte();
			}
			for (int k = 0; k < 8; k++) {
				prt->WeaponLoad[k] = stream.ReadByte();
			}
			prt->Unknown0 = 0;
			prt->Unknown0 |= stream.ReadByte() << 0;
			prt->Unknown0 |= stream.ReadByte() << 8;

			prt->Unknown1 = 0;
			prt->Unknown1 |= stream.ReadByte() << 0;
			prt->Unknown1 |= stream.ReadByte() << 8;

			prt->XAxisRelative = stream.ReadInt24LE();
			prt->YAxisRelative = stream.ReadInt24LE();
			prt->ZAxisRelative = 0;
			prt->ZAxisRelative |= stream.ReadByte() << 0;
			prt->ZAxisRelative |= stream.ReadByte() << 8;

			for (int k = 0; k < 22; k++) {
				prt->Controls[k] = stream.ReadByte();
			}
			missionObjects.push_back(prt);
		}
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
	size_t numParts = chunk->size / 14;
	ByteStream stream(chunk->data);

	for (int i = 0; i < numParts; i++) {
		SPOT* spt;
		spt = (SPOT*)malloc(sizeof(SPOT));
		if (spt != NULL) {
			spt->id = i;
			spt->unknown = 0;
			spt->unknown |= stream.ReadByte() << 0;
			spt->unknown |= stream.ReadByte() << 8;
			
			stream.ReadByte();
			spt->XAxis = stream.ReadInt24LE();
			spt->YAxis = stream.ReadInt24LE();
			spt->ZAxis = stream.ReadShort();
			stream.ReadByte();
			missionSpots.push_back(spt);
		}
	}
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