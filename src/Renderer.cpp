 //
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

Renderer renderer;

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

Renderer::Renderer() :
   initialized(false){
       
    
}

Renderer::~Renderer(){
}

Camera* Renderer::GetCamera(void){
    return &this->camera;
}

VGAPalette* Renderer::GetPalette(void){
    return &this->defaultPalette;
}


void Renderer::SetPalette(VGAPalette* palette){
    this->currentPalette = palette;
}

void Renderer::ResetPalette(void){
    this->currentPalette = &defaultPalette;
}

void Renderer::Init(int32_t width , int32_t height){
    
    //Load the default palette
    IffLexer lexer ;
    lexer.InitFromFile("PALETTE.IFF");
    //lexer.List(stdout);
    
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->defaultPalette = *palette.GetColorPalette();
    
    this->currentPalette = &defaultPalette;
    
    this->width = width;
    this->height = height;
    
   // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_HIDDEN, &sdlWindow, &sdlRenderer);
    
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Unable to initialize SDL:  %s\n",SDL_GetError());
        return ;
    }
    
    sdlWindow = SDL_CreateWindow("RealSpace OBJ Viewer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,this->width,this->height,SDL_WINDOW_OPENGL);
    
    // Create an OpenGL context associated with the window.
    SDL_GL_CreateContext(sdlWindow);
    
    
    glViewport(0,0,this->width,this->height);			// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
    
    
	// Calculate The Aspect Ratio Of The Window
    int zoom= 30;
	glOrtho(-this->width/zoom,
            this->width/zoom,
            -this->height/zoom,
            this->height/zoom,-300,300);
    
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
    
	glClearColor(0.4f, 0.4f, 1.0f, 1.0f);				// Black Background
	//glClearDepth(1.0f);								// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
    
    
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    
    SDL_HideWindow(sdlWindow);

    camera.SetPersective(50.0f,this->width/(float)this->height,10.0f,12000.0f);
    
    
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    light.SetWithCoo(300, 300, 300);
    
    initialized = true;
}

void Renderer::SetClearColor(uint8_t red, uint8_t green, uint8_t blue){
    if (!initialized)
        return;
    
    glClearColor(red/255.0f, green/255.0f, blue/255.0f, 1.0f);
}

void Renderer::SetTitle(const char* title){

    if (!initialized)
        return;
    
    SDL_SetWindowTitle(sdlWindow, title);
}

void Renderer::Clear(void){
    
    
    if (!initialized)
        return;
    
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glColor4f(1, 1, 1, 1);
}


void Renderer::ShowPalette(VGAPalette* palette){
    
    
    int CELLSIZE=16 ;
    RSImage image ;
    image.Create("PALETTE_TEXTURE", CELLSIZE*16, CELLSIZE*16);
    
    uint8_t* dst = image.GetData();
    for(int i = 0 ; i < 256 ; i++){
        
        
        int offsetY = (i / CELLSIZE)*CELLSIZE;
        int offsetX = (i % CELLSIZE)*CELLSIZE* (int)image.width;
        
        for(int h = 0 ; h < CELLSIZE ; h++){
            for(int w = 0 ; w < CELLSIZE; w++){
                *(dst +offsetX +offsetY + w +h*image.width) = i;
            }
        }
    }
   

    DrawImage(&image, 2);
    
    SetTitle("PALETTE");
    renderer.Swap();
    ShowWindow();

    Pause();
    while (IsPaused()) {
        PumpEvents();
    }
    
    exit(0);

}


void Renderer::PumpEvents(void){
    
    if (!initialized)
        return;
    
   
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN :{
                    
                    SDL_Keycode keyCode = event.key.keysym.sym;
                    
                    if (SDLK_ESCAPE == keyCode){
                        SDL_HideWindow(sdlWindow);
                        running = false;
                        exit(0);
                    }
                    if (SDLK_p == keyCode ||
                        SDLK_SPACE == keyCode){
                        paused = !paused;
                        running = false;
                    }
                    
                    if (SDLK_w == keyCode ){
                        camera.MoveForward();
                    }

                    if (SDLK_s == keyCode ){
                        camera.MoveBackward();
                    }

                    if (SDLK_a == keyCode ){
                        camera.MoveStrafLeft();
                    }
                            
                    if (SDLK_d == keyCode ){
                        camera.MoveStrafRight();
                    }
                    
//                    printf("%d\n",event.key.keysym.sym);
                    
                    
                } ; break;
                    
                case SDL_MOUSEMOTION:
                    //printf("Mouse motion: x=%d y=%d\n",event.motion.xrel,event.motion.yrel);
                    camera.Rotate(event.motion.yrel*0.01f, event.motion.xrel*0.01f , 0);
                    ;
                    
                break;
                    
                case SDL_QUIT :
                    exit(0);
                    break;
                    
            }
        }
        
    

}





void Renderer::DrawImage(RSImage* image,int zoom){
    
    if (!initialized)
        return;
    
    running = true;
    
    image->SetPalette(this->currentPalette);
    
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
    glOrtho(0, this->width, 0, this->height, -10, 10) ;
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();
    
    
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    glDisable(GL_CULL_FACE);
     
    glBindTexture(GL_TEXTURE_2D, image->GetTexture()->id);
    glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2d(0, 0);
    
            glTexCoord2f(1, 1);
            glVertex2d(image->width*zoom, 0);
    
            glTexCoord2f(1, 0);
            glVertex2d(image->width*zoom, image->height*zoom);
    
            glTexCoord2f(0, 0);
            glVertex2d(0, image->height*zoom);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    
    paused = true;
}

void Renderer::ShowWindow(void){
    SDL_ShowWindow(sdlWindow);
}

void Renderer::Swap(void){
    SDL_GL_SwapWindow(sdlWindow);
}

void Renderer::CreateTextureInGPU(Texture* texture){
    
    if (!initialized)
        return;
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    glEnable(GL_TEXTURE_2D);
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glDisable(GL_TEXTURE_2D);
}


void Renderer::UploadTextureContentToGPU(Texture* texture){
    

    if (!initialized)
        return;
    
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}

void Renderer::DeleteTextureInGPU(Texture* texture){
    
    if (!initialized)
        return;
    
    glDeleteTextures(1, &texture->id);
}


void Renderer::GetNormal(RSEntity* object,Triangle* triangle,Vector3D* normal){
    
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

void Renderer::DrawModel(RSEntity* object, size_t lodLevel ){

    if (!initialized)
        return;
    
    if (lodLevel >= object->NumLods()){
        printf("Unable to render this Level Of Details (out of range): Max level is  %lu\n",
               std::min(0UL,object->NumLods()-1));
        return;
    }
    
    
    float ambientLamber = 0.4f;
    
    Lod* lod = &object->lods[lodLevel] ;
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Matrix* modelViewMatrix = camera.GetViewMatrix();
    glLoadMatrixf(modelViewMatrix->ToGL());
        
        
        
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_BLEND);
    
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    
    //Texture pass
    if (lodLevel == 0){
        glEnable(GL_TEXTURE_2D);
        
        //glDepthFunc(GL_EQUAL);
        
        glAlphaFunc ( GL_GREATER, 0.0 ) ;
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


    
    

   
    //Pass 3: Let's draw the transparent stuff render RSEntity::TRANSPARENT)
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_ADD);

    //glDepthFunc(GL_LESS);
        
        
    for(int i = 0 ; i < lod->numTriangles ; i++){
        
        uint16_t triangleID = lod->triangleIDs[i];
        
        Triangle* triangle = &object->triangles[triangleID];
        
        if (triangle->property != RSEntity::TRANSPARENT)
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
        
        if (triangle->property == RSEntity::TRANSPARENT)
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
            
            const Texel* texel = currentPalette->GetRGBColor(triangle->color);
            
            glColor4f(texel->r/255.0f*lambertianFactor, texel->g/255.0f*lambertianFactor, texel->b/255.0f*lambertianFactor,1);
            //glColor4f(texel->r/255.0f, texel->g/255.0f, texel->b/255.0f,1);
            
            glVertex3f(object->vertices[triangle->ids[j]].x,
                       object->vertices[triangle->ids[j]].y,
                       object->vertices[triangle->ids[j]].z);
        }
        glEnd();
    }
    

}



void Renderer::SetLight(Point3D* l){
    this->light = *l;
}

VGAPalette* Renderer::GetCurrentPalette(void){
    return this->currentPalette;
}

void Renderer::DisplayModel(RSEntity* object,size_t lodLevel){
    
    if (!initialized)
        return;
    
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
        
        
        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }

}


VGAPalette* Renderer::GetDefaultPalette(void){
    return &this->defaultPalette;
}



void Renderer::RenderVerticeField(Point3D* vertices, int numVertices){
    
    glMatrixMode(GL_PROJECTION);
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());
    
    SDL_ShowWindow(sdlWindow);
    
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
        
        

        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }

}

#define TEX_ZERO (0/64.0f)
#define TEX_ONE (64/64.0f)

// What is this offset ? It is used to get rid of the red delimitations
// in the 64x64 textures.
#define OFFSET (0/64.0f)
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

void Renderer::RenderTexturedTriangle(MapVertex* tri0,
                                     MapVertex* tri1,
                                     MapVertex* tri2,
                                     RSArea* area,
                                      int triangleType){
    
    
    float white[4];
    white[0] = 1;
    white[1] = 1;
    white[2] = 1;
    white[3] = 1;
    glColor4fv(white);
    
    RSImage* image = NULL;
    if (triangleType == LOWER_TRIANGE)
        image = area->GetImageByID(tri0->lowerImageID);
    if (triangleType == UPPER_TRIANGE)
        image = area->GetImageByID(tri0->upperImageID);
    
    
    if (image == NULL){
        printf("This should never happen: Put a break point here.\n");
        return;
    }
        
    

    
    glBindTexture(GL_TEXTURE_2D,image->GetTexture()->GetTextureID());
    
    glBegin(GL_TRIANGLES);
    
    if (image->width == 64){
        glTexCoord2fv(textTrianCoo64[triangleType][0]);
        glVertex3f(tri0->v.x,tri0->v.y,tri0->v.z         );
    
  
        glTexCoord2fv(textTrianCoo64[triangleType][1]);
        glVertex3f(tri1->v.x,tri1->v.y,tri1->v.z         );
    
    
        glTexCoord2fv(textTrianCoo64[triangleType][2]);
        glVertex3f(tri2->v.x,tri2->v.y,tri2->v.z         );
    }
        else{
        glTexCoord2fv(textTrianCoo[triangleType][0]);
        glVertex3f(tri0->v.x,tri0->v.y,tri0->v.z         );
        
        
        glTexCoord2fv(textTrianCoo[triangleType][1]);
        glVertex3f(tri1->v.x,tri1->v.y,tri1->v.z         );
        
        
        glTexCoord2fv(textTrianCoo[triangleType][2]);
        glVertex3f(tri2->v.x,tri2->v.y,tri2->v.z         );
        }
    glEnd();
    
}


bool Renderer::IsTextured(MapVertex* tri0,MapVertex* tri1,MapVertex* tri2){
    return
            // tri0->type != tri1->type ||
           //tri0->type != tri2->type ||
    tri0->upperImageID == 0xFF || tri0->lowerImageID == 0xFF ;
    
}
void Renderer::RenderColoredTriangle(MapVertex* tri0,
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



void Renderer::RenderQuad(MapVertex* currentVertex,
                MapVertex* rightVertex,
                MapVertex* bottomRightVertex,
                          MapVertex* bottomVertex,RSArea* area,bool renderTexture){
    
    //Render lower triangle
    if (!renderTexture){
        //if (currentVertex->lowerImageID == 0xFF ){
            RenderColoredTriangle(currentVertex,bottomRightVertex,bottomVertex);
        //}
    }
    else{
        if (currentVertex->lowerImageID != 0xFF )
            RenderTexturedTriangle(currentVertex,bottomRightVertex,bottomVertex,area,LOWER_TRIANGE);
    }
    
      //Render Upper triangles
    
    if (!renderTexture){
       // if (currentVertex->upperImageID == 0xFF ){
            RenderColoredTriangle(currentVertex,rightVertex,bottomRightVertex);
       // }
    }
    else{
        if (currentVertex->upperImageID != 0xFF )
             RenderTexturedTriangle(currentVertex,rightVertex,bottomRightVertex,area,UPPER_TRIANGE);
    }
    
    
}

void Renderer::RenderBlock(RSArea* area, int LOD, int i, bool renderTexture){
    
    
    AreaBlock* block = area->GetAreaBlockByID(LOD, i);
    
    uint32_t sideSize = block->sideSize;
    
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

void Renderer::RenderWorldSolid(RSArea* area, int LOD, int verticesPerBlock){
    
    
    glMatrixMode(GL_PROJECTION);
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());
    
    
    
    SDL_ShowWindow(sdlWindow);
    
    running = true;
    
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    
    
   
    Point3D newPosition;
    newPosition.x=  4100;//lookAt[0] + 5256*cos(counter/2);
    newPosition.y= 100;
    newPosition.z=  3000;//lookAt[2];// + 5256*sin(counter/2);
    camera.SetPosition(&newPosition);
    
    
    Point3D lookAt = {3856,0,2856};
    camera.LookAt(&lookAt);
    
    
    while (running) {
        
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
        */
        
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
        for(int i=0 ; i < BLOCKS_PER_MAP ; i++)
            RenderBlock(area, LOD, i,true);
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        
        
        //Render objects on the map
        //for(int i=97 ; i < 98 ; i++)
        //for(int i=0 ; i < BLOCKS_PER_MAP ; i++)
        //   RenderObjects(area,i);
        
        
        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }

}

void Renderer::RenderObjects(RSArea* area,size_t blockID){
    
    float color[3] = {1,0,0};
    
    
    std::vector<MapObject> *objects = &area->objects[blockID];
    
    glColor3fv(color);
    glPointSize(3);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_POINTS);
    
    for (size_t i =8 ; i < 9; i++) {
        MapObject object = objects->at(i);
        
        int32_t offset[3];
        
        #define BLOCK_WIDTH (512)
        
        offset[0] = blockID % 18 * BLOCK_WIDTH ;
        offset[1] = area->elevation[blockID];
        offset[2] = blockID / 18 * BLOCK_WIDTH;
        
        /*
        glVertex3d(object.position[0]/255*BLOCK_WIDTH+offset[0],
                   object.position[1]+offset[1],
                   object.position[2]/255*BLOCK_WIDTH+offset[2]);
        */
        int32_t localDelta[3];
        localDelta[0] = object.position[0]/65355.0f*BLOCK_WIDTH;
        localDelta[1] = object.position[1];/// HEIGHT_DIVIDER                   ;
        localDelta[2] = object.position[2]/65355.0f*BLOCK_WIDTH;
        
        
        int32_t toDraw[3];
        toDraw[0] = localDelta[0]+offset[0];
        toDraw[1] = offset[1];
        toDraw[2] = localDelta[2]+offset[2];
        
        glVertex3d(toDraw[0],
                   toDraw[1],
                   toDraw[2]);

    }
    
    glEnd();
    
}

void Renderer::RenderWorldPoints(RSArea* area, int LOD, int verticesPerBlock)
{
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    Matrix* projectionMatrix = camera.GetProjectionMatrix();
    glLoadMatrixf(projectionMatrix->ToGL());
    
    
    
    
    glPointSize(4);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
   

    
    SDL_ShowWindow(sdlWindow);
    
    running = true;
    
    
    
    static float counter = 0;
    
    
    while (running) {
        
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        
    
        
        glMatrixMode(GL_MODELVIEW);
        
        
        Point3D lookAt = { 256*16,100,256*16 };
        
        renderer.GetCamera()->LookAt(&lookAt);
        
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
                           v->v.z         );
            }
        }
            glEnd();
        
        
        //Render objects on the map
        for(int i=0 ; i < 324 ; i++)
            RenderObjects(area,i);
        
        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }

}
