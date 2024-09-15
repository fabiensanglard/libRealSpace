#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
}
#endif

#include <stdint.h>
#include <string>
#include "RSEntity.h"
#include "IffLexer.h"
#include "TreArchive.h"

struct AREA {
	int id;
	unsigned char AreaType; 
	char AreaName[33]; 
	long XAxis; 
	long YAxis; 
	long ZAxis; 
	unsigned int AreaWidth; 
	unsigned int AreaHeight;
	unsigned char Unknown[5];
};
struct AREA_TypeS
{
	unsigned char AreaType; // offset 0, S char
	char AreaName[33]; // off 1-33, text or 0x2E
	long XAxis; // off 34-37, X pos of object
	long YAxis; // off 38-41, Y pos of object
	long ZAxis; // off 42-45, Z pos of object
	unsigned int AreaWidth; // off 46-47
	unsigned char Blank0; // off 48
};

struct AREA_TypeC
{
	unsigned char AreaType; // offset 0, C char
	char AreaName[33]; // off 1-33, text or 0x2E
	long XAxis; // off 34-37, X pos of object
	long YAxis; // off 38-41, Y pos of object
	long ZAxis; // off 42-45, Z pos of object
	unsigned int AreaWidth; // off 46-47
	unsigned int Blank0; // off 48-49
	unsigned int AreaHeight; // off 50-51
	unsigned char Blank1; // off 52
};

struct AREA_TypeB
{
	unsigned char AreaType; // offset 0, C char
	char AreaName[33]; // off 1-33, text or 0x2E
	long XAxis; // off 34-37, X pos of object
	long YAxis; // off 38-41, Y pos of object
	long ZAxis; // off 42-45, Z pos of object
	unsigned int AreaWidth; // off 46-47
	unsigned char Blank0[10]; // off 48-59
	unsigned int AreaHeight; // off 60-61
	unsigned char Unknown[5]; // off 62-67
};

struct PART
{
	unsigned int MemberNumber; // off 0-1, Cast Member Number
	char MemberName[17]; // 2-17, IFF File Record name
	char WeaponLoad[9]; // 18-25, IFF File Record name
	unsigned int Unknown0; // 26-27
	unsigned int Unknown1; // 28-29
	long XAxisRelative; // 30-33, X pos rel to AREA
	long YAxisRelative; // 34-37, Y pos rel to AREA
	unsigned int ZAxisRelative; // 38-39, Z pos rel to AREA
	unsigned char Controls[22]; // 40-61, various control bytes
	uint16_t azymuth;
	RSEntity* entity;

};

struct SPOT {
	int id;
	short unknown;

	long XAxis;
	long YAxis;
	long ZAxis;
};
struct MSGS {
	char message[255];
	int id;
};
struct CAST {
	char actor[9];
	int id;
};
/*

Mission.IFF CHUNK ids

	FORM
	****** WORLD INFO ***
	WRLD
	INFO
	PALT
	TERA
	SKYS
	GLNT
	SMOK
	LGHT
	
	***** CAMERA INFO ****
	CAMR
	STRT
	CHAS
	CKPT
	VICT
	TARG
	WEAP
	ROTA
	COMP

	**** MISSION INFO ****
	MISN
	VERS
	INFO
	TUNE
	NAME
	FILE
	AREA
	SPOT
	NUMS
	MSGS
	FLAG
	CAST
	PROG
	PART
	TEAM
	PLAY
	SCNE
	LOAD
*/
class RSMission {
public:
	RSMission();
	~RSMission();


	inline char* getMissionName() {
		return (missionName);
	}
	inline char* getMissionAreaFile() {
		return (missionAreaFile);
	}
	std::map<std::string, RSEntity*>* objCache;
	TreArchive* tre;
	std::vector<PART*> missionObjects;
	std::vector<AREA*> missionAreas;

	PART* getPlayerCoord();
	PART* getObject(const char *name);
	void InitFromIFF(IffLexer* lexer);
	void PrintMissionInfos();
private:
	void printArea(AREA* a);
	void printPart(PART* p);

	void parseLOAD(IffChunk* chunk);
	void parseCAST(IffChunk* chunk);
	void parseMSGS(IffChunk* chunk);
	void parseFILE(IffChunk* chunk);
	void parseNAME(IffChunk* chunk);
	void parseWORLD(IffChunk* chunk);
	void parseINFO(IffChunk* chunk);
	void parsePALT(IffChunk* chunk);
	void parseTERA(IffChunk* chunk);
	void parseSKYS(IffChunk* chunk);
	void parseGLNT(IffChunk* chunk);
	void parseSMOK(IffChunk* chunk);
	void parseLGHT(IffChunk* chunk);
	void parseAREA(IffChunk* chunk);
	void parsePART(IffChunk* chunk);
	void parseSPOT(IffChunk* chunk);
	void parsePROG(IffChunk* chunk);

	char missionInfo[255];
	char missionName[255];
	char missionAreaFile[9];

	
	
	std::vector<MSGS*> missionMessages;
	std::vector<SPOT*> missionSpots;
	std::vector<CAST*> missionCasting;
};