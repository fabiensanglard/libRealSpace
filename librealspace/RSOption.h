#pragma once


#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <map>


struct QUAD {
	uint8_t xa1;
	uint8_t ya1;
	uint8_t xa2;
	uint8_t ya2;
	uint8_t xb1;
	uint8_t yb1;
	uint8_t xb2;
	uint8_t yb2;
};
struct INFO {
	uint8_t ID;
	uint8_t UNKOWN;
};
struct COLR {
	uint8_t UNKOWN_1;
	uint8_t UNKOWN_2;
};
struct TUNE {
	uint8_t ID;
	uint8_t UNKOWN;
};
struct PALT {
	uint8_t ID;
	uint8_t UNKOWN;
};
struct BACK_SHAP {
	uint8_t ID;
	uint8_t UNKOWN_1;
};
struct SPRT_SHAP {
	uint8_t GID;
	uint8_t SHP_ID;
	uint8_t UNKOWN_1;
	uint8_t UNKOWN_2;
};
struct OPTION_RECT {
	uint16_t X1;
	uint16_t Y1;
	uint16_t X2;
	uint16_t Y2;
};
struct SPRT_INFO {
	uint8_t ID;
	uint8_t UNKOWN_1;
	uint8_t UNKOWN_2;
	uint8_t UNKOWN_3;
};
struct SPRT {
	SPRT_SHAP sprite;
	char* label;
	OPTION_RECT zone;
	TUNE tune;
	uint8_t CLCK;
	QUAD* quad;
	SPRT_INFO* info;
	uint8_t* SEQU;
};

struct BACK {
	PALT *palette;
	std::vector<BACK_SHAP*> images;
};
struct FORE {
	PALT *palette;
	std::vector<SPRT*> sprites;
};
struct SCEN {
	INFO infos;
	COLR colr;
	TUNE tune;
	BACK *background;
	FORE *foreground;

};
struct SHPS {
	uint8_t type; // SHAP or MOBL
	uint8_t OptshapeID;
	uint8_t UNKOWN;
	uint8_t x1;
	uint8_t y1;
	uint8_t x2;
	uint8_t y2;
};

struct SHOT {
	INFO infos;
	std::vector<SHPS*> images;
	std::vector<PALT*> palettes;
};
struct KILL {
	uint8_t UNKOWN_1;
	uint8_t UNKOWN_2;
	uint8_t UNKOWN_3;
	uint8_t UNKOWN_4;
};
struct MARK {
	KILL unkown_1;
	uint8_t *DATA;
};
class RSOption {
	std::map<std::uint8_t, SCEN*> opts;
	std::map<std::uint8_t, SHOT*> estb;
	MARK mark;

public:
	RSOption();
	~RSOption();
	void InitFromRam(uint8_t* data, size_t size);
private:
	
	SCEN* tmpscen;
	FORE* tmpfore;
	BACK* tmpback;
	SPRT* tmpsprt;
	SHOT* tmpshot;

	void parseOPTS(uint8_t* data, size_t size);
	void parseSCEN(uint8_t* data, size_t size);
	void parseSCEN_INFO(uint8_t* data, size_t size);
	void parseINFO(uint8_t* data, size_t size);
	void parseCOLR(uint8_t* data, size_t size);
	void parseTUNE(uint8_t* data, size_t size);
	void parseSPRT_TUNE(uint8_t* data, size_t size);
	void parseBACK(uint8_t* data, size_t size);
	void parseBACK_PALT(uint8_t* data, size_t size);
	void parseBACK_SHAP(uint8_t* data, size_t size);
	void parsePALT(uint8_t* data, size_t size);
	void parseSHAP(uint8_t* data, size_t size);
	void parseFORE(uint8_t* data, size_t size);
	void parseSPRT(uint8_t* data, size_t size);
	void parseCLCK(uint8_t* data, size_t size);
	void parseQUAD(uint8_t* data, size_t size);
	void parseSPRT_INFO(uint8_t* data, size_t size);
	void parseSPRT_SEQU(uint8_t* data, size_t size);
	void parseRECT(uint8_t* data, size_t size);
	void parseLABL(uint8_t* data, size_t size);
	void parseETSB(uint8_t* data, size_t size);
	void parseSHOT_INFO(uint8_t* data, size_t size);
	void parseSHOT_SHPS(uint8_t* data, size_t size);
	void parseSHOT_SHAP(uint8_t* data, size_t size);
	void parseSHOT_MOBL(uint8_t* data, size_t size);
	void parseSHOT_PALT(uint8_t* data, size_t size);
	void parseSHOT_PALS(uint8_t* data, size_t size);
	void parseSHOT(uint8_t* data, size_t size);
	void parseMARK(uint8_t* data, size_t size);

};
