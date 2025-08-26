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

void DebugScreen::setTitle(const char* title){
    SDL_SetWindowTitle(sdlWindow, title);
}

void DebugScreen::init(int w, int h, bool fullscreen){
    
    width = w;
    height = h;

    // 1. Initialiser SDL AVANT toute création de fenêtre
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return;
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // 2. Attributs OpenGL adaptés macOS 10.14 (2.1 max)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (fullscreen) {
        window_flags = (SDL_WindowFlags)(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    }

    // 3. UNE seule création de fenêtre
    sdlWindow = SDL_CreateWindow("RealSpace OBJ Viewer",
                                 SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED,
                                 this->width,
                                 this->height,
                                 window_flags);
    if (!sdlWindow) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(sdlWindow);
    if (!gl_context) {
        printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return;
    }
    SDL_GL_MakeCurrent(sdlWindow, gl_context);
    SDL_GL_SetSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsLight();

    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, gl_context);
    ImGui_ImplOpenGL2_Init();

    glGenTextures(1, &this->screen_texture);
    glBindTexture(GL_TEXTURE_2D, this->screen_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, this->width, this->height);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_ShowWindow(sdlWindow);
}

void DebugScreen::refresh(void){
    glBindTexture(GL_TEXTURE_2D, this->screen_texture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, this->width, this->height, 0);
    glViewport(0,0,this->width,this->height);			// Reset The Current Viewport
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    bool p_open = true;
    if (ImGui::Begin("##Fullscreen window", &p_open, flags)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Strike Commander Floppy")) {
                    debugGameInstance.loadSC();
                }
                if (ImGui::MenuItem("Strike Commander CD")) {
                    debugGameInstance.loadSCCD();
                }
                if (ImGui::MenuItem("Tactical Operations")) {
                    debugGameInstance.loadTO();   
                }
                if (ImGui::MenuItem("Pacific Strike")) {
                    debugGameInstance.loadPacific();   
                }
                if (ImGui::MenuItem("Test mission SC")) {
                    debugGameInstance.testMissionSC();
                }
                if (ImGui::MenuItem("Test Objects")) {
                    debugGameInstance.testObjects();
                }
                if (ImGui::MenuItem("Toggle upscale")) {
                    VGA.upscale = !VGA.upscale;
                }
                if (ImGui::MenuItem("Exit")) {
                    exit(0);
                }
                ImGui::EndMenu();
            }
            IActivity* act = Game->getCurrentActivity();
            if (act != nullptr) {
                act->renderMenu();    
            }
            ImGui::EndMenuBar();
        }
        ImVec2 winsize = ImGui::GetContentRegionAvail();
        float width = winsize.x * 0.75f; // 75% of the available width for the game screen
        ImGui::BeginChild("Game", ImVec2(width, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        ImGui::Image((void*)(intptr_t)this->screen_texture, avail_size, {0, 1}, {1, 0});
        ImGui::EndChild();
        ImGui::SameLine();
        // Calculate the remaining width for the Console child
        float remainingWidth = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("Console", ImVec2(remainingWidth, 0), ImGuiChildFlags_Border | ImGuiWindowFlags_NoSavedSettings);
        IActivity* act = Game->getCurrentActivity();
        if (act != nullptr) {
            act->renderUI();    
        }
        ImGui::EndChild();
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