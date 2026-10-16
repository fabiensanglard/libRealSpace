//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <unordered_map>
#include <unordered_set>
#include "Camera.h"
#include "../realspace/AssetManager.h"
#include "../realspace/RSMission.h"
#include "../realspace/RSImage.h"
#include "Texture.h"
#ifdef _WIN32
#include <GL/GL.h>
#include <Windows.h>
#include <algorithm>
#endif
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#define MAP_SIZE 200000
class RSArea;
class RSEntity;
struct MapVertex;
struct Triangle;
typedef struct VertexCache {
    MapVertex *lv1;
    MapVertex *lv2;
    MapVertex *lv3;
    MapVertex *uv1;
    MapVertex *uv2;
    MapVertex *uv3;

} VertexCache;


typedef std::vector<VertexCache> VertexVector;
typedef std::unordered_map<int, VertexVector> TextureVertexMap;
// Petit utilitaire pour le culling
struct Plane {
    float a, b, c, d; // ax + by + cz + d = 0
};
struct AABB {
    Vector3D min;
    Vector3D max;
};
class SCRenderer {
private:
    inline static std::unique_ptr<SCRenderer> s_instance{};
    bool initialized;
    int scale;
    VGAPalette palette;
    std::vector<PaletteRange> terrain_ranges;
    bool running;
    bool paused;
    uint32_t counter;
    
    
    Point3D playerPosition;
    TextureVertexMap textureSortedVertex;
    GLuint framebuffer;
    
    std::unordered_map<uint64_t, AABB> blockAABBCache_;
    struct AreaAABBCache {
        std::unordered_map<uint64_t, AABB> byKey;
    };
    std::unordered_map<const RSArea*, AreaAABBCache> aabbCache_;

    void extractFrustumPlanes(Plane planes[6]) const;
    static bool isAABBVisible(const AABB& box, const Plane planes[6]);
    
    const AABB &computeBlockAABB(RSArea *area, int LOD, int blockId);
    void getNormal(RSEntity *object, Triangle *triangle, Vector3D *normal);
    void renderWorldSkyAndGround();
    void renderSkydome(int rings, int slices);

public:
    Camera camera;
    static SCRenderer& getInstance() {
        if (!SCRenderer::hasInstance()) {
            SCRenderer::setInstance(std::make_unique<SCRenderer>());
        }
        SCRenderer& instance = SCRenderer::instance();
        return instance;
    };
    static SCRenderer& instance() {
        return *s_instance;
    }
    static void setInstance(std::unique_ptr<SCRenderer> inst) {
        s_instance = std::move(inst);
    }

    static bool hasInstance() { return (bool)s_instance; }
    float verticalOffset{0.45f};
    float fov{45.0f};
    int max_view_distance{160000};
    bool show_fog{true};
    float terrain_shadow_lift{0.08f};
    float terrain_shadow_range{0.35f};
    bool terrain_flat{false};
    bool show_textured{true};
    bool show_clouds{false};
    int lodLevel{0};
    GLuint texture{0};
    
    SCRenderer();
    ~SCRenderer();

    void init(int width, int height);

    void clear(void);
    void resetCameraPerspective();
    void drawParticle(Vector3D pos, float alpha);

    void drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation, Vector3D ajustement,
                   float scale);
    void drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation, Vector3D ajustement);
    void drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation);
    void drawModel(RSEntity *object, Vector3D position, Vector3D orientation);
    void drawLine(Vector3D start, Vector3D end, Vector3D color, Vector3D orientation);
    void drawLine(Vector3D start, Vector3D end, Vector3D color, Vector3D orientation, Vector3D position);
    void drawLine(Vector3D start, Vector3D end, Vector3D color);
    void drawPoint(Vector3D point, Vector3D color, Vector3D pos, Vector3D orientation);
    void drawSprite(Vector3D pos, Texture *tex, float zoom);
    void drawModelWithChilds(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation,
                             int wheel_index, int thrust, std::vector<std::tuple<Vector3D, RSEntity *>> weaps_load);
    void drawModel(RSEntity *object, size_t lodLevel);
    void drawModelColorPass(RSEntity *object, size_t lodLevel, std::vector<Vector3D> &vertexNormals, float ambientLamber, Vector3D lightEye, float *MV);
    void drawModelTexturePass(RSEntity *object, size_t lodLevel, std::vector<Vector3D> &vertexNormals, float ambientLamber, Vector3D lightEye, float *MV);
    void drawModelTransparentPass(RSEntity *object, size_t lodLevel, std::vector<Vector3D> &vertexNormals, float ambientLamber, Vector3D lightEye, float *MV);
    void drawBillboard(Vector3D pos, Texture *tex, float size, float alpha = 1.0f);
    void drawTracer(Vector3D pos, Vector3D velocityDir, float length, float pixelWidth, Vector3D color, float alpha);
    void displayModel(RSEntity *object, size_t lodLevel);
     
    void createTextureInGPU(Texture *texture);
    void uploadTextureContentToGPU(Texture *texture);
    void deleteTextureInGPU(Texture *texture);
    void drawTexturedQuad(Vector3D pos, Vector3D orientation,  std::vector<Vector3D> quad, Texture *tex);
    void renderEllipsoid(float cx, float cy, float cz, float rx, float ry, float rz, int rings, int slices, float r, float g, float b, float baseAlpha);
    VGAPalette *getPalette(void);
    void renderClouds(RSArea *area);
    // Map Rendering

    void renderTexturedTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2, RSArea *area, int triangleType,
                                RSImage *image);
    void renderColoredTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2);
    void terrainColor(uint8_t base, uint8_t light, float *color);
    int terrainTone(uint8_t base, uint8_t light);
    void toneColor(int tone, float *color);
    void triangleColors(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2, float colors[3][4]);
    MapVertex *highestVertex(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2);
    bool isTextured(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2);
    void renderQuad(MapVertex *currentVertex, MapVertex *rightVertex, MapVertex *bottomRightVertex,
                    MapVertex *bottomVertex, RSArea *area);

    void renderBlock(RSArea *area, int LOD, int i, bool renderTexture,
                 const std::unordered_set<int>* skipRight  = nullptr,
                 const std::unordered_set<int>* skipBottom = nullptr);
    void renderWorldSolid(RSArea *area, int LOD, int verticesPerBlock);
    void renderWorldByID(RSArea *area, int LOD, int verticesPerBlock, int blockId);
    void renderObjects(RSArea *area, size_t blockID);
    void renderLineCube(Vector3D position, int32_t size);
    void renderBBox(Vector3D position, Point3D min, Point3D max);
    
    Camera *getCamera(void);

    void setLight(Point3D *position);
    void setObjectLight(int ambient_weight, int light_weight, int c0, int c1, int c2);

    void setClearColor(uint8_t red, uint8_t green, uint8_t blue);

    void prepare(RSEntity *object);
    void renderMissionObjects(RSMission *mission);
    void renderMapOverlay(RSArea *area);
    void setPlayerPosition(Point3D *position);
    void renderWorldToTexture(RSArea *area);

    void initRenderToTexture();
    void getRenderToTexture();
    void initRenderCameraView();
    
    // Invalidation cache AABB
    void InvalidateAABBCache();
    void InvalidateAABBCache(RSArea* area);
    void InvalidateAABBForBlock(RSArea* area, int LOD, int blockId);

    // Pré-calcul (optionnel) sur une plage de LOD
    void PrecomputeAABBs(RSArea* area, int minLOD, int maxLOD);
    void bindCameraProjectionAndView(float verticalOffset);
    void bindCameraProjectionAndViewViewport(int32_t viewportW, int32_t viewportH, float verticalOffset);
    int32_t width;
    int32_t height;
    Point3D light;
    Vector3D object_light_direction{0.0f, 1.0f, 0.0f};
    float object_light_ambient{0.0f};
    float object_light_share{0.0f};
    Vector3D shading_light_local{0.0f, 1.0f, 0.0f};
    Vector3D shading_eye_local{0.0f, 0.0f, 0.0f};
    std::unordered_map<RSEntity *, std::vector<Vector3D>> object_normals;
    std::vector<Vector3D> &objectNormals(RSEntity *object);
    float objectIntensity(Vector3D normal);
    Vector3D shadeAlongRamp(int colour, float intensity);

     
};
