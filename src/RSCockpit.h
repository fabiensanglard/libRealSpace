#pragma once
#include "precomp.h"
#include <vector>
#include <map>
/*
    Iff chunk hierarchy:
        CKPT
            INFO    vector<uint8_t>
            ARTP    RSImageSet
            VTMP    RSImageSet
            EJEC    RSImageSet
            GUNF    RSImageSet
            GHUD    RSImageSet
            REAL
                INFO    vector<uint8_t>
                OBJS    RSEntiry
            CHUD
                FILE    string
            MONI
                INFO    vector<uint8_t>
                SPOT    vector<uint8_t>
                SHAP    RLEShape
                DAMG    RLEShape
                MFDS
                    COMM
                        INFO   vector<uint8_t>
                    AARD
                        INFO   vector<uint8_t> 
                        SHAP   RLEShape
                    ARGD
                        INFO    vector<uint8_t> 
                        SHAP    RLEShape
                    GCAM
                        INFO    vector<uint8_t>
                        SHAP    RLEShape
                    WEAP
                        INFO    vector<uint8_t>
                        SHAP    RLEShape
                    DAMG
                        INFO    vector<uint8_t>
                        SHAP    RLEShape    
                INST
                    RAWS
                        INFO    vector<uint8_t>
                        SHAP    RLEShape
                    ALTI
                        INFO    vector<uint8_t>
                        SHAP    RLEShape
                    AIRS
                        INFO    vector<uint8_t>
                        SHAP    RLEShape
                    MWRN
                        INFO    vector<uint8_t>
                        SHAP    RLEShape
            FADE    vector<uint8_t>
*/
struct InfoShape {
    std::vector<uint8_t> INFO;
    RLEShape SHAP;
};
struct RealObjs {
    std::vector<uint8_t> INFO;
    RSEntity OBJS;
};
struct Moni {
    std::vector<uint8_t> INFO;
    std::vector<uint8_t> SPOT;
    RLEShape SHAP;
    RLEShape DAMG;
    struct Mfds {
        InfoShape COMM;
        InfoShape AARD;
        InfoShape ARGD;
        InfoShape GCAM;
        InfoShape WEAP;
        InfoShape DAMG;
    } MFDS;
    struct Inst {
        InfoShape RAWS;
        InfoShape ALTI;
        InfoShape AIRS;
        InfoShape MWRN;
    } INST;
};

class RSCockpit {

private:
    std::vector<uint8_t> INFO;
    RSImageSet ARTP;
    RSImageSet VTMP;
    RSImageSet EJEC;
    RSImageSet GUNF;
    RSImageSet GHUD;
    RealObjs REAL;
    struct Chud {
        std::string FILE;
    } CHUD;
    Moni MONI;
    std::vector<uint8_t> FADE;

    void parseCKPT(uint8_t* data, size_t size);
    void parseINFO(uint8_t* data, size_t size);
    void parseARTP(uint8_t* data, size_t size);
    void parseVTMP(uint8_t* data, size_t size);
    void parseEJEC(uint8_t* data, size_t size);
    void parseGUNF(uint8_t* data, size_t size);
    void parseGHUD(uint8_t* data, size_t size);
    void parseREAL(uint8_t* data, size_t size);
    void parseCHUD(uint8_t* data, size_t size);
    void parseMONI(uint8_t* data, size_t size);
    void parseFADE(uint8_t* data, size_t size);
    void parseREAL_INFO(uint8_t* data, size_t size);
    void parseREAL_OBJS(uint8_t* data, size_t size);
    void parseCHUD_FILE(uint8_t* data, size_t size);

    void parseMONI_INFO(uint8_t* data, size_t size);
    void parseMONI_SPOT(uint8_t* data, size_t size);
    void parseMONI_SHAP(uint8_t* data, size_t size);
    void parseMONI_DAMG(uint8_t* data, size_t size);
    void parseMONI_MFDS(uint8_t* data, size_t size);
    void parseMONI_MFDS_COMM(uint8_t* data, size_t size);
    void parseMONI_MFDS_AARD(uint8_t* data, size_t size);
    void parseMONI_MFDS_ARGD(uint8_t* data, size_t size);
    void parseMONI_MFDS_GCAM(uint8_t* data, size_t size);
    void parseMONI_MFDS_WEAP(uint8_t* data, size_t size);
    void parseMONI_MFDS_DAMG(uint8_t* data, size_t size);
    void parseMONI_INST(uint8_t* data, size_t size);

    void parseMONI_INST_RAWS(uint8_t* data, size_t size);
    void parseMONI_INST_ALTI(uint8_t* data, size_t size);
    void parseMONI_INST_AIRS(uint8_t* data, size_t size);
    void parseMONI_INST_MWRN(uint8_t* data, size_t size);


public:
    RSCockpit();
    ~RSCockpit();
    void InitFromRam(uint8_t* data, size_t size);
};