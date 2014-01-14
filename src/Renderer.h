//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__gfx__
#define __iff__gfx__

class RSEntity;

class Triangle;
class RSArea;
class MapVertex;

class Renderer{
    
public:
    
     Renderer();
    ~Renderer();
    
    void Init(int32_t width,int32_t height);
    
    void Clear(void);

    void ShowPalette(VGAPalette* palette);
    void DrawImage(RSImage* image,int zoom);
    void DrawModel(RSEntity* object, size_t lodLevel);
    void DisplayModel(RSEntity* object,size_t lodLevel);
    
    void SetTitle(const char* title);
    
    void CreateTextureInGPU(Texture* texture);
    void UploadTextureContentToGPU(Texture* texture);
    void DeleteTextureInGPU(Texture* texture);
    
    VGAPalette* GetDefaultPalette(void);
    
    
    //Map Rendering
    //For research methods: Those should be deleted soon:
    void RenderVerticeField(Point3D* vertices, int numVertices);
    
    void RenderWorldPoints(RSArea* area, int LOD, int verticesPerBlock);

    void RenderTexturedTriangle(MapVertex* tri0,MapVertex* tri1,MapVertex* tri2,RSArea* area,int triangleType);
    void RenderColoredTriangle (MapVertex* tri0,MapVertex* tri1,MapVertex* tri2);
    bool IsTextured(MapVertex* tri0,MapVertex* tri1,MapVertex* tri2);
    void RenderQuad(MapVertex* currentVertex,
                           MapVertex* rightVertex,
                           MapVertex* bottomRightVertex,
                           MapVertex* bottomVertex,
                           RSArea* area,bool renderTexture);
    
    void RenderBlock(RSArea* area,int LOD, int blockID,bool renderTexture);
    void RenderWorldSolid(RSArea* area, int LOD, int verticesPerBlock);
    
    
    
    
    
    
    
    VGAPalette* GetPalette(void);
    
    void Swap(void);
    void ShowWindow(void);
    void PumpEvents(void);
    
    Camera* GetCamera(void);
    void SetLight(Point3D* position);
    
    inline bool IsPaused(void){
        return this->paused;
    }
    
    inline void Pause(void){
        this->paused = true;
    }
    
    void SetPalette(VGAPalette* palette);
    void ResetPalette(void);
    VGAPalette* GetCurrentPalette(void);
    
    void SetClearColor(uint8_t red, uint8_t green, uint8_t blue);
    
private:
    
    
    void GetNormal(RSEntity* object,Triangle* triangle,Point3D* normal);
    
    bool initialized;
    
    int32_t width;
    int32_t height;

    
    VGAPalette defaultPalette;
    VGAPalette* currentPalette;
    bool running;
    bool paused;
    
    Camera camera;
    Point3D light;
};


extern Renderer renderer;

/*
void IMG_Init(void);
void IMG_ShowPalette(Palette* palette,int cellSize);
void IMG_ShowImage(uint8_t* image, uint16_t width, uint16_t height,Palette* palette,int zoom,bool wait);
void IMG_ShowModel(RealSpaceObject* object,Palette* palette );
*/
#endif /* defined(__iff__gfx__) */
