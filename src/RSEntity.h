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
#include "IFFSaxLexer.h"
#include "IffLexer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "RSImage.h"
#include "TreArchive.h"
#include "LZBuffer.h"
#define LOD_LEVEL_MAX 0
#define LOD_LEVEL_MED 1
#define LOD_LEVEL_MIN 2

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

    uint8_t triangleID;
    uint8_t textureID;
    UV uvs[3];
} uvxyEntry;

typedef struct Triangle {

    uint8_t property;
    uint8_t ids[3];

    uint8_t color;
    uint8_t flags[3];

} Triangle;

typedef struct Lod {

    uint32_t dist;
    uint16_t numTriangles;
    uint16_t triangleIDs[256];
} Lod;
enum EntityType {
    ground = 1,
    jet = 2,
    ornt = 3,
};
class RSImage;

struct VGAPalette;


struct HPTS {
    uint8_t id;
    int32_t x;
    int32_t y;
    int32_t z;
};



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
        uint8_t unknown1{0};
        uint8_t weapon_id{0};
        uint8_t weapon_category{0};
        uint8_t radar_type{0};
        uint8_t weapon_aspec{0};
        uint32_t target_range{0};
        uint8_t tracking_cone{0};
        uint32_t effective_range{0};  
        uint8_t unknown6{0};
        uint8_t unknown7{0};
        uint8_t unknown8{0};
    };
    struct DYNN_MISS {
        uint32_t turn_degre_per_sec{0};
        uint32_t velovity_m_per_sec{0};
        uint32_t proximity_cm{0};
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
    std::vector<Lod> lods;
    std::vector<Triangle> triangles;
    std::vector<WEAPS *> weaps;
    std::vector<HPTS *> hpts;
    std::vector<CHLD *> chld;
    enum Property { SC_TRANSPARENT = 0x02 };
    EXPL *explos{nullptr};
    int32_t thrust_in_newton{0};
    int32_t weight_in_kg{0};
    WDAT *wdat{nullptr};
    DYNN_MISS *dynn_miss{nullptr};
    bool gravity{false};

    uint16_t life{0};
    std::map<std::string, std::map<std::string, uint16_t>> sysm;
    // For rendering
    Point3D position;
    Quaternion orientation;

    // Has the entity been sent to te GPU and is ready to be renderer.

    bool prepared{false};
    AssetManager *assetsManager{nullptr};

    RSEntity(AssetManager *amana);
    ~RSEntity();

    void InitFromRAM(uint8_t *data, size_t size);
    size_t NumImages(void);
    size_t NumVertice(void);
    size_t NumUVs(void);
    size_t NumLods(void);
    size_t NumTriangles(void);
    inline bool IsPrepared(void) { return this->prepared; }
    BoudingBox *GetBoudingBpx(void);
    EntityType entity_type;
private:
    BoudingBox bb;
    void CalcBoundingBox(void);

    void AddImage(RSImage *image);
    void AddVertex(Point3D *vertex);
    void AddUV(uvxyEntry *uv);
    void AddLod(Lod *lod);
    void AddTriangle(Triangle *triangle);

    void parseREAL(uint8_t *data, size_t size);
    void parseREAL_OBJT(uint8_t *data, size_t size);
    void parseREAL_OBJT_INFO(uint8_t *data, size_t size);
    void parseREAL_OBJT_JETP(uint8_t *data, size_t size);
    void parseREAL_OBJT_GRND(uint8_t *data, size_t size);
    void parseREAL_OBJT_ORNT(uint8_t *data, size_t size);
    void parseREAL_OBJT_MISS(uint8_t *data, size_t size);
    void parseREAL_OBJT_AFTB(uint8_t *data, size_t size);
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
    void parseREAL_OBJT_JETP_WEAP(uint8_t *data, size_t size);
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
    void parseREAL_APPR_POLY_TRIS(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_VTRI(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_TXMS(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_TXMS_INFO(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_TXMS_TXMP(uint8_t *data, size_t size);
    void parseREAL_APPR_POLY_TRIS_UVXY(uint8_t *data, size_t size);
};
