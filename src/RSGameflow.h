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
struct EFCT {
	uint8_t opcode;
	uint8_t value;
};

struct GAMEFLOW_SPRT {
	INFO info;
	std::vector<EFCT *>* efct;
	std::vector<REQU *>* requ;
};
struct GAMEFLOW_SCEN {
	INFO info;
	std::vector<GAMEFLOW_SPRT*> sprt;
	REQU *requ;
	std::vector<uint8_t> weap;
};

struct MISS {
	INFO info;
	std::vector<EFCT *>* efct;
	std::vector<GAMEFLOW_SCEN*> scen;
};
struct SPED {
	std::vector<uint8_t> unkown;
};
struct MAP_POINT {
	int x;
	int y;
	std::string label;
};
struct MAP_DATA {
	std::vector<uint8_t> unkown;
	std::vector<MAP_POINT*> points;
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
struct CHNG_CASH {
	uint8_t op;
	uint32_t value;
};
struct PILT_CHANGE {
	uint8_t op;
	uint8_t air;
	uint8_t pilot_id;
	uint8_t value;
};
struct WEAP_CHANGE {
	uint8_t op;
	uint8_t weap_id;
	uint16_t value;
};

struct CHNG {
	INFO info;
	std::vector<PILT_CHANGE *> *pilt;
	CHNG_CASH *cash;
	std::vector<WEAP_CHANGE *> *weap;
	CHNG_CASH *over;
};


struct GAMEFLOW {
	std::map<std::uint8_t, MISS*> game;
	std::map<std::uint8_t, MAP*> wrld;
	std::vector<LOAD*> load;
	MLST* mlst;
	std::vector<WING*> wngs;
	std::map<std::uint8_t, CHNG*> stat;
};

/**
 * @class RSGameFlow
 * @brief Class that represents a RealSpace gameflow
 *
 * This class represents a RealSpace gameflow and is used to parse the data
 * from the RealSpace game. The data is parsed from a blob of memory and the
 * resulting data is stored in this class.
 *
 * @warning This class is still in development and is not fully tested.
 *
 * @see RSGameFlow.cpp
 */
class RSGameFlow {

private:

	/**
	 * Temporary storage for the current mission
	 */
	MISS* tmpmiss;

	/**
	 * Temporary storage for the current map
	 */
	MAP* tmpmap;

	/**
	 * Temporary storage for the mission list
	 */
	MLST* tmpmisslt;

	/**
	 * Temporary storage for the current wing
	 */
	WING* tmpwings;

	/**
	 * Temporary storage for the current change
	 */
	CHNG* tmpstat;

	/**
	 * Temporary storage for the current gameflow scene
	 */
	GAMEFLOW_SCEN* tmpgfsc;

	/**
	 * Temporary storage for the current gameflow sprite
	 */
	GAMEFLOW_SPRT* tmpscsp;

	/**
	 * Parses the game data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseGAME(uint8_t* data, size_t size);

	/**
	 * Parses the mission data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS(uint8_t* data, size_t size);

	/**
	 * Parses the mission info data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_INFO(uint8_t* data, size_t size);

	/**
	 * Parses the mission effects data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_EFCT(uint8_t* data, size_t size);

	/**
	 * Parses the mission scene data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_SCEN(uint8_t* data, size_t size);

	/**
	 * Parses the mission scene info data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_SCEN_INFO(uint8_t* data, size_t size);

	/**
	 * Parses the mission scene sprite data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_SCEN_SPRT(uint8_t* data, size_t size);

	/**
	 * Parses the mission scene sprite info data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_SCEN_SPRT_INFO(uint8_t* data, size_t size);

	/**
	 * Parses the mission scene sprite effects data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_SCEN_SPRT_EFCT(uint8_t* data, size_t size);

	/**
	 * Parses the mission scene sprite requirements data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMISS_SCEN_SPRT_REQU(uint8_t* data, size_t size);

	/**
	 * Parses the world data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWRLD(uint8_t* data, size_t size);

	/**
	 * Parses the world map data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWRLD_MAPS(uint8_t* data, size_t size);

	/**
	 * Parses the world map info data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWRLD_MAPS_INFO(uint8_t* data, size_t size);

	/**
	 * Parses the world map speed data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWRLD_MAPS_SPED(uint8_t* data, size_t size);

	/**
	 * Parses the world map data data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWRLD_MAPS_DATA(uint8_t* data, size_t size);

	/**
	 * Parses the load data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseLOAD(uint8_t* data, size_t size);

	/**
	 * Parses the load load data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseLOAD_LOAD(uint8_t* data, size_t size);

	/**
	 * Parses the mission list data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMLST(uint8_t* data, size_t size);

	/**
	 * Parses the mission list data data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMLST_DATA(uint8_t* data, size_t size);

	/**
	 * Parses the mission list part data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseMLST_PRTL(uint8_t* data, size_t size);

	/**
	 * Parses the wing data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWNGS(uint8_t* data, size_t size);

	/**
	 * Parses the wing wing data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWNGS_WING(uint8_t* data, size_t size);

	/**
	 * Parses the wing wing info data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWNGS_WING_INFO(uint8_t* data, size_t size);

	/**
	 * Parses the wing wing pilot data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseWNGS_WING_PILT(uint8_t* data, size_t size);

	/**
	 * Parses the stat data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseSTAT(uint8_t* data, size_t size);

	/**
	 * Parses the stat change data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseSTAT_CHNG(uint8_t* data, size_t size);

	/**
	 * Parses the stat change info data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseSTAT_CHNG_INFO(uint8_t* data, size_t size);

	/**
	 * Parses the stat change pilot data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseSTAT_CHNG_PILT(uint8_t* data, size_t size);

	/**
	 * Parses the stat change cash data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseSTAT_CHNG_CASH(uint8_t* data, size_t size);

	/**
	 * Parses the stat change over data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseSTAT_CHNG_OVER(uint8_t* data, size_t size);

	/**
	 * Parses the stat change weapon data
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void parseSTAT_CHNG_WEAP(uint8_t* data, size_t size);

public:

	/**
	 * The game data
	 */
	GAMEFLOW game;

	/**
	 * Constructor
	 */
	RSGameFlow();

	/**
	 * Destructor
	 */
	~RSGameFlow();

	/**
	 * Initializes the gameflow from a blob of memory
	 *
	 * @param data The data to parse
	 * @param size The size of the data
	 */
	void InitFromRam(uint8_t* data, size_t size);

};