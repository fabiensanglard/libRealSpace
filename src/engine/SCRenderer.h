//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
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
    GLuint texture;
    SCRenderer();
    ~SCRenderer();

    void init(int width, int height, AssetManager *amana);

    void clear(void);

    void drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation, Vector3D ajustement);
    void drawModel(RSEntity *object, size_t lodLevel, Vector3D position, Vector3D orientation);
    void drawModel(RSEntity *object, size_t lodLevel);
    void displayModel(RSEntity *object, size_t lodLevel);

    void createTextureInGPU(Texture *texture);
    void uploadTextureContentToGPU(Texture *texture);
    void deleteTextureInGPU(Texture *texture);

    VGAPalette *getPalette(void);

    // Map Rendering

    void renderTexturedTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2, RSArea *area, int triangleType,
                                RSImage *image);
    void renderColoredTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2);
    bool isTextured(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2);
    void renderQuad(MapVertex *currentVertex, MapVertex *rightVertex, MapVertex *bottomRightVertex,
                    MapVertex *bottomVertex, RSArea *area, bool renderTexture);

    void renderBlock(RSArea *area, int LOD, int blockID, bool renderTexture);
    void renderWorldSolid(RSArea *area, int LOD, int verticesPerBlock);
    void renderWorldByID(RSArea *area, int LOD, int verticesPerBlock, int blockId);
    void renderObjects(RSArea *area, size_t blockID);
    void renderLineCube(Vector3D position, int32_t size);
    void renderBBox(Vector3D position, Point3D min, Point3D max);
    Camera *getCamera(void);

    void setLight(Point3D *position);

    void setClearColor(uint8_t red, uint8_t green, uint8_t blue);

    void prepare(RSEntity *object);
    void renderMissionObjects(RSMission *mission);
    void renderMapOverlay(RSArea *area);
    void setPlayerPosition(Point3D *position);
    void renderWorldToTexture(RSArea *area);

    void initRenderToTexture();
    void getRenderToTexture();
    void initRenderCameraView();

private:
    bool initialized;
    AssetManager *assets;
    void getNormal(RSEntity *object, Triangle *triangle, Vector3D *normal);
    void renderWorldSkyAndGround();
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
    GLuint framebuffer;
    
};
