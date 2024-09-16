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
#include <stdint.h>
#include <string>

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
struct AREA_TypeS {
    unsigned char AreaType; // offset 0, S char
    char AreaName[33];      // off 1-33, text or 0x2E
    long XAxis;             // off 34-37, X pos of object
    long YAxis;             // off 38-41, Y pos of object
    long ZAxis;             // off 42-45, Z pos of object
    unsigned int AreaWidth; // off 46-47
    unsigned char Blank0;   // off 48
};

struct AREA_TypeC {
    unsigned char AreaType;  // offset 0, C char
    char AreaName[33];       // off 1-33, text or 0x2E
    long XAxis;              // off 34-37, X pos of object
    long YAxis;              // off 38-41, Y pos of object
    long ZAxis;              // off 42-45, Z pos of object
    unsigned int AreaWidth;  // off 46-47
    unsigned int Blank0;     // off 48-49
    unsigned int AreaHeight; // off 50-51
    unsigned char Blank1;    // off 52
};

struct AREA_TypeB {
    unsigned char AreaType;   // offset 0, C char
    char AreaName[33];        // off 1-33, text or 0x2E
    long XAxis;               // off 34-37, X pos of object
    long YAxis;               // off 38-41, Y pos of object
    long ZAxis;               // off 42-45, Z pos of object
    unsigned int AreaWidth;   // off 46-47
    unsigned char Blank0[10]; // off 48-59
    unsigned int AreaHeight;  // off 60-61
    unsigned char Unknown[5]; // off 62-67
};

struct PART {
    unsigned int MemberNumber;  // off 0-1, Cast Member Number
    char MemberName[17];        // 2-17, IFF File Record name
    char WeaponLoad[9];         // 18-25, IFF File Record name
    unsigned int Unknown0;      // 26-27
    unsigned int Unknown1;      // 28-29
    long XAxisRelative;         // 30-33, X pos rel to AREA
    long YAxisRelative;         // 34-37, Y pos rel to AREA
    unsigned int ZAxisRelative; // 38-39, Z pos rel to AREA
    unsigned char Controls[22]; // 40-61, various control bytes
    uint16_t azymuth;
    RSEntity *entity;
};

struct MISN_PART {
    uint8_t id;  // off 0-1, Cast Member Number
    std::string member_name;
	std::string member_name_destroyed;
	
	uint8_t unknown0;      // 26-27
    uint8_t unknown1;      // 28-29
    int32_t XAxisRelative;         // 30-33, X pos rel to AREA
    int32_t YAxisRelative;         // 34-37, Y pos rel to AREA
    uint16_t ZAxisRelative; // 38-39, Z pos rel to AREA
    std::vector<uint8_t> controls; // 40-61, various control bytes
    uint16_t azymuth;
    RSEntity *entity;
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

struct MISN {
    uint16_t version;
    std::vector<uint8_t> info;
    uint8_t tune;
    std::string name;
    std::string world_filename;
    std::vector<AREA *> areas;
    std::vector<SPOT *> spots;
    std::vector<std::string> messages;
    std::vector<uint8_t> flags;
    std::vector<std::string> casting;
    std::vector<uint8_t> prog;
    std::vector<PART *> parts;
    std::vector<uint8_t> team;
    std::vector<std::vector<uint8_t>> scenes;
    std::vector<uint8_t> load;
};
class RSMission {
public:
    std::map<std::string, RSEntity *> *objCache;
    TreArchive *tre;
    std::vector<PART *> missionObjects;
    std::vector<AREA *> missionAreas;

    RSMission();
    ~RSMission();

    inline char *getMissionName() { return (missionName); }
    inline char *getMissionAreaFile() { return (missionAreaFile); }

    PART *getPlayerCoord();
    PART *getObject(const char *name);
    void InitFromIFF(IffLexer *lexer);
    void InitFromRAM(uint8_t *data, size_t size);

    void PrintMissionInfos();

private:
    char missionInfo[255];
    char missionName[255];
    char missionAreaFile[9];

    std::vector<MSGS *> missionMessages;
    std::vector<SPOT *> missionSpots;
    std::vector<CAST *> missionCasting;

    void printArea(AREA *a);
    void printPart(PART *p);

    void parseLOAD(IffChunk *chunk);
    void parseCAST(IffChunk *chunk);
    void parseMSGS(IffChunk *chunk);
    void parseFILE(IffChunk *chunk);
    void parseNAME(IffChunk *chunk);
    void parseWORLD(IffChunk *chunk);
    void parseINFO(IffChunk *chunk);
    void parsePALT(IffChunk *chunk);
    void parseTERA(IffChunk *chunk);
    void parseSKYS(IffChunk *chunk);
    void parseGLNT(IffChunk *chunk);
    void parseSMOK(IffChunk *chunk);
    void parseLGHT(IffChunk *chunk);
    void parseAREA(IffChunk *chunk);
    void parsePART(IffChunk *chunk);
    void parseSPOT(IffChunk *chunk);
    void parsePROG(IffChunk *chunk);

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