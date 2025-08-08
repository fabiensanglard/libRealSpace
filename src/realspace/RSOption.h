#pragma once


#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <string>
#include <map>

struct EXTR_SHAP {
    uint8_t EXTR_ID;
    uint8_t SHAPE_ID;
};
struct QUAD {
    uint16_t xa1;
    uint16_t ya1;
    uint16_t xa2;
    uint16_t ya2;
    uint16_t xb1;
    uint16_t yb1;
    uint16_t xb2;
    uint16_t yb2;
};
struct OPTS_INFO {
    uint8_t ID;
    uint8_t UNKOWN;
};
struct COLR {
    uint8_t UNKOWN_1;
    uint8_t UNKOWN_2;
};
struct TUNE {
    uint8_t ID;
    uint8_t UNKOWN;
};
struct PALT {
    uint8_t ID;
    uint8_t UNKOWN;
};
struct BACK_SHAP {
    uint8_t ID;
    uint8_t UNKOWN_1;
};
struct SPRT_SHAP {
    uint8_t GID;
    uint8_t SHP_ID;
    uint8_t UNKOWN_1;
    uint8_t UNKOWN_2;
};
struct OPTION_RECT {
    uint16_t X1;
    uint16_t Y1;
    uint16_t X2;
    uint16_t Y2;
};
struct SPRT_INFO {
    uint8_t ID;
    uint8_t UNKOWN_1;
    uint8_t UNKOWN_2;
    uint8_t UNKOWN_3;
};
struct SPRT {
    SPRT_SHAP sprite;
    std::string *label;
    OPTION_RECT *zone;
    TUNE *tune{nullptr};
    uint8_t CLCK;
    QUAD* quad;
    SPRT_INFO* info;
    std::vector<uint8_t>* SEQU;
};

struct BACK {
    PALT *palette;
    std::vector<BACK_SHAP*> images;
};
struct FORE {
    PALT *palette{nullptr};
    std::map<uint8_t, SPRT*> sprites;
};
struct SCEN {
    OPTS_INFO infos;
    COLR colr;
    TUNE *tune{nullptr};
    BACK *background{nullptr};
    FORE *foreground{nullptr};
    std::vector<EXTR_SHAP*> extr;

};
struct SHPS {
    uint8_t type; // SHAP or MOBL
    uint8_t OptshapeID;
    uint8_t UNKOWN;
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
};

struct SHOT {
    OPTS_INFO infos;
    std::vector<SHPS*> images;
    std::vector<PALT*> palettes;
};
struct KILL {
    uint8_t UNKOWN_1;
    uint8_t UNKOWN_2;
    uint8_t UNKOWN_3;
    uint8_t UNKOWN_4;
};
struct MARK {
    KILL unkown_1;
    uint8_t *DATA;
};


class RSOption {

private:
    SCEN* tmpscen;
    FORE* tmpfore;
    BACK* tmpback;
    SPRT* tmpsprt;
    SHOT* tmpshot;
    MARK mark;

    
    
    void parseOPTS(uint8_t* data, size_t size);
    void parseOPTS_SCEN(uint8_t* data, size_t size);
    void parseOPTS_SCEN_INFO(uint8_t* data, size_t size);
    void parseOPTS_SCEN_COLR(uint8_t* data, size_t size);
    void parseOPTS_SCEN_TUNE(uint8_t* data, size_t size);
    void parseOPTS_SCEN_BACK(uint8_t* data, size_t size);
    void parseOPTS_SCEN_BACK_PALT(uint8_t* data, size_t size);
    void parseOPTS_SCEN_BACK_SHAPE(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_PALT(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SEQU(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT_SHAP(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT_TUNE(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT_CLCK(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT_QUAD(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT_INFO(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT_RECT(uint8_t* data, size_t size);
    void parseOPTS_SCEN_FORE_SPRT_LABL(uint8_t* data, size_t size);
    void parseOPTS_SCEN_EXTR(uint8_t* data, size_t size);
    void parseOPTS_SCEN_EXTR_SHAP(uint8_t* data, size_t size);
    void parseETSB(uint8_t* data, size_t size);
    void parseETSB_SHOT(uint8_t* data, size_t size);
    void parseETSB_SHOT_INFO(uint8_t* data, size_t size);
    void parseETSB_SHOT_SHPS(uint8_t* data, size_t size);
    void parseETSB_SHOT_SHPS_SHAPE(uint8_t* data, size_t size);
    void parseETSB_SHOT_SHPS_MOBL(uint8_t* data, size_t size);
    void parseETSB_SHOT_PALS(uint8_t* data, size_t size);
    void parseETSB_SHOT_PALS_PALT(uint8_t* data, size_t size);
    void parseMARK(uint8_t* data, size_t size);

public:
    RSOption();
    ~RSOption();
    void InitFromRam(uint8_t* data, size_t size);

    std::map<std::uint8_t, SCEN*> opts;
    std::map<std::uint8_t, SHOT*> estb;

};
