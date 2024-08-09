#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <map>

struct GAMEFLOW_SPRT {
	INFO info;
	std::vector<uint8_t> efct;
};
struct GAMEFLOW_SCEN {
	INFO info;
	std::map<uint8_t, GAMEFLOW_SPRT*> sprt;
	REQU *requ;
};
struct REQU {
	uint8_t UNKOWN_1;
	uint8_t UNKOWN_2;
};
struct INFO {
	uint8_t ID;
	uint8_t UNKOWN;
};
struct MISS {
	INFO info;
	std::vector<uint8_t> efct;
	std::map<uint8_t, GAMEFLOW_SCEN*> scen;
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
	std::vector<char*> data;
	std::vector<char*> prtl;
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

public:
	RSGameFlow();
	~RSGameFlow();
	void InitFromRam(uint8_t* data, size_t size);
};