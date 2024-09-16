#include "RSMission.h"
#include "PakArchive.h"

void missionstrtoupper(char *src) {
    int i = 0;
    for (int i = 0; i < strlen(src); i++) {
        src[i] = toupper(src[i]);
    }
}

RSMission::RSMission() {
    printf("CREATE MISSION\n");
    this->missionName[0] = '\0';
    this->missionAreaFile[0] = '\0';
    this->missionInfo[0] = '\0';
    this->objCache = new std::map<std::string, RSEntity *>();
    this->missionAreas = std::vector<AREA *>();
    this->missionObjects = std::vector<PART *>();
    this->missionSpots = std::vector<SPOT *>();
    this->missionMessages = std::vector<MSGS *>();
    this->missionCasting = std::vector<CAST *>();
    this->tre = NULL;
}

RSMission::~RSMission() {
    printf("DELETE MISSION\n");
    for (int i = 0; i < missionAreas.size(); i++) {
        free(missionAreas[i]);
    }
    missionAreas.clear();
    for (int i = 0; i < missionObjects.size(); i++) {
        free(missionObjects[i]);
    }
    missionObjects.clear();
    for (int i = 0; i < missionSpots.size(); i++) {
        free(missionSpots[i]);
    }
    missionSpots.clear();
    for (int i = 0; i < missionMessages.size(); i++) {
        free(missionMessages[i]);
    }
    missionMessages.clear();
    for (int i = 0; i < missionCasting.size(); i++) {
        free(missionCasting[i]);
    }
    // delete objCache;
}
void RSMission::printArea(AREA *a) {
    printf("*************************************\n");
    printf("AREA[%d] :%s\n", a->id, a->AreaName);
    printf("TYPE : %c\n", a->AreaType);
    printf("AT : [%d,%d,%d]\n", a->XAxis, a->YAxis, a->ZAxis);
    printf("SIZE : {%d,%d}\n", a->AreaWidth, a->AreaHeight);
    printf("*************************************\n\n");
}

void RSMission::printPart(PART *p) {
    printf("--------------------------------------------------\n");
    printf("NEW PART :%s\n", p->MemberName);
    printf("NUMBER : %d\n", p->MemberNumber);
    printf("LOAD: %s\n", p->WeaponLoad);
    printf("AT : [%d,%d,%d]\n", p->XAxisRelative, p->YAxisRelative, p->ZAxisRelative);
    printf("U0 = %d\n", p->Unknown0);
    printf("U1 = %d\n", p->Unknown1);
    printf("--------------------------------------------------\n\n");
}
void RSMission::PrintMissionInfos() {
    printf("MISSION : %s\n", missionName);
    printf("IN AREA : %s\n", missionAreaFile);
    printf("INFOS : %s\n", missionInfo);
    printf("MSGS : \n");
    for (int i = 0; i < missionMessages.size(); i++) {
        printf("\tM[%d]: %s\n", missionMessages[i]->id, missionMessages[i]->message);
    }
    printf("CASTING : \n");
    for (int i = 0; i < missionCasting.size(); i++) {
        printf("\tCAST[%d]: %s\n", missionCasting[i]->id, missionCasting[i]->actor);
    }
    printf("MISSION AREAS OF INTEREST : \n");
    for (int i = 0; i < missionAreas.size(); i++) {
        printArea(missionAreas[i]);
    }

    printf("MISSION OBJECTS / PARTS : \n");
    for (int i = 0; i < missionObjects.size(); i++) {
        printPart(missionObjects[i]);
    }

    printf("MISSION SPOTS : \n");
    for (int i = 0; i < missionSpots.size(); i++) {
        printf("SPOT[%d] at {%d,%d,%d}, UN(%d)\n", missionSpots[i]->id, missionSpots[i]->XAxis, missionSpots[i]->YAxis,
               missionSpots[i]->ZAxis, missionSpots[i]->unknown);
    }
}
void RSMission::InitFromIFF(IffLexer *lexer) {
    IffChunk *chunk;
    printf("PARSING MISSION DATA\n");

    chunk = lexer->GetChunkByID('WRLD');
    parseWORLD(chunk);

    chunk = lexer->GetChunkByID('INFO');
    parseINFO(chunk);

    chunk = lexer->GetChunkByID('PALT');
    parsePALT(chunk);

    chunk = lexer->GetChunkByID('TERA');
    parseTERA(chunk);

    chunk = lexer->GetChunkByID('SKYS');
    parseSKYS(chunk);

    chunk = lexer->GetChunkByID('GLNT');
    parseGLNT(chunk);

    chunk = lexer->GetChunkByID('SMOK');
    parseSMOK(chunk);

    chunk = lexer->GetChunkByID('LGHT');
    parseLGHT(chunk);

    chunk = lexer->GetChunkByID('VERS');

    chunk = lexer->GetChunkByID('NAME');
    parseNAME(chunk);

    chunk = lexer->GetChunkByID('FILE');
    parseFILE(chunk);

    chunk = lexer->GetChunkByID('AREA');
    parseAREA(chunk);

    chunk = lexer->GetChunkByID('SPOT');
    parseSPOT(chunk);

    chunk = lexer->GetChunkByID('NUMS');

    chunk = lexer->GetChunkByID('MSGS');
    parseMSGS(chunk);

    chunk = lexer->GetChunkByID('FLAG');

    chunk = lexer->GetChunkByID('CAST');
    parseCAST(chunk);

    chunk = lexer->GetChunkByID('PROG');
    parsePROG(chunk);

    chunk = lexer->GetChunkByID('PART');
    parsePART(chunk);

    chunk = lexer->GetChunkByID('TEAM');

    chunk = lexer->GetChunkByID('LOAD');

    chunk = lexer->GetChunkByID('SCNE');
}
void RSMission::parseLOAD(IffChunk *chunk) {}
void RSMission::parseCAST(IffChunk *chunk) {
    if (chunk != NULL) {
        CAST *cst;
        size_t fsize = chunk->size;
        size_t nbactor = fsize / 9;
        ByteStream stream(chunk->data);
        for (int i = 0; i < nbactor; i++) {
            cst = (CAST *)malloc(sizeof(CAST));
            if (cst != NULL) {
                cst->id = i;
                for (int k = 0; k < 8; k++) {
                    cst->actor[k] = stream.ReadByte();
                }
                cst->actor[8] = '\0';
                stream.ReadByte();
                missionCasting.push_back(cst);
            }
        }
    }
}
void RSMission::parseMSGS(IffChunk *chunk) {
    if (chunk != NULL) {
        size_t fsize = chunk->size;
        size_t read = 0;
        size_t strc = 0;
        int msgc = 0;
        ByteStream stream(chunk->data);
        MSGS *scmsg = NULL;
        uint8_t r;
        while (read < fsize) {
            r = stream.ReadByte();
            read++;
            if (r != '\0') {
                if (scmsg == NULL) {
                    scmsg = (MSGS *)malloc(sizeof(MSGS));
                    if (scmsg == NULL) {
                        read = fsize + 1;
                    } else {
                        strc = 0;
                        scmsg->id = static_cast<int>(msgc++);
                    }
                }
                if (scmsg != NULL) {
                    scmsg->message[strc++] = r;
                }
            } else {
                if (scmsg != NULL) {
                    scmsg->message[strc++] = '\0';
                    missionMessages.push_back(scmsg);
                }
                scmsg = NULL;
            }
        }
    }
}

void RSMission::parseWORLD(IffChunk *chunk) {}

void RSMission::parseINFO(IffChunk *chunk) {
    size_t fsize = chunk->size;
    ByteStream stream(chunk->data);
    for (int i = 0; i < fsize; i++) {
        missionInfo[i] = stream.ReadByte();
    }
    missionInfo[fsize] = '\0';
}
void RSMission::parseNAME(IffChunk *chunk) {
    if (chunk != NULL) {
        size_t fsize = chunk->size;
        ByteStream stream(chunk->data);
        for (int i = 0; i < fsize; i++) {
            missionName[i] = stream.ReadByte();
        }
        missionName[fsize] = '\0';
    }
}
void RSMission::parseFILE(IffChunk *chunk) {
    if (chunk != NULL) {
        size_t fsize = chunk->size;
        ByteStream stream(chunk->data);
        for (int i = 0; i < fsize; i++) {
            missionAreaFile[i] = stream.ReadByte();
        }
        missionAreaFile[fsize] = '\0';
    }
}
void RSMission::parsePALT(IffChunk *chunk) {}

void RSMission::parseTERA(IffChunk *chunk) {}

void RSMission::parseSKYS(IffChunk *chunk) {}

void RSMission::parseAREA(IffChunk *chunk) {
    if (chunk != NULL) {
        printf("PARSING AREA\n");

        ByteStream stream(chunk->data);
        size_t fsize = chunk->size;
        size_t read = 0;
        uint8_t buffer;
        int cpt = 0;
        while (read < fsize) {
            buffer = stream.ReadByte();
            AREA *area;
            area = (AREA *)malloc(sizeof(AREA));
            if (area != NULL) {
                area->id = ++cpt;
                area->AreaType = '\0';
                switch (buffer) {
                case 'S':
                    area->AreaType = 'S';
                    for (int k = 0; k < 33; k++) {
                        area->AreaName[k] = stream.ReadByte();
                    }
                    area->XAxis = stream.ReadInt24LE();
                    area->YAxis = stream.ReadInt24LE();
                    area->ZAxis = stream.ReadInt24LE();
                    area->AreaWidth = stream.ReadUShort();
                    stream.ReadByte();
                    read += 49;
                    break;
                case 'C':
                    area->AreaType = 'C';
                    for (int k = 0; k < 33; k++) {
                        area->AreaName[k] = stream.ReadByte();
                    }
                    area->XAxis = stream.ReadInt24LE();
                    area->YAxis = stream.ReadInt24LE();
                    area->ZAxis = stream.ReadInt24LE();
                    area->AreaWidth = stream.ReadUShort();

                    // unsigned int Blank0; // off 48-49
                    stream.ReadByte();
                    stream.ReadByte();
                    area->AreaHeight = stream.ReadUShort();
                    // unsigned char Blank1; // off 52
                    stream.ReadByte();
                    read += 52;
                    break;
                case 'B':
                    area->AreaType = 'B';
                    for (int k = 0; k < 33; k++) {
                        area->AreaName[k] = stream.ReadByte();
                    }
                    area->XAxis = stream.ReadInt24LE();
                    area->YAxis = stream.ReadInt24LE();
                    area->ZAxis = stream.ReadInt24LE();
                    area->AreaWidth = stream.ReadUShort();

                    // unsigned char Blank0[10]; // off 48-59
                    for (int k = 0; k < 10; k++) {
                        stream.ReadByte();
                    }
                    // unsigned int AreaHeight; // off 60-61
                    area->AreaHeight = stream.ReadUShort();

                    // unsigned char Unknown[5]; // off 62-67
                    for (int k = 0; k < 5; k++) {
                        stream.ReadByte();
                    }

                    read += 67;
                    break;
                default:
                    read++;
                    printf("WE ARE\n");
                    break;
                }
                if (area->AreaType != '\0') {
                    printf("READED: %llu, SIZE: %llu\n", read, fsize);
                    missionAreas.push_back(area);
                }

            } else {
                read = fsize + 1;
                printf("Can't allocate memory for area\n");
                // exit the loop, memory error
            }
        }
    }
}

void RSMission::parsePART(IffChunk *chunk) {
    if (chunk != NULL) {
        printf("PARSING PART\n");
        printf("Number of entries %llu\n", chunk->size / 62);
        ByteStream stream(chunk->data);
        size_t numParts = chunk->size / 62;
        for (int i = 0; i < numParts; i++) {
            PART *prt;
            prt = (PART *)malloc(sizeof(PART));
            if (prt != NULL) {
                prt->MemberNumber = 0;
                prt->MemberNumber |= stream.ReadByte() << 0;
                prt->MemberNumber |= stream.ReadByte() << 8;
                for (int k = 0; k < 16; k++) {
                    prt->MemberName[k] = stream.ReadByte();
                }
                for (int k = 0; k < 8; k++) {
                    prt->WeaponLoad[k] = stream.ReadByte();
                }
                prt->Unknown0 = 0;
                prt->Unknown0 |= stream.ReadByte() << 0;
                prt->Unknown0 |= stream.ReadByte() << 8;

                prt->Unknown1 = 0;
                prt->Unknown1 |= stream.ReadByte() << 0;
                prt->Unknown1 |= stream.ReadByte() << 8;

                prt->XAxisRelative = stream.ReadInt24LE();
                prt->YAxisRelative = stream.ReadInt24LE();

                prt->ZAxisRelative = 0;
                prt->ZAxisRelative |= stream.ReadByte() << 0;
                prt->ZAxisRelative |= stream.ReadByte() << 8;

                for (int k = 0; k < 22; k++) {
                    prt->Controls[k] = stream.ReadByte();
                }
                prt->azymuth = 0;
                prt->azymuth |= prt->Controls[1] << 0;
                prt->azymuth |= prt->Controls[2] << 8;
                if (prt->Unknown0 != 65535) {
                    for (int k = 0; k < missionAreas.size(); k++) {
                        if (missionAreas[k]->id - 1 == prt->Unknown0) {
                            prt->XAxisRelative += missionAreas[k]->XAxis;
                            prt->YAxisRelative += missionAreas[k]->YAxis;
                            prt->ZAxisRelative += missionAreas[k]->ZAxis;
                            prt->Unknown0 = 65535;
                        }
                    }
                }
                missionstrtoupper(prt->MemberName);
                std::string hash = prt->MemberName;
                std::map<std::string, RSEntity *>::iterator it;
                it = objCache->find(hash);
                RSEntity *entity = new RSEntity();
                if (it == objCache->end()) {
                    char modelPath[512];
                    const char *OBJ_PATH = "..\\..\\DATA\\OBJECTS\\";
                    const char *OBJ_EXTENSION = ".IFF";

                    strcpy(modelPath, OBJ_PATH);
                    strcat(modelPath, prt->MemberName);
                    strcat(modelPath, OBJ_EXTENSION);
                    missionstrtoupper(modelPath);
                    TreEntry *entry = tre->GetEntryByName(modelPath);

                    if (entry == NULL) {
                        printf("Object reference '%s' not found in TRE.\n", modelPath);
                        // continue;
                    } else {
                        entity->InitFromRAM(entry->data, entry->size);
                        objCache->emplace(hash, entity);
                    }
                } else {
                    entity = it->second;
                }

                prt->entity = entity;
                missionObjects.push_back(prt);
            }
        }
    }
}
void RSMission::parseGLNT(IffChunk *chunk) {

    if (chunk != NULL) {
    }
}

void RSMission::parseSMOK(IffChunk *chunk) {}

void RSMission::parseLGHT(IffChunk *chunk) {}

void RSMission::parseSPOT(IffChunk *chunk) {
    if (chunk != NULL) {
        size_t numParts = chunk->size / 14;
        ByteStream stream(chunk->data);

        for (int i = 0; i < numParts; i++) {
            SPOT *spt;
            spt = (SPOT *)malloc(sizeof(SPOT));
            if (spt != NULL) {
                spt->id = i;
                spt->unknown = 0;
                spt->unknown |= stream.ReadByte() << 0;
                spt->unknown |= stream.ReadByte() << 8;

                stream.ReadByte();
                spt->XAxis = stream.ReadInt24LE();
                spt->YAxis = stream.ReadInt24LE();
                spt->ZAxis = stream.ReadShort();
                stream.ReadByte();
                missionSpots.push_back(spt);
            }
        }
    }
}

void RSMission::parsePROG(IffChunk *chunk) {}
PART *RSMission::getPlayerCoord() {
    for (int i = 0; i < missionObjects.size(); i++) {
        if (missionObjects[i]->MemberNumber == 0) {
            if (missionObjects[i]->Unknown0 != 65535) {
                for (int k = 0; k < missionAreas.size(); k++) {
                    if (missionAreas[k]->id == missionObjects[i]->Unknown0) {
                        missionObjects[i]->XAxisRelative += missionAreas[k]->XAxis;
                        missionObjects[i]->YAxisRelative += missionAreas[k]->YAxis;
                        missionObjects[i]->ZAxisRelative += missionAreas[k]->ZAxis;
                        missionObjects[i]->Unknown0 = 65535;
                    }
                }
            }
            return (missionObjects[i]);
        }
    }
    return (NULL);
}
PART *RSMission::getObject(const char *name) {
    for (int i = 0; i < missionObjects.size(); i++) {
        if (strcmp(missionObjects[i]->MemberName, name) == 0) {
            if (missionObjects[i]->Unknown0 != 65535) {
                for (int k = 0; k < missionAreas.size(); k++) {
                    if (missionAreas[k]->id == missionObjects[i]->Unknown0) {
                        missionObjects[i]->XAxisRelative += missionAreas[k]->XAxis;
                        missionObjects[i]->YAxisRelative += missionAreas[k]->YAxis;
                        missionObjects[i]->ZAxisRelative += missionAreas[k]->ZAxis;
                        missionObjects[i]->Unknown0 = 65535;
                    }
                }
            }
            return (missionObjects[i]);
        }
    }
    return (NULL);
}

void RSMission::InitFromRAM(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["MISN"] = std::bind(&RSMission::parseMISN, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSMission::parseMISN(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["VERS"] = std::bind(&RSMission::parseMISN_VERS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["INFO"] = std::bind(&RSMission::parseMISN_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TUNE"] = std::bind(&RSMission::parseMISN_TUNE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["NAME"] = std::bind(&RSMission::parseMISN_NAME, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WRLD"] = std::bind(&RSMission::parseMISN_WRLD, this, std::placeholders::_1, std::placeholders::_2);

    handlers["AREA"] = std::bind(&RSMission::parseMISN_AREA, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SPOT"] = std::bind(&RSMission::parseMISN_SPOT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["NUMS"] = std::bind(&RSMission::parseMISN_NUMS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MSGS"] = std::bind(&RSMission::parseMISN_MSGS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["FLAG"] = std::bind(&RSMission::parseMISN_FLAG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CAST"] = std::bind(&RSMission::parseMISN_CAST, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PROG"] = std::bind(&RSMission::parseMISN_PROG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PART"] = std::bind(&RSMission::parseMISN_PART, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TEAM"] = std::bind(&RSMission::parseMISN_TEAM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PLAY"] = std::bind(&RSMission::parseMISN_PLAY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["LOAD"] = std::bind(&RSMission::parseMISN_LOAD, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}

void RSMission::parseMISN_VERS(uint8_t *data, size_t size) {}
void RSMission::parseMISN_INFO(uint8_t *data, size_t size) {}
void RSMission::parseMISN_TUNE(uint8_t *data, size_t size) {}
void RSMission::parseMISN_NAME(uint8_t *data, size_t size) {}
void RSMission::parseMISN_WRLD(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["FILE"] = std::bind(&RSMission::parseMISN_WRLD_FILE, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSMission::parseMISN_WRLD_FILE(uint8_t *data, size_t size) {}
void RSMission::parseMISN_AREA(uint8_t *data, size_t size) {}
void RSMission::parseMISN_SPOT(uint8_t *data, size_t size) {}
void RSMission::parseMISN_NUMS(uint8_t *data, size_t size) {}
void RSMission::parseMISN_MSGS(uint8_t *data, size_t size) {}
void RSMission::parseMISN_FLAG(uint8_t *data, size_t size) {}
void RSMission::parseMISN_CAST(uint8_t *data, size_t size) {}
void RSMission::parseMISN_PROG(uint8_t *data, size_t size) {}
void RSMission::parseMISN_PART(uint8_t *data, size_t size) {}
void RSMission::parseMISN_TEAM(uint8_t *data, size_t size) {}
void RSMission::parseMISN_PLAY(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["SCEN"] = std::bind(&RSMission::parseMISN_PLAY_SCEN, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSMission::parseMISN_PLAY_SCEN(uint8_t *data, size_t size) {}
void RSMission::parseMISN_LOAD(uint8_t *data, size_t size) {}
