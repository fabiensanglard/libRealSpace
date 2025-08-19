//
//  RSMap.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSArea.h"
#include "../engine/SCRenderer.h"

extern SCRenderer Renderer;
// ...existing code...
#include <cmath>
// Helpers HSV pour varier S/V autour de la teinte d’origine
static inline void rgbToHsv_(float r, float g, float b, float& h, float& s, float& v) {
    float mx = (std::max)((std::max)(r, g), b);
    float mn = (std::min)((std::min)(r, g), b);
    v = mx;
    float d = mx - mn;
    s = (mx == 0.0f) ? 0.0f : d / mx;
    if (d == 0.0f) { h = 0.0f; return; }
    if (mx == r)      h = fmodf(((g - b) / d), 6.0f);
    else if (mx == g) h = ((b - r) / d) + 2.0f;
    else              h = ((r - g) / d) + 4.0f;
    h /= 6.0f; if (h < 0.0f) h += 1.0f;
}
static inline void hsvToRgb_(float h, float s, float v, float& r, float& g, float& b) {
    if (s <= 0.0f) { r = g = b = v; return; }
    h = fmodf(h, 1.0f); if (h < 0.0f) h += 1.0f;
    float f = h * 6.0f; int i = (int)floorf(f);
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * (f - i));
    float t = v * (1.0f - s * (1.0f - (f - i)));
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
}
static inline float smooth01_(float t){ return t * t * (3.0f - 2.0f * t); }
static inline float clamp01_(float x){ return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }
static inline float noise2_(float x, float z, float seed){
    float n = sinf(x * 12.9898f + z * 78.233f + seed) * 43758.5453f;
    return n - floorf(n);
}

void RSArea::BuildSkirts() {
    skirts_.tris.clear();
    if (BLOCKS_PER_MAP == 0) return;

    const int lod = BLOCK_LOD_MAX;
    const int N = BLOCK_PER_MAP_SIDE;

    // Bord du monde (cohérent avec renderWorldSkyAndGround)
    const float worldHalf = (BLOCK_WIDTH * BLOCK_PER_MAP_SIDE) * 2.0f;
    const float worldMinX = -worldHalf, worldMaxX = worldHalf;
    const float worldMinZ = -worldHalf, worldMaxZ = worldHalf;

    // Délais pour éviter micro-jours
    const float epsOut = 0.02f; // vers l’extérieur (bord du monde)
    const uint32_t s0 = blocks[lod][0].sideSize;
    const float gridStep = BLOCK_WIDTH / (float)(s0 - 1);
    const float epsIn = gridStep * 0.2f; // 20% d’une cellule vers l’intérieur

    // Couleur moyenne d’un côté de bloc
    enum class Side { Top, Bottom, Left, Right };
    auto sideColorAvg = [&](AreaBlock* blk, Side s, float out[3]) {
        out[0]=out[1]=out[2]=0.0f;
        uint32_t n=0, sz = blk->sideSize;
        switch (s) {
            case Side::Top:    for (uint32_t x=0; x<sz; ++x){ auto* p=blk->GetVertice(x,0);       out[0]+=p->color[0]; out[1]+=p->color[1]; out[2]+=p->color[2]; ++n; } break;
            case Side::Bottom: for (uint32_t x=0; x<sz; ++x){ auto* p=blk->GetVertice(x,sz-1);    out[0]+=p->color[0]; out[1]+=p->color[1]; out[2]+=p->color[2]; ++n; } break;
            case Side::Left:   for (uint32_t y=0; y<sz; ++y){ auto* p=blk->GetVertice(0,y);       out[0]+=p->color[0]; out[1]+=p->color[1]; out[2]+=p->color[2]; ++n; } break;
            case Side::Right:  for (uint32_t y=0; y<sz; ++y){ auto* p=blk->GetVertice(sz-1,y);    out[0]+=p->color[0]; out[1]+=p->color[1]; out[2]+=p->color[2]; ++n; } break;
        }
        if (n>0){ out[0]/=n; out[1]/=n; out[2]/=n; }
    };
    auto avg2 = [](const float a[3], const float b[3], float out[3]){
        out[0]=(a[0]+b[0])*0.5f; out[1]=(a[1]+b[1])*0.5f; out[2]=(a[2]+b[2])*0.5f;
    };

    struct RingVert {
        // inner point (à la carte, décalé epsIn)
        float ix, iy, iz;
        // outer point (projeté au bord du monde ±epsOut)
        float ox, oy, oz;
        // couleur
        float r, g, b;
    };
    std::vector<RingVert> ring;
    ring.reserve(N * s0 * 4 + 4);

    // 1) Construire une boucle “inner/outer” continue, sens horaire:
    //    - Top (x croissant, z=+epsIn) -> coin TR
    //    - Right (y croissant, x=-epsIn) -> coin BR
    //    - Bottom (x décroissant, z=-epsIn) -> coin BL
    //    - Left (y décroissant, x=+epsIn) -> coin TL
    //
    // À chaque transition de côté, insérer un VERTEX DE COIN explicite
    // (inner au coin carte avec epsIn sur X et Z, outer au coin monde).
    auto pushTop = [&](){
        for (int bx=0; bx<N; ++bx) {
            AreaBlock* blk = &blocks[lod][0 * N + bx];
            uint32_t s = blk->sideSize;
            float col[3]; sideColorAvg(blk, Side::Top, col);
            uint32_t startX = (bx==0)?0u:1u; // éviter le doublon aux joints de blocs
            for (uint32_t x=startX; x<s; ++x) {
                auto* p = blk->GetVertice(x, 0);
                RingVert v{
                    p->v.x, p->v.y, p->v.z + epsIn,     // inner
                    p->v.x, p->v.y, worldMinZ - epsOut, // outer
                    col[0], col[1], col[2]
                };
                ring.push_back(v);
            }
        }
        // Coin TR: bloc [0][N-1], sommet (s-1,0)
        AreaBlock* trb = &blocks[lod][N-1];
        uint32_t s = trb->sideSize;
        MapVertex* pc = trb->GetVertice(s-1, 0);
        float topC[3];   sideColorAvg(trb, Side::Top,   topC);
        float rightC[3]; sideColorAvg(trb, Side::Right, rightC);
        float col[3]; avg2(topC, rightC, col);
        ring.push_back(RingVert{
            pc->v.x - epsIn, pc->v.y, pc->v.z + epsIn,    // inner coin
            worldMaxX + epsOut, pc->v.y, worldMinZ - epsOut, // outer coin (coin monde)
            col[0], col[1], col[2]
        });
    };
    auto pushRight = [&](){
        for (int by=0; by<N; ++by) {
            AreaBlock* blk = &blocks[lod][by * N + (N-1)];
            uint32_t s = blk->sideSize;
            float col[3]; sideColorAvg(blk, Side::Right, col);
            uint32_t startY = (by==0)?1u:1u; // on saute (0) car le coin TR a déjà été inséré
            for (uint32_t y=startY; y<s; ++y) {
                auto* p = blk->GetVertice(s-1, y);
                RingVert v{
                    p->v.x - epsIn, p->v.y, p->v.z,       // inner
                    worldMaxX + epsOut, p->v.y, p->v.z,   // outer
                    col[0], col[1], col[2]
                };
                ring.push_back(v);
            }
        }
        // Coin BR
        AreaBlock* brb = &blocks[lod][(N-1)*N + (N-1)];
        uint32_t s = brb->sideSize;
        MapVertex* pc = brb->GetVertice(s-1, s-1);
        float bottomC[3]; sideColorAvg(brb, Side::Bottom, bottomC);
        float rightC[3];  sideColorAvg(brb, Side::Right,  rightC);
        float col[3]; avg2(bottomC, rightC, col);
        ring.push_back(RingVert{
            pc->v.x - epsIn, pc->v.y, pc->v.z - epsIn,
            worldMaxX + epsOut, pc->v.y, worldMaxZ + epsOut,
            col[0], col[1], col[2]
        });
    };
    auto pushBottom = [&](){
        for (int bx=N-1; bx>=0; --bx) {
            AreaBlock* blk = &blocks[lod][(N-1) * N + bx];
            uint32_t s = blk->sideSize;
            float col[3]; sideColorAvg(blk, Side::Bottom, col);
            uint32_t endX = (bx==0)?0u:0u; // on ira jusqu’à 0 inclus
            uint32_t startX = (bx==N-1)?(s-2):0u; // éviter doublon coin BR
            for (int x=(int)startX; x>= (int)endX; --x) {
                auto* p = blk->GetVertice((uint32_t)x, s-1);
                RingVert v{
                    p->v.x, p->v.y, p->v.z - epsIn,       // inner
                    p->v.x, p->v.y, worldMaxZ + epsOut,   // outer
                    col[0], col[1], col[2]
                };
                ring.push_back(v);
            }
        }
        // Coin BL
        AreaBlock* blb = &blocks[lod][(N-1)*N + 0];
        uint32_t s = blb->sideSize;
        MapVertex* pc = blb->GetVertice(0, s-1);
        float bottomC[3]; sideColorAvg(blb, Side::Bottom, bottomC);
        float leftC[3];   sideColorAvg(blb, Side::Left,   leftC);
        float col[3]; avg2(bottomC, leftC, col);
        ring.push_back(RingVert{
            pc->v.x + epsIn, pc->v.y, pc->v.z - epsIn,
            worldMinX - epsOut, pc->v.y, worldMaxZ + epsOut,
            col[0], col[1], col[2]
        });
    };
    auto pushLeft = [&](){
        for (int by=N-1; by>=0; --by) {
            AreaBlock* blk = &blocks[lod][by * N + 0];
            uint32_t s = blk->sideSize;
            float col[3]; sideColorAvg(blk, Side::Left, col);
            uint32_t endY = (by==0)?0u:0u;
            uint32_t startY = (by==N-1)?(s-2):0u; // éviter doublon coin BL
            for (int y=(int)startY; y>= (int)endY; --y) {
                auto* p = blk->GetVertice(0, (uint32_t)y);
                RingVert v{
                    p->v.x + epsIn, p->v.y, p->v.z,       // inner
                    worldMinX - epsOut, p->v.y, p->v.z,   // outer
                    col[0], col[1], col[2]
                };
                ring.push_back(v);
            }
        }
        // Coin TL (dernier coin avant fermeture)
        AreaBlock* tlb = &blocks[lod][0];
        uint32_t s = tlb->sideSize;
        MapVertex* pc = tlb->GetVertice(0, 0);
        float topC[3];  sideColorAvg(tlb, Side::Top,  topC);
        float leftC[3]; sideColorAvg(tlb, Side::Left, leftC);
        float col[3]; avg2(topC, leftC, col);
        ring.push_back(RingVert{
            pc->v.x + epsIn, pc->v.y, pc->v.z + epsIn,
            worldMinX - epsOut, pc->v.y, worldMinZ - epsOut,
            col[0], col[1], col[2]
        });
    };

    ring.clear();
    ring.reserve(N * s0 * 4 + 4);
    pushTop();
    pushRight();
    pushBottom();
    pushLeft();
    if (ring.size() < 4) return;

    // 2) Trianguler la bande entre inner et outer (quad strip fermé)
    auto pushTri = [&](const RingVert& A, const RingVert& B, const RingVert& C) {
        skirts_.tris.push_back({{
            {A.ix, A.iy, A.iz, A.r, A.g, A.b},
            {B.ix, B.iy, B.iz, B.r, B.g, B.b},
            {C.ox, C.oy, C.oz, C.r, C.g, C.b}
        }});
    };
    auto pushTri2 = [&](const RingVert& A, const RingVert& C, const RingVert& D) {
        skirts_.tris.push_back({{
            {A.ix, A.iy, A.iz, A.r, A.g, A.b},
            {C.ox, C.oy, C.oz, C.r, C.g, C.b},
            {D.ox, D.oy, D.oz, D.r, D.g, D.b}
        }});
    };

    const size_t M = ring.size();
    for (size_t i=0; i<M; ++i) {
        const size_t i1 = (i+1) % M;
        const RingVert& A = ring[i];
        const RingVert& B = ring[i1];
        // Deux triangles: [inner A, inner B, outer B] et [inner A, outer B, outer A]
        pushTri(A, B, B);
        pushTri2(A, B, A);
    }
}
RSArea::RSArea() {

}
RSArea::RSArea(AssetManager *amanager) {
    this->assetsManager = amanager;
}
RSArea::~RSArea() {
    //delete archive;

    while (!textures.empty()) {
        RSMapTextureSet *set = textures.back();
        textures.pop_back();
        delete set;
    }
}

#define OBJ_ENTRY_SIZE 0x46
#define OBJ_ENTRY_NUM_OBJECTS_FIELD 0x2
void RSArea::ParseObjects() {
    /*
        The OBJ file seems to have a pattern:
            It is a PAK archive
            For each entry in the PAK
            short: num records
                X records of length 0x46
            Record format :
                7 bytes for name
                1 bytes unknown (sometimes 0x00 sometimes 0xC3
                4 bytes unknown
                12 bytes for coordinates ?
    */
    PakEntry *objectsFilesLocation = archive->GetEntry(5);
    objectsFilesLocation = archive->GetEntry(5);

    PakArchive objectFiles;
    objectFiles.InitFromRAM("PAK Objects from RAM", objectsFilesLocation->data, objectsFilesLocation->size);

    for (size_t i = 0; i < objectFiles.GetNumEntries(); i++) {
        PakEntry *entry = objectFiles.GetEntry(i);
        if (entry->size == 0)
            continue;
        entry = objectFiles.GetEntry(i);
        ByteStream sizeGetter(entry->data);
        uint16_t numObjs = sizeGetter.ReadUShort();

        for (int j = 0; j < numObjs; j++) {

            ByteStream reader(entry->data + OBJ_ENTRY_NUM_OBJECTS_FIELD + OBJ_ENTRY_SIZE * j);
            MapObject mapObject;

            for (int k = 0; k < 8; k++)
                mapObject.name[k] = reader.ReadByte();
            mapObject.name[8] = 0;

            uint8_t unknown09 = reader.ReadByte();
            uint8_t unknown10 = reader.ReadByte();
            uint8_t unknown11 = reader.ReadByte();
            uint8_t unknown12 = reader.ReadByte();
            uint8_t unknown13 = reader.ReadByte();

            for (int k = 0; k < 8; k++)
                mapObject.destroyedName[k] = reader.ReadByte();
            mapObject.destroyedName[8] = 0;

            uint8_t unknown14 = reader.ReadByte();
            uint8_t unknown15 = reader.ReadByte();
            mapObject.position.x = reader.ReadInt24LE() * BLOCK_COORD_SCALE;
            mapObject.position.z = -reader.ReadInt24LE() * BLOCK_COORD_SCALE;
            mapObject.position.y = reader.ReadInt24LE() * HEIGH_MAP_SCALE;

            uint8_t unknowns[3];

            for (int k = 0; k < 2; k++)
                unknowns[k] = reader.ReadByte();

            mapObject.progs_id.push_back(reader.ReadUShort());
            mapObject.progs_id.push_back(reader.ReadUShort());
            mapObject.progs_id.push_back(reader.ReadUShort());
            mapObject.progs_id.push_back(reader.ReadUShort());
            mapObject.entity = nullptr;
            objects.push_back(mapObject);
        }
    }
}

void RSArea::ParseTriFile(PakEntry *entry) {
    if (entry->size > 0) {
        PakArchive triFiles;

        AreaOverlay overTheMapIsTheRunway;
        size_t read = 0;
        ByteStream stream(entry->data);
        int numvertice = stream.ReadShort();
        read += 2;
        int nbpoly = stream.ReadShort();
        read += 2;
        stream.MoveForward(2);
        read += 4;
        AoVPoints *vertices = new AoVPoints[numvertice];
        overTheMapIsTheRunway.lx = 0;
        overTheMapIsTheRunway.ly = 0;
        overTheMapIsTheRunway.hx = 0;
        overTheMapIsTheRunway.hy = 0;

        for (int i = 0; i < numvertice; i++) {
            AoVPoints *v = &vertices[i];
            int32_t coo;
            v->u0 = stream.ReadByte();
            v->u1 = stream.ReadByte();
            v->u2 = stream.ReadByte();
            coo = stream.ReadInt24LE();
            read += 4;
            v->x = coo * (int)(BLOCK_COORD_SCALE);
            coo = stream.ReadInt24LE();
            read += 4;
            v->z = coo * (int)(BLOCK_COORD_SCALE);
            coo = stream.ReadShort();
            read += 2;
            v->y = coo * (int) HEIGH_MAP_SCALE;
            overTheMapIsTheRunway.lx = ((overTheMapIsTheRunway.lx == 0) && (i == 0)) ? v->x : overTheMapIsTheRunway.lx;
            overTheMapIsTheRunway.hx = ((overTheMapIsTheRunway.hx == 0) && (i == 0)) ? v->x : overTheMapIsTheRunway.hx;
            overTheMapIsTheRunway.ly = ((overTheMapIsTheRunway.ly == 0) && (i == 0)) ? v->z : overTheMapIsTheRunway.ly;
            overTheMapIsTheRunway.hy = ((overTheMapIsTheRunway.hy == 0) && (i == 0)) ? v->z : overTheMapIsTheRunway.hy;

            overTheMapIsTheRunway.lx = v->x < overTheMapIsTheRunway.lx ? v->x : overTheMapIsTheRunway.lx;
            overTheMapIsTheRunway.ly = v->z < overTheMapIsTheRunway.ly ? v->z : overTheMapIsTheRunway.ly;
            overTheMapIsTheRunway.hx = v->x > overTheMapIsTheRunway.hx ? v->x : overTheMapIsTheRunway.hx;
            overTheMapIsTheRunway.hy = v->z > overTheMapIsTheRunway.hy ? v->z : overTheMapIsTheRunway.hy;
        }
        overTheMapIsTheRunway.vertices = vertices;

        short cpt = 0;
        overTheMapIsTheRunway.nbTriangles = 0;
        for (int i = 0; i < nbpoly; i++) {

            AreaOverlayTriangles aot;

            aot.u0 = stream.ReadByte();
            aot.u1 = stream.ReadByte();
            read += 2;

            aot.u7 = stream.ReadByte();
            aot.verticesIdx[0] = stream.ReadByte();
            read += 2;
            aot.u8 = stream.ReadByte();
            aot.verticesIdx[1] = stream.ReadByte();
            read += 2;
            aot.u9 = stream.ReadByte();
            aot.verticesIdx[2] = stream.ReadByte();
            read += 2;

            aot.u2 = stream.ReadByte();
            aot.u3 = stream.ReadByte();
            aot.color = stream.ReadByte();
            aot.u4 = stream.ReadByte();
            aot.u5 = stream.ReadByte();
            aot.u6 = stream.ReadByte();
            aot.u10 = stream.ReadByte();
            aot.u11 = stream.ReadByte();
            read += 8;

            overTheMapIsTheRunway.trianles[overTheMapIsTheRunway.nbTriangles++] = aot;
        }
        // TODO figure out what is the remaining data is used for.
        stream.MoveForward(entry->size - read);
        objectOverlay.push_back(overTheMapIsTheRunway);
    }
}

void RSArea::ParseTrigo() {

    PakEntry *entry;
    entry = archive->GetEntry(4);

    PakArchive triFiles;
    triFiles.InitFromRAM(".TRI", entry->data, entry->size);

    for (size_t i = 0; i < triFiles.GetNumEntries(); i++) {

        PakEntry *entry = triFiles.GetEntry(i);
        ParseTriFile(entry);
    }
}

#define LAND_TYPE_SEA 0
#define LAND_TYPE_DESERT 1
#define LAND_TYPE_GROUND 2
#define LAND_TYPE_SAVANNAH 3
#define LAND_TYPE_TAIGA 4
#define LAND_TYPE_TUNDRA 5
#define LAND_TYPE_SNOW 6

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)                                                                                             \
    (byte & 0x80 ? 1 : 0), (byte & 0x40 ? 1 : 0), (byte & 0x20 ? 1 : 0), (byte & 0x10 ? 1 : 0), (byte & 0x08 ? 1 : 0), \
        (byte & 0x04 ? 1 : 0), (byte & 0x02 ? 1 : 0), (byte & 0x01 ? 1 : 0)

// A lod features
// A block features either 25, 100 or 400 vertex
void RSArea::ParseBlocks(size_t lod, PakEntry *entry, size_t blockDim) {

    PakArchive blocksPAL;
    blocksPAL.InitFromRAM("BLOCKS", entry->data, entry->size);
    for (size_t i = 0; i < blocksPAL.GetNumEntries(); i++) { // Iterate over the BLOCKS_PER_MAP block entries.

        // SRC Asset Block
        PakEntry *blockEntry = blocksPAL.GetEntry(i);

        // DST custom block
        AreaBlock *block = &blocks[lod][i];

        block->sideSize = static_cast<int32_t>(blockDim);

        ByteStream vertStream(blockEntry->data);
        for (size_t vertexID = 0; vertexID < blockDim * blockDim; vertexID++) {

            MapVertex *vertex = &block->vertice[vertexID];

            int16_t height;
            height = vertStream.ReadShort();
            
            vertex->flag = vertStream.ReadByte();
            vertex->type = vertStream.ReadByte();

            uint8_t paletteColor = 0;

            // Hardcoding the values since I have no idea where those
            // are coming from. Maybe it was hard-coded in STRIKE.EXE ?
            switch (vertex->type) {
            case LAND_TYPE_SEA:
                paletteColor = 0xA;
                break;
            case LAND_TYPE_DESERT:
                paletteColor = 0x3;
                break;
            case LAND_TYPE_GROUND:
                paletteColor = 0x7;
                break;
            case LAND_TYPE_SAVANNAH:
                paletteColor = 0x5;
                break;
            case LAND_TYPE_TAIGA:
                paletteColor = 0x9;
                break;
            case LAND_TYPE_TUNDRA:
                paletteColor = 0x1;
                break;
            case LAND_TYPE_SNOW:
                paletteColor = 0xC;
                break;
            default:
                printf("No color for type %d\n", vertex->type);
            }

            uint8_t shade = (vertex->flag & 0x0F);
            shade = shade >> 1;

            /*
            if (shade & 1)
                shade = 0;
            */

            int16_t unknown = (vertex->flag & 0xF0);
            unknown = unknown >> 8;

            vertex->upperImageID = vertStream.ReadByte();
            vertex->lowerImageID = vertStream.ReadByte();

            Texel *t = Renderer.getPalette()->GetRGBColor(paletteColor * 16 + shade);

            // Texel* t = renderer.GetDefaultPalette()->GetRGBColor(vertex->text);
            /*
              TODO: Figure out what are:
                    - flag high 4 bits.
                    - flag low  1 bits.
                    - textSet
                    - text
            */

            vertex->v.y = height * HEIGH_MAP_SCALE;

            vertex->v.x = i % BLOCK_PER_MAP_SIDE * BLOCK_WIDTH + (vertexID % blockDim) / (float)(blockDim)*BLOCK_WIDTH;
            vertex->v.z = i / BLOCK_PER_MAP_SIDE * BLOCK_WIDTH + (vertexID / blockDim) / (float)(blockDim)*BLOCK_WIDTH;
            vertex->v.x += -BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
            vertex->v.z += -BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;

            vertex->color[0] = t->r / 255.0f;
            vertex->color[1] = t->g / 255.0f;
            vertex->color[2] = t->b / 255.0f;
            vertex->color[3] = 255;
        }
    }
}

void RSArea::ParseElevations(void) {

    PakEntry *entry;

    entry = archive->GetEntry(6);

    ByteStream stream(entry->data);

    for (size_t i = 0; i < BLOCKS_PER_MAP; i++) {
        elevation[i] = stream.ReadUShort();
    }
}

void RSArea::ParseHeightMap(void) {

    PakEntry *entry;

    entry = archive->GetEntry(1);
    PakArchive fullPak;
    fullPak.InitFromRAM("FULLSIZE", entry->data, entry->size);
    ParseBlocks(BLOCK_LOD_MAX, entry, 20);

    entry = archive->GetEntry(2);
    PakArchive medPak;
    medPak.InitFromRAM("MED SIZE", entry->data, entry->size);
    ParseBlocks(BLOCK_LOD_MED, entry, 10);

    entry = archive->GetEntry(3);
    PakArchive smallPak;
    smallPak.InitFromRAM("SMALSIZE", entry->data, entry->size);
    ParseBlocks(BLOCK_LOD_MIN, entry, 5);

    BuildSkirts();
}

RSImage *RSArea::GetImageByID(size_t ID) { return textures[0]->GetImageById(ID); }

void RSArea::InitFromPAKFileName(const char *pakFilename) {

    strcpy(name, pakFilename);

    // Check the PAK has 5 entries
    FileData *fileData = assetsManager->GetFileData(pakFilename);
    this->archive = new PakArchive();
    this->archive->InitFromRAM(pakFilename, fileData->data, fileData->size);
    this->objects.clear();
    this->objects.shrink_to_fit();

    this->objectOverlay.clear();
    this->textures.clear();
    // Check that we have 6 entries.
    if (archive->GetNumEntries() != 7) {
        printf("***Error: An area PAK file should have 7 files:\n");
        printf("        - IFF file.\n");
        printf("        - 1 file containing records of size 2400.\n");
        printf("        - 1 file containing records of size 600 (mipmaps of the 2400 records ?)\n");
        printf("        - 1  file containing records of size 150 (mipmaps of the 600  records ?)\n");
        printf("        - 1 file containing the map 3D data (MAURITAN.TRI).\n");
        printf("        - 1 file containing the objects locations on the map (MAURITAN.OBJ).\n"); // COMPLETELY REVERSE
                                                                                                  // ENGINEERED !!!!
        printf("        - 1 file containing MAURITAN.AVG (I assume Average of something ?)\n");
        return;
    }

    // Load the textures from the PAKs (TXMPACK.PAK and ACCPACK.PAK) within TEXTURES.TRE.
    /*
            Note: This is the bruteforce approach and not very good:
                I feel like the right way would be to be able to parse the AREA info
                (the first IFF entry which seems to contain textures references.)
                And load only those from the TXM PACK. This is probably how they did
                it on a machine with only 4MB of RAM.

    */

    // Find the texture PAKS.
    TreEntry *treEntry = NULL;
    RSMapTextureSet *set;

    treEntry = assetsManager->GetEntryByName(Assets.txm_filename);
    PakArchive txmPakArchive;
    txmPakArchive.InitFromRAM("TXMPACK", treEntry->data, treEntry->size);
    set = new RSMapTextureSet();
    set->InitFromPAK(&txmPakArchive);
    textures.push_back(set);

    // ACCPACK.PAK seems to contain runway textures
    treEntry = assetsManager->GetEntryByName(Assets.acc_filename);
    PakArchive accPakArchive;
    accPakArchive.InitFromRAM("ACCPACK.PAK", treEntry->data, treEntry->size);
    set = new RSMapTextureSet();
    set->InitFromPAK(&accPakArchive);
    textures.push_back(set);

    // Parse the meta datas.
    ParseElevations();
    //ParseMetadata();
    PakEntry *entry = archive->GetEntry(0);

    parseTERA(entry->data, entry->size);

    ParseObjects();
    ParseTrigo();

    ParseHeightMap();
}

float RSArea::getGroundLevel(int BLOC, float x, float y) {
    if (BLOC < 0 || BLOC >= BLOCKS_PER_MAP)
        return 0;

    bool newcode = true;
    if (newcode) {
        // x et y correspondent respectivement à x et z des vertices
        float bestDistance = (std::numeric_limits<float>::max)();
        float resultY = 0;

        for (int i = 0; i < 400; i++) {
            const auto &vertex = blocks[0][BLOC].vertice[i].v;
            float dx = x - vertex.x;
            float dz = y - vertex.z;
            float distanceSquared = dx * dx + dz * dz;

            if (distanceSquared < bestDistance) {
                bestDistance = distanceSquared;
                resultY = vertex.y;
            }
        }
        return resultY+5;
    }
    

    if (BLOC < 0 || BLOC >= BLOCKS_PER_MAP)
        return 0;

    int verticeIndex = 0;
    int centerX = 0;
    int centerY = 0;
    int vX = static_cast<int>((x) + centerX);
    int vY = static_cast<int>((y) + centerY);

    vX = vX / 1000;
    vY = vY / 1000;

    vX = vX * 1000;
    vY = vY * 1000;

    for (int i = 0; i < 400; i++) {
        if ((blocks[0][BLOC].vertice[i].v.x >= vX) && (blocks[0][BLOC].vertice[i].v.z >= vY)) {
            return (blocks[0][BLOC].vertice[i].v.y + 5);
        }
    }
    return (blocks[0][BLOC].vertice[0].v.y);

    
}

float RSArea::getY(float x, float z) {

    int centerX = BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    int centerY = BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    int blocX = (int)(x + centerX) / BLOCK_WIDTH;
    int blocY = (int)(z + centerY) / BLOCK_WIDTH;

    return (this->getGroundLevel(blocY * BLOCK_PER_MAP_SIDE + blocX, x, z));
}

void RSArea::parseTERA(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["VERS"] = std::bind(&RSArea::parseTERA_VERS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["INFO"] = std::bind(&RSArea::parseTERA_INFO, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSArea::parseTERA_VERS(uint8_t *data, size_t size) {}

void RSArea::parseTERA_INFO(uint8_t *data, size_t size) {}

void RSArea::parseTERA_BLOX(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["ELEV"] = std::bind(&RSArea::parseTERA_BLOX_ELEV, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ATRI"] = std::bind(&RSArea::parseTERA_BLOX_ATRI, this, std::placeholders::_1, std::placeholders::_2);
    handlers["OBJS"] = std::bind(&RSArea::parseTERA_BLOX_OBJS, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSArea::parseTERA_BLOX_ELEV(uint8_t *data, size_t size) {
    printf("Content of elevation chunk:\n");
    size_t numEleRecords = size / 46;
    ByteStream elevStream(data);
    for (size_t e = 0; e < numEleRecords; e++) {

        uint8_t unknownsElev[20];
        for (int i = 0; i < 20; i++)
            unknownsElev[i] = elevStream.ReadByte();

        char elevName[14];
        for (int i = 0; i < 13; i++)
            elevName[i] = elevStream.ReadByte();
        elevName[13] = 0;

        char elevOtherName[14];
        for (int i = 0; i < 13; i++)
            elevOtherName[i] = elevStream.ReadByte();
        elevOtherName[13] = 0;
    }

}

void RSArea::parseTERA_BLOX_ATRI(uint8_t *data, size_t size) {
    ByteStream triStream(data);

    char triFileName[13];
    for (int i = 0; i < 13; i++)
        triFileName[i] = triStream.ReadByte();
}

void RSArea::parseTERA_BLOX_OBJS(uint8_t *data, size_t size) {}

void RSArea::parseTERA_TXML(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSArea::parseTERA_TXML_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MAPS"] = std::bind(&RSArea::parseTERA_TXMS_MAPS, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSArea::parseTERA_TXML_INFO(uint8_t *data, size_t size) {}

void RSArea::parseTERA_TXMS_MAPS(uint8_t *data, size_t size) {
    ByteStream textureRefStrean(data);
    size_t numTexturesSets = size / 12;
    for (size_t i = 0; i < numTexturesSets; i++) {
        uint16_t fastID = textureRefStrean.ReadUShort();
        char setName[9];
        for (int n = 0; n < 8; n++) {
            setName[n] = textureRefStrean.ReadByte();
        }
        uint8_t unknown = textureRefStrean.ReadByte();
        uint8_t numImages = textureRefStrean.ReadByte();

        printf("Texture Set Ref [%3llu] 0x0x%X[%-8s] %02X (%2u files).\n", i, fastID, setName, unknown, numImages);
    }
}
