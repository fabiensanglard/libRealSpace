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
static SDL_Texture * sdlTexture;

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

    camera.Init(50.0f,this->width/(float)this->height,0.1f,10000000.0f);
    
    vec3_t lookAt = {0,0,0};
    camera.SetLookAt(lookAt);
    
    vec3_t up = {0,1,0};
    camera.SetUp(up);
    
    //This is better for jets.
    vec3_t position = {28,20,-38};
    
    //vec3_t position = {300,300,-300};
    camera.SetPosition(position);
    
    
    light[0] = 300;
    light[1] = 300;
    light[2] = 300;
    
    
    
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
                        return;
                    }
                    if (SDLK_p == keyCode ||
                        SDLK_SPACE == keyCode){
                        paused = !paused;
                        running = false;
                        return;
                    }
                    
                    printf("%d\n",event.key.keysym.sym);
                    
                    
                } ; break;
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
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
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


void Renderer::GetNormal(RSEntity* object,Triangle* triangle,vec3_t normal){
    //Calculate the normal for this triangle
    vec3_t edge1 ;
    edge1[0] = object->vertices[triangle->ids[0]].x - object->vertices[triangle->ids[1]].x;
    edge1[1] = object->vertices[triangle->ids[0]].y - object->vertices[triangle->ids[1]].y;
    edge1[2] = object->vertices[triangle->ids[0]].z - object->vertices[triangle->ids[1]].z;
    
    vec3_t edge2 ;
    edge2[0] = object->vertices[triangle->ids[2]].x - object->vertices[triangle->ids[1]].x;
    edge2[1] = object->vertices[triangle->ids[2]].y - object->vertices[triangle->ids[1]].y;
    edge2[2] = object->vertices[triangle->ids[2]].z - object->vertices[triangle->ids[1]].z;
    
    vectorCrossProduct(edge1, edge2, normal);
    normalize(normal);
    
    
    
    
    // All normals are supposed to point outward in modern GPU but SC triangles
    // don't have consistent winding. They can be CW or CCW (the back governal of a jet  is
    // typically one triangle that must be visible from both sides !
    // As a result, gouraud shading was probably performed in screen space.
    // How can we replicate this ?
    //        - Take the normal and compare it to the sign of the direction to the camera.
    //        - If the signs don't match: reverse the normal.
    vec3_t cameraPosition;
    camera.GetPosition(cameraPosition);
    
    
    vec3_t vertexPositon;
    vertexPositon[0] = object->vertices[triangle->ids[0]].x;
    vertexPositon[1] = object->vertices[triangle->ids[0]].y;
    vertexPositon[2] = object->vertices[triangle->ids[0]].z;
    
    vec3_t cameraDirection;
    vectorSubtract(cameraPosition, vertexPositon, cameraDirection);
    normalize(cameraDirection);
    
    if (DotProduct(cameraDirection, normal) < 0){
        vectorNegate(normal,normal);
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
    matrix_t projectionMatrix;
    camera.gluPerspective(projectionMatrix);
    glLoadMatrixf(projectionMatrix);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    matrix_t modelViewMatrix;
    camera.gluLookAt(modelViewMatrix);
    glLoadMatrixf(modelViewMatrix);
        
        
        
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
            
            vec3_t normal;
            GetNormal(object, triangle, normal);
            
            
            glBegin(GL_TRIANGLES);
            for(int j=0 ; j < 3 ; j++){
                
                vec3_t vertice;
                vertice[0] = object->vertices[triangle->ids[j]].x;
                vertice[1] = object->vertices[triangle->ids[j]].y;
                vertice[2] = object->vertices[triangle->ids[j]].z;
                
                vec3_t lighDirection;
                vectorSubtract(light, vertice, lighDirection);
                normalize(lighDirection);
                
                float lambertianFactor = DotProduct(lighDirection,normal);
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
        
        
        vec3_t normal;
        GetNormal(object,triangle,normal);
        
        glBegin(GL_TRIANGLES);
        for(int j=0 ; j < 3 ; j++){
            
            
            
            vec3_t vertice;
            vertice[0] = object->vertices[triangle->ids[j]].x;
            vertice[1] = object->vertices[triangle->ids[j]].y;
            vertice[2] = object->vertices[triangle->ids[j]].z;
            
            
            
            vec3_t sunDirection;
            vectorSubtract(light, vertice, sunDirection);
            normalize(sunDirection);
            
            float lambertianFactor = DotProduct(sunDirection,normal);
            if (lambertianFactor < 0  )
                lambertianFactor = 0;
            
            lambertianFactor=0.2f;
            
            // int8_t gouraud = 255 * lambertianFactor;
            
            
            //gouraud = 255;
            
            glColor4f(lambertianFactor, lambertianFactor, lambertianFactor,1);
            
            glVertex3f(object->vertices[triangle->ids[j]].x,
                       object->vertices[triangle->ids[j]].y,
                       object->vertices[triangle->ids[j]].z);
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
        
        vec3_t normal;
        GetNormal(object, triangle, normal);
        
        glBegin(GL_TRIANGLES);
        for(int j=0 ; j < 3 ; j++){
            
            vec3_t vertice;
            vertice[0] = object->vertices[triangle->ids[j]].x;
            vertice[1] = object->vertices[triangle->ids[j]].y;
            vertice[2] = object->vertices[triangle->ids[j]].z;
            
            vec3_t lighDirection;
            vectorSubtract(light, vertice, lighDirection);
            normalize(lighDirection);
            
            float lambertianFactor = DotProduct(lighDirection,normal);
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



void Renderer::SetLight(vec3_t l){
     vectorCopy(this->light,l);
}

VGAPalette* Renderer::GetCurrentPalette(void){
    return this->currentPalette;
}

void Renderer::DisplayModel(RSEntity* object,size_t lodLevel){
    
    if (!initialized)
        return;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    matrix_t projectionMatrix;
    camera.gluPerspective(projectionMatrix);
    glLoadMatrixf(projectionMatrix);
    
    running = true;
    float counter=0;
    while (running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        matrix_t modelViewMatrix;
        
        
        
        light[0]= 20*cos(counter);
        light[1]= 10;
        light[2]= 20*sin(counter);
        counter += 0.02;
        
        //camera.SetPosition(position);
        
        
        camera.gluLookAt(modelViewMatrix);
        glLoadMatrixf(modelViewMatrix);
        
        DrawModel(object, lodLevel );
        
        //Render light
        
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glPointSize(6);
        
        glBegin(GL_POINTS);
            glColor4f(1, 1,0 , 1);
            glVertex3f(light[0],light[1], light[2]);
        glEnd();
        
        
        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }

}


VGAPalette* Renderer::GetDefaultPalette(void){
    return &this->defaultPalette;
}



void Renderer::RenderVerticeField(Vertex* vertices, int numVertices){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    matrix_t projectionMatrix;
    camera.gluPerspective(projectionMatrix);
    glLoadMatrixf(projectionMatrix);
    
    SDL_ShowWindow(sdlWindow);
    
    running = true;
    float counter=0;
    while (running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        matrix_t modelViewMatrix;
        
        vec3_t newPosition;
        newPosition[0]= 256*cos(counter);
        newPosition[1]= 0;
        newPosition[2]= 256*sin(counter);
        counter += 0.02;
        
        camera.SetPosition(newPosition);
        
        
        camera.gluLookAt(modelViewMatrix);
        glLoadMatrixf(modelViewMatrix);
        
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

void Renderer::RenderWorldSolid(RSArea* area, int LOD, int verticesPerBlock){
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    matrix_t projectionMatrix;
    camera.gluPerspective(projectionMatrix);
    glLoadMatrixf(projectionMatrix);
    
    SDL_ShowWindow(sdlWindow);
    
    running = true;
    
    static float counter=0;
    
    vec3_t lookAt = {256*16,100,256*16};
    
    renderer.GetCamera()->SetLookAt(lookAt);
    
    
    glPointSize(4);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    
    
    while (running) {
        
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        
        
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        matrix_t modelViewMatrix;
        vec3_t newPosition;
        newPosition[0]=  lookAt[0] + 5256*cos(counter/2);
        newPosition[1]= 3700;
        newPosition[2]=  lookAt[2] + 5256*sin(counter/2);
        counter += 0.02;
        
        camera.SetPosition(newPosition);
        camera.gluLookAt(modelViewMatrix);
        glLoadMatrixf(modelViewMatrix);
        
        
        
        glBegin(GL_QUADS);
        
        for(int i=0 ; i < 324 ; i++){
            //for(int i=96 ; i < 99 ; i++){
            AreaBlock* block = area->GetAreaBlock(LOD, i);
            for (size_t i=0 ; i < verticesPerBlock ; i ++){
                
                MapVertex* v = &block->vertice[i];
                glColor3fv(v->color);
                glVertex3f(v->x,
                           v->y,
                           v->z         );
            }
        }
        glEnd();
        
        
        
        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }

}

void Renderer::RenderWorldPoints(RSArea* area, int LOD, int verticesPerBlock)
{
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    matrix_t projectionMatrix;
    camera.gluPerspective(projectionMatrix);
    glLoadMatrixf(projectionMatrix);
    
    SDL_ShowWindow(sdlWindow);
    
    running = true;
    
    static float counter=0;
    
    vec3_t lookAt = {256*16,100,256*16};
    
    renderer.GetCamera()->SetLookAt(lookAt);
    
    
    glPointSize(4);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    
    
    while (running) {
        
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        
    
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        matrix_t modelViewMatrix;
        vec3_t newPosition;
        newPosition[0]=  lookAt[0] + 5256*cos(counter/2);
        newPosition[1]= 3700;
        newPosition[2]=  lookAt[2] + 5256*sin(counter/2);
        counter += 0.02;

        camera.SetPosition(newPosition);
        camera.gluLookAt(modelViewMatrix);
        glLoadMatrixf(modelViewMatrix);
        
        
        
        glBegin(GL_POINTS);
        
        for(int i=0 ; i < 324 ; i++){
        //for(int i=96 ; i < 99 ; i++){
            AreaBlock* block = area->GetAreaBlock(LOD, i);
            for (size_t i=0 ; i < verticesPerBlock ; i ++){
                MapVertex* v = &block->vertice[i];
                
                
                glColor3fv(v->color);
                
                glVertex3f(v->x,
                           v->y,
                           v->z         );
            }
        }
            glEnd();
        
        
        
        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }

}
