#include "RSMission.h"
#include "PakArchive.h"

void missionstrtoupper(char *src) {
    int i = 0;
    for (int i = 0; i < strlen(src); i++) {
        src[i] = toupper(src[i]);
    }
}

RSMission::RSMission() { printf("CREATE MISSION\n"); }

RSMission::~RSMission() {}

MISN_PART *RSMission::getPlayerCoord() { 
    int search_id = 0;
    for (auto cast : this->mission_data.casting) {
        if (cast->actor == "PLAYER") {
            for (auto part : this->mission_data.parts) {
                if (part->id == search_id) {
                    return part;
                }
            }
        }
        search_id++;
    }
}
MISN_PART *RSMission::getObject(const char *name) { 
    for (auto obj : this->mission_data.parts) {
        if (obj->member_name == name) {
            return obj;
        }
    }
    return NULL;
 }

void RSMission::InitFromRAM(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["MISN"] = std::bind(&RSMission::parseMISN, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
    fixMissionObjectsCoords();
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

void RSMission::parseMISN_VERS(uint8_t *data, size_t size) {
    ByteStream stream(data);
    this->mission_data.version = stream.ReadUShort();
}
void RSMission::parseMISN_INFO(uint8_t *data, size_t size) {
    ByteStream stream(data);
    for (int i = 0; i < size; i++) {
        this->mission_data.info.push_back(stream.ReadByte());
    }
}
void RSMission::parseMISN_TUNE(uint8_t *data, size_t size) {
    ByteStream stream(data);
    this->mission_data.tune = stream.ReadByte();
}
void RSMission::parseMISN_NAME(uint8_t *data, size_t size) {
    ByteStream stream(data);
    for (int i = 0; i < size; i++) {
        this->mission_data.name.push_back(stream.ReadByte());
    }
    std::transform(this->mission_data.name.begin(), this->mission_data.name.end(), this->mission_data.name.begin(), ::toupper);
}
void RSMission::parseMISN_WRLD(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["FILE"] = std::bind(&RSMission::parseMISN_WRLD_FILE, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSMission::parseMISN_WRLD_FILE(uint8_t *data, size_t size) {
    ByteStream stream(data);
    for (int i = 0; i < size; i++) {
        this->mission_data.world_filename.push_back(stream.ReadByte());
    }
}
void RSMission::parseMISN_AREA(uint8_t *data, size_t size) {
    ByteStream stream(data);

    size_t read = 0;
    uint8_t buffer;
    int cpt = 0;
    while (read < size) {
        buffer = stream.ReadByte();
        AREA *tmparea = new AREA();

        tmparea->id = ++cpt;
        tmparea->AreaType = '\0';
        uint8_t Blank0;
        switch (buffer) {
        case 'S':
            tmparea->AreaType = 'S';
            for (int k = 0; k < 33; k++) {
                tmparea->AreaName[k] = stream.ReadByte();
            }
            missionstrtoupper(tmparea->AreaName);
            tmparea->XAxis = stream.ReadInt24LE();
            tmparea->YAxis = stream.ReadInt24LE();
            tmparea->ZAxis = stream.ReadInt24LE();
            tmparea->AreaWidth = stream.ReadUShort();
            Blank0 = stream.ReadByte();
            read += 49;
            break;
        case 'C':
            tmparea->AreaType = 'C';
            for (int k = 0; k < 33; k++) {
                tmparea->AreaName[k] = stream.ReadByte();
            }
            missionstrtoupper(tmparea->AreaName);
            tmparea->XAxis = stream.ReadInt24LE();
            tmparea->YAxis = stream.ReadInt24LE();
            tmparea->ZAxis = stream.ReadInt24LE();
            tmparea->AreaWidth = stream.ReadUShort();

            // unsigned int Blank0; // off 48-49
            stream.ReadByte();
            stream.ReadByte();
            tmparea->AreaHeight = stream.ReadUShort();
            // unsigned char Blank1; // off 52
            stream.ReadByte();
            read += 52;
            break;
        case 'B':
            tmparea->AreaType = 'B';
            for (int k = 0; k < 33; k++) {
                tmparea->AreaName[k] = stream.ReadByte();
            }
            missionstrtoupper(tmparea->AreaName);
            tmparea->XAxis = stream.ReadInt24LE();
            tmparea->YAxis = stream.ReadInt24LE();
            tmparea->ZAxis = stream.ReadInt24LE();
            tmparea->AreaWidth = stream.ReadUShort();

            // unsigned char Blank0[10]; // off 48-59
            for (int k = 0; k < 10; k++) {
                stream.ReadByte();
            }
            // unsigned int AreaHeight; // off 60-61
            tmparea->AreaHeight = stream.ReadUShort();

            // unsigned char Unknown[5]; // off 62-67
            for (int k = 0; k < 5; k++) {
                stream.ReadByte();
            }

            read += 67;
            break;
        default:
            read++;
            printf("ERROR IN PARSING AREA\n");
            break;
        }
        if (tmparea->AreaType != '\0') {
            this->mission_data.areas.push_back(tmparea);
        }
    }
}
void RSMission::parseMISN_SPOT(uint8_t *data, size_t size) {
    size_t numParts = size / 14;
    ByteStream stream(data);

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
            this->mission_data.spots.push_back(spt);
        }
    }
}
void RSMission::parseMISN_NUMS(uint8_t *data, size_t size) {
    ByteStream stream(data);
    for (int i = 0; i < size; i++) {
        this->mission_data.nums.push_back(stream.ReadByte());
    }
}
void RSMission::parseMISN_MSGS(uint8_t *data, size_t size) {
    size_t read = 0;
    while (read < size) {
        
        char buffer[512];
        int i = 0;
        while (data[read] != '\0') {
            buffer[i]= data[read];
            read++;
            i++;
        }
        if (data[read] == '\0') {
            buffer[i] = '\0';
            std::string *msg = new std::string(buffer);
            read++;
            this->mission_data.messages.push_back(msg);
        }
    }
}
void RSMission::parseMISN_FLAG(uint8_t *data, size_t size) {
    for (int i = 0; i < size; i++) {
        this->mission_data.flags.push_back(data[i]);
    }
}
void RSMission::parseMISN_CAST(uint8_t *data, size_t size) {
    size_t nbactor = size / 9;
    ByteStream stream(data);
    
    for (int i = 0; i < nbactor; i++) {
        CAST *tmpcast = new CAST();
        
        std::string actor = stream.ReadString(8);
        stream.ReadByte();
        tmpcast->actor = actor;
        TreArchive *tre = new TreArchive();	
        tre->InitFromFile("MISSIONS.TRE");
        TreEntry *treEntry = NULL;
        char *prof_intel_filename = new char[512];
        sprintf(prof_intel_filename, "..\\..\\DATA\\\INTEL\\%s.IFF", actor.c_str());
        treEntry = tre->GetEntryByName(prof_intel_filename);
        RSProf * rsprof = new RSProf();
        rsprof->InitFromRAM(treEntry->data, treEntry->size);
        tmpcast->profile = rsprof;
        this->mission_data.casting.push_back(tmpcast);
    }
}
void RSMission::parseMISN_PROG(uint8_t *data, size_t size) {
    for (int i = 0; i < size; i++) {
        this->mission_data.prog.push_back(data[i]);
    }
}
void RSMission::parseMISN_PART(uint8_t *data, size_t size) {
    printf("PARSING PART\n");
    printf("Number of entries %llu\n", size / 62);
    ByteStream stream(data);
    size_t numParts = size / 62;
    for (int i = 0; i < numParts; i++) {
        MISN_PART *prt = new MISN_PART();
        prt->id = 0;
        prt->id |= stream.ReadByte() << 0;
        prt->id |= stream.ReadByte() << 8;
        prt->member_name = stream.ReadString(8);
        std::transform(prt->member_name.begin(), prt->member_name.end(), prt->member_name.begin(), ::toupper);
        prt->member_name_destroyed = stream.ReadString(8);
        std::transform(prt->member_name_destroyed.begin(), prt->member_name_destroyed.end(), prt->member_name_destroyed.begin(), ::toupper);
        prt->weapon_load = stream.ReadString(8);
        std::transform(prt->weapon_load.begin(), prt->weapon_load.end(), prt->weapon_load.begin(), ::toupper);
        
        prt->area_id = stream.ReadByte();
        prt->unknown1 = stream.ReadByte();
        prt->unknown2 = 0;
        prt->unknown2 |= stream.ReadByte() << 0;
        prt->unknown2 |= stream.ReadByte() << 8;

        prt->x = stream.ReadInt24LE();
        prt->y = stream.ReadInt24LE();

        prt->z = 0;
        prt->z |= stream.ReadByte() << 0;
        prt->z |= stream.ReadByte() << 8;

        for (int k = 0; k < 22; k++) {
            prt->unknown_bytes.push_back(stream.ReadByte());
        }
        prt->azymuth = 0;
        prt->azymuth |= prt->unknown_bytes[1] << 0;
        prt->azymuth |= prt->unknown_bytes[2] << 8;
        
        std::string hash = prt->member_name;
        std::map<std::string, RSEntity *>::iterator it;
        it = objCache->find(hash);
        RSEntity *entity = new RSEntity();
        if (it == objCache->end()) {
            char modelPath[512];
            const char *OBJ_PATH = "..\\..\\DATA\\OBJECTS\\";
            const char *OBJ_EXTENSION = ".IFF";

            strcpy(modelPath, OBJ_PATH);
            strcat(modelPath, prt->member_name.c_str());
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
        this->mission_data.parts.push_back(prt);
    }
}
void RSMission::parseMISN_TEAM(uint8_t *data, size_t size) {
    for (int i = 0; i < size; i++) {
        this->mission_data.team.push_back(data[i]);
    }
}
void RSMission::parseMISN_PLAY(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["SCEN"] = std::bind(&RSMission::parseMISN_PLAY_SCEN, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSMission::parseMISN_PLAY_SCEN(uint8_t *data, size_t size) {
    for (int i = 0; i < size; i++) {
        std::vector<uint8_t> scene;
        for (int j = 0; j < 8; j++) {
            scene.push_back(data[i]);
            i++;
        }
        this->mission_data.scenes.push_back(scene);
    }
}
void RSMission::parseMISN_LOAD(uint8_t *data, size_t size) {
    for (int i = 0; i < size; i++) {
        this->mission_data.load.push_back(data[i]);
    }
}
void RSMission::fixMissionObjectsCoords(void) {
    for (auto obj : this->mission_data.parts) {
        if (obj->area_id != 255) {
            for (auto area : this->mission_data.areas) {
                if (area->id-1 == obj->area_id) {
                    obj->x += area->XAxis;
                    obj->y += area->YAxis;
                    obj->z += area->ZAxis;
                    break;
                }
            }
        }
    }
}