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
#include "imgui_impl_opengl3.h"

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

RSScreen::RSScreen(){
    
}


RSScreen::~RSScreen(){
    
}

void RSScreen::SetTitle(const char* title){
    SDL_SetWindowTitle(sdlWindow, title);
}

void RSScreen::Init(int32_t zoomFactor){
    this->scale =zoomFactor;
    
    int32_t width  = 320 * scale;
    int32_t height = 200 * scale;
    
    
    this->width = width;
    this->height = height;
    
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_HIDDEN, &sdlWindow, &sdlRenderer);
    
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("Unable to initialize SDL:  %s\n",SDL_GetError());
        return ;
    }
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
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
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    glViewport(0,0,this->width,this->height);			// Reset The Current Viewport
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    SDL_ShowWindow(sdlWindow);
}

void RSScreen::Refresh(void){
    SDL_GL_SwapWindow(sdlWindow);
}