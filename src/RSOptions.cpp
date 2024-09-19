#include "IFFSaxLexer.h"
#include "RSOption.h"
#include <functional>
#include <map>
#include <string>

RSOption::RSOption() {
    this->mark.DATA = nullptr;
    this->tmpback = nullptr;
    this->tmpfore = nullptr;
    this->tmpscen = nullptr;
    this->tmpsprt = nullptr;
    this->tmpshot = nullptr;
}

RSOption::~RSOption() {
    for (auto &it : this->opts) {
        delete it.second;
    }
    for (auto &it : this->estb) {
        delete it.second;
    }
    if (this->mark.DATA != nullptr) {
        delete[] this->mark.DATA;
    }
}

void RSOption::InitFromRam(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["OPTS"] = std::bind(&RSOption::parseOPTS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ESTB"] = std::bind(&RSOption::parseETSB, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MARK"] = std::bind(&RSOption::parseMARK, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSOption::parseOPTS(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;

    handlers["SCEN"] = std::bind(&RSOption::parseOPTS_SCEN, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}

void RSOption::parseOPTS_SCEN(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;

    printf("Parsing SCEN %llu\n", size);
    this->tmpscen = new SCEN();

    handlers["INFO"] = std::bind(&RSOption::parseOPTS_SCEN_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["COLR"] = std::bind(&RSOption::parseOPTS_SCEN_COLR, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TUNE"] = std::bind(&RSOption::parseOPTS_SCEN_TUNE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["BACK"] = std::bind(&RSOption::parseOPTS_SCEN_BACK, this, std::placeholders::_1, std::placeholders::_2);
    handlers["FORE"] = std::bind(&RSOption::parseOPTS_SCEN_FORE, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
    printf("SCEN INFO ID %d\n", this->tmpscen->infos.ID);
    this->opts[this->tmpscen->infos.ID] = this->tmpscen;
}

void RSOption::parseOPTS_SCEN_INFO(uint8_t *data, size_t size) {
    printf("Parsing INFO %llu\n", size);
    this->tmpscen->infos.ID = *data++;
    this->tmpscen->infos.UNKOWN = *data;
}

void RSOption::parseOPTS_SCEN_COLR(uint8_t *data, size_t size) {
    printf("Parsing COLR %llu\n", size);
    this->tmpscen->colr.UNKOWN_1 = *data++;
    this->tmpscen->colr.UNKOWN_2 = *data;
}

void RSOption::parseOPTS_SCEN_TUNE(uint8_t *data, size_t size) {
    TUNE *tmptune = new TUNE();
    printf("Parsing TUNE %llu\n", size);
    tmptune->ID = *data++;
    tmptune->UNKOWN = *data;
    this->tmpscen->tune = tmptune;
}

void RSOption::parseOPTS_SCEN_BACK(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;

    printf("Parsing BACK %llu\n", size);
    this->tmpback = new BACK();
    handlers["PALT"] =
        std::bind(&RSOption::parseOPTS_SCEN_BACK_PALT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SHAP"] =
        std::bind(&RSOption::parseOPTS_SCEN_BACK_SHAPE, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
    this->tmpscen->background = this->tmpback;
}

void RSOption::parseOPTS_SCEN_BACK_PALT(uint8_t *data, size_t size) {
    PALT *palt = new PALT();
    palt->ID = *data++;
    palt->UNKOWN = *data;
    this->tmpback->palette = palt;
}

void RSOption::parseOPTS_SCEN_BACK_SHAPE(uint8_t *data, size_t size) {
    BACK_SHAP *shps = new BACK_SHAP();
    shps->ID = *data++;
    shps->UNKOWN_1 = *data;
    this->tmpback->images.push_back(shps);
}

void RSOption::parseOPTS_SCEN_FORE(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    this->tmpfore = new FORE();

    handlers["PALT"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_PALT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SPRT"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
    this->tmpscen->foreground = this->tmpfore;
}

void RSOption::parseOPTS_SCEN_FORE_PALT(uint8_t *data, size_t size) {
    PALT *palette = new PALT();
    palette->ID = *data++;
    palette->UNKOWN = *data;
    this->tmpfore->palette = palette;
}

void RSOption::parseOPTS_SCEN_FORE_SPRT(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;

    handlers["SHAP"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT_SHAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["LABL"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT_LABL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["RECT"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT_RECT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TUNE"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT_TUNE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SEQU"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SEQU, this, std::placeholders::_1, std::placeholders::_2);
    handlers["INFO"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CLCK"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT_CLCK, this, std::placeholders::_1, std::placeholders::_2);
    handlers["QUAD"] =
        std::bind(&RSOption::parseOPTS_SCEN_FORE_SPRT_QUAD, this, std::placeholders::_1, std::placeholders::_2);
    this->tmpsprt = new SPRT();
    this->tmpsprt->CLCK = 0;
    lexer.InitFromRAM(data, size, handlers);
    this->tmpfore->sprites[this->tmpsprt->sprite.GID] = this->tmpsprt;
}

void RSOption::parseOPTS_SCEN_FORE_SPRT_TUNE(uint8_t *data, size_t size) {
    TUNE *tune = new TUNE();
    tune->ID = *data++; 
    tune->UNKOWN = *data;
    this->tmpsprt->tune = tune;
}

void RSOption::parseOPTS_SCEN_FORE_SPRT_SHAP(uint8_t *data, size_t size) {
    this->tmpsprt->sprite.GID = *data++;
    this->tmpsprt->sprite.SHP_ID = *data++;
    this->tmpsprt->sprite.UNKOWN_1 = *data++;
    this->tmpsprt->sprite.UNKOWN_2 = *data;
}

void RSOption::parseOPTS_SCEN_FORE_SPRT_CLCK(uint8_t *data, size_t size) { this->tmpsprt->CLCK = 1; }

void RSOption::parseOPTS_SCEN_FORE_SPRT_QUAD(uint8_t *data, size_t size) {
    ByteStream *reader = new ByteStream(data);
    QUAD *quad = new QUAD();
    quad->xa1 = reader->ReadUShort();
    quad->ya1 = reader->ReadUShort();
    quad->xb2 = reader->ReadUShort();
    quad->yb2 = reader->ReadUShort();
    quad->xa2 = reader->ReadUShort();
    quad->ya2 = reader->ReadUShort();
    quad->xb1 = reader->ReadUShort();
    quad->yb1 = reader->ReadUShort();

    quad->xa1 = quad->xa1 == 0 ? 1 : quad->xa1;
    quad->ya1 = quad->ya1 == 200 ? 199 : quad->ya1;
    quad->xa2 = quad->xa2 == 0 ? 1 : quad->xa2;
    quad->ya2 = quad->ya2 == 200 ? 199 : quad->ya2;
    quad->xb1 = quad->xb1 == 0 ? 1 : quad->xb1;
    quad->yb1 = quad->yb1 == 200 ? 199 : quad->yb1;
    quad->xb2 = quad->xb2 == 0 ? 1 : quad->xb2;
    quad->yb2 = quad->yb2 == 200 ? 199 : quad->yb2;

    this->tmpsprt->quad = quad;
}

void RSOption::parseOPTS_SCEN_FORE_SPRT_INFO(uint8_t *data, size_t size) {
    SPRT_INFO *info = new SPRT_INFO();
    info->ID = *data++;
    info->UNKOWN_1 = *data++;
    info->UNKOWN_2 = *data++;
    info->UNKOWN_3 = *data;
}

void RSOption::parseOPTS_SCEN_FORE_SEQU(uint8_t *data, size_t size) {
    if (size > 0) {
        this->tmpsprt->SEQU = new std::vector<uint8_t>();
        for (int i = 0; i < size; i++) {
            if (data[i] != 0) {
                this->tmpsprt->SEQU->push_back(data[i]);
            }
        }
    }
}

void RSOption::parseOPTS_SCEN_FORE_SPRT_RECT(uint8_t *data, size_t size) {
    OPTION_RECT *zone = new OPTION_RECT();
    ByteStream *reader = new ByteStream(data);
    zone->X1 = reader->ReadUShort();
    zone->Y1 = reader->ReadUShort();
    zone->X2 = reader->ReadUShort();
    zone->Y2 = reader->ReadUShort();

    this->tmpsprt->zone = zone;
}

void RSOption::parseOPTS_SCEN_FORE_SPRT_LABL(uint8_t *data, size_t size) {
    char *label = new char[size + 1];
    memcpy(label, data, size);
    label[size] = '\0';
    this->tmpsprt->label = new std::string(label);
}

void RSOption::parseETSB(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;

    handlers["SHOT"] = std::bind(&RSOption::parseETSB_SHOT, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSOption::parseETSB_SHOT(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSOption::parseETSB_SHOT_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SHPS"] = std::bind(&RSOption::parseETSB_SHOT_SHPS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PALS"] = std::bind(&RSOption::parseETSB_SHOT_PALS, this, std::placeholders::_1, std::placeholders::_2);
    this->tmpshot = new SHOT();
    lexer.InitFromRAM(data, size, handlers);
    this->estb[tmpshot->infos.ID] = tmpshot;
}

void RSOption::parseETSB_SHOT_INFO(uint8_t *data, size_t size) {
    this->tmpshot->infos.ID = *data++;
    this->tmpshot->infos.UNKOWN = *data;
}

void RSOption::parseETSB_SHOT_SHPS(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["SHAP"] =
        std::bind(&RSOption::parseETSB_SHOT_SHPS_SHAPE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MOBL"] =
        std::bind(&RSOption::parseETSB_SHOT_SHPS_MOBL, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSOption::parseETSB_SHOT_SHPS_SHAPE(uint8_t *data, size_t size) {
    SHPS *shps = new SHPS();
    shps->type = 1;
    shps->OptshapeID = *data++;
    shps->UNKOWN = *data++;
    shps->x1 = *data++;
    shps->y1 = *data++;
    shps->x2 = *data++;
    shps->y2 = *data;
    this->tmpshot->images.push_back(shps);
}

void RSOption::parseETSB_SHOT_SHPS_MOBL(uint8_t *data, size_t size) {
    SHPS *shps = new SHPS();
    shps->type = 2;
    shps->OptshapeID = *data++;
    shps->UNKOWN = *data++;
    shps->x1 = *data++;
    shps->y1 = *data++;
    shps->x2 = *data++;
    shps->y2 = *data;
    this->tmpshot->images.push_back(shps);
}

void RSOption::parseETSB_SHOT_PALS(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["PALT"] =
        std::bind(&RSOption::parseETSB_SHOT_PALS_PALT, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSOption::parseETSB_SHOT_PALS_PALT(uint8_t *data, size_t size) {
    PALT *pal = new PALT();
    pal->ID = *data++;
    pal->UNKOWN = *data;
    this->tmpshot->palettes.push_back(pal);
}

void RSOption::parseMARK(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;

    // handlers["SHOT"] = std::bind(&RSOption::parseSHOT, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}