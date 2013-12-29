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

Renderer::Renderer() :
   initialized(false){
}

Renderer::~Renderer(){
}

void Renderer::Init(int32_t width , int32_t height){
    
    this->width = width;
    this->height = height;
    
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_HIDDEN, &sdlWindow, &sdlRenderer);
    
    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   width, height);
    
    backBuffer = (uint8_t*)calloc(width*height*4,1);
    
    
    SDL_HideWindow(sdlWindow);

    
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
    
    while(1){
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN :{
                    
                    if (SDLK_ESCAPE == event.key.keysym.sym){
                        SDL_HideWindow(sdlWindow);
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

}





void Renderer::ShowImage(uint8_t* image, uint16_t imageWidth, uint16_t imageHeight,VGAPalette* palette,int zoom,bool wait){
    
    //Need to copy all images in the offscreen buffer
    if (imageWidth*zoom > this->width ||
        imageHeight*zoom > this->height)
    {
        printf("Image is too big for this windows.\n");
        return;
    }
    
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
    
    if (wait)
        PumpEvents();
}











void Renderer::ShowModel(RSEntity* object,VGAPalette* palette ){
    
    //Create a window with OpenGL
    SDL_Window* window = NULL;
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Unable to initialize SDL:  %s\n",SDL_GetError());
        return ;
    }
    
    window = SDL_CreateWindow("RealSpace OBJ Viewer",0,0,this->width,this->height,SDL_WINDOW_OPENGL);
    
    // Create an OpenGL context associated with the window.
    SDL_GL_CreateContext(window);
    
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
    
	glClearColor(0.8f, 0.8f, 8.5f, 1.0f);				// Black Background
	//glClearDepth(1.0f);								// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
    

    glClear(GL_COLOR_BUFFER_BIT);
    
    
    
    glDisable(GL_CULL_FACE);
    
    SDL_ShowWindow(window);

    //Draw
    glBegin(GL_TRIANGLES);
    //glPointSize(3.2);
    //glBegin(GL_POINTS);
    for(int i = 1 ; i < object->lods[0].numTriangles ; i++){
        
        uint16_t triangleID = object->lods[0].triangleIDs[i];
        
        Triangle* triangle = &object->triangles[triangleID];
        
        const Texel* texel = palette->GetRGBColor(triangle->color);
        glColor4f(texel->r/255.0f, texel->g/255.0f, texel->b/255.0f,texel->a/255.0f);
        
        glVertex3f(object->vertices[triangle->ids[0]].y,
                   object->vertices[triangle->ids[0]].z,
                   object->vertices[triangle->ids[0]].x);
        
        glVertex3f(object->vertices[triangle->ids[1]].y,
                   object->vertices[triangle->ids[1]].z,
                   object->vertices[triangle->ids[1]].x);
        
        glVertex3f(object->vertices[triangle->ids[2]].y,
                   object->vertices[triangle->ids[2]].z,
                   object->vertices[triangle->ids[2]].x);
        
    }
    glEnd();
    
    /*TODO !!!!
       - Figure out texture (likely a second pass).
       - Figure out transluctancy (for cockpit glass).
       - Make the camera rotate like the Object Viewer in Strike Commander
    */
    
    SDL_GL_SwapWindow(window);
    
    PumpEvents();
}


