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
                    AGRD
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
    RSImageSet ARTS;
};
struct RealObjs {
    std::vector<uint8_t> INFO;
    RSEntity OBJS;
};

/**
 * @struct Moni
 * @brief Struct that represents the Moni hierarchy
 *
 * This struct represents the Moni hierarchy from the RealSpace game.
 * It contains the following fields:
 * - INFO: A vector of bytes containing the INFO data
 * - SPOT: A vector of bytes containing the SPOT data
 * - SHAP: An RLEShape object representing the shape of the cockpit
 * - DAMG: An RLEShape object representing the damaged shape of the cockpit
 * - MFDS: A struct containing the MFDs of the cockpit
 * - INST: A struct containing the instruments of the cockpit
 */
struct Moni {
    std::vector<uint8_t> INFO;
    std::vector<uint8_t> SPOT;
    RLEShape SHAP;
    RLEShape DAMG;
    struct Mfds {
        InfoShape COMM;
        InfoShape AARD;
        InfoShape AGRD;
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

/**
 * @class RSCockpit
 * @brief Class that represents a RealSpace cockpit
 *
 * This class represents a RealSpace cockpit and is used to parse the data
 * from the RealSpace game. The data is parsed from a blob of memory and the
 * resulting data is stored in this class.
 *
 * @warning This class is still in development and is not fully tested.
 *
 * @see RSCockpit.cpp
 */
class RSCockpit {

private:
    std::vector<uint8_t> INFO;
    
    RLEShape VTMP;
    RSImageSet EJEC;
    RSImageSet GUNF;
    RSImageSet GHUD;
    
    struct Chud {
        std::string FILE;
    } CHUD;
    
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
    void parseMONI_MFDS_COMM_INFO(uint8_t* data, size_t size);
    void parseMONI_MFDS_AARD(uint8_t* data, size_t size);
    void parseMONI_MFDS_AARD_INFO(uint8_t* data, size_t size);
    void parseMONI_MFDS_AARD_SHAP(uint8_t* data, size_t size);
    void parseMONI_MFDS_AGRD(uint8_t* data, size_t size);
    void parseMONI_MFDS_AGRD_INFO(uint8_t* data, size_t size);
    void parseMONI_MFDS_AGRD_SHAP(uint8_t* data, size_t size);
    void parseMONI_MFDS_GCAM(uint8_t* data, size_t size);
    void parseMONI_MFDS_GCAM_INFO(uint8_t* data, size_t size);
    void parseMONI_MFDS_GCAM_SHAP(uint8_t* data, size_t size);
    void parseMONI_MFDS_WEAP(uint8_t* data, size_t size);
    void parseMONI_MFDS_WEAP_INFO(uint8_t* data, size_t size);
    void parseMONI_MFDS_WEAP_SHAP(uint8_t* data, size_t size);
    void parseMONI_MFDS_DAMG(uint8_t* data, size_t size);
    void parseMONI_MFDS_DAMG_INFO(uint8_t* data, size_t size);
    void parseMONI_MFDS_DAMG_SHAP(uint8_t* data, size_t size);
    void parseMONI_INST(uint8_t* data, size_t size);
    void parseMONI_INST_RAWS_INFO(uint8_t* data, size_t size);
    void parseMONI_INST_RAWS_SHAP(uint8_t* data, size_t size);
    void parseMONI_INST_RAWS(uint8_t* data, size_t size);
    void parseMONI_INST_ALTI(uint8_t* data, size_t size);
    void parseMONI_INST_ALTI_INFO(uint8_t* data, size_t size);
    void parseMONI_INST_ALTI_SHAP(uint8_t* data, size_t size);
    void parseMONI_INST_AIRS(uint8_t* data, size_t size);
    void parseMONI_INST_AIRS_INFO(uint8_t* data, size_t size);
    void parseMONI_INST_AIRS_SHAP(uint8_t* data, size_t size);
    void parseMONI_INST_MWRN(uint8_t* data, size_t size);
    void parseMONI_INST_MWRN_INFO(uint8_t* data, size_t size);
    void parseMONI_INST_MWRN_SHAP(uint8_t* data, size_t size);


public:

    RSImageSet ARTP;
    RealObjs REAL;
    Moni MONI;
    
    RSCockpit();
    ~RSCockpit();
    void InitFromRam(uint8_t* data, size_t size);
};
