//
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "SCRenderer.h"
#include "IffLexer.h"
#include "RSArea.h"
#include "RSEntity.h"
#include "RSPalette.h"
#include "RSVGA.h"
#include "Texture.h"

#ifdef BLOCK_WIDTH
#undef BLOCK_WIDTH
#endif
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#define BLOCK_WIDTH (25000)

extern SCRenderer Renderer;

SCRenderer::SCRenderer() : initialized(false) {}

SCRenderer::~SCRenderer() {}

Camera *SCRenderer::GetCamera(void) { return &this->camera; }

VGAPalette *SCRenderer::GetPalette(void) { return &this->palette; }

void SCRenderer::setPlayerPosition(Point3D *position) { camera.SetPosition(position); }
void SCRenderer::Init(int32_t zoomFactor) {

    this->scale = zoomFactor;
    this->counter = 0;
    int32_t width = 320 * scale;
    int32_t height = 200 * scale;

    // Load the default palette
    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");

    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();

    this->width = width;
    this->height = height;

    glClearColor(0.0f, 0.5f, 1.0f, 1.0f); // Black Background
    // glClearDepth(1.0f);								// Depth Buffer Setup
    glDisable(GL_DEPTH_TEST); // Disable Depth Testing

    // camera.SetPersective(50.0f,this->width/(float)this->height,10.0f,12000.0f);
    camera.SetPersective(50.0f, this->width / (float)this->height, 1.0f, MAP_SIZE*6);

    light.SetWithCoo(300, 300, 300);

    initialized = true;
}

void SCRenderer::SetClearColor(uint8_t red, uint8_t green, uint8_t blue) {
    if (!initialized)
        return;

    glClearColor(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
}

void SCRenderer::Clear(void) {

    if (!initialized)
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(0, 0, 1, 1);
}

void SCRenderer::CreateTextureInGPU(Texture *texture) {

    if (!initialized)
        return;

    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texture->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void SCRenderer::UploadTextureContentToGPU(Texture *texture) {
    if (!initialized)
        return;
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texture->data);
}

void SCRenderer::DeleteTextureInGPU(Texture *texture) {
    if (!initialized)
        return;
    glDeleteTextures(1, &texture->id);
}

void SCRenderer::GetNormal(RSEntity *object, Triangle *triangle, Vector3D *normal) {
    // Calculate the normal for this triangle
    Vector3D edge1;
    edge1 = object->vertices[triangle->ids[0]];
    edge1.Substract(&object->vertices[triangle->ids[1]]);

    Vector3D edge2;
    edge2 = object->vertices[triangle->ids[2]];
    edge2.Substract(&object->vertices[triangle->ids[1]]);

    *normal = edge1;
    *normal = normal->CrossProduct(&edge2);
    normal->Normalize();

    // All normals are supposed to point outward in modern GPU but SC triangles
    // don't have consistent winding. They can be CW or CCW (the back governal of a jet  is
    // typically one triangle that must be visible from both sides !
    // As a result, gouraud shading was probably performed in screen space.
    // How can we replicate this ?
    //        - Take the normal and compare it to the sign of the direction to the camera.
    //        - If the signs don't match: reverse the normal.
    Point3D cameraPosition = camera.GetPosition();

    Point3D *vertexOnTriangle = &object->vertices[triangle->ids[0]];

    Point3D cameraDirection;
    cameraDirection = cameraPosition;
    cameraDirection.Substract(vertexOnTriangle);
    cameraDirection.Normalize();

    if (cameraDirection.DotProduct(normal) < 0) {
        normal->Negate();
    }
}

void SCRenderer::DrawModel(RSEntity *object, size_t lodLevel) {

    if (!initialized)
        return;

    if (object->vertices.size() == 0)
        return;

    if (lodLevel >= object->NumLods()) {
        printf("Unable to render this Level Of Details (out of range): Max level is  %llu %llu\n",
               (object->NumLods() - 1), lodLevel);
        return;
    }

    float ambientLamber = 0.4f;

    Lod *lod = &object->lods[lodLevel];

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Texture pass
    if (lodLevel == 0) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        // glAlphaFunc ( GL_ALWAYS, 1.0f ) ;
        // glEnable ( GL_ALPHA_TEST ) ;

        for (int i = 0; i < object->NumUVs(); i++) {

            uvxyEntry *textInfo = &object->uvs[i];

            // Seems we have a textureID that we don't have :( !
            if (textInfo->textureID >= object->images.size())
                continue;

            RSImage *image = object->images[textInfo->textureID];

            Texture *texture = image->GetTexture();
            Triangle *triangle = &object->triangles[textInfo->triangleID];
            float alpha = 1.0f;
            int colored = 0;
            if (triangle->property == 6) {
                alpha = 0.0f;
            }
            if (triangle->property == 7) {
                alpha = 1.0f;
                colored = 1;
            }
            if (triangle->property == 8) {
                alpha = 1.0f;
                colored = 1;
            }
            if (triangle->property == 9) {
                alpha = 0.0f;
            }

            glBindTexture(GL_TEXTURE_2D, texture->id);
            Vector3D normal;
            GetNormal(object, triangle, &normal);

            glBegin(GL_TRIANGLES);
            for (int j = 0; j < 3; j++) {

                Point3D vertice = object->vertices[triangle->ids[j]];

                Vector3D lighDirection;
                lighDirection = light;
                lighDirection.Substract(&vertice);
                lighDirection.Normalize();

                float lambertianFactor = lighDirection.DotProduct(&normal);
                if (lambertianFactor < 0)
                    lambertianFactor = 0;

                lambertianFactor += ambientLamber;
                if (lambertianFactor > 1)
                    lambertianFactor = 1;

                const Texel *texel = palette.GetRGBColor(triangle->color);

                //
                // glColor4f(texel->r/255.0f, texel->g/255.0f, texel->b/255.0f,alpha);
                // glColor4f(0, 0, 0,1);
                if (colored) {
                    glColor4f(texel->r / 255.0f * lambertianFactor, texel->g / 255.0f * lambertianFactor,
                              texel->b / 255.0f * lambertianFactor, alpha);
                } else {
                    glColor4f(lambertianFactor, lambertianFactor, lambertianFactor, alpha);
                }
                glTexCoord2f(textInfo->uvs[j].u / (float)texture->width, textInfo->uvs[j].v / (float)texture->height);
                glVertex3f(object->vertices[triangle->ids[j]].x, object->vertices[triangle->ids[j]].y,
                           object->vertices[triangle->ids[j]].z);
            }
            glEnd();
        }
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }

    // Pass 3: Let's draw the transparent stuff render RSEntity::SC_TRANSPARENT)
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
#ifndef _WIN32

    glBlendEquation(GL_ADD);
#else
    typedef void(APIENTRY * PFNGLBLENDEQUATIONPROC)(GLenum mode);
    PFNGLBLENDEQUATIONPROC glBlendEquation = NULL;
    glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation");
    glBlendEquation(GL_ADD);
    // glDepthFunc(GL_LESS);
#endif

    for (int i = 0; i < lod->numTriangles; i++) {

        uint16_t triangleID = lod->triangleIDs[i];

        Triangle *triangle = &object->triangles[triangleID];

        if (triangle->property != RSEntity::SC_TRANSPARENT)
            continue;

        Vector3D normal;
        GetNormal(object, triangle, &normal);

        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; j++) {

            Point3D vertice = object->vertices[triangle->ids[j]];

            Vector3D sunDirection;
            sunDirection = light;
            sunDirection.Substract(&vertice);
            sunDirection.Normalize();

            float lambertianFactor = sunDirection.DotProduct(&normal);
            if (lambertianFactor < 0)
                lambertianFactor = 0;

            lambertianFactor = 0.2f;

            // int8_t gouraud = 255 * lambertianFactor;

            // gouraud = 255;

            glColor4f(lambertianFactor, lambertianFactor, lambertianFactor, 1);

            glVertex3f(vertice.x, vertice.y, vertice.z);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Pass 1, draw color
    for (int i = 0; i < lod->numTriangles; i++) {
        // for(int i = 60 ; i < 62 ; i++){  //Debug purpose only back governal of F-16 is 60-62

        uint16_t triangleID = lod->triangleIDs[i];

        Triangle *triangle = &object->triangles[triangleID];

        if (triangle->property == RSEntity::SC_TRANSPARENT)
            continue;
        if (triangle->property == 6) {
            continue;
        }
        if (triangle->property == 7) {
            continue;
        }
        if (triangle->property == 8) {
            continue;
        }
        if (triangle->property == 9) {
            continue;
        }
        Vector3D normal;
        GetNormal(object, triangle, &normal);

        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; j++) {

            Point3D vertice = object->vertices[triangle->ids[j]];

            Vector3D lighDirection;
            lighDirection = light;
            lighDirection.Substract(&vertice);
            lighDirection.Normalize();

            float lambertianFactor = lighDirection.DotProduct(&normal);
            if (lambertianFactor < 0)
                lambertianFactor = 0;

            lambertianFactor += ambientLamber;
            if (lambertianFactor > 1)
                lambertianFactor = 1;

            const Texel *texel = palette.GetRGBColor(triangle->color);

            glColor4f(texel->r / 255.0f * lambertianFactor, texel->g / 255.0f * lambertianFactor,
                      texel->b / 255.0f * lambertianFactor, texel->a);
            // glColor4f(texel->r/255.0f, texel->g/255.0f, texel->b/255.0f,1);

            glVertex3f(object->vertices[triangle->ids[j]].x, object->vertices[triangle->ids[j]].y,
                       object->vertices[triangle->ids[j]].z);
        }
        glEnd();
    }

    glDisable(GL_BLEND);
}

void SCRenderer::SetLight(Point3D *l) { this->light = *l; }

void SCRenderer::Prepare(RSEntity *object) {

    for (size_t i = 0; i < object->NumImages(); i++) {
        object->images[i]->SyncTexture();
    }

    object->prepared = true;
}

void SCRenderer::DisplayModel(RSEntity *object, size_t lodLevel) {

    if (!initialized)
        return;

    if (object->IsPrepared())
        Prepare(object);

    glMatrixMode(GL_PROJECTION);
    Matrix *projectionMatrix = camera.GetProjectionMatrix();
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

        modelViewMatrix = camera.GetViewMatrix();
        glLoadMatrixf(modelViewMatrix->ToGL());

        DrawModel(object, lodLevel);

        // Render light

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
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
#define OFFSET (2 / 64.0f)
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

void SCRenderer::RenderTexturedTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2, RSArea *area,
                                        int triangleType, RSImage *image) {

    int mainColor = 0;
    if (tri0->type != tri1->type || tri0->type != tri2->type) {
        mainColor = 1;
        if (tri1->type > tri0->type)
            if (tri1->type > tri2->type)
                glColor4fv(tri1->color);
            else
                glColor4fv(tri2->color);
        else if (tri0->type > tri2->type)
            glColor4fv(tri0->color);
        else
            glColor4fv(tri2->color);
    }

    if (image->width == 64) {
        glTexCoord2fv(textTrianCoo64[triangleType][0]);
        if (!mainColor) {
            glColor4fv(tri0->color);
        }
        glVertex3f(tri0->v.x, tri0->v.y, tri0->v.z);

        if (!mainColor) {
            glColor4fv(tri1->color);
        }
        glTexCoord2fv(textTrianCoo64[triangleType][1]);
        glVertex3f(tri1->v.x, tri1->v.y, tri1->v.z);

        if (!mainColor) {
            glColor4fv(tri2->color);
        }
        glTexCoord2fv(textTrianCoo64[triangleType][2]);
        glVertex3f(tri2->v.x, tri2->v.y, tri2->v.z);
    } else {
        glTexCoord2fv(textTrianCoo[triangleType][0]);
        if (!mainColor) {
            glColor4fv(tri0->color);
        }
        glVertex3f(tri0->v.x, tri0->v.y, tri0->v.z);

        glTexCoord2fv(textTrianCoo[triangleType][1]);
        if (!mainColor) {
            glColor4fv(tri1->color);
        }
        glVertex3f(tri1->v.x, tri1->v.y, tri1->v.z);

        glTexCoord2fv(textTrianCoo[triangleType][2]);
        if (!mainColor) {
            glColor4fv(tri2->color);
        }
        glVertex3f(tri2->v.x, tri2->v.y, tri2->v.z);
    }
}

bool SCRenderer::IsTextured(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2) {
    return
        // tri0->type != tri1->type ||
        // tri0->type != tri2->type ||
        tri0->upperImageID == 0xFF || tri0->lowerImageID == 0xFF;
}
void SCRenderer::RenderColoredTriangle(MapVertex *tri0, MapVertex *tri1, MapVertex *tri2) {

    if (tri0->type != tri1->type || tri0->type != tri2->type

    ) {

        if (tri1->type > tri0->type)
            if (tri1->type > tri2->type)
                glColor4fv(tri1->color);
            else
                glColor4fv(tri2->color);
        else if (tri0->type > tri2->type)
            glColor4fv(tri0->color);
        else
            glColor4fv(tri2->color);

        glVertex3f(tri0->v.x, tri0->v.y, tri0->v.z);

        glVertex3f(tri1->v.x, tri1->v.y, tri1->v.z);

        glVertex3f(tri2->v.x, tri2->v.y, tri2->v.z);
    } else {
        glColor4fv(tri0->color);
        glVertex3f(tri0->v.x, tri0->v.y, tri0->v.z);

        glColor4fv(tri1->color);
        glVertex3f(tri1->v.x, tri1->v.y, tri1->v.z);

        glColor4fv(tri2->color);
        glVertex3f(tri2->v.x, tri2->v.y, tri2->v.z);
    }
}

void SCRenderer::RenderQuad(MapVertex *currentVertex, MapVertex *rightVertex, MapVertex *bottomRightVertex,
                            MapVertex *bottomVertex, RSArea *area, bool renderTexture) {

    if (!renderTexture) {
        if (currentVertex->lowerImageID == 0xFF) {
            // Render lower triangle
            RenderColoredTriangle(currentVertex, bottomRightVertex, bottomVertex);
        }
        if (currentVertex->upperImageID == 0xFF) {
            // Render Upper triangles
            RenderColoredTriangle(currentVertex, rightVertex, bottomRightVertex);
        }
    } else {

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

void SCRenderer::RenderBlock(RSArea *area, int LOD, int i, bool renderTexture) {

    AreaBlock *block = area->GetAreaBlockByID(LOD, i);

    uint32_t sideSize = block->sideSize;

    // printf("Rendering block %d at x %f,z %f\n", i, block->vertice[0].v.x, block->vertice[0].v.z);
    for (size_t x = 0; x < sideSize - 1; x++) {
        for (size_t y = 0; y < sideSize - 1; y++) {

            MapVertex *currentVertex = &block->vertice[x + y * sideSize];
            MapVertex *rightVertex = &block->vertice[(x + 1) + y * sideSize];
            MapVertex *bottomRightVertex = &block->vertice[(x + 1) + (y + 1) * sideSize];
            MapVertex *bottomVertex = &block->vertice[x + (y + 1) * sideSize];

            RenderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area, renderTexture);
        }
    }

    // Inter-block right side
    if (i % 18 != 17) {
        AreaBlock *currentBlock = area->GetAreaBlockByID(LOD, static_cast<size_t>(i));
        AreaBlock *rightBlock = area->GetAreaBlockByID(LOD, static_cast<size_t>(i + 1));

        for (uint32_t y = 0; y < sideSize - 1; y++) {
            MapVertex *currentVertex = currentBlock->GetVertice(currentBlock->sideSize - 1, y);
            MapVertex *rightVertex = rightBlock->GetVertice(0, y);
            MapVertex *bottomRightVertex = rightBlock->GetVertice(0, y + 1);
            MapVertex *bottomVertex = currentBlock->GetVertice(currentBlock->sideSize - 1, y + 1);

            RenderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area, renderTexture);
        }
    }

    // Inter-block bottom side
    if (i / 18 != 17) {

        AreaBlock *currentBlock = area->GetAreaBlockByID(LOD, i);
        AreaBlock *bottomBlock = area->GetAreaBlockByID(LOD, i + BLOCK_PER_MAP_SIDE);

        for (uint32_t x = 0; x < sideSize - 1; x++) {
            MapVertex *currentVertex = currentBlock->GetVertice(x, currentBlock->sideSize - 1);
            MapVertex *rightVertex = currentBlock->GetVertice(x + 1, currentBlock->sideSize - 1);
            MapVertex *bottomRightVertex = bottomBlock->GetVertice(x + 1, 0);
            MapVertex *bottomVertex = bottomBlock->GetVertice(x, 0);

            RenderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area, renderTexture);
        }
    }

    // Inter bottom-right quad
    if (i % 18 != 17 && i / 18 != 17) {

        AreaBlock *currentBlock = area->GetAreaBlockByID(LOD, i);
        AreaBlock *rightBlock = area->GetAreaBlockByID(LOD, i + 1);
        AreaBlock *rightBottonBlock = area->GetAreaBlockByID(LOD, i + 1 + BLOCK_PER_MAP_SIDE);
        AreaBlock *bottomBlock = area->GetAreaBlockByID(LOD, i + BLOCK_PER_MAP_SIDE);

        MapVertex *currentVertex = currentBlock->GetVertice(currentBlock->sideSize - 1, currentBlock->sideSize - 1);
        MapVertex *rightVertex = rightBlock->GetVertice(0, currentBlock->sideSize - 1);
        MapVertex *bottomRightVertex = rightBottonBlock->GetVertice(0, 0);
        MapVertex *bottomVertex = bottomBlock->GetVertice(currentBlock->sideSize - 1, 0);

        RenderQuad(currentVertex, rightVertex, bottomRightVertex, bottomVertex, area, renderTexture);
    }
}
void SCRenderer::RenderWorldSkyAndGround() {
    static const int max_int = MAP_SIZE;
    int skyblue = 0;
    int nightblue = 1;
    GLfloat skycolor[2][3] = {
        {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f},
        {40.0f / 255.0f, 80.0f / 255.0f, 120.0f / 255.0f},
    };

    /*static GLuint sky1[][4] = {
        {-max_int*2,		0,			-max_int *2,skyblue},
        {-max_int*2,		0,			max_int*2 ,	skyblue},
        {0,					max_int ,	0,			nightblue},
    };
    static GLuint sky2[][4] = {
        {max_int*2 ,		0,			-max_int*2,	skyblue},
        {max_int*2 ,		0,			max_int*2 ,	skyblue},
        {0,					max_int,	0,			nightblue},
    };
    static GLuint sky3[][4] = {
        {-max_int*2,		0,			-max_int*2,	skyblue},
        {max_int*2,			0,			-max_int*2,	skyblue},
        {0,					max_int,	0,			nightblue},
    };
    static GLuint sky4[][4] = {
        {-max_int*2,		0,			max_int*2 ,	skyblue},
        {max_int*2,			0,			max_int*2 ,	skyblue},
        {0,					max_int ,	0,			nightblue},
    };

    glBegin(GL_POLYGON);
	for (int i = 0; i < 3; i++) {
		glColor3f(skycolor[sky1[i][3]][0], skycolor[sky1[i][3]][1], skycolor[sky1[i][3]][2]);
		glVertex3iv((const GLint *)sky1[i]);
	}
	glEnd();

    glBegin(GL_POLYGON);
	for (int i = 0; i < 3; i++) {
		glColor3f(skycolor[sky2[i][3]][0], skycolor[sky2[i][3]][1], skycolor[sky2[i][3]][2]);
		glVertex3iv((const GLint *)sky2[i]);
	}
	glEnd();

    glBegin(GL_POLYGON);
	for (int i = 0; i < 3; i++) {
		glColor3f(skycolor[sky3[i][3]][0], skycolor[sky3[i][3]][1], skycolor[sky3[i][3]][2]);
		glVertex3iv((const GLint *)sky3[i]);
	}
	glEnd();

    glBegin(GL_POLYGON);
	for (int i = 0; i < 3; i++) {
		glColor3f(skycolor[sky4[i][3]][0], skycolor[sky4[i][3]][1], skycolor[sky4[i][3]][2]);
		glVertex3iv((const GLint *)sky4[i]);
	}
	glEnd();*/

    glBegin(GL_QUADS);

    // sol
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(-max_int, -max_int, -max_int);
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(max_int, -max_int, -max_int);
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(max_int, -max_int, max_int);
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(-max_int, -max_int, max_int);

    //ciel
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(-max_int, max_int, -max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(max_int, max_int, -max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(max_int, max_int, max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(-max_int, max_int, max_int);

    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(-max_int, -max_int, max_int);
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(max_int, -max_int, max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(max_int, max_int, max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(-max_int, max_int, max_int);

    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(-max_int, -max_int, -max_int);
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(max_int, -max_int, -max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(max_int, max_int, -max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(-max_int, max_int, -max_int);

    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(-max_int, -max_int, -max_int);
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(-max_int, -max_int, max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(-max_int, max_int, max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(-max_int, max_int, -max_int);

    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(max_int, -max_int, -max_int);
    glColor3f(skycolor[0][0], skycolor[0][1], skycolor[0][2]);
    glVertex3f(max_int, -max_int, max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(max_int, max_int, max_int);
    glColor3f(skycolor[1][0], skycolor[1][1], skycolor[1][2]);
    glVertex3f(max_int, max_int, -max_int);

    glEnd();
}



void SCRenderer::RenderWorldSolid(RSArea *area, int LOD, int verticesPerBlock) {

    glMatrixMode(GL_PROJECTION);
    Matrix *projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());

    glDisable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    GLuint fogMode[] = {GL_EXP, GL_EXP2, GL_LINEAR}; // Storage For Three Types Of Fog
    GLuint fogfilter = 0;                            // Which Fog To Use
    GLfloat fogColor[4] = {0.7f, 0.8f, 1.0f, 1.0f};
    float model_view_mat[4][4];
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)model_view_mat);
    glFogi(GL_FOG_MODE, GL_EXP2); // Fog Mode
    glFogfv(GL_FOG_COLOR, fogColor);         // Set Fog Color
    glFogf(GL_FOG_DENSITY, 0.000009f);       // How Dense Will The Fog Be
    glHint(GL_FOG_HINT, GL_DONT_CARE);       // Fog Hint Value
    glFogf(GL_FOG_START, 5000.0f);           // Fog Start Depth
    glFogf(GL_FOG_END, 16000.0f);            // Fog End Depth
    glEnable(GL_FOG);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    Matrix *modelViewMatrix = camera.GetViewMatrix();
    glLoadMatrixf(modelViewMatrix->ToGL());
    
    
    this->RenderWorldSkyAndGround();
    textureSortedVertex.clear();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < BLOCKS_PER_MAP; i++) {
        RenderBlock(area, LOD, i, false);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    for (int i = 0; i < BLOCKS_PER_MAP; i++) {
        RenderBlock(area, LOD, i, true);
    }

    for (auto const &x : textureSortedVertex) {
        RSImage *image = NULL;
        image = area->GetImageByID(x.first);
        if (image == NULL) {
            printf("This should never happen: Put a break point here.\n");
            return;
        }
        glBindTexture(GL_TEXTURE_2D, image->GetTexture()->GetTextureID());

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < x.second.size(); i++) {
            VertexCache v = x.second.at(i);
            if (v.lv1 != NULL && v.lv1->lowerImageID == x.first) {
                RenderTexturedTriangle(v.lv1, v.lv2, v.lv3, area, LOWER_TRIANGE, image);
            }
            if (v.uv1 != NULL && v.uv1->upperImageID == x.first) {
                RenderTexturedTriangle(v.uv1, v.uv2, v.uv3, area, UPPER_TRIANGE, image);
            }
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);

    RenderMapOverlay(area);

    RenderObjects(area, 0);

    
    glDisable(GL_FOG);
}

void SCRenderer::RenderObjects(RSArea *area, size_t blockID) {

    float y = 0;
    for (auto object : area->objects) {

        glPushMatrix();

        glTranslatef(static_cast<GLfloat>(object.position[0]), static_cast<GLfloat>(object.position[1]),
                     -static_cast<GLfloat>(object.position[2]));

        std::map<std::string, RSEntity *>::iterator it;
        it = area->objCache->find(object.name);
        if (it != area->objCache->end()) {
            DrawModel(it->second, BLOCK_LOD_MAX);
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

void SCRenderer::RenderMissionObjects(RSMission *mission) {

    float y = 0;
    for (auto object : mission->mission_data.parts) {
        glPushMatrix();
        
        glTranslatef(static_cast<GLfloat>(object->x), static_cast<GLfloat>(object->z),
                     static_cast<GLfloat>(-object->y));

        float model_view_mat[4][4];
        glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)model_view_mat);
        glRotatef((360.0f-(float)object->azymuth + 90.0f), 0, 1, 0);
        glRotatef((float)object->pitch, 0, 0, 1);
        glRotatef(-(float)object->roll, 1, 0, 0);
        if (object->entity != NULL) {
            DrawModel(object->entity, BLOCK_LOD_MAX);
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

void SCRenderer::RenderMapOverlay(RSArea *area) {

    // glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
    for (int i = 0; i < area->objectOverlay.size(); i++) {
        AoVPoints *v = area->objectOverlay[i].vertices;
        for (int j = 0; j < area->objectOverlay[i].nbTriangles; j++) {
            AoVPoints v1, v2, v3;
            v1 = area->objectOverlay[i].vertices[area->objectOverlay[i].trianles[j].verticesIdx[0]];
            v2 = area->objectOverlay[i].vertices[area->objectOverlay[i].trianles[j].verticesIdx[1]];
            v3 = area->objectOverlay[i].vertices[area->objectOverlay[i].trianles[j].verticesIdx[2]];
            glBegin(GL_TRIANGLES);
            const Texel *texel = palette.GetRGBColor(area->objectOverlay[i].trianles[j].color);
            glColor4f(texel->r / 255.0f, texel->g / 255.0f, texel->b / 255.0f, 1);
            glVertex3f((GLfloat)v1.x, (GLfloat)v1.y, (GLfloat)-v1.z);
            glVertex3f((GLfloat)v2.x, (GLfloat)v2.y, (GLfloat)-v2.z);
            glVertex3f((GLfloat)v3.x, (GLfloat)v3.y, (GLfloat)-v3.z);
            glEnd();
        }
    }
    glEnable(GL_DEPTH_TEST);
}
void SCRenderer::RenderWorldByID(RSArea *area, int LOD, int verticesPerBlock, int blockId) {
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
    RenderBlock(area, LOD, blockId, false);
    glEnd();

    RenderBlock(area, LOD, blockId, true);
    glEnable(GL_TEXTURE_2D);
    for (auto const &x : textureSortedVertex) {

        RSImage *image = NULL;

        image = area->GetImageByID(x.first);
        if (image == NULL) {
            printf("This should never happen: Put a break point here.\n");
            return;
        }
        glBindTexture(GL_TEXTURE_2D, image->GetTexture()->GetTextureID());

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < x.second.size(); i++) {
            VertexCache v = x.second.at(i);
            if (v.lv1 != NULL && v.lv1->lowerImageID == x.first) {
                RenderTexturedTriangle(v.lv1, v.lv2, v.lv3, area, LOWER_TRIANGE, image);
            }
            if (v.uv1 != NULL && v.uv1->upperImageID == x.first) {
                RenderTexturedTriangle(v.uv1, v.uv2, v.uv3, area, UPPER_TRIANGE, image);
            }
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
    RenderObjects(area, blockId);
    glPopMatrix();
}