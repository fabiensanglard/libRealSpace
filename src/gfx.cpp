//
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "gfx.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"


SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture * sdlTexture;

void IMG_PumpEvents(void){
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

uint8_t* offScreenBuffer;

void IMG_ShowPalette(Palette* palette,int CELLSIZE){
    for(int i = 0 ; i < 256 ; i++){
        
        const Texel* color = palette->GetRGBColor(i);
        
        int offsetY = (i / CELLSIZE)*CELLSIZE*              4;
        int offsetX = (i % CELLSIZE)*CELLSIZE* WINDOW_WIDTH*4;
        
        for(int h = 0 ; h < CELLSIZE ; h++){
            for(int w = 0 ; w < CELLSIZE; w++){
                *(offScreenBuffer +offsetX +offsetY + h*4 +w*4*WINDOW_WIDTH+0) = color->a;
                *(offScreenBuffer +offsetX +offsetY + h*4 +w*4*WINDOW_WIDTH+1) = color->b;
                *(offScreenBuffer +offsetX +offsetY + h*4 +w*4*WINDOW_WIDTH+2) = color->g;
                *(offScreenBuffer +offsetX +offsetY + h*4 +w*4*WINDOW_WIDTH+3) = color->r;
            }
        }
    }
    
    
    SDL_ShowWindow(sdlWindow);
    
    SDL_UpdateTexture(sdlTexture, NULL, offScreenBuffer, WINDOW_WIDTH*4);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    
    IMG_PumpEvents();
    
    
    
}

void IMG_Init(void){
    
    
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_HIDDEN, &sdlWindow, &sdlRenderer);
    
    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   WINDOW_WIDTH, WINDOW_HEIGHT);
    
    offScreenBuffer = (uint8_t*)calloc(WINDOW_WIDTH*WINDOW_HEIGHT*4,1);
    
    
    SDL_HideWindow(sdlWindow);
}

void IMG_Clear(void){
    
    
    
    //memset(offScreenBuffer,0xFF,WINDOW_WIDTH*WINDOW_HEIGHT*4);
    
    Texel color ;
    color.r = 220;
    color.g = 220;
    color.b = 255;
    color.a = 255;
    
    for(int i=0 ; i < WINDOW_HEIGHT*WINDOW_WIDTH; i++){
        *(offScreenBuffer+i*4+0) = color.a ;
        *(offScreenBuffer+i*4+1) = color.b ;
        *(offScreenBuffer+i*4+2) = color.g ;
        *(offScreenBuffer+i*4+3) = color.r ;
    }
    
    SDL_UpdateTexture(sdlTexture, NULL, offScreenBuffer, WINDOW_WIDTH*4);
}

void IMG_ShowImage(uint8_t* image, uint16_t width, uint16_t height,Palette* palette,int zoom,bool wait){
    
    //Need to copy all images in the offscreen buffer
    if (width*zoom > WINDOW_WIDTH || height*zoom > WINDOW_HEIGHT){
        printf("Image is too big for this windows.\n");
        return;
    }
    
    IMG_Clear();
    
    uint8_t* dst = offScreenBuffer;
    
    for(int i = 0 ; i < height *zoom ; i++){
        for (int j=0; j < width *zoom; j++) {
            
            float widthCoef =  j/((float)width *zoom) ;
            float heightCoef = i/((float)height*zoom) ;
            
            const Texel* src = palette->GetRGBColor( *(image +
                                                         (int)( widthCoef  *width ) +
                                                         (int)( heightCoef *height) *width
            
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
        dst += (WINDOW_WIDTH - width*zoom)*4;
    }
    
    SDL_ShowWindow(sdlWindow);
    
    //draw texture
    SDL_UpdateTexture(sdlTexture, NULL, offScreenBuffer, WINDOW_WIDTH*4);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    
    if (wait)
        IMG_PumpEvents();
}



int testGFX(int argc, const char * argv[])
{
    
    Palette palette ;
    
    IMG_Init();
    
    
    
    for(int i =0 ; i < 256 ; i++){
        Texel texel;
        texel.r = texel.g = texel.b = i;
        texel.a = 255;
        palette.SetColor(i,  &texel);
    }
    IMG_ShowPalette(&palette,16);
    
    return 0;
}







void IMG_ShowModel(RealSpaceObject* object,Palette* palette ){
    
    //Create a window with OpenGL
    SDL_Window* window = NULL;
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Unable to initialize SDL:  %s\n",SDL_GetError());
        return ;
    }
    
    window = SDL_CreateWindow("RealSpace OBJ Viewer",0,0,WINDOW_WIDTH,WINDOW_HEIGHT,SDL_WINDOW_OPENGL);
    
    // Create an OpenGL context associated with the window.
    SDL_GL_CreateContext(window);
    
    glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);			// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
    
    
	// Calculate The Aspect Ratio Of The Window
    int zoom= 30;
	glOrtho(-WINDOW_WIDTH/zoom,WINDOW_WIDTH/zoom,-WINDOW_HEIGHT/zoom,WINDOW_HEIGHT/zoom,-300,300);
    
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
    
    IMG_PumpEvents();
}


