//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "AssetManager.h"
#include "Camera.h"
#include "RSMission.h"
#include "Texture.h"
#ifdef _WIN32
#include <GL/GL.h>
#include <Windows.h>
#include <algorithm>
#endif
#define MAP_SIZE 200000
class RSArea;
class Texture;
class RSEntity;
// struct VGAPalette;
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
typedef std::map<int, VertexVector> TextureVertexMap;

class SCRenderer {

public:
    SCRenderer();
    ~SCRenderer();

    void Prepare(void);

    void Init(int width, int height, AssetManager *amana);

    void Clear(void);

    void DrawModel(RSEntity *object, size_t lodLevel);
    void DisplayModel(RSEntity *object, size_t lodLevel);

    void CreateTextureInGPU(Texture *texture);
    void UploadTextureContentToGPU(Texture *texture);
    void DeleteTextureInGPU(Texture *texture);

    VGAPalette *GetPalette(void);

    // Map Rendering

    void RenderTexturedTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2, RSArea *area, int triangleType,
                                RSImage *image);
    void RenderColoredTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2);
    bool IsTextured(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2);
    void RenderQuad(MapVertex *currentVertex, MapVertex *rightVertex, MapVertex *bottomRightVertex,
                    MapVertex *bottomVertex, RSArea *area, bool renderTexture);

    void RenderBlock(RSArea *area, int LOD, int blockID, bool renderTexture);
    void RenderWorldSolid(RSArea *area, int LOD, int verticesPerBlock);
    void RenderWorldByID(RSArea *area, int LOD, int verticesPerBlock, int blockId);
    void RenderObjects(RSArea *area, size_t blockID);
    void RenderLineCube(Vector3D position, int32_t size);
    void RenderBBox(Vector3D position, Point3D min, Point3D max);
    Camera *GetCamera(void);

    void SetLight(Point3D *position);

    inline bool IsPaused(void) { return this->paused; }

    inline void Pause(void) { this->paused = true; }

    void SetClearColor(uint8_t red, uint8_t green, uint8_t blue);

    void Prepare(RSEntity *object);
    void RenderMissionObjects(RSMission *mission);
    void RenderMapOverlay(RSArea *area);
    void setPlayerPosition(Point3D *position);

private:
    bool initialized;
    AssetManager *assets;
    void GetNormal(RSEntity *object, Triangle *triangle, Vector3D *normal);
    void RenderWorldSkyAndGround();
    int32_t width;
    int32_t height;
    int scale;

    VGAPalette palette;
    bool running;
    bool paused;
    uint32_t counter;
    Camera camera;
    Point3D light;
    Point3D playerPosition;
    TextureVertexMap textureSortedVertex;
};
