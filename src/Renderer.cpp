//
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"




Renderer renderer;

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;
static SDL_Texture * sdlTexture;

VGAPalette* const Renderer::USE_DEFAULT_PALETTE = NULL;

Renderer::Renderer() :
   initialized(false){
       
    
}

Renderer::~Renderer(){
}

void Renderer::Init(int32_t width , int32_t height){
    
    //Load the default palette
    IffLexer lexer ;
    lexer.InitFromFile("PALETTE.IFF");
    //lexer.List(stdout);
    
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->defaultPalette = *palette.GetColorPalette();
    
    this->width = width;
    this->height = height;
    
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_HIDDEN, &sdlWindow, &sdlRenderer);
    
    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   width, height);
    
    backBuffer = (uint8_t*)calloc(width*height*4,1);
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Unable to initialize SDL:  %s\n",SDL_GetError());
        return ;
    }
    
    sdlWindow = SDL_CreateWindow("RealSpace OBJ Viewer",0,0,this->width,this->height,SDL_WINDOW_OPENGL);
    
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

    camera.Init(50.0f,this->width/(float)this->height,0.1f,10000.0f);
    
    vec3_t lookAt = {0,0,0};
    camera.SetLookAt(lookAt);
    
    vec3_t up = {0,1,0};
    camera.SetUp(up);
    
    vec3_t position = {18,10,-18};
    camera.SetPosition(position);
    
    
    light[0] = 300;
    light[1] = 300;
    light[2] = 300;
    
    initialized = true;
}

void Renderer::SetTitle(const char* title){

    if (!initialized)
        return;
    
    SDL_SetWindowTitle(sdlWindow, title);
}

void Renderer::Clear(void){
    
    Texel color ;
    color.r = 220;
    color.g = 220;
    color.b = 255;
    color.a = 255;
    
    for(int i=0 ; i < height * width; i++){
        *(backBuffer+i*4+0) = color.a ;
        *(backBuffer+i*4+1) = color.b ;
        *(backBuffer+i*4+2) = color.g ;
        *(backBuffer+i*4+3) = color.r ;
    }
    
    SDL_UpdateTexture(sdlTexture, NULL, backBuffer, width*4);
}

#define CELLSIZE 16
void Renderer::ShowPalette(VGAPalette* palette){
    
    for(int i = 0 ; i < 256 ; i++){
        
        const Texel* color = palette->GetRGBColor(i);
        
        int offsetY = (i / CELLSIZE)*CELLSIZE*              4;
        int offsetX = (i % CELLSIZE)*CELLSIZE* width*4;
        
        for(int h = 0 ; h < CELLSIZE ; h++){
            for(int w = 0 ; w < CELLSIZE; w++){
                *(backBuffer +offsetX +offsetY + h*4 +w*4*width+0) = color->a;
                *(backBuffer +offsetX +offsetY + h*4 +w*4*width+1) = color->b;
                *(backBuffer +offsetX +offsetY + h*4 +w*4*width+2) = color->g;
                *(backBuffer +offsetX +offsetY + h*4 +w*4*width+3) = color->r;
            }
        }
    }
    
    
    SDL_ShowWindow(sdlWindow);
    
    SDL_UpdateTexture(sdlTexture, NULL, backBuffer, width*4);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    
    PumpEvents();
    
    

}


void Renderer::PumpEvents(void){
    
    if (!initialized)
        return;
    
   
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN :{
                    
                    if (SDLK_ESCAPE == event.key.keysym.sym){
                        SDL_HideWindow(sdlWindow);
                        running = false;
                        return;
                    }
                    if (SDLK_p == event.key.keysym.sym){
                        paused = !paused;
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





void Renderer::DrawImage(uint8_t* image, uint16_t imageWidth, uint16_t imageHeight,VGAPalette* palette,int zoom){
    
    //Need to copy all images in the offscreen buffer
    if (imageWidth*zoom > this->width ||
        imageHeight*zoom > this->height)
    {
        printf("Image is too big for this windows.\n");
        return;
    }
    
    running = true;
    
    Clear();
    
    uint8_t* dst = this->backBuffer;
    
    for(int i = 0 ; i < imageHeight *zoom ; i++){
        for (int j=0; j < imageWidth *zoom; j++) {
            
            float widthCoef =  j/((float)imageWidth *zoom) ;
            float heightCoef = i/((float)imageHeight*zoom) ;
            
            const Texel* src = palette->GetRGBColor( *(image +
                                                       (int)( widthCoef  *imageWidth ) +
                                                       (int)( heightCoef *imageHeight) * this->width
                                                       
                                                       )
                                                    );
            if (src->a != 0){
                dst[0] = src->a;
                dst[1] = src->b;
                dst[2] = src->g;
                dst[3] = src->r;
            }
            dst+=4;
        }
        dst += (this->width - imageWidth*zoom)*4;
    }
    
    SDL_ShowWindow(sdlWindow);
    
    //draw texture
    SDL_UpdateTexture(sdlTexture, NULL, backBuffer, this->width*4);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    
    if (running)
        PumpEvents();
}








void Renderer::UploadTextureToVRAM(Texture* texture, VGAPalette* palette){
    
    if (!initialized)
        return;
    
    if (palette == USE_DEFAULT_PALETTE)
        palette = &this->defaultPalette;
    
    //Convert texture from VGA to RGBA using the palette
    uint8_t* rgbaData = (uint8_t*)malloc(4 * this->width * this->height);
    
    uint8_t* rgbaDst = rgbaData;
    for(int i=0 ; i < this->height * this->width ; i++){
        
        const Texel* texel;
        texel = palette->GetRGBColor(texture->data[i]);
        *rgbaDst++ = texel->r;
        *rgbaDst++ = texel->g;
        *rgbaDst++ = texel->b;
        *rgbaDst++ = texel->a;
    }
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
    
    free(rgbaData);
}

void Renderer::UnloadTextureToVRAM(Texture* texture){
    glDeleteTextures(1, &texture->id);
}


void Renderer::DrawModel(RSEntity* object,VGAPalette* palette, size_t lodLevel ){
    
    if (palette== USE_DEFAULT_PALETTE)
        palette=&defaultPalette;
    
    if (lodLevel >= object->numLods){
        printf("Unable to render this Level Of Details (out of range): Max level is  %d\n",object->numLods-1);
        return;
    }
    
    glEnable(GL_DEPTH_TEST);
    
    glDisable(GL_CULL_FACE);
    
    SDL_ShowWindow(sdlWindow);

    //Pass 1, draw color
   
    //Pass two for the textures:
    
    glDepthFunc(GL_LESS);
    
    for(int i = 0 ; i < object->lods[lodLevel].numTriangles ; i++){
        
        uint16_t triangleID = object->lods[lodLevel].triangleIDs[i];
        
        Triangle* triangle = &object->triangles[triangleID];
        
        if (triangle->property == RSEntity::TRANSPARENT)
            continue;
        
        const Texel* texel = palette->GetRGBColor(triangle->color);
        glColor4f(texel->r/255.0f, texel->g/255.0f, texel->b/255.0f,texel->a/255.0f);
        
        glBegin(GL_TRIANGLES);
        for(int j=0 ; j < 3 ; j++)
        glVertex4f(object->vertices[triangle->ids[j]].x,
                   object->vertices[triangle->ids[j]].y,
                   object->vertices[triangle->ids[j]].z,
                   1.0f);
        glEnd();
        
        /*
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(sdlWindow);
        SDL_GL_SwapWindow(sdlWindow);
        SDL_GL_SwapWindow(sdlWindow);
        paused=true;
        while(paused){
            //SDL_GL_SwapWindow(sdlWindow);
            PumpEvents();
        }
        */
    }
    
    
    
    
    
    if (lodLevel == 0){
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        
        glDepthFunc(GL_EQUAL);
        
        glAlphaFunc ( GL_GREATER, 0.0 ) ;
        glEnable ( GL_ALPHA_TEST ) ;
        
        for (int i=0 ; i < object->numUVs; i++) {
            
            uvxyEntry* textInfo = &object->uvs[i];
            
            Texture* texture = &object->textures[textInfo->textureID];
            
            glBindTexture(GL_TEXTURE_2D, texture->GetTextureID());
            
            Triangle* triangle = &object->triangles[textInfo->triangleID];
            
            glBegin(GL_TRIANGLES);
            for(int j=0 ; j < 3 ; j++){
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

    
    
    
    if (1){
    //Pass 3: Let's makde a gouraud pass.
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA);
    
    //We only write a pixel if the depth is EQUAL to what is in the depth buffer.
    glDepthFunc(GL_EQUAL);
        
    //glDepthFunc(GL_LESS);
        
        Lod* lod = &object->lods[lodLevel] ;
    for(int i = 0 ; i < lod->numTriangles ; i++){
        
        uint16_t triangleID = lod->triangleIDs[i];
        
        Triangle* triangle = &object->triangles[triangleID];
        
        //Calculate the normal for this triangle
        vec3_t edge1 ;
        edge1[0] = object->vertices[triangle->ids[0]].x - object->vertices[triangle->ids[1]].x;
        edge1[1] = object->vertices[triangle->ids[0]].y - object->vertices[triangle->ids[1]].y;
        edge1[2] = object->vertices[triangle->ids[0]].z - object->vertices[triangle->ids[1]].z;
        
        vec3_t edge2 ;
        edge2[0] = object->vertices[triangle->ids[2]].x - object->vertices[triangle->ids[1]].x;
        edge2[1] = object->vertices[triangle->ids[2]].y - object->vertices[triangle->ids[1]].y;
        edge2[2] = object->vertices[triangle->ids[2]].z - object->vertices[triangle->ids[1]].z;
        
        
        
        vec3_t normal ;
        vectorCrossProduct(edge1, edge2, normal);
        
        // All normals are supposed to point outward in modern GPU but SC triangles
        // don't have consistent winding. They can be CW or CCW (the back governal of a jet  is
        // typically one triangle that must be visible from both sides !
        // As a result, gouraud shading was probably performed in screen space.
        // How can we replicate this ?
        //        - Take the normal and compare it to the sign of the direction to the camera.
        //        - If the signs don't match: reverse the normal.
        normalize(normal);
        
        vec3_t cameraPosition;
        camera.GetPosition(cameraPosition);
        
        vec3_t cameraDirection;
        vec3_t vertexPositon;
        vertexPositon[0] = object->vertices[triangle->ids[0]].x;
        vertexPositon[1] = object->vertices[triangle->ids[0]].y;
        vertexPositon[2] = object->vertices[triangle->ids[0]].z;
        
        vectorSubtract(cameraPosition, vertexPositon, cameraDirection);
        normalize(cameraDirection);
        
        if (DotProduct(cameraDirection, normal) < 0){
            vectorNegate(normal,normal);
        }
        
        
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
            
            lambertianFactor*=0.8f;
            
            // int8_t gouraud = 255 * lambertianFactor;
            
            
            //gouraud = 255;
            
            glColor4f(lambertianFactor, lambertianFactor, lambertianFactor,0.5f);
            
            glVertex3f(object->vertices[triangle->ids[j]].x,
                       object->vertices[triangle->ids[j]].y,
                       object->vertices[triangle->ids[j]].z);
        }
        glEnd();
    }
    
    
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    }
    
    /*TODO !!!!
       - Figure out transluctancy (for cockpit glass): Pilot if not visible !.
       - Make the camera rotate like the Object Viewer in Strike Commander
    */
    
    
    
    
}





void Renderer::DisplayModel(RSEntity* object,size_t lodLevel){
    
    //Make sure all textures are within the GPU
    //We cannot load the texture within glBegin: Preload them here
    for (int i=0 ; i < object->numUVs; i++){
        uvxyEntry* textInfo = &object->uvs[i];
        Texture* texture = &object->textures[textInfo->textureID];
        texture->GetTextureID();
    }
    
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
        
        DrawModel(object,USE_DEFAULT_PALETTE, lodLevel );
        
        //Render light
        /*
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glPointSize(6);
        
        glBegin(GL_POINTS);
            glColor4f(1, 1,0 , 1);
            glVertex3f(light[0],light[1], light[2]);
        glEnd();
        */
        
        SDL_GL_SwapWindow(sdlWindow);
        PumpEvents();
    }
    
}
