#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
}
#endif

#include "IFFSaxLexer.h"
#include "IffLexer.h"
#include "RSEntity.h"
#include "TreArchive.h"
#include "RSProf.h"
#include <stdint.h>
#include <string>
#include <algorithm>

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

struct MISN_PART {
    uint8_t id{0};
    std::string member_name;
	std::string member_name_destroyed;
	std::string weapon_load;
	uint8_t area_id{0};      
    uint8_t unknown1{0};
    uint16_t unknown2{0};
    int32_t x{0};
    uint16_t y{0};
    int32_t z{0};
    uint16_t azymuth{0};
    uint16_t roll{0};
    uint16_t pitch{0};
    std::vector<uint8_t> unknown_bytes;
    RSEntity *entity;
    bool alive{true};
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
    std::string actor;
    RSProf *profile;
};

struct MISN {
    uint16_t version;
    std::vector<uint8_t> info;
    uint8_t tune;
    std::string name;
    std::string world_filename;
    std::vector<AREA *> areas;
    std::vector<SPOT *> spots;
    std::vector<std::string *> messages;
    std::vector<uint8_t> flags;
    std::vector<CAST *> casting;
    std::vector<uint8_t> prog;
    std::vector<uint8_t> nums;
    std::vector<MISN_PART *> parts;
    std::vector<uint8_t> team;
    std::vector<std::vector<uint8_t>> scenes;
    std::vector<uint8_t> load;
};
class RSMission {
public:
    std::map<std::string, RSEntity *> *objCache;
    TreArchive *tre;
    
    MISN mission_data;
    RSMission();
    ~RSMission();

    MISN_PART *getPlayerCoord();
    MISN_PART *getObject(const char *name);
    void InitFromRAM(uint8_t *data, size_t size);

private:
    MISN_PART *player{nullptr};
    void parseMISN(uint8_t *data, size_t size);
    void parseMISN_VERS(uint8_t *data, size_t size);
    void parseMISN_INFO(uint8_t *data, size_t size);
    void parseMISN_TUNE(uint8_t *data, size_t size);
    void parseMISN_NAME(uint8_t *data, size_t size);
    void parseMISN_WRLD(uint8_t *data, size_t size);
    void parseMISN_AREA(uint8_t *data, size_t size);
    void parseMISN_SPOT(uint8_t *data, size_t size);
    void parseMISN_NUMS(uint8_t *data, size_t size);
    void parseMISN_MSGS(uint8_t *data, size_t size);
    void parseMISN_FLAG(uint8_t *data, size_t size);
    void parseMISN_CAST(uint8_t *data, size_t size);
    void parseMISN_PROG(uint8_t *data, size_t size);
    void parseMISN_PART(uint8_t *data, size_t size);
    void parseMISN_TEAM(uint8_t *data, size_t size);
    void parseMISN_PLAY(uint8_t *data, size_t size);
    void parseMISN_LOAD(uint8_t *data, size_t size);

    void parseMISN_PLAY_SCEN(uint8_t *data, size_t size);
    void parseMISN_WRLD_FILE(uint8_t *data, size_t size);

    void fixMissionObjectsCoords(void);
};