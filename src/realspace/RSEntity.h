//
//  RSEntity.h
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <stdint.h>
#include <vector>

#include "AssetManager.h"
#include "../commons/IFFSaxLexer.h"
#include "../commons/Maths.h"
#include "../commons/Matrix.h"
#include "../commons/Quaternion.h"
#include "../commons/PKWareDecompressor.h"
#include "RSImage.h"

#include "TreArchive.h"
#include "../commons/LZBuffer.h"
#define LOD_LEVEL_MAX 0
#define LOD_LEVEL_MED 1
#define LOD_LEVEL_MIN 2

class RSImageSet;

typedef struct MapVertex {
    Point3D v;

    uint8_t flag;
    uint8_t type;
    uint8_t lowerImageID;
    uint8_t upperImageID;

    float color[4];

} MapVertex;

typedef struct BoudingBox {
    Point3D min;
    Point3D max;
} BoudingBox;

typedef struct UV {
    uint8_t u;
    uint8_t v;
} UV;

typedef struct uvxyEntry {

    uint16_t triangleID;
    uint16_t textureID;
    UV uvs[3];
} uvxyEntry;

typedef struct qmapuvxyEntry {
    uint16_t triangleID;
    uint16_t textureID;
    UV uvs[4];
} qmapuvxyEntry;

typedef struct Triangle {

    uint8_t property;
    uint16_t ids[3];

    uint8_t color;
    uint8_t flags[3];

} Triangle;

typedef struct Quads {

    uint8_t property;
    uint16_t ids[4];

    uint8_t color;
    uint8_t flags[3];

} Quads;

typedef struct Lod {

    uint32_t dist;
    uint16_t numTriangles;
    uint16_t triangleIDs[16336];
} Lod;

typedef struct Attr {
    uint16_t id;
    char type;
    uint8_t props1;
    uint8_t props2;
} Attr;

enum EntityType {
    ground = 1,
    jet = 2,
    ornt = 3,
    swpn = 4,
    aftb = 5,
    missiles = 6,
    bomb = 7,
    tracer = 8,
    explosion = 9,
    object_mobile = 10,
    debris = 11,
    destroyed_object = 12,
    rnwy = 13,
    podr = 14
};
class RSImage;

struct VGAPalette;


struct HPTS {
    uint8_t id;
    int32_t x;
    int32_t y;
    int32_t z;
};

// Chunk JDYN (FORM DYNM d'un JETP) : 28 champs sequentiels, 73 octets disque, encodage
// 24.8 pour les u32 (ReadFixedFloatLE -> valeur SI) et brut pour les u8. Noms decodes par
// retro-ingenierie (strike_commander_re/analysis/PHYSICS.md §1.1, DATA_MODEL.md §6.2).
// L'offset indique est la position dans la struct 0xC5 cote jeu original.
typedef struct JDYN {
    uint32_t fuel_capacity;         // #1  [+0x69]  capacite carburant (kg)
    float sfc;                      // #2  [+0x33]  consommation specifique
    float drag_airbrake;            // #3  [+0x37]  increment de trainee aerofrein
    float drag_gear;               // #4  [+0x3B]  increment de trainee train
    float ground_moment_1;          // #5  [+0x3F]  deceleration de roulage sol
    float ground_moment_2;          // #6  [+0x43]  deceleration de roulage sol (+ aerofrein)
    float rate_limit_dps;           // #7  [+0x47]  limite de variation du taux de controle (deg/s)
    float max_turn_rate_dps;        // #8  [+0x71]  taux de rotation max (deg/s)
    uint8_t stall_alpha_deg;        // #9  [+0x4B]  incidence de decrochage (deg)
    uint8_t wing_incidence_deg;     // #10 [+0x4C]  calage d'aile (deg)
    uint8_t flap_lift_increment_deg;// #11 [+0x4D]  increment d'incidence volets (deg)
    float stall_speed_ms;           // #12 [+0x4E]  vitesse de decrochage (m/s) -- a confirmer
    float max_speed_ms;             // #13 [+0x52]  vitesse max (m/s) -- a confirmer
    uint8_t max_bank_deg;           // #14 [+0x56]  inclinaison max (deg) -- a confirmer
    uint8_t pitch_rate_limit_dps;   // #15 [+0x57]  limite du taux de tangage (deg/s)
    uint8_t pitch_margin_deg;       // #16 [+0x58]  marge de tangage (deg) -- a confirmer
    float control_speed_ms;         // #17 [+0x59]  vitesse d'efficacite des gouvernes (m/s)
    float induced_drag_k;           // #18 [+0x5D]  1/(pi*e*AR)
    float lift_gain;                // #19 [+0x61]  gain de portance (~ Cl_alpha * S)
    uint8_t pitch_stick_gain;       // #20 [+0x65]  borne finale de la consigne de tangage
    uint8_t yaw_authority;          // #21 [+0x66]  gain palonnier -> consigne de lacet
    uint8_t max_g;                  // #22 [+0x67]  facteur de charge max (G)
    int16_t  ai_speed_max;         // #23 [+0x80]  IA : vitesse de poursuite max (defaut 500) -- AI_InterceptSpeedControlLaw (movsx)
    int16_t  ai_speed_min;         // #24 [+0x82]  IA : vitesse de poursuite min / plancher de consigne (defaut 100, movsx)
    int16_t  ai_speed_cruise;      // #25 [+0x84]  IA : vitesse de croisiere/manoeuvre + seuil de distance (defaut 231, movsx)
    float ai_engage_range;      // #26 [+0x86]  IA : seuil de portee/distance d'engagement (defaut 11005)
    uint8_t  ai_unknown_8a;        // #27 [+0x8A]  IA : parametre de decision, consommateur non localise (defaut 3)
    uint8_t  ai_decision_weight;   // #28 [+0x8B]  IA : poids d'un score de decision = ((v-2)*3)/2+3 (defaut 2)
} JDYN;

class RSEntity {

    struct CHLD {
        std::string name;
        int32_t x;
        int32_t y;
        int32_t z;
        std::vector<uint8_t> data;
        RSEntity *objct;
    };
    struct EXPL {
        std::string name;
        int16_t x;
        int16_t y;
        RSEntity *objct;
    };
    struct WDAT {
        uint16_t damage{0};
        uint16_t radius{0};
        uint8_t launch_class{0};     // +0x4A : 8 missile, 9 bombe, 10 panier, 13 canon (WeaponSystem_LaunchFromStation_3E744)
        uint8_t weapon_id{0};
        uint8_t weapon_category{0};
        uint8_t target_domain{0};
        uint8_t weapon_aspec{0};
        uint32_t target_range{0};
        uint8_t tracking_cone{0};
        uint32_t effective_range{0};  
        float fire_interval{0.0f};   // +0x5A : secondes entre deux tirs de la famille
    };
    struct SWPN_DATA {
        std::string weapon_name;
        int32_t weapons_round{0};
        int32_t detection_range{0};
        int32_t effective_range{0};
        uint16_t unknown1{0};
        uint16_t unknown2{0};
        uint16_t unknown3{0};
        uint8_t max_simultaneous_shots{0};
        uint16_t weapons_round2{0};
        int32_t unknown4{0};
        RSEntity *weapon_entity{nullptr};
    };
    struct DYNN_MISS {
        uint32_t turn_degre_per_sec{0};
        uint32_t velovity_m_per_sec{0};
        uint32_t proximity_cm{0};
    };
    struct RADAR_SIGN {
        uint8_t unknown1{0};
        uint8_t unknown2{0};
        uint8_t unknown3{0};
    }; 


public:
    struct WEAPS {
        int nb_weap;
        std::string name;
        RSEntity *objct;
    };

    std::vector<RSImage *> images;
    std::vector<Point3D> vertices;
    std::vector<uvxyEntry> uvs;
    std::vector<qmapuvxyEntry *> qmapuvs;
    std::vector<Lod> lods;
    std::unordered_map<uint16_t, Attr *> attrs;
    std::vector<Triangle> triangles;
    std::vector<Quads *> quads;
    std::vector<WEAPS *> weaps;
    std::vector<HPTS *> hpts;
    std::vector<CHLD *> chld;
    enum Property { SC_TRANSPARENT = 0x02 };
    EXPL *explos{nullptr};
    int32_t thrust_in_newton{0};
    int32_t weight_in_kg{0};
    int32_t drag{0};
    // Chunk THRS, octets restants apres la poussee (non lus jusqu'ici) : voir
    // strike_commander_re/analysis/DATA_MODEL.md §6.2 "Poussee - courbe manette".
    float thrust_mil_fraction{0.0f};      // part de la poussee PC max delivree en MIL (cran 5/10)
    float thrust_ref_alt_fraction{1.0f};  // part de poussee restante a l'altitude de reference (11000 m)
    uint8_t thrust_cutoff_alt_raw{0};     // altitude de coupure = valeur * 100 m
    // Chunk STBL (jamais lu jusqu'ici) : coefficient d'autorite de tangage/lacet
    // utilise par l'asservissement d'attitude (q' = q * stability_gain / 100).
    float stability_gain{0.0f};
    float parasite_drag{1.0f};  // chunk ATMO (u32 24.8) : multiplie la trainee parasite
    // Chunk TOFF (TakeoffBehavior_Start_11D03), valeurs par defaut de l'original
    int16_t takeoff_roll_accel{20};     // m/s2
    int16_t takeoff_rotate_speed{150};  // m/s
    int16_t takeoff_climb_pitch{30};    // deg
    int16_t takeoff_pitch_gain{8};
    // Chunk LAND (LandingBehavior_Start_75746)
    int16_t landing_speed{200};         // m/s
    int32_t landing_unused{0x64};
    int16_t landing_aim_height{6};      // m
    int16_t landing_pitch_steps{20};
    RADAR_SIGN *radar_signature{nullptr};
    uint8_t target_type{0};
    uint8_t combat_class{2};           // chunk JINF, modele +0x52 (9 et plus : chasseur)
    uint8_t bomb_guided{0};            // chunk DATA des BOMB, modele +0x5E
    int16_t bomb_lock_cone_rate{0};    // modele +0x61, deg/s
    float decoy_lifetime{0.0f};        // secondes
    uint8_t health{0};
    WDAT *wdat{nullptr};
    DYNN_MISS *dynn_miss{nullptr};
    SWPN_DATA *swpn_data{nullptr};
    RSEntity *destroyed_object{nullptr};
    std::string destroyed_object_name;
    std::string cockpit_name;
    bool gravity{false};
    float wing_area{0};
    JDYN *jdyn{nullptr};
    uint16_t life{0};
    std::unordered_map<std::string, std::unordered_map<std::string, uint16_t>> sysm;
    // For rendering
    Point3D position;
    Quaternion orientation;
    std::vector<RSImageSet *> images_set;
    std::vector<Texture *> animations;
    // Has the entity been sent to te GPU and is ready to be renderer.

    bool prepared{false};
    AssetManager &assetsManager = AssetManager::instance();
    std::string name;
    ~RSEntity();

    void InitFromRAM(uint8_t *data, size_t size, std::string name);
    size_t NumImages(void);
    size_t NumVertice(void);
    size_t NumUVs(void);
    size_t NumLods(void);
    size_t NumTriangles(void);
    inline bool IsPrepared(void) { return this->prepared; }
    BoudingBox *GetBoudingBpx(void);
    EntityType entity_type;
    void parseREAL_OBJT_JETP(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_WEAP(uint8_t *data, size_t size);
    BoudingBox bb;
private:
    void CalcBoundingBox(void);
    void calcWingArea(void);
    void AddImage(RSImage *image);
    void AddVertex(Point3D *vertex);
    void AddUV(uvxyEntry *uv);
    void AddLod(Lod *lod);
    void AddTriangle(Triangle *triangle);

    void parseREAL(uint8_t *data, size_t size);
    void parseREAL_INFO(uint8_t *data, size_t size);
    void parseREAL_OBJT(uint8_t *data, size_t size);
    void parseREAL_OBJT_INFO(uint8_t *data, size_t size);
    void parseREAL_OBJT_GRND(uint8_t *data, size_t size);
    void parseREAL_OBJT_SWPN(uint8_t *data, size_t size);
    void parseREAL_OBJT_RNWY(uint8_t *data, size_t size);
    void parseREAL_OBJT_ORNT(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS(uint8_t *data, size_t size);
    void parseREAL_OBJT_BOMB(uint8_t *data, size_t size);
    void parseREAL_OBJT_BOMB_DATA(uint8_t *data, size_t size);
    void parseREAL_OBJT_DECY(uint8_t *data, size_t size);
    void parseREAL_OBJT_DECY_DATA(uint8_t *data, size_t size);
    void parseREAL_OBJT_TRCR(uint8_t *data, size_t size); 
    void parseREAL_OBJT_AFTB(uint8_t *data, size_t size);
    void parseREAL_OBJT_EXPL(uint8_t *data, size_t size);
    void parseREAL_OBJT_SMKG(uint8_t *data, size_t size);
    void parseREAL_OBJT_OMOB(uint8_t *data, size_t size);
    void parseREAL_OBJT_DEBR(uint8_t *data, size_t size);
    void parseREAL_OBJT_PODR(uint8_t *data, size_t size);
    void parseREAL_OBJT_AFTB_APPR(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_EXPL(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_SIGN(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_TRGT(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_SMOK(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_DAMG(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_WDAT(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_DATA(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_DYNM(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_DYNM_MISS(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_DYNM_ATMO(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_DYNM_AGRV(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS_DYNM_GRAV(uint8_t *data, size_t size);
    void parseREAL_OBJT_SWPN_DYNM(uint8_t *data, size_t size);
    void parseREAL_OBJT_SWPN_DATA(uint8_t *data, size_t size);
    void parseREAL_OBJT_SWPN_ALGN(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_INFO(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_EXPL(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DEBR(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DEST(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_SMOK(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_CHLD(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_JINF(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DAMG(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_WEAP_DAMG_SYSM(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_EJEC(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_SIGN(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_TRGT(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_CTRL(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_CKPT(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_TOFF(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_LAND(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM_DYNM(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM_ORDY(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM_STBL(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM_ATMO(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM_GRAV(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM_THRS(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_DYNM_JDYN(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_WEAP_INFO(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_WEAP_DCOY(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_WEAP_WPNS(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_WEAP_HPTS(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP_WEAP_DAMG(uint8_t *data, size_t size);
    void parseREAL_OBJT_EXTE(uint8_t *data, size_t size);
    void parseREAL_APPR(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_INFO(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_VERT(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_DETA(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_DETA_LVLX(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_ATTR(uint8_t *data, size_t size);
    void parseREAL_OBJT_PODR_DATA(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_LNTH(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_VTRI(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_FACE(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_TXMS(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_TXMS_INFO(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_TXMS_TXMP(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_TXMS_TXMA(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_UVXY(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_MAPS(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_QUAD(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_QUAD_FACE(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_QUAD_MAPS(uint8_t *data, size_t size);

    void parseREAL_APPR_ANIM(uint8_t *data, size_t size);
    void parseREAL_APPR_ANIM_INFO(uint8_t *data, size_t size);
    void parseREAL_APPR_ANIM_SEQU(uint8_t *data, size_t size);
    void parseREAL_APPR_ANIM_SHAP(uint8_t *data, size_t size);
};
