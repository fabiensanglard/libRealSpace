#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
}
#endif

#include "../commons/IFFSaxLexer.h"
#include "../commons/IffLexer.h"
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
    Vector3D position;
    unsigned int AreaWidth;
    unsigned int AreaHeight;
    std::vector<uint8_t> unknown_bytes;
};

struct MISN_PART {
    uint8_t id{0};
    std::string member_name;
	std::string member_name_destroyed;
	std::string weapon_load;
	uint8_t area_id{0};      
    uint8_t unknown1{0};
    int16_t unknown2{0};
    Vector3D position{0, 0, 0};
    uint8_t unknown3{0};
    uint16_t azymuth{0};
    uint16_t roll{0};
    uint16_t pitch{0};
    std::vector<uint8_t> progs_id;
    
    uint8_t on_is_activated{255};
    uint8_t on_mission_update{255};
    uint8_t on_is_destroyed{255};
    uint8_t on_missions_init{255};

    std::vector<uint8_t> unknown_bytes;
    RSEntity *entity{nullptr};
    bool alive{true};
};

struct SPOT {
    int id;
    short area_id;
    Vector3D position;
    Vector3D origin;
    uint8_t unknown1;
    uint8_t unknown2;
};
struct MSGS {
    char message[255];
    int id;
};
struct CAST {
    std::string actor;
    RSProf *profile;
};
struct PROG {
    uint8_t opcode;
    uint8_t arg;
};
struct MISN_SCEN {
    uint8_t is_active {0};
    int16_t area_id {0};
    std::vector<int16_t> progs_id;
    int16_t on_is_activated {-1};
    int16_t on_mission_update {-1};
    int16_t on_leaving {-1};
    bool is_coord_on_area {false};
    std::vector<uint8_t> unknown_bytes;
    std::vector<uint8_t> cast;
};
struct MISN {
    uint16_t version;
    std::string info;
    uint8_t tune;
    std::string name;
    std::string world_filename;
    std::vector<AREA *> areas;
    std::vector<SPOT *> spots;
    std::vector<std::string *> messages;
    std::vector<int16_t> flags;
    std::vector<CAST *> casting;
    std::vector<std::vector<PROG> *> prog;
    std::vector<uint8_t> nums;
    std::vector<MISN_PART *> parts;
    std::vector<uint16_t> team;
    std::vector<MISN_SCEN *> scenes;
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
};