#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
}
#endif

#include "IffLexer.h"

#define BYTE unsigned char
#define WORD unsigned int

struct AREA_TypeC
{
	BYTE AreaType; // offset 0, C char
	char AreaName[33]; // off 1-33, text or 0x2E
	long XAxis; // off 34-37, X pos of object
	long YAxis; // off 38-41, Y pos of object
	long ZAxis; // off 42-45, Z pos of object
	WORD AreaWidth; // off 46-47
	WORD Blank0; // off 48-49
	WORD AreaHeight; // off 50-51
	BYTE Blank1; // off 52
};

struct AREA_TypeB
{
	BYTE AreaType; // offset 0, C char
	char AreaName[33]; // off 1-33, text or 0x2E
	long XAxis; // off 34-37, X pos of object
	long YAxis; // off 38-41, Y pos of object
	long ZAxis; // off 42-45, Z pos of object
	WORD AreaWidth; // off 46-47
	BYTE Blank0[10]; // off 48-59
	WORD AreaHeight; // off 60-61
	BYTE Unknown[5]; // off 62-67
};

struct AREA_TypeS
{
	BYTE AreaType; // offset 0, S char
	char AreaName[33]; // off 1-33, text or 0x2E
	long XAxis; // off 34-37, X pos of object
	long YAxis; // off 38-41, Y pos of object
	long ZAxis; // off 42-45, Z pos of object
	WORD AreaWidth; // off 46-47
	BYTE Blank0; // off 48
};

struct PART
{
	WORD MemberNumber; // off 0-1, Cast Member Number
	char MemberName[16]; // 2-17, IFF File Record name
	char WeaponLoad[8]; // 18-25, IFF File Record name
	WORD Unknown0; // 26-27
	WORD Unknown1; // 28-29
	long XAxisRelative; // 30-33, X pos rel to AREA
	long YAxisRelative; // 34-37, Y pos rel to AREA
	WORD ZAxisRelative; // 38-39, Z pos rel to AREA
	BYTE Controls[22]; // 40-61, various control bytes
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

	void InitFromIFF(IffLexer* lexer);
private:
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
};