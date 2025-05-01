//
//  Screen->cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "DebugScreen.h"
#include "DebugGame.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;
extern DebugGame debugGameInstance;

DebugScreen::DebugScreen(){
    
}


DebugScreen::~DebugScreen(){
    
}

void DebugScreen::SetTitle(const char* title){
    SDL_SetWindowTitle(sdlWindow, title);
}

void DebugScreen::init(int w, int h, bool fullscreen){
    
    width = w;
    height = h;
    
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
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (fullscreen) {
        window_flags = (SDL_WindowFlags)(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    }
    
    sdlWindow = SDL_CreateWindow("RealSpace OBJ Viewer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,this->width,this->height,window_flags);
    
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
    int wi = (int)((float)this->height * (4.0f/3.0f));

    glGenTextures(1, &this->screen_texture);
    glBindTexture(GL_TEXTURE_2D, this->screen_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport((int)((float)(this->width - w)/2.0f),0,wi,this->height);			// Reset The Current Viewport
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    SDL_ShowWindow(sdlWindow);
}

void DebugScreen::Refresh(void){
    glBindTexture(GL_TEXTURE_2D, this->screen_texture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, this->width, this->height, 0);
    glViewport(0,0,this->width,this->height);			// Reset The Current Viewport
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Strike Commander")) {
                debugGameInstance.loadSC();
            }
            if (ImGui::MenuItem("Tactical Operations")) {
                debugGameInstance.loadTO();   
            }
            if (ImGui::MenuItem("Pacific Strike")) {
                debugGameInstance.loadPacific();   
            }
            if (ImGui::MenuItem("Exit")) {
                exit(0);
            }
            ImGui::EndMenu();
        }
        IActivity* act = Game->GetCurrentActivity();
        if (act != nullptr) {
            act->renderMenu();    
        }
        ImGui::EndMainMenuBar();
    }
    if (ImGui::Begin("Game")) {
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        ImGui::Image((void*)(intptr_t)this->screen_texture, avail_size, {0, 1}, {1, 0});
        ImGui::End();
    }
    ImGui::Render();
    ImDrawData *dt=ImGui::GetDrawData();
    if (dt != nullptr) {
        ImGui_ImplOpenGL2_RenderDrawData(dt);
    }
    SDL_GL_SwapWindow(sdlWindow);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
}