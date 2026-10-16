#include "RSWorld.h"

RSWorld::RSWorld() {

}
RSWorld::~RSWorld() {

}
void RSWorld::InitFromRAM(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["WRLD"] = std::bind(&RSWorld::parseWRLD, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSWorld::parseWRLD(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSWorld::parseWRLD_INFO, this, std::placeholders::_1, std::placeholders::_2);

    handlers["HORZ"] = std::bind(&RSWorld::parseWRLD_HORZ, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WTCH"] = std::bind(&RSWorld::parseWRLD_WTCH, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PALT"] = std::bind(&RSWorld::parseWRLD_PALT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TERA"] = std::bind(&RSWorld::parseWRLD_TERA, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SKYS"] = std::bind(&RSWorld::parseWRLD_SKYS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["GLNT"] = std::bind(&RSWorld::parseWRLD_GLNT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SMOK"] = std::bind(&RSWorld::parseWRLD_SMOK, this, std::placeholders::_1, std::placeholders::_2);
    handlers["LGHT"] = std::bind(&RSWorld::parseWRLD_LGHT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CAMR"] = std::bind(&RSWorld::parseWRLD_CAMR, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSWorld::parseWRLD_INFO(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_HORZ(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_WTCH(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_PALT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_TERA(uint8_t *data, size_t size) {IFFSaxLexer lexer;

    ByteStream stream(data, size);
    this->tera = stream.ReadStringNoSize(size);
}

void RSWorld::parseWRLD_SKYS(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_GLNT(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_SMOK(uint8_t *data, size_t size) {}

void RSWorld::parseWRLD_LGHT(uint8_t *data, size_t size) {
    ByteStream bs(data, size);
    this->light_ambient_weight = bs.ReadUShort();
    this->light_percent = bs.ReadUShort();
    this->light_weight = bs.ReadUShort();
    for (int i = 0; i < 3; i++) {
        this->light_direction[i] = bs.ReadShort();
    }
    this->has_light = true;
}

static void readSimpleCamera(ByteStream &s, size_t size, bool lookAtGap,
                             RSCameraType typeCode, std::vector<RSCameraDef> &out) {
    RSCameraDef c;
    c.typeCode = typeCode;
    c.name     = s.ReadStringNoSize(8);                // +0x00
    s.MoveForward(lookAtGap ? 14 : 2);                 // slot look-at (CHAS = 14, autres = 2)
    c.subject  = s.ReadStringNoSize(8);
    c.farClip  = s.ReadFixedFloatLE();
    c.fov      = s.ReadFixedFloat16LE();               // 8.8 -> degrés, converti au décodage
    c.nearClip = s.ReadFixedFloatLE();                     // l'ASM lit UN dword ici
    c.viewX    = s.ReadUShort();                       // rect viewport : x, y, w, h
    c.viewY    = s.ReadUShort();
    c.viewW    = s.ReadUShort();
    c.viewH    = s.ReadUShort();
    // CHAS/TARG/ROTA s'arrêtent ici (payload 0x30). VICT/WEAP ont une queue
    // d'offsets 24.8 (CAMERA_SYSTEM.md §2.4) mêlée à de petits params bruts
    // (angle/marqueur) : un octet bas non nul signale un param brut, pas
    // du 24.8 — converti ici, au décodage, jamais au runtime.
    while ((size_t)s.GetCurrentPosition() + 4 <= size) {
        int32_t raw = s.ReadInt32LE();
        if ((raw & 0xFF) != 0) {
            c.params.push_back((float)raw);
        } else {
            c.params.push_back((float)raw / 256.0f);
        }
    }
    out.push_back(c);
}

void RSWorld::parseWRLD_CAMR(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["STRT"] = std::bind(&RSWorld::parseWRLD_CAMR_STRT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CHAS"] = std::bind(&RSWorld::parseWRLD_CAMR_CHAS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CKPT"] = std::bind(&RSWorld::parseWRLD_CAMR_CKPT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["VICT"] = std::bind(&RSWorld::parseWRLD_CAMR_VICT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TARG"] = std::bind(&RSWorld::parseWRLD_CAMR_TARG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WEAP"] = std::bind(&RSWorld::parseWRLD_CAMR_WEAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ROTA"] = std::bind(&RSWorld::parseWRLD_CAMR_ROTA, this, std::placeholders::_1, std::placeholders::_2);
    handlers["COMP"] = std::bind(&RSWorld::parseWRLD_CAMR_COMP, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSWorld::parseWRLD_CAMR_STRT(uint8_t *data, size_t size) {
    ByteStream s(data, size);
    this->cameraSetName = s.ReadStringNoSize((int)size);
}

void RSWorld::parseWRLD_CAMR_CHAS(uint8_t *data, size_t size) {
    ByteStream s(data, size);
    readSimpleCamera(s, size, true,  RSCAM_CHAS, this->cameras);
}
void RSWorld::parseWRLD_CAMR_VICT(uint8_t *data, size_t size) {
    ByteStream s(data, size);
    readSimpleCamera(s, size, false, RSCAM_VICT, this->cameras);
}
void RSWorld::parseWRLD_CAMR_TARG(uint8_t *data, size_t size) {
    ByteStream s(data, size);
    readSimpleCamera(s, size, false, RSCAM_TARG, this->cameras);
}
void RSWorld::parseWRLD_CAMR_WEAP(uint8_t *data, size_t size) {
    ByteStream s(data, size);
    readSimpleCamera(s, size, false, RSCAM_WEAP, this->cameras);
}
void RSWorld::parseWRLD_CAMR_ROTA(uint8_t *data, size_t size) {
    ByteStream s(data, size);
    readSimpleCamera(s, size, false, RSCAM_ROTA, this->cameras);
}

void RSWorld::parseWRLD_CAMR_CKPT(uint8_t *data, size_t size) {
    ByteStream s(data, size);
    RSCameraDef c;
    c.typeCode   = RSCAM_CKPT;
    c.name       = s.ReadString(8);                     // "COCKPIT"
    s.MoveForward(2);
    c.subject    = s.ReadString(8);                     // "PLAYER"
    c.cockpitArt = s.ReadString(8);                     // "F16-CKPT"
    c.farClip    = s.ReadFixedFloatLE();
    c.fov        = s.ReadFixedFloat16LE();              // 8.8 -> degrés, converti au décodage
    if ((size_t)s.GetCurrentPosition() + 4 <= size) {
        c.nearClip = s.ReadFixedFloatLE();
    }
        
    while ((size_t)s.GetCurrentPosition() + 4 <= size) {
        c.params.push_back(s.ReadInt32LE());
    }
    this->cameras.push_back(c);
}

void RSWorld::parseWRLD_CAMR_COMP(uint8_t *data, size_t size) {
    this->cameraSequences.push_back(decodeCOMPSequence(data, size));
}