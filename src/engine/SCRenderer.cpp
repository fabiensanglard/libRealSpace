//
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "SCRenderer.h"
#include "Config.hpp"
#include "../realspace/AssetManager.h"
#include "../realspace/RSArea.h"
#include "../realspace/RSEntity.h"
#include "../realspace/RSPalette.h"

#include "RSVGA.h"
#include "Texture.h"
#include <SDL_opengl_glext.h>
#include <SDL_opengl.h>

#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <limits>

SCRenderer &Renderer = SCRenderer::getInstance();

static inline void FixEntityWinding(RSEntity* obj) {
    if (!obj) return;
    Vector3D center{0,0,0};
    if (!obj->vertices.empty()) {
        for (const auto& v : obj->vertices) { center.x += v.x; center.y += v.y; center.z += v.z; }
        float invN = 1.0f / (float)obj->vertices.size();
        center.x *= invN; center.y *= invN; center.z *= invN;
    }
    auto ensureTriOutward = [&](uint16_t ids[3]) {
        const Vector3D& a = obj->vertices[ids[0]];
        const Vector3D& b = obj->vertices[ids[1]];
        const Vector3D& c = obj->vertices[ids[2]];
        Vector3D e1{ b.x-a.x, b.y-a.y, b.z-a.z };
        Vector3D e2{ c.x-a.x, c.y-a.y, c.z-a.z };
        Vector3D n{ e1.y*e2.z - e1.z*e2.y,
                    e1.z*e2.x - e1.x*e2.z,
                    e1.x*e2.y - e1.y*e2.x };
        Vector3D faceC{ (a.x+b.x+c.x)/3.0f, (a.y+b.y+c.y)/3.0f, (a.z+b.z+c.z)/3.0f };
        Vector3D outward{ faceC.x-center.x, faceC.y-center.y, faceC.z-center.z };
        float d = n.x*outward.x + n.y*outward.y + n.z*outward.z;
        if (d < 0.0f) std::swap(ids[1], ids[2]); // flip winding
    };
    for (auto& t : obj->triangles) ensureTriOutward(t.ids);
    for (auto* q : obj->quads) { // flip tout le quad si nécessaire
        if (!q) continue;
        uint16_t tri[3] = { q->ids[0], q->ids[1], q->ids[2] };
        uint16_t before[3] = { tri[0], tri[1], tri[2] };
        ensureTriOutward(tri);
        bool flipped = !(tri[0]==before[0] && tri[1]==before[1] && tri[2]==before[2]);
        if (flipped) std::swap(q->ids[1], q->ids[3]);
    }
}
static inline GLenum DetectTerrainFrontFace(AreaBlock* blk) {
    if (!blk || blk->sideSize < 2) return GL_CCW;
    MapVertex* a = &blk->vertice[0];
    MapVertex* b = &blk->vertice[1];
    MapVertex* c = &blk->vertice[blk->sideSize + 1];
    Vector3D e1{ b->v.x-a->v.x, b->v.y-a->v.y, b->v.z-a->v.z };
    Vector3D e2{ c->v.x-a->v.x, c->v.y-a->v.y, c->v.z-a->v.z };
    Vector3D n{ e1.y*e2.z - e1.z*e2.y,
                e1.z*e2.x - e1.x*e2.z,
                e1.x*e2.y - e1.y*e2.x };
    // On veut les faces "vers le haut" comme faces avant
    return (n.y >= 0.0f) ? GL_CCW : GL_CW;
}
static inline void normalizePlane(Plane& p) {
    float invLen = 1.0f / std::sqrt(p.a*p.a + p.b*p.b + p.c*p.c);
    p.a *= invLen; p.b *= invLen; p.c *= invLen; p.d *= invLen;
}

// Multiplie des matrices 4x4 (column-major OpenGL): out = A * B
static inline void mulMat4(const float A[16], const float B[16], float out[16]) {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            out[c*4 + r] =
                A[0*4 + r] * B[c*4 + 0] +
                A[1*4 + r] * B[c*4 + 1] +
                A[2*4 + r] * B[c*4 + 2] +
                A[3*4 + r] * B[c*4 + 3];
        }
    }
}
// Transformations auxiliaires (column-major OpenGL)
static inline Vector3D TransformPointCM(const float M[16], const Vector3D& v) {
    Vector3D out;
    out.x = M[0]*v.x + M[4]*v.y + M[8] *v.z + M[12];
    out.y = M[1]*v.x + M[5]*v.y + M[9] *v.z + M[13];
    out.z = M[2]*v.x + M[6]*v.y + M[10]*v.z + M[14];
    return out;
}
static inline Vector3D TransformDirCM3x3(const float M[16], const Vector3D& v) {
    Vector3D out;
    out.x = M[0]*v.x + M[4]*v.y + M[8] *v.z;
    out.y = M[1]*v.x + M[5]*v.y + M[9] *v.z;
    out.z = M[2]*v.x + M[6]*v.y + M[10]*v.z;
    return out;
}
static inline float ComputeLambertAt(const Vector3D& vLocal,
                                     const Vector3D& nLocal,
                                     const float MV[16],
                                     const Vector3D& lightEye,
                                     float ambient) {
    Vector3D vEye = TransformPointCM(MV, vLocal);
    Vector3D nEye = TransformDirCM3x3(MV, nLocal);
    nEye.Normalize();

    Vector3D L = lightEye;
    L.Substract(&vEye);
    L.Normalize();

    float d = nEye.DotProduct(&L);
    if (d < 0.0f) d = 0.0f;
    d += ambient;
    if (d > 1.0f) d = 1.0f;
    return d;
}
static inline float ComputeLambertAtTwoSided(const Vector3D& vLocal,
                                             const Vector3D& nLocal,
                                             const float MV[16],
                                             const Vector3D& lightEye,
                                             float ambient) {
    Vector3D vEye = TransformPointCM(MV, vLocal);
    Vector3D nEye = TransformDirCM3x3(MV, nLocal);
    nEye.Normalize();

    Vector3D L = lightEye;
    L.Substract(&vEye);
    L.Normalize();

    float d = nEye.DotProduct(&L);
    d = std::fabs(d);            // éclairage double-face
    d += ambient;
    if (d > 1.0f) d = 1.0f;
    return d;
}
void SCRenderer::extractFrustumPlanes(Plane planes[6]) const {
    float proj[16], modl[16], clip[16];
    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    glGetFloatv(GL_MODELVIEW_MATRIX,  modl);

    // MVP = PROJECTION * MODELVIEW
    mulMat4(proj, modl, clip);

    // Left, Right, Bottom, Top, Near, Far
    planes[0] = { clip[ 3] + clip[ 0], clip[ 7] + clip[ 4], clip[11] + clip[ 8], clip[15] + clip[12] }; // Left
    planes[1] = { clip[ 3] - clip[ 0], clip[ 7] - clip[ 4], clip[11] - clip[ 8], clip[15] - clip[12] }; // Right
    planes[2] = { clip[ 3] + clip[ 1], clip[ 7] + clip[ 5], clip[11] + clip[ 9], clip[15] + clip[13] }; // Bottom
    planes[3] = { clip[ 3] - clip[ 1], clip[ 7] - clip[ 5], clip[11] - clip[ 9], clip[15] - clip[13] }; // Top
    planes[4] = { clip[ 3] + clip[ 2], clip[ 7] + clip[ 6], clip[11] + clip[10], clip[15] + clip[14] }; // Near
    planes[5] = { clip[ 3] - clip[ 2], clip[ 7] - clip[ 6], clip[11] - clip[10], clip[15] - clip[14] }; // Far

    for (int i=0; i<6; ++i) normalizePlane(planes[i]);
}

bool SCRenderer::isAABBVisible(const AABB& box, const Plane planes[6]) {
    // Rendre le test moins strict: on accepte une petite pénétration négative
    const float cullingEpsilon = 15.0f; // ajustable
    for (int i=0; i<6; ++i) {
        const Plane& p = planes[i];
        Vector3D v;
        v.x = (p.a >= 0.0f) ? box.max.x : box.min.x;
        v.y = (p.b >= 0.0f) ? box.max.y : box.min.y;
        v.z = (p.c >= 0.0f) ? box.max.z : box.min.z;
        float dist = p.a*v.x + p.b*v.y + p.c*v.z + p.d;
        if (dist < -cullingEpsilon) {
            return false;
        }
    }
    return true;
}
void SCRenderer::bindCameraProjectionAndViewViewport(int32_t viewportW, int32_t viewportH, float verticalOffset /*=0.45f*/) {
    glViewport(0, 0, viewportW, viewportH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef(0.0f, verticalOffset, 0.0f);
    glMultMatrixf(camera.getProjectionMatrix()->ToGL());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera.getViewMatrix()->ToGL());
}

// Helper historique (viewport = taille fenêtre)
void SCRenderer::bindCameraProjectionAndView(float verticalOffset /*=0.45f*/) {
    bindCameraProjectionAndViewViewport(this->width, this->height, verticalOffset);
}
// Remplace computeBlockAABB pour utiliser un cache par RSArea
const AABB& SCRenderer::computeBlockAABB(RSArea* area, int LOD, int blockId) {
    auto& areaCache = aabbCache_[area].byKey;
    const uint64_t key = (static_cast<uint64_t>(LOD) << 32) | static_cast<uint32_t>(blockId);
    if (auto it = areaCache.find(key); it != areaCache.end()) return it->second;

    AABB box;
    box.min = {  std::numeric_limits<float>::infinity(),
                 std::numeric_limits<float>::infinity(),
                 std::numeric_limits<float>::infinity() };
    box.max = { -std::numeric_limits<float>::infinity(),
                -std::numeric_limits<float>::infinity(),
                -std::numeric_limits<float>::infinity() };

    AreaBlock* block = area->GetAreaBlockByID(LOD, blockId);
    if (!block) {
        box.min = {0,0,0}; box.max = {0,0,0};
        return areaCache.emplace(key, box).first->second;
    }

    const uint32_t s = block->sideSize;
    for (uint32_t y=0; y<s; ++y) {
        for (uint32_t x=0; x<s; ++x) {
            const auto& v = block->vertice[x + y * s].v;
            if (v.x < box.min.x) box.min.x = v.x;
            if (v.y < box.min.y) box.min.y = v.y;
            if (v.z < box.min.z) box.min.z = v.z;
            if (v.x > box.max.x) box.max.x = v.x;
            if (v.y > box.max.y) box.max.y = v.y;
            if (v.z > box.max.z) box.max.z = v.z;
        }
    }
    // Padding pour éviter les clips sur bords
    float stepX = 0.f, stepZ = 0.f;
    if (s > 1) {
        stepX = std::abs(block->vertice[1].v.x - block->vertice[0].v.x);
        stepZ = std::abs(block->vertice[s].v.z - block->vertice[0].v.z);
    }
    const float padX = (std::max)(100.0f, stepX * 2.0f);
    const float padZ = (std::max)(100.0f, stepZ * 2.0f);
    const float padY = 200.0f;

    box.min.x -= padX; box.max.x += padX;
    box.min.z -= padZ; box.max.z += padZ;
    box.min.y -= padY; box.max.y += padY;

    return areaCache.emplace(key, box).first->second;
}

// Invalidation globale
void SCRenderer::InvalidateAABBCache() {
    aabbCache_.clear();
}

// Invalidation pour une carte
void SCRenderer::InvalidateAABBCache(RSArea* area) {
    if (!area) return;
    aabbCache_.erase(area);
}

// Invalidation fine d’un bloc
void SCRenderer::InvalidateAABBForBlock(RSArea* area, int LOD, int blockId) {
    if (!area) return;
    auto itArea = aabbCache_.find(area);
    if (itArea == aabbCache_.end()) return;
    const uint64_t key = (static_cast<uint64_t>(LOD) << 32) | static_cast<uint32_t>(blockId);
    itArea->second.byKey.erase(key);
}

// Pré-calcul sur une plage de LOD (lazy-safe: réutilise computeBlockAABB)
void SCRenderer::PrecomputeAABBs(RSArea* area, int minLOD, int maxLOD) {
    if (!area) return;
    if (minLOD > maxLOD) std::swap(minLOD, maxLOD);
    for (int lod = minLOD; lod <= maxLOD; ++lod) {
        for (int i = 0; i < BLOCKS_PER_MAP; ++i) {
            (void)computeBlockAABB(area, lod, i);
        }
    }
}
// Helpers: calcule les normales par sommet pour un LOD donné (Gouraud)
static void AccumulateFaceNormal(const RSEntity* obj, int i0, int i1, int i2, const Point3D& camPos, std::vector<Vector3D>& acc) {
    Vector3D e1 = obj->vertices[i0]; Vector3D pivot = obj->vertices[i1]; e1.Substract(&pivot);
    Vector3D e2 = obj->vertices[i2]; e2.Substract(&pivot);
    Vector3D n = e1.CrossProduct(&e2);
    n.Normalize();
    // plus de flip vers la caméra
    acc[i0].x+=n.x; acc[i0].y+=n.y; acc[i0].z+=n.z;
    acc[i1].x+=n.x; acc[i1].y+=n.y; acc[i1].z+=n.z;
    acc[i2].x+=n.x; acc[i2].y+=n.y; acc[i2].z+=n.z;
}

static void ComputeVertexNormalsForLOD(RSEntity* obj, size_t lodLevel, const Point3D& camPos, std::vector<Vector3D>& outNormals) {
    outNormals.assign(obj->vertices.size(), Vector3D{0,0,0});
    if (lodLevel >= obj->NumLods()) return;

    const Lod* lod = &obj->lods[lodLevel];

    // Triangles utilisés par ce LOD
    for (int i = 0; i < lod->numTriangles; ++i) {
        uint16_t triID = lod->triangleIDs[i];
        if (obj->attrs.size() > 0) {
            if (obj->attrs[triID] == nullptr) continue;
            // ignorer quads et lignes dans ce passage
            if (obj->attrs[triID]->type == 'Q' || obj->attrs[triID]->type == 'L') continue;
            triID = obj->attrs[triID]->id;
        }
        if (triID >= obj->triangles.size()) continue;
        const Triangle& t = obj->triangles[triID];
        AccumulateFaceNormal(obj, t.ids[0], t.ids[1], t.ids[2], camPos, outNormals);
    }

    // Quads utilisés par ce LOD (comme faces, sans split pour l’accumulation)
    if (obj->quads.size() > 0) {
        for (int i = 0; i < lod->numTriangles; ++i) {
            uint16_t qid = lod->triangleIDs[i];
            if (obj->attrs.size() > 0) {
                if (obj->attrs[qid] == nullptr) continue;
                // ignorer triangles et lignes dans ce passage
                if (obj->attrs[qid]->type == 'T' || obj->attrs[qid]->type == 'L') continue;
                qid = obj->attrs[qid]->id;
            }
            if (qid >= obj->quads.size()) continue;
            const Quads* q = obj->quads[qid];
            // Accumuler avec deux triangles (quad -> 2 tris) pour lisser
            AccumulateFaceNormal(obj, q->ids[0], q->ids[1], q->ids[2], camPos, outNormals);
            AccumulateFaceNormal(obj, q->ids[0], q->ids[2], q->ids[3], camPos, outNormals);
        }
    }

    // Normalisation finale
    for (auto& n : outNormals) {
        n.Normalize();
    }
}

SCRenderer::SCRenderer() : initialized(false) {}

SCRenderer::~SCRenderer() {}

Camera *SCRenderer::getCamera(void) { return &this->camera; }

VGAPalette *SCRenderer::getPalette(void) { return &this->palette; }

void SCRenderer::setPlayerPosition(Point3D *position) { camera.SetPosition(position); }

void SCRenderer::init(int width, int height) {
    AssetManager &assets = AssetManager::instance();
    Config &config = Config::instance();
    this->max_view_distance = config.getInt("Game", "max_view_distance", 160000);
    this->counter = 0;

    RSPalette palette;
    
    TreEntry *entries = (TreEntry *)assets.GetEntryByName("..\\..\\DATA\\PALETTE\\PALETTE.IFF");
    if (entries) {
        palette.initFromFileRam(entries->data, entries->size);    
        this->palette = *palette.GetColorPalette();
        this->terrain_ranges = palette.ranges;
    }
    
    this->lodLevel = config.getInt("Game", "object_detail", 0);
    this->show_textured = config.getBool("Game", "show_texture", true);
    this->show_fog = config.getBool("Game", "show_fog", true);
    this->terrain_black_level = config.getInt("Game", "terrain_black_level", 15) / 100.0f;
    this->show_clouds = config.getBool("Game", "clouds_enabled", false);
    this->width = width;
    this->height = height;

    glClearColor(0.0f, 0.5f, 1.0f, 1.0f); // Black Background
    // glClearDepth(1.0f);								// Depth Buffer Setup
    glDisable(GL_DEPTH_TEST); // Disable Depth Testing
    glShadeModel(GL_SMOOTH);

    camera.setPersective(this->fov, this->width / (float)this->height, 1.5f, this->max_view_distance+this->max_view_distance*0.2f);

    light.SetWithCoo(300, 300, 300);
    initialized = true;
}
void SCRenderer::resetCameraPerspective() {
    camera.setPersective(this->fov, this->width / (float)this->height, 1.5f, this->max_view_distance+this->max_view_distance*0.2f);
}

void SCRenderer::setClearColor(uint8_t red, uint8_t green, uint8_t blue) {
    if (!initialized)
        return;

    glClearColor(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
}

void SCRenderer::clear(void) {

    if (!initialized)
        return;
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
}

void SCRenderer::createTextureInGPU(Texture *texture) {

    if (!initialized)
        return;
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texture->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void SCRenderer::uploadTextureContentToGPU(Texture *texture) {
    if (!initialized)
        return;
   
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texture->data);
}

void SCRenderer::deleteTextureInGPU(Texture *texture) {
    if (!initialized)
        return;
    glDeleteTextures(1, &texture->id);
}

void SCRenderer::drawTexturedQuad(Vector3D pos, Vector3D orientation, std::vector<Vector3D> quad, Texture *tex) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glDisable(GL_CULL_FACE);
    if (!tex->initialized) {
        glGenTextures(1, &tex->id);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)tex->width, (GLsizei)tex->height, 
                     0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
        
        tex->initialized = true;
    } else {
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }
    
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(orientation.x, 0, 1, 0);
    glRotatef(orientation.y, 0, 0, 1);
    glRotatef(orientation.z, 1, 0, 0);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(quad[0].x, quad[0].y, quad[0].z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(quad[1].x, quad[1].y, quad[1].z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(quad[2].x, quad[2].y, quad[2].z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(quad[3].x, quad[3].y, quad[3].z);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_CULL_FACE);
}

void SCRenderer::getNormal(RSEntity *object, Triangle *triangle, Vector3D *normal) {
    Vector3D edge1 = object->vertices[triangle->ids[0]];
    edge1.Substract(&object->vertices[triangle->ids[1]]);
    Vector3D edge2 = object->vertices[triangle->ids[2]];
    edge2.Substract(&object->vertices[triangle->ids[1]]);
    *normal = edge1.CrossProduct(&edge2);
    normal->Normalize();
    // plus d’orientation vers la caméra
}

void SCRenderer::drawParticle(Vector3D pos, float alpha) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glPushMatrix();
    Matrix smoke_rotation;
    smoke_rotation.Clear();
    smoke_rotation.Identity();
    smoke_rotation.translateM(pos.x, pos.y, pos.z);
    glMultMatrixf((float *)smoke_rotation.v);
    glBegin(GL_QUADS);
    glColor4f(1.0f,1.0f,1.0f, alpha);
    glVertex3f(1.0f,-1.0f,-1.0f);
    glVertex3f(1.0f,1.0f,-1.0f);
    glVertex3f(-1.0f,1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,1.0f);
    glEnd();
    glBegin(GL_QUADS);
    glColor4f(1.0f,1.0f,1.0f, alpha);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,1.0f,-1.0f);
    glVertex3f(1.0f,1.0f,1.0f);
    glVertex3f(1.0f,-1.0f,1.0f);
    glEnd();
    glPopMatrix();
    glDisable( GL_BLEND );
}
void SCRenderer::drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation, Vector3D ajustement, float scale) { 
    glPushMatrix();
    glTranslatef(static_cast<GLfloat>(position.x), static_cast<GLfloat>(position.y),
                static_cast<GLfloat>(position.z));
    glRotatef(orientation.x, 0, 1, 0);
    glRotatef(orientation.y, 0, 0, 1);
    glRotatef(orientation.z, 1, 0, 0);
    glTranslatef(ajustement.x, ajustement.y, ajustement.z);
    glScalef(scale, scale, scale);
    drawModel(object, lodLevel);
    glPopMatrix(); 
}
void SCRenderer::drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation, Vector3D ajustement) { 
    glPushMatrix();
    glTranslatef(static_cast<GLfloat>(position.x), static_cast<GLfloat>(position.y),
                static_cast<GLfloat>(position.z));
    glRotatef(orientation.x, 0, 1, 0);
    glRotatef(orientation.y, 0, 0, 1);
    glRotatef(orientation.z, 1, 0, 0);
    glTranslatef(ajustement.x, ajustement.y, ajustement.z);
    drawModel(object, lodLevel);
    glPopMatrix(); 
}
void SCRenderer::drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation) {
    glPushMatrix();
    glTranslatef(static_cast<GLfloat>(position.x), static_cast<GLfloat>(position.y),
                static_cast<GLfloat>(position.z));
    glRotatef(orientation.x, 0, 1, 0);
    glRotatef(orientation.y, 0, 0, 1);
    glRotatef(orientation.z, 1, 0, 0);
    drawModel(object, lodLevel);
    glPopMatrix();
}
void SCRenderer::drawPoint(Vector3D point, Vector3D color, Vector3D pos, Vector3D orientation) {
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(orientation.x, 0, 1, 0);
    glRotatef(orientation.y, 0, 0, 1);
    glRotatef(orientation.z, 1, 0, 0);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(color.x, color.y, color.z);
    glVertex3f(point.x, point.y, point.z);
    glEnd();
    glPopMatrix();
}
void SCRenderer::drawModel(RSEntity *object, Vector3D position, Vector3D orientation) {
    glPushMatrix();
    Matrix rotation;
    rotation.Clear();
    rotation.Identity();
    rotation.translateM(position.x, position.y, position.z);
    rotation.rotateM(orientation.x, 0.0f, 1.0f, 0.0f);
    rotation.rotateM(orientation.y, 0.0f, 0.0f, 1.0f);
    rotation.rotateM(orientation.z, 1.0f, 0.0f, 0.0f);
    
    glMultMatrixf((float *)rotation.v);
    drawModel(object, lodLevel);
    glPopMatrix();
}
void SCRenderer::drawLine(Vector3D start, Vector3D end, Vector3D color, Vector3D orientation) {
    glPushMatrix();
    glTranslatef(start.x, start.y, start.z);
    glRotatef(orientation.x, 0, 1, 0);
    glRotatef(orientation.y, 0, 0, 1);
    glRotatef(orientation.z, 1, 0, 0);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    glColor3f(color.x, color.y, color.z);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(end.x, end.y, end.z);
    glEnd();
    glPopMatrix();
}
void SCRenderer::drawLine(Vector3D start, Vector3D end, Vector3D color, Vector3D orientation, Vector3D position) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(orientation.x, 0, 1, 0);
    glRotatef(orientation.y, 0, 0, 1);
    glRotatef(orientation.z, 1, 0, 0);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    glColor3f(color.x, color.y, color.z);
    glVertex3f(start.x, start.y, start.z);
    glVertex3f(end.x, end.y, end.z);
    glEnd();
    glPopMatrix();
}
void SCRenderer::drawLine(Vector3D start, Vector3D end, Vector3D color) {
    glPushMatrix();
    glTranslatef(start.x, start.y, start.z);
    glLineWidth(10.0f);
    glBegin(GL_LINES);
    glColor3f(color.x, color.y, color.z);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(end.x, end.y, end.z);
    glEnd();
    glPopMatrix();
}
void SCRenderer::drawTracer(Vector3D pos, Vector3D velocityDir, float length, float pixelWidth, Vector3D color, float alpha) {
    if (!initialized) return;

    Point3D camPos = camera.getPosition();
    Vector3D look = { camPos.x - pos.x, camPos.y - pos.y, camPos.z - pos.z };
    float dist = look.Length();
    if (dist < 1e-4f) return;
    look = look * (1.0f / dist);

    float dirLen = velocityDir.Length();
    if (dirLen < 1e-4f) return;
    Vector3D dir = velocityDir * (1.0f / dirLen);

    // Composante de 'dir' perpendiculaire à 'look' (foreshortening naturel)
    float dotLook = dir.DotProduct(&look);
    Vector3D upRaw = dir - look * dotLook;
    float upRawLen = upRaw.Length();          // dans [0,1]: 1 = perpendiculaire à la vue, 0 = dans l'axe de vue

    Vector3D up;
    if (upRawLen > 1e-4f) {
        up = upRaw * (1.0f / upRawLen);
    } else {
        // Direction quasi-nulle après projection: n'importe quelle base orthogonale convient,
        // car halfLen sera ~0 (le trait sera visuellement un point).
        Vector3D worldUp = {0.0f, 1.0f, 0.0f};
        up = worldUp - look * worldUp.DotProduct(&look);
        float l = up.Length();
        up = (l > 1e-4f) ? up * (1.0f / l) : Vector3D{1.0f, 0.0f, 0.0f};
    }

    Vector3D right = up.CrossProduct(&look);
    right.Normalize();

    float fovRad = this->fov * (float)M_PI / 180.0f;
    float halfWidth = (pixelWidth * dist * tanf(fovRad * 0.5f)) / (this->height * 0.5f);

    // La longueur visible dépend de l'angle entre la trajectoire et l'axe de vue (foreshortening réel)
    float halfLen = (length * 0.5f) * upRawLen;
    if (halfLen < halfWidth) halfLen = halfWidth; // garde un minimum "point" visible

    Vector3D rOff = right * halfWidth;
    Vector3D uOff = up * halfLen;

    Vector3D front = pos + uOff;
    Vector3D back  = pos - uOff;

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glBegin(GL_QUADS);
    glColor4f(color.x, color.y, color.z, alpha);
    glVertex3f(back.x  - rOff.x, back.y  - rOff.y, back.z  - rOff.z);
    glVertex3f(back.x  + rOff.x, back.y  + rOff.y, back.z  + rOff.z);
    glVertex3f(front.x + rOff.x, front.y + rOff.y, front.z + rOff.z);
    glVertex3f(front.x - rOff.x, front.y - rOff.y, front.z - rOff.z);
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}
void SCRenderer::drawSprite(Vector3D pos, Texture *tex, float zoom) {
    size_t cpt=0;
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
    glEnable(GL_BLEND);
    
    glPushMatrix();
    Matrix smoke_rotation;
    smoke_rotation.Clear();
    smoke_rotation.Identity();
    smoke_rotation.translateM(pos.x, pos.y, pos.z);
    smoke_rotation.rotateM(0.0f, 1.0f, 0.0f, 0.0f);

    glMultMatrixf((float *)smoke_rotation.v);
    if (tex->initialized == false) {
        glGenTextures(1, &tex->id);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // Upload pixels into texture
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)tex->width, (GLsizei)tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            tex->data);
        
        tex->initialized = true;
    } else {
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }
    float smoke_size = 4.0f * zoom + 1.0f;
    glBegin(GL_QUADS);
    glColor4f(1.0f,1.0f,1.0f,0.0f);
    glTexCoord2f (0.0, 0.0);
    glVertex3f(smoke_size,-smoke_size,-smoke_size);
    glTexCoord2f (1.0, 0.0);
    glVertex3f(smoke_size,smoke_size,-smoke_size);
    glTexCoord2f (1.0, 1.0);
    glVertex3f(-smoke_size,smoke_size,-smoke_size);
    glTexCoord2f (0.0, 1.0);
    glVertex3f(-smoke_size,-smoke_size,smoke_size);
    glEnd();
    glBegin(GL_QUADS);
    glColor4f(1.0f,1.0f,1.0f,0.0f);
    glTexCoord2f (0.0, 0.0);
    glVertex3f(-smoke_size,-smoke_size,-smoke_size);
    glTexCoord2f (1.0, 0.0);
    glVertex3f(-smoke_size,smoke_size,-smoke_size);
    glTexCoord2f (1.0, 1.0);
    glVertex3f(smoke_size,smoke_size,smoke_size);
    glTexCoord2f (0.0, 1.0);
    glVertex3f(smoke_size,-smoke_size,smoke_size);
    glEnd();
    glPopMatrix();
    glDisable( GL_BLEND );
    glDisable(GL_TEXTURE_2D);
}
void SCRenderer::drawModelWithChilds(
    RSEntity *object,
    size_t lodLevel, 
    Vector3D position,
    Vector3D orientation, 
    int wheel_index, 
    int thrust, 
    std::vector<std::tuple<Vector3D, RSEntity *>> weaps_load
) {
    if (object != nullptr) {
        glPushMatrix();
        Matrix rotation;
        rotation.Clear();
        rotation.Identity();
        rotation.translateM(position.x, position.y, position.z);
        rotation.rotateM(orientation.x, 0.0f, 1.0f, 0.0f);
        rotation.rotateM(orientation.y, 0.0f, 0.0f, 1.0f);
        rotation.rotateM(orientation.z, 1.0f, 0.0f, 0.0f);

        glMultMatrixf((float *)rotation.v);
        
        Renderer.drawModel(object, this->lodLevel);
        if (wheel_index) {
            if (object->chld.size() > wheel_index) {
                Renderer.drawModel(object->chld[wheel_index]->objct, this->lodLevel);
            }
        }
        if (thrust > 50) {
            if (object->chld.size() > 0) {
                glPushMatrix();
                Vector3D pos = {
                    (float) object->chld[0]->x,
                    (float) object->chld[0]->y,
                    (float) object->chld[0]->z
                };
                glTranslatef(pos.z/250 , pos.y /250 , pos.x /250);
                glScalef(1+thrust/100.0f,1,1);
                Renderer.drawModel(object->chld[0]->objct, this->lodLevel);
                glPopMatrix();
            }
        }
        for (auto weaps_it: weaps_load) {
            float decy=0.5f;
            Vector3D weaps_pos = std::get<0>(weaps_it);
            RSEntity *weaps = std::get<1>(weaps_it);
            if (weaps == nullptr) {
                continue;  
            }
            
            glPushMatrix();
            glTranslatef(weaps_pos.x, weaps_pos.y, weaps_pos.z);
            Renderer.drawModel(weaps, this->lodLevel);
            glPopMatrix();            
        }
        glPopMatrix();
    }
}
void SCRenderer::drawModelColorPass(RSEntity *object, size_t lodLevel, std::vector<Vector3D> &vertexNormals, float ambientLamber, Vector3D lightEye, float *MV) {
    Lod *lod = &object->lods[lodLevel];
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Pass 1, draw color
    for (int i = 0; i < lod->numTriangles; i++) {
        uint16_t triangleID = lod->triangleIDs[i];
        if (object->attrs.size() > 0) {
            if (object->attrs[triangleID] == nullptr) {
                continue;
            }
            if (object->attrs[triangleID]->type == 'Q') {
                continue;
            }
            if (object->attrs[triangleID]->type == 'L') {
                continue;
            }
            triangleID = object->attrs[triangleID]->id;
        }
        if (triangleID >= object->triangles.size()) {
            continue;
        }
        Triangle *triangle = &object->triangles[triangleID];
        if (triangle->property == RSEntity::SC_TRANSPARENT)
            continue;
        float alpha = 1.0f;
        if (triangle->property == 6 && this->show_textured) {
            continue;
            
        }
        if (triangle->property == 9 && this->show_textured) {
            continue;
        }
        bool twoSided = false;
        if (triangle->flags[2] == 1) {
            twoSided = true; // If all vertices are at the same Z, we assume it's a 2D quad
        }
        if (twoSided) {
            glDisable(GL_CULL_FACE);
        }
        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; j++) {
            Vector3D vLocal = object->vertices[triangle->ids[j]];
            Vector3D nLocal = vertexNormals[triangle->ids[j]];

            float lambertianFactor = twoSided
                ? ComputeLambertAtTwoSided(vLocal, nLocal, MV, lightEye, ambientLamber)
                : ComputeLambertAt(vLocal, nLocal, MV, lightEye, ambientLamber);

            const Texel *texel = palette.GetRGBColor(triangle->color);
            glColor4f(texel->r / 255.0f * lambertianFactor, texel->g / 255.0f * lambertianFactor,
                      texel->b / 255.0f * lambertianFactor, alpha);

            glVertex3f(vLocal.x, vLocal.y, vLocal.z);
        }
        glEnd();
        if (twoSided) {
            glEnable(GL_CULL_FACE);
        }
        
    }
    if (object->quads.size() > 0) {
        for (int i = 0; i < lod->numTriangles; i++) {
            uint16_t triangleID = lod->triangleIDs[i];
            int prop1 = 0;
            int prop2 = 0;
            if (object->attrs.size() > 0) {
                if (object->attrs[triangleID]->type == 'T') {
                    continue;
                }
                if (object->attrs[triangleID]->type == 'L') {
                    continue;
                }
                prop1 = object->attrs[triangleID]->props1;
                prop2 = object->attrs[triangleID]->props2;
                triangleID = object->attrs[triangleID]->id;
                
            }
            if (triangleID >= object->quads.size()) {
                continue;
            }
            Quads *triangle = object->quads[triangleID];
    
            if (triangle->property == RSEntity::SC_TRANSPARENT)
                continue;
            float alpha = 1.0f;
            if (triangle->property == 6 && this->show_textured) {
                continue;
                
            }
            if (triangle->property == 9 && this->show_textured) {
                continue;
                
            }

            bool twoSided = false;
            
            
            twoSided = prop2 == 1;
            if (twoSided) {
                glDisable(GL_CULL_FACE);
            }
            glBegin(GL_QUADS);
            for (int j = 0; j < 4; j++) {
                Vector3D vLocal = object->vertices[triangle->ids[j]];
                Vector3D nLocal = vertexNormals[triangle->ids[j]];

                float lambertianFactor = twoSided
                ? ComputeLambertAtTwoSided(vLocal, nLocal, MV, lightEye, ambientLamber)
                : ComputeLambertAt(vLocal, nLocal, MV, lightEye, ambientLamber);

                const Texel *texel = palette.GetRGBColor(triangle->color-1);
                glColor4f(texel->r / 255.0f * lambertianFactor, texel->g / 255.0f * lambertianFactor,
                          texel->b / 255.0f * lambertianFactor, alpha);
                
                
                glVertex3f(vLocal.x, vLocal.y, vLocal.z);
            }
            glEnd();
            if (twoSided) {
                glEnable(GL_CULL_FACE);
            }
        }
    }
    glDisable(GL_BLEND);
}
void SCRenderer::drawModelTexturePass(RSEntity *object, size_t lodLevel, std::vector<Vector3D> &vertexNormals, float ambientLamber, Vector3D lightEye, float *MV) {
    // Texture pass
    if (lodLevel == 0) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        for (int i = 0; i < object->NumUVs(); i++) {

            uvxyEntry *textInfo = &object->uvs[i];

            // Seems we have a textureID that we don't have :( !
            if (textInfo->textureID >= object->images.size())
                continue;

            RSImage *image = object->images[textInfo->textureID];

            Texture *texture = image->GetTexture();
            Triangle *triangle = &object->triangles[textInfo->triangleID];
            float alpha = 1.0f;
            bool twoSided = false;
            if (triangle->property == 6) {
                alpha = 0.0f;
            }
            if (triangle->property == 9) {
                alpha = 0.0f;
            }
            
            if (triangle->flags[2] == 1) {
                twoSided = true; // If all vertices are at the same Z, we assume it's a 2D quad
            }
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture->id);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            if (alpha == 0.0f) {
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE);
            } else {
                glDepthFunc(GL_EQUAL);
                glDepthMask(GL_FALSE);
            }
            
            if (twoSided) {
                glDisable(GL_CULL_FACE);
            }
            glBegin(GL_TRIANGLES);
            for (int j = 0; j < 3; j++) {
                Vector3D vLocal = object->vertices[triangle->ids[j]];
                Vector3D nLocal = vertexNormals[triangle->ids[j]];

                float lambertianFactor = twoSided
                ? ComputeLambertAtTwoSided(vLocal, nLocal, MV, lightEye, ambientLamber)
                : ComputeLambertAt(vLocal, nLocal, MV, lightEye, ambientLamber);

                const Texel *texel = palette.GetRGBColor(triangle->color-1);
                
                glColor4f(lambertianFactor, lambertianFactor, lambertianFactor, 1.0f);
                
                glTexCoord2f(textInfo->uvs[j].u / (float)texture->width, textInfo->uvs[j].v / (float)texture->height);
                glVertex3f(vLocal.x, vLocal.y, vLocal.z);
            }
            glEnd();
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            glDisable(GL_TEXTURE_2D);
            if (twoSided) {
                glEnable(GL_CULL_FACE);
            }
        }

        for (auto quv: object->qmapuvs) {
            if (quv->textureID >= object->images.size())
                continue;

            RSImage *image = object->images[quv->textureID];

            Texture *texture = image->GetTexture();
            Quads *triangle = object->quads[quv->triangleID];
            float alpha = 1.0f;
            
            if (triangle->property == 6) {
                alpha = 0.0f;
            }
            if (triangle->property == 9) {
                alpha = 0.0f;
            }

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture->id);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            
            if (alpha == 0.0f) {
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE);
            } else {
                glDepthFunc(GL_EQUAL);
                glDepthMask(GL_FALSE);
            }

            bool twoSided = false;
            for (auto attr : object->attrs) {
                if (attr.second->id == quv->triangleID && attr.second->type == 'Q') {
                    int prop1 = attr.second->props1;
                    int prop2 = attr.second->props2;
                    
                    twoSided = prop2 == 1;
                    break;
                }
            }
            if (twoSided) {
                glDisable(GL_CULL_FACE);
            }
            glBegin(GL_QUADS);
            for (int j = 0; j < 4; j++) {
                Vector3D vLocal = object->vertices[triangle->ids[j]];
                Vector3D nLocal = vertexNormals[triangle->ids[j]];

                float lambertianFactor = twoSided
                ? ComputeLambertAtTwoSided(vLocal, nLocal, MV, lightEye, ambientLamber)
                : ComputeLambertAt(vLocal, nLocal, MV, lightEye, ambientLamber);

                const Texel *texel = palette.GetRGBColor(triangle->color-1);
                glColor4f(lambertianFactor, lambertianFactor, lambertianFactor, 1.0f);                glTexCoord2f(quv->uvs[j].u / (float)texture->width, quv->uvs[j].v / (float)texture->height);
                glVertex3f(vLocal.x, vLocal.y, vLocal.z);
            }
            glEnd();

            if (twoSided) {
                glEnable(GL_CULL_FACE);
            }
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            glDisable(GL_TEXTURE_2D);
        }
        glDisable(GL_BLEND);
    }
}
void SCRenderer::drawModelTransparentPass(RSEntity *object, size_t lodLevel, std::vector<Vector3D> &vertexNormals, float ambientLamber, Vector3D lightEye, float *MV) {
    // Pass 3: Let's draw the transparent stuff render RSEntity::SC_TRANSPARENT)
    Lod *lod = &object->lods[lodLevel];
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
#ifndef _WIN32
    glBlendEquation(GL_ADD);
#else
    typedef void(APIENTRY * PFNGLBLENDEQUATIONPROC)(GLenum mode);
    PFNGLBLENDEQUATIONPROC glBlendEquation = NULL;
    glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation");
    glBlendEquation(GL_FUNC_ADD);
#endif
    for (int i = 0; i < lod->numTriangles; i++) {
        uint16_t triangleID = lod->triangleIDs[i];
        if (object->attrs.size() > 0) {
            if (object->attrs[triangleID] == nullptr) {
                continue;
            }
            if (object->attrs[triangleID]->type == 'Q') {
                continue;
            }
            if (object->attrs[triangleID]->type == 'L') {
                continue;
            }
            triangleID = object->attrs[triangleID]->id;
        }
        if (triangleID >= object->triangles.size()) {
            continue;
        }
        Triangle *triangle = &object->triangles[triangleID];
        bool twoSided = false;
        if (triangle->property != RSEntity::SC_TRANSPARENT)
            continue;
        if (triangle->flags[2] == 1) {
            twoSided = true; // If all vertices are at the same Z, we assume it's a 2D quad
        }
        if (twoSided) {
            glDisable(GL_CULL_FACE);
        }
        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; j++) {
            Vector3D vLocal = object->vertices[triangle->ids[j]];
            Vector3D nLocal = vertexNormals[triangle->ids[j]];

            float lambertianFactor = twoSided
                ? ComputeLambertAtTwoSided(vLocal, nLocal, MV, lightEye, ambientLamber)
                : ComputeLambertAt(vLocal, nLocal, MV, lightEye, ambientLamber);

            const Texel *texel = palette.GetRGBColor(triangle->color-1);
            glColor4f(texel->r / 255.0f * lambertianFactor, texel->g / 255.0f * lambertianFactor,
                      texel->b / 255.0f * lambertianFactor, texel->a);

            glVertex3f(vLocal.x, vLocal.y, vLocal.z);
        }
        glEnd();
        if (twoSided) {
            glEnable(GL_CULL_FACE);
        }
    }

    if (object->quads.size() > 0) {
        for (int i = 0; i < lod->numTriangles; i++) {
            uint16_t triangleID = lod->triangleIDs[i];
            if (object->attrs.size() > 0) {
                if (object->attrs[triangleID]->type == 'T') {
                    continue;
                }
                if (object->attrs[triangleID]->type == 'L') {
                    continue;
                }
                triangleID = object->attrs[triangleID]->id;
            }
            if (triangleID >= object->triangles.size()) {
                continue;
            }
            if (triangleID >= object->quads.size()) {
                continue;
            }
            Quads *triangle = object->quads[triangleID];
            bool twoSided = false;
            if (object->attrs[triangleID]->props2 == 1) {
                twoSided = true;
            }
            if (triangle->property != RSEntity::SC_TRANSPARENT)
                continue;

            if (twoSided) {
                glDisable(GL_CULL_FACE);
            }
            glBegin(GL_QUADS);
            for (int j = 0; j < 4; j++) {
                Vector3D vLocal = object->vertices[triangle->ids[j]];
                Vector3D nLocal = vertexNormals[triangle->ids[j]];

                float lambertianFactor = twoSided
                ? ComputeLambertAtTwoSided(vLocal, nLocal, MV, lightEye, ambientLamber)
                : ComputeLambertAt(vLocal, nLocal, MV, lightEye, ambientLamber);

                const Texel *texel = palette.GetRGBColor(triangle->color-1);
                glColor4f(texel->r / 255.0f * lambertianFactor, texel->g / 255.0f * lambertianFactor,
                          texel->b / 255.0f * lambertianFactor, texel->a);

                glVertex3f(vLocal.x, vLocal.y, vLocal.z);
            }
            glEnd();
            if (twoSided) {
                glEnable(GL_CULL_FACE);
            }
        }    
    }
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}
void SCRenderer::drawModel(RSEntity *object, size_t lodLevel) {
    if (!initialized)
        return;

    if (object->vertices.size() == 0)
        return;

    size_t currentLodLevel = lodLevel;
    if (currentLodLevel >= object->NumLods()) {
        currentLodLevel = object->NumLods() - 1;
    }
    
    for (auto img: object->images) {
        if (img->nbframes > 1) {
            img->GetNextFrame();
        }
    }
    
    float ambientLamber = 0.4f;

    Lod *lod = &object->lods[currentLodLevel];
    std::vector<Vector3D> vertexNormals;
    ComputeVertexNormalsForLOD(object, currentLodLevel, camera.getPosition(), vertexNormals);

    // Prépare les matrices pour faire l’éclairage en espace œil
    float MV[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MV);
    const float* V = camera.getViewMatrix()->ToGL();
    Vector3D lightWorld{light.x, light.y, light.z};
    Vector3D lightEye = TransformPointCM(V, lightWorld);

    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    

    drawModelColorPass(object, currentLodLevel, vertexNormals, ambientLamber, lightEye, MV);

    if (this->show_textured) {
        drawModelTexturePass(object, currentLodLevel, vertexNormals, ambientLamber, lightEye, MV);
    }
    
    drawModelTransparentPass(object, currentLodLevel, vertexNormals, ambientLamber, lightEye, MV);
    
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void SCRenderer::setLight(Point3D *l) { this->light = *l; }

void SCRenderer::prepare(RSEntity *object) {

    for (size_t i = 0; i < object->NumImages(); i++) {
        object->images[i]->SyncTexture();
    }
    FixEntityWinding(object);
    object->prepared = true;
}

void SCRenderer::displayModel(RSEntity *object, size_t lodLevel) {

    if (!initialized)
        return;

    if (object->IsPrepared())
        prepare(object);

    glMatrixMode(GL_PROJECTION);
    Matrix *projectionMatrix = camera.getProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());

    running = true;
    float counter = 0;
    while (running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        Matrix *modelViewMatrix;

        light.x = 20.0f * cosf(counter);
        light.y = 10.0f;
        light.z = 20.0f * sinf(counter);
        counter += 0.02f;

        // camera.SetPosition(position);

        modelViewMatrix = camera.getViewMatrix();
        glLoadMatrixf(modelViewMatrix->ToGL());

        drawModel(object, lodLevel);

        // Render light

        glPointSize(6);

        glBegin(GL_POINTS);
        glColor4f(1, 1, 0, 1);
        glVertex3f(light.x, light.y, light.z);
        glEnd();
    }
}

#define TEX_ZERO (0 / 64.0f)
#define TEX_ONE (64 / 64.0f)

// What is this offset ? It is used to get rid of the red delimitations
// in the 64x64 textures.
#define OFFSET (2.0f / 64.0f)
float textTrianCoo64[2][3][2] = {

    {{TEX_ZERO, TEX_ZERO + OFFSET},
     {TEX_ONE - 2 * OFFSET, TEX_ONE - OFFSET},
     {TEX_ZERO, TEX_ONE - OFFSET}}, // LOWER_TRIANGE

    {{TEX_ZERO + 2 * OFFSET, TEX_ZERO + OFFSET},
     {TEX_ONE, TEX_ZERO + OFFSET},
     {TEX_ONE, TEX_ONE - OFFSET}} // UPPER_TRIANGE
};

float textTrianCoo[2][3][2] = {

    {{TEX_ZERO, TEX_ZERO}, {TEX_ONE, TEX_ONE}, {TEX_ZERO, TEX_ONE}}, // LOWER_TRIANGE

    {{TEX_ZERO, TEX_ZERO}, {TEX_ONE, TEX_ZERO}, {TEX_ONE, TEX_ONE}} // UPPER_TRIANGE
};

#define LOWER_TRIANGE 0
#define UPPER_TRIANGE 1

void SCRenderer::terrainColor(uint8_t base, uint8_t light, float *color) {
    PaletteRange *range = &this->terrain_ranges[0];
    if (base < this->terrain_ranges.size()) {
        range = &this->terrain_ranges[base];
    }
    int tone = range->first + ((light * range->shades + 128) >> 8);
    if (tone > 255) {
        tone = 255;
    }
    Texel *texel = this->palette.GetRGBColor((uint8_t)tone);
    for (int i = 0; i < 3; i++) {
        uint8_t component = texel->r;
        if (i == 1) {
            component = texel->g;
        }
        if (i == 2) {
            component = texel->b;
        }
        color[i] = this->terrain_black_level + component / 255.0f * (1.0f - this->terrain_black_level);
    }
    color[3] = 1.0f;
}

MapVertex *SCRenderer::highestVertex(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2) {
    MapVertex *highest = tri0;
    if ((uint16_t)tri1->height >= (uint16_t)highest->height) {
        highest = tri1;
    }
    if ((uint16_t)tri2->height >= (uint16_t)highest->height) {
        highest = tri2;
    }
    return highest;
}

void SCRenderer::renderTexturedTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2, RSArea *area,
                                        int triangleType, RSImage *image) {

    uint8_t base = this->highestVertex(tri0, tri1, tri2)->type;
    float color[4];
    float (*coords)[3][2] = textTrianCoo;
    if (image->width < 128) {
        coords = textTrianCoo64;
    }

    this->terrainColor(base, tri0->flag, color);
    glColor4fv(color);
    glTexCoord2fv(coords[triangleType][0]);
    glVertex3f(tri0->v.x, tri0->v.y, tri0->v.z);

    this->terrainColor(base, tri1->flag, color);
    glColor4fv(color);
    glTexCoord2fv(coords[triangleType][1]);
    glVertex3f(tri1->v.x, tri1->v.y, tri1->v.z);

    this->terrainColor(base, tri2->flag, color);
    glColor4fv(color);
    glTexCoord2fv(coords[triangleType][2]);
    glVertex3f(tri2->v.x, tri2->v.y, tri2->v.z);
}

bool SCRenderer::isTextured(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2) {
    return
        // tri0->type != tri1->type ||
        // tri0->type != tri2->type ||
        tri0->upperImageID == 0xFF || tri0->lowerImageID == 0xFF;
}
void SCRenderer::renderColoredTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2) {

    uint8_t base = this->highestVertex(tri0, tri1, tri2)->type;
    float color[4];

    this->terrainColor(base, tri0->flag, color);
    glColor4fv(color);
    glVertex3f(tri0->v.x, tri0->v.y, tri0->v.z);

    this->terrainColor(base, tri1->flag, color);
    glColor4fv(color);
    glVertex3f(tri1->v.x, tri1->v.y, tri1->v.z);

    this->terrainColor(base, tri2->flag, color);
    glColor4fv(color);
    glVertex3f(tri2->v.x, tri2->v.y, tri2->v.z);
}

void SCRenderer::renderQuad(MapVertex *currentVertex, MapVertex *rightVertex, MapVertex *bottomRightVertex,
                            MapVertex *bottomVertex, RSArea *area) {

    
    if (currentVertex->lowerImageID == 0xFF || !this->show_textured) {
        // Render lower triangle
        renderColoredTriangle(currentVertex, bottomRightVertex, bottomVertex);
    }
    if (currentVertex->upperImageID == 0xFF || !this->show_textured) {
        // Render Upper triangles
        renderColoredTriangle(currentVertex, rightVertex, bottomRightVertex);
    }
    if (this->show_textured){
        if (currentVertex->lowerImageID != 0xFF) {
            VertexVector &vcache = textureSortedVertex[currentVertex->lowerImageID];
            VertexCache v = {currentVertex, bottomRightVertex, bottomVertex};
            v.lv1 = currentVertex;
            v.lv2 = bottomRightVertex;
            v.lv3 = bottomVertex;
            v.uv1 = v.uv2 = v.uv3 = NULL;
            vcache.push_back(v);
        }
        if (currentVertex->upperImageID != 0xFF) {
            VertexVector &vcache = textureSortedVertex[currentVertex->upperImageID];
            VertexCache v;
            v.uv1 = currentVertex;
            v.uv2 = rightVertex;
            v.uv3 = bottomRightVertex;
            v.lv1 = v.lv2 = v.lv3 = NULL;
            vcache.push_back(v);
        }
    }
}

void SCRenderer::renderBlock(RSArea *area, int LOD, int i, bool renderTexture,
                 const std::unordered_set<int>* skipRight,
                 const std::unordered_set<int>* skipBottom) {

    AreaBlock *block = area->GetAreaBlockByID(LOD, i);
    if (block == nullptr) {
        return;
    }
    uint32_t sideSize = block->sideSize;

    // printf("Rendering block %d at x %f,z %f\n", i, block->vertice[0].v.x, block->vertice[0].v.z);
    for (size_t x = 0; x < sideSize - 1; x++) {
        for (size_t y = 0; y < sideSize - 1; y++) {

            MapVertex *currentVertex = &block->vertice[x + y * sideSize];
            MapVertex *rightVertex = &block->vertice[(x + 1) + y * sideSize];
            MapVertex *bottomRightVertex = &block->vertice[(x + 1) + (y + 1) * sideSize];
            MapVertex *bottomVertex = &block->vertice[x + (y + 1) * sideSize];

            renderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area);
        }
    }
    
    // Inter-block right side
    if (i % BLOCK_PER_MAP_SIDE != 17) {
        int rightId = static_cast<int>(i + 1);
        if (!skipRight || !skipRight->count(rightId)) {     // <-- ajout
            AreaBlock *currentBlock = area->GetAreaBlockByID(LOD, (size_t)i);
            AreaBlock *rightBlock   = area->GetAreaBlockByID(LOD, (size_t)rightId);
            for (uint32_t y = 0; y < sideSize - 1; y++) {
                MapVertex *currentVertex = currentBlock->GetVertice(currentBlock->sideSize - 1, y);
                MapVertex *rightVertex = rightBlock->GetVertice(0, y);
                MapVertex *bottomRightVertex = rightBlock->GetVertice(0, y + 1);
                MapVertex *bottomVertex = currentBlock->GetVertice(currentBlock->sideSize - 1, y + 1);

                renderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area);
            }
        }
    }

    // Inter-block bottom side
    if (i / BLOCK_PER_MAP_SIDE != 17) {
    int bottomId = i + BLOCK_PER_MAP_SIDE;
        if (!skipBottom || !skipBottom->count(bottomId)) {  // <-- ajout
            AreaBlock *currentBlock = area->GetAreaBlockByID(LOD, i);
            AreaBlock *bottomBlock  = area->GetAreaBlockByID(LOD, bottomId);
            for (uint32_t x = 0; x < sideSize - 1; x++) {
                MapVertex *currentVertex = currentBlock->GetVertice(x, currentBlock->sideSize - 1);
                MapVertex *rightVertex = currentBlock->GetVertice(x + 1, currentBlock->sideSize - 1);
                MapVertex *bottomRightVertex = bottomBlock->GetVertice(x + 1, 0);
                MapVertex *bottomVertex = bottomBlock->GetVertice(x, 0);

                renderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area);
            }
        }
    }

    // Inter bottom-right quad
    if (i % BLOCK_PER_MAP_SIDE != 17 && i / BLOCK_PER_MAP_SIDE != 17) {
        int rightId  = i + 1;
        int bottomId = i + BLOCK_PER_MAP_SIDE;
        bool skipR = skipRight  && skipRight->count(rightId);
        bool skipB = skipBottom && skipBottom->count(bottomId);
        if (!skipR && !skipB) { 
            AreaBlock *currentBlock = area->GetAreaBlockByID(LOD, i);
            AreaBlock *rightBlock = area->GetAreaBlockByID(LOD, i + 1);
            AreaBlock *rightBottonBlock = area->GetAreaBlockByID(LOD, i + 1 + BLOCK_PER_MAP_SIDE);
            AreaBlock *bottomBlock = area->GetAreaBlockByID(LOD, i + BLOCK_PER_MAP_SIDE);

            MapVertex *currentVertex = currentBlock->GetVertice(currentBlock->sideSize - 1, currentBlock->sideSize - 1);
            MapVertex *rightVertex = rightBlock->GetVertice(0, currentBlock->sideSize - 1);
            MapVertex *bottomRightVertex = rightBottonBlock->GetVertice(0, 0);
            MapVertex *bottomVertex = bottomBlock->GetVertice(currentBlock->sideSize - 1, 0);

            renderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area);
        }
    }
    
}
void SCRenderer::renderSkydome(int rings, int slices) {
    // Le dôme ne doit JAMAIS recevoir le brouillard du terrain
    GLboolean fogWasEnabled = glIsEnabled(GL_FOG);
    glDisable(GL_FOG);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);

    const float radius = this->max_view_distance * 0.98f;
    Point3D cam = camera.getPosition();

    // ── Palette : horizon blanc → ciel bleu ─────────────────────────────
    // Clé de couleur : t=0 (équateur) … t=1 (zénith)
    static const float skyT[]   = { 0.00f, 0.08f, 0.25f, 0.60f, 1.00f };
    static const float skyRGB[][3] = {
        { 1.00f, 1.00f, 1.00f },   // horizon : filet blanc pur
        { 0.72f, 0.83f, 0.97f },   // juste au-dessus : bleu très pâle
        { 0.40f, 0.62f, 0.92f },   // ciel moyen
        { 0.18f, 0.44f, 0.82f },   // bleu soutenu
        { 0.08f, 0.28f, 0.72f },   // zénith : bleu profond
    };
    constexpr int N = 5;

    auto sampleSky = [&](float t, float& r, float& g, float& b) {
        if (t <= 0.f) { r=skyRGB[0][0]; g=skyRGB[0][1]; b=skyRGB[0][2]; return; }
        if (t >= 1.f) { r=skyRGB[N-1][0]; g=skyRGB[N-1][1]; b=skyRGB[N-1][2]; return; }
        for (int i = 0; i < N-1; ++i) {
            if (t >= skyT[i] && t <= skyT[i+1]) {
                float f = (t - skyT[i]) / (skyT[i+1] - skyT[i]);
                r = skyRGB[i][0] + f*(skyRGB[i+1][0]-skyRGB[i][0]);
                g = skyRGB[i][1] + f*(skyRGB[i+1][1]-skyRGB[i][1]);
                b = skyRGB[i][2] + f*(skyRGB[i+1][2]-skyRGB[i][2]);
                return;
            }
        }
    };

    // ── Hémisphère supérieure ────────────────────────────────────────────
    for (int r = 0; r < rings; ++r) {
        float t0  = (float)r       / rings;
        float t1  = (float)(r + 1) / rings;
        float phi0 = (float)M_PI_2 * t0;
        float phi1 = (float)M_PI_2 * t1;
        float r0,g0,b0, r1,g1,b1;
        sampleSky(t0, r0,g0,b0);
        sampleSky(t1, r1,g1,b1);

        glBegin(GL_TRIANGLE_STRIP);
        for (int s = 0; s <= slices; ++s) {
            float theta = 2.0f*(float)M_PI*s/slices;
            float cosT=cosf(theta), sinT=sinf(theta);
            glColor3f(r0,g0,b0);
            glVertex3f(cam.x+radius*cosf(phi0)*cosT, cam.y+radius*sinf(phi0), cam.z+radius*cosf(phi0)*sinT);
            glColor3f(r1,g1,b1);
            glVertex3f(cam.x+radius*cosf(phi1)*cosT, cam.y+radius*sinf(phi1), cam.z+radius*cosf(phi1)*sinT);
        }
        glEnd();
    }

    // ── Hémisphère inférieure (masque depth + sol) ───────────────────────
    static const float nadirRGB[3] = { 0.52f, 0.47f, 0.35f };
    for (int r = 0; r < rings; ++r) {
        float t0  = (float)r       / rings;
        float t1  = (float)(r + 1) / rings;
        float phi0 = -(float)M_PI_2 * t0;
        float phi1 = -(float)M_PI_2 * t1;
        float e0 = t0*t0, e1 = t1*t1; // ease-in quadratique

        glBegin(GL_TRIANGLE_STRIP);
        for (int s = 0; s <= slices; ++s) {
            float theta = 2.0f*(float)M_PI*s/slices;
            float cosT=cosf(theta), sinT=sinf(theta);
            glColor3f(1.f+e0*(nadirRGB[0]-1.f), 1.f+e0*(nadirRGB[1]-1.f), 1.f+e0*(nadirRGB[2]-1.f));
            glVertex3f(cam.x+radius*cosf(phi0)*cosT, cam.y+radius*sinf(phi0), cam.z+radius*cosf(phi0)*sinT);
            glColor3f(1.f+e1*(nadirRGB[0]-1.f), 1.f+e1*(nadirRGB[1]-1.f), 1.f+e1*(nadirRGB[2]-1.f));
            glVertex3f(cam.x+radius*cosf(phi1)*cosT, cam.y+radius*sinf(phi1), cam.z+radius*cosf(phi1)*sinT);
        }
        glEnd();
    }

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    if (fogWasEnabled) glEnable(GL_FOG);
}
void SCRenderer::renderEllipsoid(float cx, float cy, float cz, float rx, float ry, float rz, int rings, int slices, float r, float g, float b, float baseAlpha) {
    for (int ring = 0; ring < rings; ++ring) {
        float phi0 = (float)M_PI * ring / rings - (float)M_PI_2;
        float phi1 = (float)M_PI * (ring + 1) / rings - (float)M_PI_2;

        glBegin(GL_TRIANGLE_STRIP);
        for (int s = 0; s <= slices; ++s) {
            float theta = 2.0f * (float)M_PI * s / slices;
            float cosT = cosf(theta), sinT = sinf(theta);

            for (int pass = 0; pass < 2; ++pass) {
                float phi = (pass == 0) ? phi0 : phi1;
                float cosPhi = cosf(phi), sinPhi = sinf(phi);
                // Dégradé : bas transparent, haut opaque
                float t = (sinPhi + 1.0f) * 0.5f;
                glColor4f(r, g, b, baseAlpha * t);
                glVertex3f(cx + rx * cosPhi * cosT,
                           cy + ry * sinPhi,
                           cz + rz * cosPhi * sinT);
            }
        }
        glEnd();
    }
}
void SCRenderer::renderClouds(RSArea *area) {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);  // ou GL_LEQUAL sans depth write
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Point3D cam = camera.getPosition();


    // Trier back-to-front pour l'alpha blending correct
    std::vector<const Cloud*> sorted;
    for (auto& c : area->clouds) {
        if (c.position.x < cam.x - this->max_view_distance || c.position.x > cam.x + this->max_view_distance ||
            c.position.z < cam.z - this->max_view_distance || c.position.z > cam.z + this->max_view_distance) {
            continue; // trop loin
        }
        sorted.push_back(&c);
    }
    std::sort(sorted.begin(), sorted.end(), [&](const Cloud* a, const Cloud* b) {
        float da = (a->position.x-cam.x)*(a->position.x-cam.x)
                 + (a->position.z-cam.z)*(a->position.z-cam.z);
        float db = (b->position.x-cam.x)*(b->position.x-cam.x)
                 + (b->position.z-cam.z)*(b->position.z-cam.z);
        return da > db; // plus loin d'abord
    });

    for (const Cloud* c : sorted) {
        for (const auto& p : c->puffs) {
            this->renderEllipsoid(
                c->position.x + p.ox,
                c->position.y + p.oy,
                c->position.z + p.oz,
                p.rx, p.ry, p.rz,
                6, 12,           // rings, slices (peu de polys = perf ok)
                1.0f, 1.0f, 1.0f, // blanc
                c->alpha
            );
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}
void SCRenderer::renderWorldSkyAndGround() {
    this->renderSkydome(12, 48);

}

void SCRenderer::renderWorldSolid(RSArea *area, int LOD, int verticesPerBlock) {
    GLfloat fogColor[4] = {0.89f, 0.89f, 0.98f, 1.0f};
    if (this->show_fog) {
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_START, this->max_view_distance * 0.40f); // début du fondu
        glFogf(GL_FOG_END,   this->max_view_distance * 0.93f); // finit AVANT le dôme
        glHint(GL_FOG_HINT, GL_DONT_CARE);
        glEnable(GL_FOG);
    } else {
        glDisable(GL_FOG);
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    this->renderWorldSkyAndGround();
    if (this->show_clouds) {
        this->renderClouds(area);
    }
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    textureSortedVertex.clear();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    Plane frustum[6];
    extractFrustumPlanes(frustum);

    Vector3D pos = camera.getPosition();
    std::vector<int> visibleHiLOD;

    for (int i = 0; i < BLOCKS_PER_MAP; ++i) {
        int bx = i % BLOCK_PER_MAP_SIDE;
        int by = i / BLOCK_PER_MAP_SIDE;
        float block_cx = ((float)bx - (float)BLOCK_PER_MAP_SIDE_DIV_2) * (float)BLOCK_WIDTH + (float)BLOCK_WIDTH * 0.5f;
        float block_cz = ((float)by - (float)BLOCK_PER_MAP_SIDE_DIV_2) * (float)BLOCK_WIDTH + (float)BLOCK_WIDTH * 0.5f;
        float dx = pos.x - block_cx;
        float dz = pos.z - block_cz;
        if (dx*dx + dz*dz > (float)this->max_view_distance * (float)this->max_view_distance)
            continue;
        const AABB& box = computeBlockAABB(area, LOD, i);
        if (isAABBVisible(box, frustum))
            visibleHiLOD.push_back(i);
    }


    GLenum terrainFront = GL_CCW;
    int refId = -1; 
    bool refHi = false;
    if (!visibleHiLOD.empty()) {
        AreaBlock* refBlk = area->GetAreaBlockByID(LOD, visibleHiLOD[0]);
        terrainFront = DetectTerrainFrontFace(refBlk);
    }
    if (refId >= 0) {
        AreaBlock* refBlk = area->GetAreaBlockByID(refHi ? LOD : (LOD+1), refId);
        terrainFront = DetectTerrainFrontFace(refBlk);
    }
    glFrontFace(terrainFront);
    // Passe géométrie (non texturée)
    glBegin(GL_TRIANGLES);
    textureSortedVertex.clear();
    for (int id : visibleHiLOD) {
        renderBlock(area, LOD, id, true);
    }
    glEnd();

    // Passe textures: réutilise les mêmes listes visibles (pas de culling)
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    for (auto const &x : textureSortedVertex) {
        RSImage *image = area->GetImageByID(x.first);
        if (!image) {
            printf("This should never happen: Put a break point here.\n");
            continue;
        }
        glBindTexture(GL_TEXTURE_2D, image->GetTexture()->getTextureID());
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < (int)x.second.size(); i++) {
            VertexCache v = x.second.at(i);
            if (v.lv1 != NULL && v.lv1->lowerImageID == x.first) {
                renderTexturedTriangle(v.lv1, v.lv2, v.lv3, area, LOWER_TRIANGE, image);
            }
            if (v.uv1 != NULL && v.uv1->upperImageID == x.first) {
                renderTexturedTriangle(v.uv1, v.uv2, v.uv3, area, UPPER_TRIANGE, image);
            }
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
    // Overlay: inversion de Z -> winding mirroir, on inverse temporairement la front face
    
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    glFrontFace(GL_CCW);
    const auto& skirts = area->GetSkirts();
    glBegin(GL_TRIANGLES);
    for (const auto& tri : skirts.tris) {
        glColor3f(tri.v[0].r, tri.v[0].g, tri.v[0].b);
        glVertex3f(tri.v[0].x, tri.v[0].y, tri.v[0].z);
        glColor3f(tri.v[1].r, tri.v[1].g, tri.v[1].b);
        glVertex3f(tri.v[1].x, tri.v[1].y, tri.v[1].z);
        glColor3f(tri.v[2].r, tri.v[2].g, tri.v[2].b);
        glVertex3f(tri.v[2].x, tri.v[2].y, tri.v[2].z);
    }
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthFunc(GL_LESS);
    glFrontFace(terrainFront);

    renderMapOverlay(area);

    //glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glFrontFace(GL_CCW);
}

void SCRenderer::renderObjects(RSArea *area, size_t blockID) {
    float y = 0;
    for (auto object : area->objects) {

        glPushMatrix();

        glTranslatef(object.position.x, object.position.y, object.position.z);
        if (object.entity != nullptr) {
            drawModel(object.entity, this->lodLevel);
        } else {
            printf("OBJECT [%s] NOT FOUND\n", object.name);
            glBegin(GL_POINTS);
            glColor3f(0, 1, 0);
            glVertex3d(0, 0, 0);
            glEnd();
        }

        glPopMatrix();
    }
}
void SCRenderer::renderWorldToTexture(RSArea *area) {
    

    Vector3D pos = camera.getPosition();
    int centerX = BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    int centerY = BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    int blocX = (int)(pos.x + centerX) / BLOCK_WIDTH;
    int blocY = (int)(pos.z + centerY) / BLOCK_WIDTH;

    
    int block_id = blocY * BLOCK_PER_MAP_SIDE + blocX;
    
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    this->renderWorldSkyAndGround();
    /**/
    bool save_is_textured = this->show_textured;
    this->show_textured = false;
    textureSortedVertex.clear();
    // Render your scene here
    glBegin(GL_TRIANGLES);
    renderBlock(area, 0, block_id, false);
    renderBlock(area, 0, block_id+1, false);
    renderBlock(area, 0, block_id-1, false);
    renderBlock(area, 0, block_id+1*18, false);
    renderBlock(area, 0, block_id-1*18, false);
    renderBlock(area, 0, block_id+1+1*18, false);
    renderBlock(area, 0, block_id+1-1*18, false);
    renderBlock(area, 0, block_id-1+1*18, false);
    renderBlock(area, 0, block_id-1-1*18, false);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    renderBlock(area, 0, block_id, true);
    for (auto const &x : textureSortedVertex) {
        RSImage *image = NULL;
        image = area->GetImageByID(x.first);
        if (image == NULL) {
            printf("This should never happen: Put a break point here.\n");
            return;
        }
        glBindTexture(GL_TEXTURE_2D, image->GetTexture()->getTextureID());

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < x.second.size(); i++) {
            VertexCache v = x.second.at(i);
            if (v.lv1 != NULL && v.lv1->lowerImageID == x.first) {
                renderTexturedTriangle(v.lv1, v.lv2, v.lv3, area, LOWER_TRIANGE, image);
            }
            if (v.uv1 != NULL && v.uv1->upperImageID == x.first) {
                renderTexturedTriangle(v.uv1, v.uv2, v.uv3, area, UPPER_TRIANGE, image);
            }
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
    this->show_textured = save_is_textured;
}
void SCRenderer::initRenderToTexture() {
    if (this->texture == 0) {
        glGenTextures(1, &this->texture);
        glBindTexture(GL_TEXTURE_2D, this->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 107, 75, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glViewport(0, 0, 107, 75);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void SCRenderer::getRenderToTexture() {
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 107, 75, 0);
}
void SCRenderer::initRenderCameraView(){ 
    Matrix *projectionMatrix = camera.getProjectionMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef(0.0f, verticalOffset, 0.0f);
    glMultMatrixf(projectionMatrix->ToGL());
    glMatrixMode(GL_MODELVIEW);
    Matrix *modelViewMatrix = camera.getViewMatrix();
    glLoadMatrixf(modelViewMatrix->ToGL());
}
void SCRenderer::renderMissionObjects(RSMission *mission) {

    float y = 0;
    for (auto object : mission->mission_data.parts) {
        if (object == mission->getPlayerCoord()) {
            continue;
        }
        glPushMatrix();

        glTranslatef(static_cast<GLfloat>(object->position.x), static_cast<GLfloat>(object->position.y),
                     static_cast<GLfloat>(object->position.z));

        float model_view_mat[4][4];
        glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)model_view_mat);
        glRotatef((360.0f - (float)object->azymuth + 90.0f), 0, 1, 0);
        glRotatef((float)object->pitch, 0, 0, 1);
        glRotatef(-(float)object->roll, 1, 0, 0);
        if (object->entity != NULL) {
            drawModel(object->entity, this->lodLevel);
        } else {
            printf("OBJECT [%s] NOT FOUND\n", object->member_name.c_str());
            glBegin(GL_POINTS);
            glColor3f(0, 1, 0);
            glVertex3d(0, 0, 0);
            glEnd();
        }
        glPopMatrix();
    }
}
void SCRenderer::renderLineCube(Vector3D position, int32_t size) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef((float)size, (float)size, (float)size);
    glBegin(GL_LINES);
    // Front face
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);

    // Back face
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);

    // Edges
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);

    glEnd();
    glPopMatrix();
}
void SCRenderer::renderBBox(Vector3D position, Point3D min, Point3D max) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(max.x - min.x, max.y - min.y, max.z - min.z);
    glBegin(GL_LINES);
    // Front face
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);

    // Back face
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);

    // Edges
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);

    glEnd();
    glPopMatrix();
}
void SCRenderer::renderMapOverlay(RSArea *area) {

    glDepthMask(GL_FALSE);            // ne pas écrire dans le depth buffer
    glDepthFunc(GL_LEQUAL);           // accepter l'égalité de profondeur
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    // UV du .TRI : entiers, plage 0..125 (255 = pas de texture). 128 = carreau plein.
    const float TRI_UV_SCALE = 128.0f;

    for (int i = 0; i < area->objectOverlay.size(); i++) {
        AreaOverlay &ov = area->objectOverlay[i];

        // drawList = ordre de dessin résolu depuis la grille param3
        // (ParseTriFile garantit un repli = 0..nbTriangles-1 si la grille est triviale)
        for (size_t k = 0; k < ov.drawList.size(); k++) {
            int j = ov.drawList[k];
            const AreaOverlayTriangles &tri = ov.trianles[j];

            AoVPoints v1 = ov.vertices[tri.verticesIdx[0]];
            AoVPoints v2 = ov.vertices[tri.verticesIdx[1]];
            AoVPoints v3 = ov.vertices[tri.verticesIdx[2]];

            // hypothèse : type == 6 => face texturée, color == index de texture.
            // color == 255 (<=> uv 255) reste une face à-plat même si type == 6 (région 0).
            bool textured = this->show_textured && (tri.color != 255);
            if (textured) {
                RSImage *tex = area->GetImageByID(tri.color+255);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, tex->GetTexture()->id);
                const Texel *texel = palette.GetRGBColor(tri.color);
                glColor4f(texel->r / 255.0f, texel->g / 255.0f, texel->b / 255.0f, 1.0f);

                glBegin(GL_TRIANGLES);
                glTexCoord2f(tri.uv[0].u / TRI_UV_SCALE, tri.uv[0].v / TRI_UV_SCALE);
                glVertex3f((GLfloat)v1.x, (GLfloat)v1.y, (GLfloat)v1.z);
                glTexCoord2f(tri.uv[1].u / TRI_UV_SCALE, tri.uv[1].v / TRI_UV_SCALE);
                glVertex3f((GLfloat)v2.x, (GLfloat)v2.y, (GLfloat)v2.z);
                glTexCoord2f(tri.uv[2].u / TRI_UV_SCALE, tri.uv[2].v / TRI_UV_SCALE);
                glVertex3f((GLfloat)v3.x, (GLfloat)v3.y, (GLfloat)v3.z);
                glEnd();
            } else {
                glDisable(GL_TEXTURE_2D);
                const Texel *texel = palette.GetRGBColor(tri.color);
                glColor4f(texel->r / 255.0f, texel->g / 255.0f, texel->b / 255.0f, 1.0f);

                glBegin(GL_TRIANGLES);
                glVertex3f((GLfloat)v1.x, (GLfloat)v1.y, (GLfloat)v1.z);
                glVertex3f((GLfloat)v2.x, (GLfloat)v2.y, (GLfloat)v2.z);
                glVertex3f((GLfloat)v3.x, (GLfloat)v3.y, (GLfloat)v3.z);
                glEnd();
            }
        }
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}
void SCRenderer::renderWorldByID(RSArea *area, int LOD, int verticesPerBlock, int blockId) {
    textureSortedVertex.clear();
    printf("X:%f,Y:%f", area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.x,
           area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.z);
    glPushMatrix();
    glScalef(1 / 100.0f, 1 / 100.0f, 1 / 100.0f);
    glTranslatef(-area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.x - 12500, 0,
                 -area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.z - 12500);
    //
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBegin(GL_TRIANGLES);
    renderBlock(area, LOD, blockId, false);
    glEnd();

    renderBlock(area, LOD, blockId, true);
    glEnable(GL_TEXTURE_2D);
    for (auto const &x : textureSortedVertex) {

        RSImage *image = NULL;

        image = area->GetImageByID(x.first);
        if (image == NULL) {
            printf("This should never happen: Put a break point here.\n");
            return;
        }
        glBindTexture(GL_TEXTURE_2D, image->GetTexture()->getTextureID());

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < x.second.size(); i++) {
            VertexCache v = x.second.at(i);
            if (v.lv1 != NULL && v.lv1->lowerImageID == x.first) {
                renderTexturedTriangle(v.lv1, v.lv2, v.lv3, area, LOWER_TRIANGE, image);
            }
            if (v.uv1 != NULL && v.uv1->upperImageID == x.first) {
                renderTexturedTriangle(v.uv1, v.uv2, v.uv3, area, UPPER_TRIANGE, image);
            }
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
    renderObjects(area, blockId);
    glPopMatrix();
}

void SCRenderer::drawBillboard(Vector3D pos, Texture *tex, float size, float alpha) {
    if (!initialized || tex == nullptr)
        return;
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    // Initialize texture if needed
    if (!tex->initialized) {
        glGenTextures(1, &tex->id);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)tex->width, (GLsizei)tex->height, 
                     0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
        
        tex->initialized = true;
    } else {
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }
    
    // Get camera position
    Point3D cameraPos = camera.getPosition();
    
    // Calculate billboard vectors
    Vector3D look, right, up;
    
    // Look vector points from billboard to camera
    look.x = cameraPos.x - pos.x;
    look.y = cameraPos.y - pos.y;
    look.z = cameraPos.z - pos.z;
    look.Normalize();
    
    // Right vector is perpendicular to look and global up
    Vector3D globalUp = {0.0f, 1.0f, 0.0f};
    right = globalUp.CrossProduct(&look);
    right.Normalize();
    
    // Up vector completes the orthogonal basis
    up = look.CrossProduct(&right);
    
    // Calculate the corners of the billboard quad
    float halfSize = size * 0.5f;
    
    Vector3D bottomLeft, bottomRight, topRight, topLeft;
    
    bottomLeft.x = pos.x - right.x * halfSize - up.x * halfSize;
    bottomLeft.y = pos.y - right.y * halfSize - up.y * halfSize;
    bottomLeft.z = pos.z - right.z * halfSize - up.z * halfSize;
    
    bottomRight.x = pos.x + right.x * halfSize - up.x * halfSize;
    bottomRight.y = pos.y + right.y * halfSize - up.y * halfSize;
    bottomRight.z = pos.z + right.z * halfSize - up.z * halfSize;
    
    topRight.x = pos.x + right.x * halfSize + up.x * halfSize;
    topRight.y = pos.y + right.y * halfSize + up.y * halfSize;
    topRight.z = pos.z + right.z * halfSize + up.z * halfSize;
    
    topLeft.x = pos.x - right.x * halfSize + up.x * halfSize;
    topLeft.y = pos.y - right.y * halfSize + up.y * halfSize;
    topLeft.z = pos.z - right.z * halfSize + up.z * halfSize;
    
    // Draw the billboard quad
    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
    
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
    
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(topRight.x, topRight.y, topRight.z);
    
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(topLeft.x, topLeft.y, topLeft.z);
    glEnd();
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
}