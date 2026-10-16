//
//  RSWorld.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/12/2024.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//
#pragma once

#include "../commons/IFFSaxLexer.h"
#include "RSCameraCOMP.h"

// Sous-chunk CAMR (caméra simple). Layout confirmé sur les constructeurs ASM
// (sub_85790 CHAS, sub_82CDC VICT, sub_85ACB CKPT ...) :
//   +0x00 char[8] nom
//   +0x08 slot look-at : 14 o (CHAS) ou 2 o (autres) — nuls sur les WRLD livrés
//   +..   char[8] sujet ("PLAYER")   [CKPT insère ici char[8] cockpitArt]
//   +..   u32     farClip   (le moteur applique <<8 -> 24.8)
//   +..   u16     fov       (<<8)
//   +..   u32     nearClip / flags   (le moteur lit UN dword ici, pas pad+octet)
//   +..   u16 x4  rect viewport : x, y, w, h
//   +..   (VICT/WEAP seulement) payload étendue -> params
// Aucune coordonnée d'offset : la position de la caméra externe est calculée
// côté moteur (sous-composant de corps rigide attaché à l'avion).
//
// Codes de type reconnus par Cinematic_LoadCameraDef (ASM) — RSCameraDef::typeCode
// ET CameraViewRequest::camera_type (même vocabulaire, du fichier jusqu'à la
// requête d'activation : pas de traduction intermédiaire, cf. SCCameraDirector).
enum RSCameraType : uint8_t {
    RSCAM_NONE    = 0,     // CameraViewRequest : aucune caméra CAMR demandée
    RSCAM_CHAS    = 3,     // chase / poursuite
    RSCAM_CKPT    = 4,     // cockpit
    RSCAM_CONT    = 6,
    RSCAM_VICT    = 7,     // victime (avion touché)
    RSCAM_ROTA    = 8,     // orbitale (recul + orbite pilotable par le joueur)
    RSCAM_TARG    = 9,     // cible verrouillée
    RSCAM_WEAP    = 0x0B,
    RSCAM_COMP    = 0x13,  // séquence scriptée (STARTCAM/TAKEOFF/LANDING/AUTOPILT...) ; plusieurs entrées possibles, distinguées par nom
    RSCAM_UNKNOWN = 0x14,  // tag non reconnu par Cinematic_LoadCameraDef (ASM) ; défaut de RSCameraDef::typeCode
};

struct RSCameraDef {
    std::string          name;                    // "CHASECAM"/"COCKPIT"/"VICTIM"/"AUTOTRAC"/"WEAPON"/"ROTATCAM"
    RSCameraType         typeCode = RSCAM_UNKNOWN;
    std::string          subject;      // entité porteuse, ex. "PLAYER"
    std::string          cockpitArt;   // CKPT seul, ex. "F16-CKPT"
    float              farClip  = 0; // 50000
    float                fov      = 0; // 40.0 (converti 8.8 -> degrés au décodage, cf. ByteStream::ReadFixedFloat16LE)
    float                nearClip = 0; // dword ; contient ~10
    uint16_t             viewX = 0;
    uint16_t             viewY = 0;
    uint16_t             viewW = 0;
    uint16_t             viewH = 0;  // rect de rendu (0, 0, 319, 199)
    // VICT/WEAP : queue étendue après l'en-tête commun (CAMERA_SYSTEM.md
    // §2.4). Chaque i32 brut est soit un offset 24.8 relatif au sujet
    // (converti /256 au décodage), soit un petit param séparé (angle /
    // marqueur, repéré par un octet bas non nul dans le fichier — PAS du
    // 24.8, gardé tel quel). Les deux cas sont résolus ici, au décodage
    // (cf. readSimpleCamera dans RSWorld.cpp) : `params` ne contient plus
    // que des valeurs directement utilisables, jamais de brut 24.8.
    std::vector<float>   params;
};



class RSWorld {
private:
    void parseWRLD(uint8_t *data, size_t size);
    void parseWRLD_INFO(uint8_t *data, size_t size);
    void parseWRLD_HORZ(uint8_t *data, size_t size);
    void parseWRLD_WTCH(uint8_t *data, size_t size);
    void parseWRLD_PALT(uint8_t *data, size_t size);
    void parseWRLD_TERA(uint8_t *data, size_t size);
    void parseWRLD_SKYS(uint8_t *data, size_t size);
    void parseWRLD_GLNT(uint8_t *data, size_t size);
    void parseWRLD_SMOK(uint8_t *data, size_t size);
    void parseWRLD_LGHT(uint8_t *data, size_t size);
    void parseWRLD_CAMR(uint8_t *data, size_t size);
    void parseWRLD_CAMR_STRT(uint8_t *data, size_t size);
    void parseWRLD_CAMR_CHAS(uint8_t *data, size_t size);
    void parseWRLD_CAMR_CKPT(uint8_t *data, size_t size);
    void parseWRLD_CAMR_VICT(uint8_t *data, size_t size);
    void parseWRLD_CAMR_TARG(uint8_t *data, size_t size);
    void parseWRLD_CAMR_WEAP(uint8_t *data, size_t size);
    void parseWRLD_CAMR_ROTA(uint8_t *data, size_t size);
    void parseWRLD_CAMR_COMP(uint8_t *data, size_t size);

public:
    std::string tera;
    std::string cameraSetName;
    bool has_light{false};
    uint16_t light_ambient_weight{0};
    uint16_t light_percent{0};
    uint16_t light_weight{0};
    int16_t light_direction[3]{0, 0, 0};
    std::vector<RSCameraDef> cameras;
    std::vector<RSCameraSequence> cameraSequences;
    RSWorld();
    ~RSWorld();
    void InitFromRAM(uint8_t *data, size_t size);
};
