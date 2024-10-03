#pragma once

#include "precomp.h"


struct CHUD_SHAPE {
    RLEShape *SHAP;
    RLEShape *SHP2;
    std::vector<uint8_t> INFO;
};
struct HINF {
    std::vector<uint8_t> INFO;
    std::string font;
};
struct LADD {
    std::vector<uint8_t> INFO;
    CHUD_SHAPE *VECT;
};
struct HUD_POS {
    uint16_t x;
    uint16_t y;
};
struct TTAG {
    HUD_POS CLSR;
    HUD_POS TARG;
    HUD_POS NUMW;
    HUD_POS HUDM;
    HUD_POS IRNG;
    HUD_POS GFRC;
    HUD_POS MAXG;
    HUD_POS MACH;
    HUD_POS WAYP;
    HUD_POS RALT;
    HUD_POS LNDG;
    HUD_POS FLAP;
    HUD_POS SPDB;
    HUD_POS THRO;
    HUD_POS CALA;
};

struct CHUD {
    uint8_t type;
    HINF *HINF;
    CHUD_SHAPE *ASPD;
    CHUD_SHAPE *ALTI;
    LADD *LADD;
    CHUD_SHAPE *HEAD;
    CHUD_SHAPE *COLL;
    CHUD_SHAPE *STAL;
    CHUD_SHAPE *FUEL;
    CHUD_SHAPE *LCOS;
    CHUD_SHAPE *TARG;
    CHUD_SHAPE *MISD;
    std::vector<uint8_t> CIRC;
    CHUD_SHAPE *CROS;
    CHUD_SHAPE *CCIP;
    CHUD_SHAPE *CCRP;
    CHUD_SHAPE *STRF;
    TTAG *TTAG;
};


class RSHud {
    void parseREAL(uint8_t* data, size_t size);
    void parseREAL_CHUD(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG(uint8_t* data, size_t size);
    void parseREAL_CHUD_SMAL(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_HINF(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_HINF_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_ASPD(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_ASPD_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_ASPD_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_ALTI(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_ALTI_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_ALTI_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_ALTI_SHP2(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LADD(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LADD_VECT(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LADD_VECT_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LADD_VECT_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LADD_LADD(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LADD_LADD_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_HEAD(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_HEAD_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_HEAD_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_HEAD_SHP2(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_COLL(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_COLL_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_COLL_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_STAL(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_STAL_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_STAL_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_FUEL(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_FUEL_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_FUEL_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LCOS(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LCOS_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_LCOS_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TARG(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TARG_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TARG_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_MISD(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_MISD_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_MISD_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CIRC(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CIRC_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CROS(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CROS_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CROS_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CCIP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CCIP_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CCIP_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CCRP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CCRP_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_CCRP_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_STRF(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_STRF_INFO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_STRF_SHAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_CLSR(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_TARG(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_NUMW(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_HUDM(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_IRNG(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_GFRC(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_MAXG(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_MACH(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_WAYP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_RALT(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_LNDG(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_FLAP(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_SPDB(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_THRO(uint8_t* data, size_t size);
    void parseREAL_CHUD_LARG_TTAG_CALA(uint8_t* data, size_t size);

    CHUD *tmp_hud;
public:
    CHUD *small_hud;
    CHUD *large_hud;
    RSHud();
    ~RSHud();
    void InitFromRam(uint8_t* data, size_t size);
};