#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <string>
#include <map>

struct INFO {
	uint8_t ID;
	uint8_t UNKOWN;
};
struct REQU {
	uint8_t op;
	uint8_t value;
};
struct GAMEFLOW_SPRT {
	INFO info;
	std::vector<uint8_t> efct;
	std::vector<REQU *>*  requ;
};
struct GAMEFLOW_SCEN {
	INFO info;
	std::vector<GAMEFLOW_SPRT*> sprt;
	REQU *requ;
	std::vector<uint8_t> weap;
};

struct MISS {
	INFO info;
	std::vector<uint8_t> efct;
	std::vector<GAMEFLOW_SCEN*> scen;
};
struct SPED {
	std::vector<uint8_t> unkown;
};
struct MAP_DATA {
	std::vector<uint8_t> unkown;
};
struct MAP {
	INFO info;
	SPED *sped;
	MAP_DATA *data;
};
struct LOAD {
	std::vector<uint8_t> load;
};
struct MLST {
	std::vector<std::string *> data;
	std::vector<std::string *> prtl;
};
struct WING {
	INFO info;
	std::vector<uint8_t> pilt;
};
struct CHNG {
	INFO info;
	std::vector<uint8_t> *pilt;
	std::vector<uint8_t> *cash;
	std::vector<uint8_t> *weap;
	std::vector<uint8_t>* over;
};
struct GAMEFLOW {
	std::map<std::uint8_t, MISS*> game;
	std::map<std::uint8_t, MAP*> wrld;
	std::vector<LOAD*> load;
	MLST* mlst;
	std::vector<WING*> wngs;
	std::map<std::uint8_t, CHNG*> stat;
};

class RSGameFlow {

private:
	
	MISS* tmpmiss;
	MAP* tmpmap;
	MLST* tmpmisslt;
	WING* tmpwings;
	CHNG* tmpstat;
	GAMEFLOW_SCEN* tmpgfsc;
	GAMEFLOW_SPRT* tmpscsp;

	void parseGAME(uint8_t* data, size_t size);
	void parseMISS(uint8_t* data, size_t size);
	void parseMISS_INFO(uint8_t* data, size_t size);
	void parseMISS_EFCT(uint8_t* data, size_t size);
	void parseMISS_SCEN(uint8_t* data, size_t size);
	void parseMISS_SCEN_INFO(uint8_t* data, size_t size);
	void parseMISS_SCEN_SPRT(uint8_t* data, size_t size);
	void parseMISS_SCEN_SPRT_INFO(uint8_t* data, size_t size);
	void parseMISS_SCEN_SPRT_EFCT(uint8_t* data, size_t size);
	void parseMISS_SCEN_SPRT_REQU(uint8_t* data, size_t size);
	void parseWRLD(uint8_t* data, size_t size);
	void parseWRLD_MAPS(uint8_t* data, size_t size);
	void parseWRLD_MAPS_INFO(uint8_t* data, size_t size);
	void parseWRLD_MAPS_SPED(uint8_t* data, size_t size);
	void parseWRLD_MAPS_DATA(uint8_t* data, size_t size);
	void parseLOAD(uint8_t* data, size_t size);
	void parseLOAD_LOAD(uint8_t* data, size_t size);
	void parseMLST(uint8_t* data, size_t size);
	void parseMLST_DATA(uint8_t* data, size_t size);
	void parseMLST_PRTL(uint8_t* data, size_t size);
	void parseWNGS(uint8_t* data, size_t size);
	void parseWNGS_WING(uint8_t* data, size_t size);
	void parseWNGS_WING_INFO(uint8_t* data, size_t size);
	void parseWNGS_WING_PILT(uint8_t* data, size_t size);
	void parseSTAT(uint8_t* data, size_t size);
	void parseSTAT_CHNG(uint8_t* data, size_t size);
	void parseSTAT_CHNG_INFO(uint8_t* data, size_t size);
	void parseSTAT_CHNG_PILT(uint8_t* data, size_t size);
	void parseSTAT_CHNG_CASH(uint8_t* data, size_t size);
	void parseSTAT_CHNG_OVER(uint8_t* data, size_t size);
	void parseSTAT_CHNG_WEAP(uint8_t* data, size_t size);

public:
	GAMEFLOW game;

	RSGameFlow();
	~RSGameFlow();
	void InitFromRam(uint8_t* data, size_t size);
	
};