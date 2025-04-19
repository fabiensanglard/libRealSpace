//
//  Screen.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSScreen.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

RSScreen::RSScreen(){
    
}


RSScreen::~RSScreen(){
    
}

void RSScreen::SetTitle(const char* title){
    SDL_SetWindowTitle(sdlWindow, title);
}

void RSScreen::init(int width, int height, bool fullscreen){
    
    this->width = width;
    this->height = height;

    /*this->width = 1200;
    this->height = 800;*/
    
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_HIDDEN, &sdlWindow, &sdlRenderer);
    
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("Unable to initialize SDL:  %s\n",SDL_GetError());
        return ;
    }


#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    if (fullscreen) {
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    } else {
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    }
    
    sdlWindow = SDL_CreateWindow("RealSpace OBJ Viewer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,this->width,this->height,SDL_WINDOW_OPENGL);
    
    // Create an OpenGL context associated with the window.
    SDL_GLContext gl_context = SDL_GL_CreateContext(sdlWindow);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, gl_context);
    ImGui_ImplOpenGL2_Init();
    int w = this->height * (4.0f/3.0f);
    glViewport((this->width - w)/2.0,0,w,this->height);			// Reset The Current Viewport
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    SDL_ShowWindow(sdlWindow);
}

void RSScreen::Refresh(void){
    SDL_GL_SwapWindow(sdlWindow);
}