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