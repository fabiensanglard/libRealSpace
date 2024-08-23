//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <Windows.h>
#include <GL/GL.h>
#include "Camera.h"
#include "Texture.h"
#include "RSMission.h"

class RSArea;
class Texture;
class RSEntity;
//struct VGAPalette;
struct MapVertex;
struct Triangle;
typedef struct VertexCache {
    MapVertex *lv1;
    MapVertex *lv2;
    MapVertex *lv3;
    MapVertex* uv1;
    MapVertex* uv2;
    MapVertex* uv3;

}VertexCache;

typedef std::vector<VertexCache>    VertexVector;
typedef std::map<int, VertexVector>   TextureVertexMap;


class SCRenderer{
    
public:
    
     SCRenderer();
    ~SCRenderer();
    
    void Prepare(void);
    
    void Init(int32_t zoom);
    
    void Clear(void);



    void DrawModel(RSEntity* object, size_t lodLevel);
    void DisplayModel(RSEntity* object,size_t lodLevel);
    
   
    
    void CreateTextureInGPU(Texture* texture);
    void UploadTextureContentToGPU(Texture* texture);
    void DeleteTextureInGPU(Texture* texture);
    
    VGAPalette* GetPalette(void);
    
    
    //Map Rendering
    //For research methods: Those should be deleted soon:
    void RenderVerticeField(Point3D* vertices, int numVertices);
    
    void RenderWorldPoints(RSArea* area, int LOD, int verticesPerBlock);

    void RenderTexturedTriangle(MapVertex* tri0,MapVertex* tri1,MapVertex* tri2,RSArea* area,int triangleType,RSImage* image);
    void RenderColoredTriangle (MapVertex* tri0,MapVertex* tri1,MapVertex* tri2);
    bool IsTextured(MapVertex* tri0,MapVertex* tri1,MapVertex* tri2);
    void RenderQuad(MapVertex* currentVertex,
                           MapVertex* rightVertex,
                           MapVertex* bottomRightVertex,
                           MapVertex* bottomVertex,
                           RSArea* area,bool renderTexture);
    
    void RenderBlock(RSArea* area,int LOD, int blockID,bool renderTexture);
    void RenderWorldSolid(RSArea* area, int LOD, int verticesPerBlock);
	void RenderWorld(RSArea* area, int LOD, int verticesPerBlock);
    void RenderWorldByID(RSArea* area, int LOD, int verticesPerBlock, int blockId);
    void RenderObjects(RSArea* area,size_t blockID);
    
    
    void RenderJets(RSArea* area);
    
    
    
   
    Camera* GetCamera(void);
    
    void SetLight(Point3D* position);
    
    inline bool IsPaused(void){
        return this->paused;
    }
    
    inline void Pause(void){
        this->paused = true;
    }

   
    
    void SetClearColor(uint8_t red, uint8_t green, uint8_t blue);
    
    void Prepare(RSEntity* object);
    void RenderMissionObjects(RSMission* mission);
    void RenderMapOverlay(RSArea* area);
    void setPlayerPosition(Point3D* position);
private:
    
    
    bool initialized;
    
    void GetNormal(RSEntity* object,Triangle* triangle,Point3D* normal);
    
    
    
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


/*
void IMG_Init(void);
void IMG_ShowPalette(Palette* palette,int cellSize);
void IMG_ShowImage(uint8_t* image, uint16_t width, uint16_t height,Palette* palette,int zoom,bool wait);
void IMG_ShowModel(RealSpaceObject* object,Palette* palette );
*/

