#include "RSGameflow.h"
#include "RSGameflow.h"
#include "IFFSaxLexer.h"
#include <map>
#include <string>
#include <functional>

RSGameFlow::RSGameFlow() {
}

RSGameFlow::~RSGameFlow() {
}

void RSGameFlow::InitFromRam(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["GAME"] = std::bind(&RSGameFlow::parseGAME, this, std::placeholders::_1, std::placeholders::_2);
	handlers["WRLD"] = std::bind(&RSGameFlow::parseWRLD, this, std::placeholders::_1, std::placeholders::_2);
	handlers["LOAD"] = std::bind(&RSGameFlow::parseLOAD, this, std::placeholders::_1, std::placeholders::_2);
	handlers["MLST"] = std::bind(&RSGameFlow::parseMLST, this, std::placeholders::_1, std::placeholders::_2);
	handlers["WNGS"] = std::bind(&RSGameFlow::parseWNGS, this, std::placeholders::_1, std::placeholders::_2);
	handlers["STAT"] = std::bind(&RSGameFlow::parseSTAT, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseGAME(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["MISS"] = std::bind(&RSGameFlow::parseMISS, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseMISS(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	this->tmpmiss = new MISS();
	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseMISS_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["EFCT"] = std::bind(&RSGameFlow::parseMISS_EFCT, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SCEN"] = std::bind(&RSGameFlow::parseMISS_SCEN, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);

	this->game.game[this->tmpmiss->info.ID] = this->tmpmiss;
}

void RSGameFlow::parseMISS_INFO(uint8_t* data, size_t size) {
	this->tmpmiss->info.ID = *data++;
	this->tmpmiss->info.UNKOWN = *data;
}

void RSGameFlow::parseMISS_EFCT(uint8_t* data, size_t size) {
	this->tmpmiss->efct = new std::vector<EFCT *>();
	for (int i = 0; i < size; i=i+2) {
		EFCT* efct = new EFCT();
		efct->opcode = data[i];
		efct->value = data[i + 1];
		this->tmpmiss->efct->push_back(efct);
	}
}

void RSGameFlow::parseMISS_SCEN(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	this->tmpgfsc = new GAMEFLOW_SCEN();

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseMISS_SCEN_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SPRT"] = std::bind(&RSGameFlow::parseMISS_SCEN_SPRT, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
	this->tmpmiss->scen.push_back(this->tmpgfsc);
}

void RSGameFlow::parseMISS_SCEN_INFO(uint8_t* data, size_t size) {
	this->tmpgfsc->info.ID = *data++;
	this->tmpgfsc->info.UNKOWN = *data;
}

void RSGameFlow::parseMISS_SCEN_SPRT(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	this->tmpscsp = new GAMEFLOW_SPRT();
	this->tmpscsp->efct = nullptr;
	this->tmpscsp->requ = nullptr;
	
	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseMISS_SCEN_SPRT_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["EFCT"] = std::bind(&RSGameFlow::parseMISS_SCEN_SPRT_EFCT, this, std::placeholders::_1, std::placeholders::_2);
	handlers["REQU"] = std::bind(&RSGameFlow::parseMISS_SCEN_SPRT_REQU, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
	this->tmpgfsc->sprt.push_back(this->tmpscsp);
}

void RSGameFlow::parseMISS_SCEN_SPRT_INFO(uint8_t* data, size_t size) {
	this->tmpscsp->info.ID = *data++;
	this->tmpscsp->info.UNKOWN = *data;
}

void RSGameFlow::parseMISS_SCEN_SPRT_EFCT(uint8_t* data, size_t size) {
	this->tmpscsp->efct = new std::vector<EFCT *>();
	for (int i = 0; i < size; i=i+2) {
		EFCT* efct = new EFCT();
		efct->opcode = data[i];
		efct->value = data[i + 1];
		this->tmpscsp->efct->push_back(efct);
	}
}

void RSGameFlow::parseMISS_SCEN_SPRT_REQU(uint8_t* data, size_t size) {
	this->tmpscsp->requ = new std::vector<REQU *>();
	
	for (int i=0; i<size; i+=2) {
		REQU* tmprequ = new REQU();
		tmprequ->op = data[i];
		tmprequ->value = data[i+1];
		this->tmpscsp->requ->push_back(tmprequ);
	}
}

void RSGameFlow::parseWRLD(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["MAPS"] = std::bind(&RSGameFlow::parseWRLD_MAPS, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseWRLD_MAPS(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	this->tmpmap = new MAP();
	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseWRLD_MAPS_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SPED"] = std::bind(&RSGameFlow::parseWRLD_MAPS_SPED, this, std::placeholders::_1, std::placeholders::_2);
	handlers["DATA"] = std::bind(&RSGameFlow::parseWRLD_MAPS_DATA, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
	this->game.wrld[this->tmpmap->info.ID] = this->tmpmap;
}

void RSGameFlow::parseWRLD_MAPS_INFO(uint8_t* data, size_t size) {
	this->tmpmap->info.ID = *data++;
	this->tmpmap->info.UNKOWN = *data;
}

void RSGameFlow::parseWRLD_MAPS_SPED(uint8_t* data, size_t size) {
	SPED* tmpsped = new SPED();
	for (int i = 0; i < size; i++) {
		tmpsped->unkown.push_back(data[i]);
	}
	this->tmpmap->sped = tmpsped;
}

void RSGameFlow::parseWRLD_MAPS_DATA(uint8_t* data, size_t size) {
	MAP_DATA* tmpmapdata = new MAP_DATA();
	for (int i = 0; i < size; i++) {
		tmpmapdata->unkown.push_back(data[i]);
	}
	int read = 0;
	ByteStream bs(data);
	int nbpoint = bs.ReadByte();
	
	while (read < nbpoint) {
		MAP_POINT* tmp_point = new MAP_POINT();
		tmp_point->x = bs.ReadUShort();
		tmp_point->y = bs.ReadUShort();
		tmp_point->label = bs.ReadStringNoSize(128);
		tmpmapdata->points.push_back(tmp_point);
		read++;
	}
	this->tmpmap->data = tmpmapdata;
}

void RSGameFlow::parseLOAD(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["LOAD"] = std::bind(&RSGameFlow::parseLOAD_LOAD, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseLOAD_LOAD(uint8_t* data, size_t size) {
	LOAD* tmpload = new LOAD();
	for (int i = 0; i < size; i++) {
		tmpload->load.push_back(data[i]);
	}
	this->game.load.push_back(tmpload);
}

void RSGameFlow::parseMLST(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	this->tmpmisslt = new MLST();
	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["DATA"] = std::bind(&RSGameFlow::parseMLST_DATA, this, std::placeholders::_1, std::placeholders::_2);
	handlers["PRTL"] = std::bind(&RSGameFlow::parseMLST_PRTL, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
	this->game.mlst = tmpmisslt;
}

void RSGameFlow::parseMLST_DATA(uint8_t* data, size_t size) {
	size_t read = 0;
	while (read < size) {
		char* tmpstring =  (char *)malloc(sizeof(char)*9);
#pragma warning( push )
#pragma warning( disable : 6387)
		memcpy(tmpstring, data, 9);
		tmpstring[8] = '\0';
		std::string* misname = new std::string(tmpstring);
#pragma warning( pop )
		data += 8;
		read += 8;
		this->tmpmisslt->data.push_back(misname);
	}
}

void RSGameFlow::parseMLST_PRTL(uint8_t* data, size_t size) {
	size_t read = 0;
	while (read < size) {
		char* tmpstring = (char*)malloc(sizeof(char) * 9);
#pragma warning( push )
#pragma warning( disable : 6387)
		memcpy(tmpstring, data, 9);
		std::string* prtlst = new std::string(tmpstring);
#pragma warning( pop )
		data += 9;
		read += 9;
		this->tmpmisslt->prtl.push_back(prtlst);
	}
}

void RSGameFlow::parseWNGS(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["WING"] = std::bind(&RSGameFlow::parseWNGS_WING, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseWNGS_WING(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	this->tmpwings = new WING();
	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseWNGS_WING_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["PILT"] = std::bind(&RSGameFlow::parseWNGS_WING_PILT, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
	this->game.wngs.push_back(tmpwings);
}

void RSGameFlow::parseWNGS_WING_INFO(uint8_t* data, size_t size) {
	this->tmpwings->info.ID = *data++;
	this->tmpwings->info.UNKOWN = *data;
}

void RSGameFlow::parseWNGS_WING_PILT(uint8_t* data, size_t size) {
	for (int i = 0; i < size; i++) {
		this->tmpwings->pilt.push_back(data[i]);
	}
}

void RSGameFlow::parseSTAT(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["CHNG"] = std::bind(&RSGameFlow::parseSTAT_CHNG, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
}

void RSGameFlow::parseSTAT_CHNG(uint8_t* data, size_t size) {
	IFFSaxLexer lexer;
	this->tmpstat = new CHNG();
	this->tmpstat->cash = nullptr;
	this->tmpstat->pilt = nullptr;
	this->tmpstat->weap = nullptr;
	this->tmpstat->over = nullptr;
	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["INFO"] = std::bind(&RSGameFlow::parseSTAT_CHNG_INFO, this, std::placeholders::_1, std::placeholders::_2);
	handlers["PILT"] = std::bind(&RSGameFlow::parseSTAT_CHNG_PILT, this, std::placeholders::_1, std::placeholders::_2);
	handlers["CASH"] = std::bind(&RSGameFlow::parseSTAT_CHNG_CASH, this, std::placeholders::_1, std::placeholders::_2);
	handlers["OVER"] = std::bind(&RSGameFlow::parseSTAT_CHNG_OVER, this, std::placeholders::_1, std::placeholders::_2);
	handlers["WEAP"] = std::bind(&RSGameFlow::parseSTAT_CHNG_WEAP, this, std::placeholders::_1, std::placeholders::_2);
	
	lexer.InitFromRAM(data, size, handlers);
	this->game.stat[this->tmpstat->info.ID] = tmpstat;
}

void RSGameFlow::parseSTAT_CHNG_INFO(uint8_t* data, size_t size) {
	this->tmpstat->info.ID = *data++;
	this->tmpstat->info.UNKOWN = *data;
}

void RSGameFlow::parseSTAT_CHNG_PILT(uint8_t* data, size_t size) {
	std::vector<uint8_t>* tpilt = new std::vector<uint8_t>();
	for (int i = 0; i < size; i++) {
		tpilt->push_back(data[i]);
	}
	this->tmpstat->pilt = tpilt;
}

void RSGameFlow::parseSTAT_CHNG_CASH(uint8_t* data, size_t size) {
	std::vector<uint8_t>* tcash = new std::vector<uint8_t>();
	for (int i = 0; i < size; i++) {
		tcash->push_back(data[i]);
	}
	this->tmpstat->cash = tcash;
}

void RSGameFlow::parseSTAT_CHNG_OVER(uint8_t* data, size_t size) {
	std::vector<uint8_t>* tover = new std::vector<uint8_t>();
	for (int i = 0; i < size; i++) {
		tover->push_back(data[i]);
	}
	this->tmpstat->over = tover;
}

void RSGameFlow::parseSTAT_CHNG_WEAP(uint8_t* data, size_t size) {
	std::vector<uint8_t>* tweap = new std::vector<uint8_t>();
	for (int i = 0; i < size; i++) {
		tweap->push_back(data[i]);
	}
	this->tmpstat->weap = tweap;
}