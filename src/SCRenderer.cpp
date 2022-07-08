 //
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "SCRenderer.h"
#include "IffLexer.h"
#include "RSPalette.h"
#include "RSArea.h"
#include "RSEntity.h"
#include "RSVGA.h"
#include "Texture.h"
#define BLOCK_WIDTH (25000)

extern SCRenderer Renderer;

SCRenderer::SCRenderer() :
   initialized(false){
}

SCRenderer::~SCRenderer(){
}

Camera* SCRenderer::GetCamera(void){
    return &this->camera;
}

VGAPalette* SCRenderer::GetPalette(void){
    return &this->palette;
}

void SCRenderer::Init(int32_t zoomFactor){
    
    this->scale =zoomFactor;
	this->counter = 0;
    int32_t width  = 320 * scale;
    int32_t height = 200 * scale;
    
    //Load the default palette
    IffLexer lexer ;
    lexer.InitFromFile("PALETTE.IFF");
    lexer.List(stdout);
    
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();
    
    this->width = width;
    this->height = height;
    
  
	
    
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
	//glClearDepth(1.0f);								// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
    
    
    
    camera.SetPersective(50.0f,this->width/(float)this->height,10.0f,12000.0f);
    
    
    
    light.SetWithCoo(300, 300, 300);
    
    
    
    initialized = true;
}

void SCRenderer::SetClearColor(uint8_t red, uint8_t green, uint8_t blue){
    if (!initialized)
        return;
    
    glClearColor(red/255.0f, green/255.0f, blue/255.0f, 1.0f);
}


void SCRenderer::Clear(void){
    
    
    if (!initialized)
        return;
    
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glColor4f(1, 1, 1, 1);
}

void SCRenderer::CreateTextureInGPU(Texture* texture){
    
    if (!initialized)
        return;
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
	//printf("TEXTURE [%s] : %d\n", texture->name, texture->id);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glDisable(GL_TEXTURE_2D);
}


void SCRenderer::UploadTextureContentToGPU(Texture* texture){
    

    if (!initialized)
        return;
	//printf("UPLOAD TEXTURE [%s] : %d\n", texture->name, texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}

void SCRenderer::DeleteTextureInGPU(Texture* texture){
    
    if (!initialized)
        return;
	//printf("DELETE TEXTURE [%s] : %d\n", texture->name, texture->id);
    glDeleteTextures(1, &texture->id);
}


void SCRenderer::GetNormal(RSEntity* object,Triangle* triangle,Vector3D* normal){
    
    //Calculate the normal for this triangle
    Vector3D edge1 ;
    edge1 = object->vertices[triangle->ids[0]];
    edge1.Substract(& object->vertices[triangle->ids[1]]);
    
    
    Vector3D edge2 ;
    edge2 = object->vertices[triangle->ids[2]];
    edge2.Substract(& object->vertices[triangle->ids[1]]);
    
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
    Point3D cameraPosition= camera.GetPosition();
    
    
    Point3D *vertexOnTriangle = &object->vertices[triangle->ids[0]];

    Point3D cameraDirection;
    cameraDirection = cameraPosition;
    cameraDirection.Substract(vertexOnTriangle);
    cameraDirection.Normalize();
    
    if (cameraDirection.DotProduct(normal) < 0){
        normal->Negate();
    }

    
}

void SCRenderer::DrawModel(RSEntity* object, size_t lodLevel ){

    if (!initialized)
        return;

    if (lodLevel >= object->NumLods()){
        printf("Unable to render this Level Of Details (out of range): Max level is  %lu\n",
               min(0UL,object->NumLods()-1));
        return;
    }
    
    
    float ambientLamber = 0.4f;
    
    Lod* lod = &object->lods[lodLevel] ;
    
    
    
        
        
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    
    //Texture pass
    if (lodLevel == 0){
        glEnable(GL_TEXTURE_2D);
        
        //glDepthFunc(GL_EQUAL);
        
        glAlphaFunc ( GL_ALWAYS, 1.0 ) ;
        glEnable ( GL_ALPHA_TEST ) ;
        
        
        for (int i=0 ; i < object->NumUVs(); i++) {
            
            uvxyEntry* textInfo = &object->uvs[i];
            
            //Seems we have a textureID that we don't have :( !
            if (textInfo->textureID >= object->images.size())
                continue;
            
            RSImage* image = object->images[textInfo->textureID];
            
            Texture* texture = image->GetTexture();
            
            glBindTexture(GL_TEXTURE_2D, texture->id);
            Triangle* triangle = &object->triangles[textInfo->triangleID];
            
            Vector3D normal;
            GetNormal(object, triangle, &normal);
            
            
            glBegin(GL_TRIANGLES);
            for(int j=0 ; j < 3 ; j++){
                
                Point3D vertice = object->vertices[triangle->ids[j]];
                
                Vector3D lighDirection;
                lighDirection = light;
                lighDirection.Substract(&vertice);
                lighDirection.Normalize();
                
                float lambertianFactor = lighDirection.DotProduct(&normal);
                if (lambertianFactor < 0  )
                    lambertianFactor = 0;
                
                lambertianFactor+= ambientLamber;
                if (lambertianFactor > 1)
                    lambertianFactor = 1;
                
                
                
                glColor4f(lambertianFactor, lambertianFactor, lambertianFactor,1);
                glTexCoord2f(textInfo->uvs[j].u/(float)texture->width, textInfo->uvs[j].v/(float)texture->height);
                glVertex3f(object->vertices[triangle->ids[j]].x,
                           object->vertices[triangle->ids[j]].y,
                           object->vertices[triangle->ids[j]].z);
            }
            glEnd();
            
            
        }
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }


    
    

   
    //Pass 3: Let's draw the transparent stuff render RSEntity::SC_TRANSPARENT)
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
#ifndef _WIN32
	

	glBlendEquation(GL_ADD);
#else 
	typedef void (APIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
	PFNGLBLENDEQUATIONPROC glBlendEquation = NULL;
	glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation");
	glBlendEquation(GL_ADD);
	//glDepthFunc(GL_LESS);
#endif
        
    for(int i = 0 ; i < lod->numTriangles ; i++){
        
        uint16_t triangleID = lod->triangleIDs[i];
        
        Triangle* triangle = &object->triangles[triangleID];
        
        if (triangle->property != RSEntity::SC_TRANSPARENT)
            continue;
        
        
        Vector3D normal;
        GetNormal(object,triangle,&normal);
        
        glBegin(GL_TRIANGLES);
        for(int j=0 ; j < 3 ; j++){
            
            
            
            Point3D vertice = object->vertices[triangle->ids[j]];
            
            
            Vector3D sunDirection;
            sunDirection = light;
            sunDirection.Substract(&vertice);
            sunDirection.Normalize();

            
            float lambertianFactor = sunDirection.DotProduct(&normal);
            if (lambertianFactor < 0  )
                lambertianFactor = 0;
            
            lambertianFactor=0.2f;
            
            // int8_t gouraud = 255 * lambertianFactor;
            
            
            //gouraud = 255;
            
            glColor4f(lambertianFactor, lambertianFactor, lambertianFactor,1);
            
            glVertex3f(vertice.x,
                       vertice.y,
                       vertice.z);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    
    
    
    
    
    
    //Pass 1, draw color
    for(int i = 0 ; i < lod->numTriangles ; i++){
        //for(int i = 60 ; i < 62 ; i++){  //Debug purpose only back governal of F-16 is 60-62
        
        uint16_t triangleID = lod->triangleIDs[i];
        
        Triangle* triangle = &object->triangles[triangleID];
        
        if (triangle->property == RSEntity::SC_TRANSPARENT)
            continue;
        
        Vector3D normal;
        GetNormal(object, triangle, &normal);
        
        glBegin(GL_TRIANGLES);
        for(int j=0 ; j < 3 ; j++){
            
            Point3D vertice = object->vertices[triangle->ids[j]];
            
            Vector3D lighDirection;
            lighDirection = light;
            lighDirection.Substract(&vertice);
            lighDirection.Normalize();
            
            float lambertianFactor = lighDirection.DotProduct(&normal);
            if (lambertianFactor < 0  )
                lambertianFactor = 0;
            
            lambertianFactor+= ambientLamber;
            if (lambertianFactor > 1)
                lambertianFactor = 1;
            
            const Texel* texel = palette.GetRGBColor(triangle->color);
            
            //glColor4f(texel->r/255.0f*lambertianFactor, texel->g/255.0f*lambertianFactor, texel->b/255.0f*lambertianFactor,1);
            glColor4f(texel->r/255.0f, texel->g/255.0f, texel->b/255.0f,1);
            
            glVertex3f(object->vertices[triangle->ids[j]].x,
                       object->vertices[triangle->ids[j]].y,
                       object->vertices[triangle->ids[j]].z);
        }
        glEnd();
    }
}



void SCRenderer::SetLight(Point3D* l){
    this->light = *l;
}


void SCRenderer::Prepare(RSEntity* object){
    
    for (size_t i = 0; i < object->NumImages(); i++) {
        object->images[i]->SyncTexture();
    }
    
    object->prepared = true;
}

void SCRenderer::DisplayModel(RSEntity* object,size_t lodLevel){
    
    if (!initialized)
        return;
    
    if (object->IsPrepared())
        Prepare(object);
    
    glMatrixMode(GL_PROJECTION);
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());
    
    running = true;
    float counter=0;
    while (running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        Matrix* modelViewMatrix;
        
        
        
        light.x= 20*cos(counter);
        light.y= 10;
        light.z= 20*sin(counter);
        counter += 0.02;
        
        //camera.SetPosition(position);
        
        
        modelViewMatrix = camera.GetViewMatrix();
        glLoadMatrixf(modelViewMatrix->ToGL());
        
        DrawModel(object, lodLevel );
        
        //Render light
        
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glPointSize(6);
        
        glBegin(GL_POINTS);
            glColor4f(1, 1,0 , 1);
            glVertex3f(light.x,light.y, light.z);
        glEnd();
        
       
    }

}



void SCRenderer::RenderVerticeField(Point3D* vertices, int numVertices){
    
    glMatrixMode(GL_PROJECTION);
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());
    
    
    
    running = true;
    float counter=0;
    while (running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        Matrix* modelViewMatrix;
        
        Point3D newPosition;
        newPosition.x= 256*cos(counter);
        newPosition.y= 0;
        newPosition.z= 256*sin(counter);
        counter += 0.02;
        
        camera.SetPosition(&newPosition);
        
        
        modelViewMatrix = camera.GetViewMatrix();
        glLoadMatrixf(modelViewMatrix->ToGL());
        
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(5);
        glBegin(GL_POINTS);
        for(int i=0; i < numVertices ; i ++)
            glVertex3f(vertices[i].x,
                       vertices[i].y,
                       vertices[i].z     );
        glEnd();
        
        

    }

}

#define TEX_ZERO (0/64.0f)
#define TEX_ONE (64/64.0f)

// What is this offset ? It is used to get rid of the red delimitations
// in the 64x64 textures.
#define OFFSET (2/64.0f)
float textTrianCoo64[2][3][2] = {
    
    {{TEX_ZERO,TEX_ZERO+OFFSET},    {TEX_ONE-2*OFFSET,TEX_ONE-OFFSET},    {TEX_ZERO,TEX_ONE-OFFSET} }, // LOWER_TRIANGE
    
    {{TEX_ZERO+2*OFFSET,TEX_ZERO+OFFSET},    {TEX_ONE,TEX_ZERO+OFFSET},    {TEX_ONE,TEX_ONE-OFFSET} }  //UPPER_TRIANGE
};

float textTrianCoo[2][3][2] = {
    
    {{TEX_ZERO,TEX_ZERO},    {TEX_ONE,TEX_ONE},    {TEX_ZERO,TEX_ONE} }, // LOWER_TRIANGE
    
    {{TEX_ZERO,TEX_ZERO},    {TEX_ONE,TEX_ZERO},    {TEX_ONE,TEX_ONE} }  //UPPER_TRIANGE
};


#define LOWER_TRIANGE 0
#define UPPER_TRIANGE 1

void SCRenderer::RenderTexturedTriangle(MapVertex* tri0,
                                     MapVertex* tri1,
                                     MapVertex* tri2,
                                     RSArea* area,
                                     int triangleType,
                                     RSImage* image){
    
    
	int mainColor = 0;
	if (tri0->type != tri1->type || tri0->type != tri2->type) {
		mainColor = 1;
		if (tri1->type > tri0->type)
			if (tri1->type > tri2->type)
				glColor4fv(tri1->color);
			else
				glColor4fv(tri2->color);
		else
			if (tri0->type > tri2->type)
				glColor4fv(tri0->color);
			else
				glColor4fv(tri2->color);
	}



    
    

    
    if (image->width == 64){
        glTexCoord2fv(textTrianCoo64[triangleType][0]);
		if (!mainColor) {
			glColor4fv(tri0->color);
		}
        glVertex3f(tri0->v.x,tri0->v.y,tri0->v.z);
    
		if (!mainColor) {
			glColor4fv(tri1->color);
		}
        glTexCoord2fv(textTrianCoo64[triangleType][1]);
        glVertex3f(tri1->v.x,tri1->v.y,tri1->v.z);
    
		if (!mainColor) {
			glColor4fv(tri2->color);
		}
        glTexCoord2fv(textTrianCoo64[triangleType][2]);
        glVertex3f(tri2->v.x,tri2->v.y,tri2->v.z);
    } else {
        glTexCoord2fv(textTrianCoo[triangleType][0]);
		if (!mainColor) {
			glColor4fv(tri0->color);
		}
        glVertex3f(tri0->v.x,tri0->v.y,tri0->v.z);
        
        
        glTexCoord2fv(textTrianCoo[triangleType][1]);
		if (!mainColor) {
			glColor4fv(tri1->color);
		}
        glVertex3f(tri1->v.x,tri1->v.y,tri1->v.z);
        
        
        glTexCoord2fv(textTrianCoo[triangleType][2]);
		if (!mainColor) {
			glColor4fv(tri2->color);
		}
        glVertex3f(tri2->v.x,tri2->v.y,tri2->v.z);
        }
    
     
        
	
}


bool SCRenderer::IsTextured(MapVertex* tri0,MapVertex* tri1,MapVertex* tri2){
    return
            // tri0->type != tri1->type ||
           //tri0->type != tri2->type ||
    tri0->upperImageID == 0xFF || tri0->lowerImageID == 0xFF ;
    
}
void SCRenderer::RenderColoredTriangle(MapVertex* tri0,
                                     MapVertex* tri1,
                                     MapVertex* tri2){
    

    if (tri0->type != tri1->type || tri0->type != tri2->type
        
        ){
        
        if (tri1->type > tri0->type)
            if (tri1->type > tri2->type)
                glColor4fv(tri1->color);
            else
                glColor4fv(tri2->color);
        else
            if (tri0->type > tri2->type)
                glColor4fv(tri0->color);
            else
                glColor4fv(tri2->color);
        
        
        
        glVertex3f(tri0->v.x,
                   tri0->v.y,
                   tri0->v.z         );
        
        
        glVertex3f(tri1->v.x,
                   tri1->v.y,
                   tri1->v.z         );
        
        
        glVertex3f(tri2->v.x,
                   tri2->v.y,
                   tri2->v.z         );
    }
    else{
        glColor4fv(tri0->color);
        glVertex3f(tri0->v.x,
                   tri0->v.y,
                   tri0->v.z         );
        
        glColor4fv(tri1->color);
        glVertex3f(tri1->v.x,
                   tri1->v.y,
                   tri1->v.z         );
        
        glColor4fv(tri2->color);
        glVertex3f(tri2->v.x,
                   tri2->v.y,
                   tri2->v.z         );
    }
    
   
}



void SCRenderer::RenderQuad( MapVertex* currentVertex,
                             MapVertex* rightVertex,
                             MapVertex* bottomRightVertex,
                             MapVertex* bottomVertex,RSArea* area,bool renderTexture){
    
    
    if (!renderTexture){
        if (currentVertex->lowerImageID == 0xFF ){
            //Render lower triangle
            RenderColoredTriangle(currentVertex,bottomRightVertex,bottomVertex);
            
        }
        if (currentVertex->upperImageID == 0xFF) {
            //Render Upper triangles
            RenderColoredTriangle(currentVertex, rightVertex, bottomRightVertex);
        }
    }
    else{
        
		if (currentVertex->lowerImageID != 0xFF) {
            VertexVector& vcache = textureSortedVertex[currentVertex->lowerImageID];
            VertexCache v = { currentVertex, bottomRightVertex, bottomVertex };
            v.lv1 = currentVertex;
            v.lv2 = bottomRightVertex;
            v.lv3 = bottomVertex;
            v.uv1 = v.uv2 = v.uv3 = NULL;
            vcache.push_back(v);
            /*
            RSImage* image = NULL;

            image = area->GetImageByID(currentVertex->lowerImageID);
            if (image == NULL) {
                printf("This should never happen: Put a break point here.\n");
                return;
            }
            glBindTexture(GL_TEXTURE_2D, image->GetTexture()->GetTextureID());
            glBegin(GL_TRIANGLES);
            RenderTexturedTriangle(currentVertex, bottomRightVertex, bottomVertex, area, LOWER_TRIANGE, image);
            glEnd();
            /**/
        }
        if (currentVertex->upperImageID != 0xFF) {
            VertexVector& vcache = textureSortedVertex[currentVertex->upperImageID];
            VertexCache v;
            v.uv1 = currentVertex;
            v.uv2 = rightVertex;
            v.uv3 = bottomRightVertex;
            v.lv1 = v.lv2 = v.lv3 = NULL;
            vcache.push_back(v);
            /*
            RSImage* image = NULL;
            image = area->GetImageByID(currentVertex->upperImageID);
            if (image == NULL) {
                printf("This should never happen: Put a break point here.\n");
                return;
            }
            glBindTexture(GL_TEXTURE_2D, image->GetTexture()->GetTextureID());
            glBegin(GL_TRIANGLES);
            RenderTexturedTriangle(currentVertex, rightVertex, bottomRightVertex, area, UPPER_TRIANGE, image);
            glEnd();
            /**/
        }
        
    }
    
       
}

void SCRenderer::RenderBlock(RSArea* area, int LOD, int i, bool renderTexture){
    
    
    AreaBlock* block = area->GetAreaBlockByID(LOD, i);
    
    uint32_t sideSize = block->sideSize;
    
	//printf("Rendering block %d at x %f,z %f\n", i, block->vertice[0].v.x, block->vertice[0].v.z);
    for (size_t x=0 ; x < sideSize-1 ; x ++){
        for (size_t y=0 ; y < sideSize-1 ; y ++){
            
            
            MapVertex* currentVertex     =   &block->vertice[x+y*sideSize];
            MapVertex* rightVertex       =   &block->vertice[(x+1)+y*sideSize];
            MapVertex* bottomRightVertex =   &block->vertice[(x+1)+(y+1)*sideSize];
            MapVertex* bottomVertex      =   &block->vertice[x+(y+1)*sideSize];
            
            
            RenderQuad(currentVertex,rightVertex, bottomRightVertex, bottomVertex,area,renderTexture);
        }
        
        
    }
    
    
    //Inter-block right side
    if ( i % 18 != 17){
        AreaBlock* currentBlock = area->GetAreaBlockByID(LOD, i);
        AreaBlock* rightBlock = area->GetAreaBlockByID(LOD, i+1);
        
        for (int y=0 ; y < sideSize-1 ; y ++){
            MapVertex* currentVertex     =   currentBlock->GetVertice(currentBlock->sideSize-1, y);
            MapVertex* rightVertex       =   rightBlock->GetVertice(0, y);
            MapVertex* bottomRightVertex =   rightBlock->GetVertice(0, y+1);
            MapVertex* bottomVertex      =   currentBlock->GetVertice(currentBlock->sideSize-1, y+1);
            
            RenderQuad(currentVertex,rightVertex, bottomRightVertex, bottomVertex,area,renderTexture);
        }
    }
    
    //Inter-block bottom side
    if ( i / 18 != 17){
        
        AreaBlock* currentBlock = area->GetAreaBlockByID(LOD, i);
        AreaBlock* bottomBlock = area->GetAreaBlockByID(LOD, i+BLOCK_PER_MAP_SIDE);
        
        for (int x=0 ; x < sideSize-1 ; x++){
            MapVertex* currentVertex     =   currentBlock->GetVertice(x,currentBlock->sideSize-1);
            MapVertex* rightVertex       =   currentBlock->GetVertice(x+1,currentBlock->sideSize-1);
            MapVertex* bottomRightVertex =   bottomBlock->GetVertice(x+1,0);
            MapVertex* bottomVertex      =   bottomBlock->GetVertice(x,0);
            
            RenderQuad(currentVertex,rightVertex, bottomRightVertex, bottomVertex,area,renderTexture);
        }
    }
    
    
    
    //Inter bottom-right quad
    if ( i % 18 != 17 && i / 18 != 17){
        
        AreaBlock* currentBlock = area->GetAreaBlockByID(LOD, i);
        AreaBlock* rightBlock = area->GetAreaBlockByID(LOD, i+1);
        AreaBlock* rightBottonBlock = area->GetAreaBlockByID(LOD, i+1+BLOCK_PER_MAP_SIDE);
        AreaBlock* bottomBlock = area->GetAreaBlockByID(LOD, i+BLOCK_PER_MAP_SIDE);
        
        MapVertex* currentVertex     =   currentBlock->GetVertice(currentBlock->sideSize-1,currentBlock->sideSize-1);
        MapVertex* rightVertex       =   rightBlock->GetVertice(0,currentBlock->sideSize-1);
        MapVertex* bottomRightVertex =   rightBottonBlock->GetVertice(0,0);
        MapVertex* bottomVertex      =   bottomBlock->GetVertice(currentBlock->sideSize-1,0);
        
        RenderQuad(currentVertex,rightVertex, bottomRightVertex, bottomVertex,area,renderTexture);
        
        
    }

    
}

void SCRenderer::RenderJets(RSArea* area){
    
    glMatrixMode(GL_MODELVIEW);
    
    
    
    for(size_t i=0 ; i < area->GetNumJets(); i++){
        RSEntity* entity = area->GetJet(i);
    
        glPushMatrix();
        
        Matrix objMatrix = entity->orientation.ToMatrix();
        Point3D pos = entity->position;
        objMatrix.v[3][0] = pos.x;
        objMatrix.v[3][1] = pos.y;
        objMatrix.v[3][2] = pos.z;
        
        glMultMatrixf(objMatrix.ToGL());
        
        DrawModel(entity, LOD_LEVEL_MAX);
        
        glPopMatrix();
    }
    
}

void SCRenderer::RenderWorldSolid(RSArea* area, int LOD, int verticesPerBlock){
    
    
    glMatrixMode(GL_PROJECTION);
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());
	counter++;
    
    running = true;
    
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    
    
	Point3D lookAt = { 3856,0,2856 };

    Point3D newPosition;
    newPosition.x=  lookAt.x + counter;
    newPosition.y= 100;
	newPosition.z = lookAt.z + counter;
    camera.SetPosition(&newPosition);
    
    
    
    camera.LookAt(&lookAt);
    
    
    GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };   // Storage For Three Types Of Fog
    GLuint fogfilter= 0;                    // Which Fog To Use
    GLfloat fogColor[4]= {1.0f, 1.0f, 1.0f, 1.0f};
    glFogi(GL_FOG_MODE, fogMode[fogfilter]);        // Fog Mode
    glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
    glFogf(GL_FOG_DENSITY, 0.0002f);              // How Dense Will The Fog Be
    glHint(GL_FOG_HINT, GL_DONT_CARE);          // Fog Hint Value
    glFogf(GL_FOG_START, 600.0f);             // Fog Start Depth
    glFogf(GL_FOG_END, 8000.0f);               // Fog End Depth
    glEnable(GL_FOG);
    
    
        
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_MODELVIEW);
        Matrix* modelViewMatrix = camera.GetViewMatrix();
        glLoadMatrixf(modelViewMatrix->ToGL());

        
        
        
        //Island
        /*
        newPosition[0]=  2500;//lookAt[0] + 5256*cos(counter/2);
        newPosition[1]= 350;
        newPosition[2]=  600;//lookAt[2];// + 5256*sin(counter/2);
        vec3_t lookAt = {2456,0,256};
        */

        
        //City Top
        
        
        
        //City view on mountains
        /*
        counter = 23;
        vec3_t lookAt = {3856,30,2856};
        newPosition[0]=  lookAt[0] + 256*cos(counter/2);
        newPosition[1]= 60;
        newPosition[2]=  lookAt[2] + 256*sin(counter/2);
        /**/
        
        //Canyon
        ///*
        
        
        //*/
        
        //counter += 0.02;
        
        glDepthFunc(GL_LESS);
        glBegin(GL_TRIANGLES);
        //for(int i=97 ; i < 98 ; i++)
        for(int i=0 ; i < BLOCKS_PER_MAP ; i++)
            RenderBlock(area, LOD, i,false);
        glEnd();
        
        
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_EQUAL);
        //for(int i=97 ; i < 98 ; i++)
		for (int i = 0; i < BLOCKS_PER_MAP; i++) {
			printf("Rendering block %d\n", i);
			RenderBlock(area, LOD, i, true);
		}
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        
        
        //Render objects on the map
        //for(int i=97 ; i < 98 ; i++)
        //for(int i=0 ; i < BLOCKS_PER_MAP ; i++)
        //   RenderObjects(area,i);
        
        RenderJets(area);
        
      
    

}

void SCRenderer::RenderObjects(RSArea* area,size_t blockID){

    std::vector<MapObject> *objects = &area->objects[blockID];

	float y = 0;
    for (size_t i =0 ; i < objects->size(); i++) {
        MapObject object = objects->at(i);
		
		glPushMatrix();
		
		glTranslatef(object.position[0], object.position[1], -object.position[2]);
		
		std::map<std::string, RSEntity *>::iterator it;
		it = area->objCache->find(object.name);
		if (it != area->objCache->end()) {
			printf("Rendering [%s] at {%d,%d,%d}\n",
				object.name,
                object.position[0], object.position[1], object.position[2]
			);
			DrawModel(it->second, BLOCK_LOD_MAX);
		}
		else {
			printf("OBJECT [%s] NOT FOUND\n", object.name);
			glBegin(GL_POINTS);
				glColor3f(0, 1, 0);
				glVertex3d(0, 0, 0);
			glEnd();
		}

		glPopMatrix();

		
    }
}

void SCRenderer::RenderMissionObjects(RSMission* mission) {

    std::vector<PART*> *objects = &mission->missionObjects;
   
    float y = 0;
    for (size_t i = 0; i < objects->size(); i++) {
        PART *object = objects->at(i);

        glPushMatrix();

        glTranslatef(object->XAxisRelative, object->ZAxisRelative, -object->YAxisRelative);
        if (object->entity!=NULL) {
            printf("RENDERING MISSION OBJ : %s at [%d,%d,%d]\n",
                object->MemberName,
                object->XAxisRelative,
                object->ZAxisRelative,
                object->YAxisRelative
            );
            DrawModel(object->entity, BLOCK_LOD_MAX);
        }
        else {
            printf("OBJECT [%s] NOT FOUND\n", object->MemberName);
            glBegin(GL_POINTS);
            glColor3f(0, 1, 0);
            glVertex3d(0, 0, 0);
            glEnd();
        }
        glPopMatrix();
    }    
}


void SCRenderer::RenderWorldPoints(RSArea* area, int LOD, int verticesPerBlock)
{
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());

    glPointSize(4);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    running = true;

    static float counter = 0;

    while (running) {
        
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);

        Point3D lookAt = { 256*16,100,256*16 };       
        Renderer.GetCamera()->LookAt(&lookAt);

        Point3D newPosition = camera.GetPosition();
        newPosition.x= lookAt.x + 5256*cos(counter/2);
        newPosition.y= 3700;
        newPosition.z= lookAt.z + 5256*sin(counter/2);

        camera.SetPosition(&newPosition);
        
        
        Matrix* modelViewMatrix = camera.GetViewMatrix();
        glLoadMatrixf(modelViewMatrix->ToGL());
        
        
        glBegin(GL_POINTS);
        
        for(int i=0 ; i < 324 ; i++){
        //for(int i=96 ; i < 99 ; i++){
            AreaBlock* block = area->GetAreaBlockByID(LOD, i);
            for (size_t i=0 ; i < verticesPerBlock ; i ++){
                MapVertex* v = &block->vertice[i];
                
                
                glColor3fv(v->color);
                
                glVertex3f(v->v.x,
                           v->v.y,
                           v->v.z);
            }
        }
        glEnd();
        
        
        //Render objects on the map
        for(int i=0 ; i < 324 ; i++)
            RenderObjects(area,i);
        
        
    }

}

void SCRenderer::RenderWorld(RSArea* area, int LOD, int verticesPerBlock) {
    textureSortedVertex.clear();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glBegin(GL_TRIANGLES);
    for (int i = 0; i < BLOCKS_PER_MAP; i++) {
        RenderBlock(area, LOD, i, false);
    }
	glEnd();
	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < BLOCKS_PER_MAP; i++) {
		RenderBlock(area, LOD, i, true);
	}
    for (auto const& x : textureSortedVertex)
    {
         
         RSImage* image = NULL;
         
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
	for (int i = 0; i < BLOCKS_PER_MAP; i++) {
       RenderObjects(area, i);
	}
    
    
}
void SCRenderer::RenderMapOverlay(RSArea* area) {
    
    //glDepthFunc(GL_LESS);
    
    for (int i = 0; i < area->objectOverlay.size(); i++) {
        AoVPoints*v = area->objectOverlay[i].vertices;
        for (int j = 0; j < area->objectOverlay[i].nbTriangles; j++) {
            AoVPoints v1, v2, v3;
            v1 = area->objectOverlay[i].vertices[area->objectOverlay[i].trianles[j].verticesIdx[0]];
            v2 = area->objectOverlay[i].vertices[area->objectOverlay[i].trianles[j].verticesIdx[1]];
            v3 = area->objectOverlay[i].vertices[area->objectOverlay[i].trianles[j].verticesIdx[2]];
            glBegin(GL_TRIANGLES);
                const Texel* texel = palette.GetRGBColor(area->objectOverlay[i].trianles[j].color);
                glColor4f(texel->r / 255.0f, texel->g / 255.0f, texel->b / 255.0f, 1);
                glVertex3f(v1.x, v1.y, -v1.z);
                glVertex3f(v2.x, v2.y, -v2.z);
                glVertex3f(v3.x, v3.y, -v3.z);
            glEnd();
        }
    }
}
void SCRenderer::RenderWorldByID(RSArea* area, int LOD, int verticesPerBlock, int blockId) {
    textureSortedVertex.clear();
    printf("X:%f,Y:%f", area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.x, area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.z);
    glPushMatrix();
    glScalef(1/100.0f, 1/100.0f, 1/100.0f);
    glTranslatef(-area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.x-12500, 0, -area->GetAreaBlockByID(LOD, blockId)->vertice[0].v.z- 12500);
    //
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBegin(GL_TRIANGLES);
    RenderBlock(area, LOD, blockId, false);
    glEnd();

    
    RenderBlock(area, LOD, blockId, true);
    glEnable(GL_TEXTURE_2D);
    for (auto const& x : textureSortedVertex)
    {

        RSImage* image = NULL;

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