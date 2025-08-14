//
//  Screen->cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSScreen.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"
#include <functional>

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

RSScreen::RSScreen(){
    
}


RSScreen::~RSScreen(){
    
}

void RSScreen::setTitle(const char* title){
    SDL_SetWindowTitle(sdlWindow, title);
}

void RSScreen::init(int width, int height, bool fullscreen){
    
    this->width = width;
    this->height = height;

    
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
    int w = (int) ((float)this->height * (4.0f/3.0f));
    glViewport((int)((float)(this->width - w)/2.0f),0,w,this->height);			// Reset The Current Viewport
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    SDL_ShowWindow(sdlWindow);
}

void RSScreen::refresh(void){
    SDL_GL_SwapWindow(sdlWindow);
}

void RSScreen::fxTurnOnTv() {
    // TV turn-on animation variables
    static bool initialized = false;
    static float animationTime = 0.0f;
    static const float animationDuration = 3.0f;  // 1.5 seconds for the whole animation
    static GLuint scanlineTexture = 0;

    // Initialize the scanline texture the first time
    if (!initialized) {
        // Create scanline texture
        glGenTextures(1, &scanlineTexture);
        glBindTexture(GL_TEXTURE_2D, scanlineTexture);
        
        // Create a 1D pattern for scanlines
        const int texHeight = 256;
        unsigned char* scanlineData = new unsigned char[texHeight * 4];
        for (int i = 0; i < texHeight; i++) {
            float intensity = 0.85f + 0.15f * sin(i * 0.5f);
            scanlineData[i * 4 + 0] = static_cast<unsigned char>(255 * intensity);
            scanlineData[i * 4 + 1] = static_cast<unsigned char>(255 * intensity);
            scanlineData[i * 4 + 2] = static_cast<unsigned char>(255 * intensity);
            scanlineData[i * 4 + 3] = 255;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, scanlineData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        delete[] scanlineData;
        initialized = true;
    }

    // Update animation time
    const float deltaTime = 1.0f / 60.0f;  // Assuming 60fps
    animationTime = (std::min)(animationTime + deltaTime, animationDuration);
    float animProgress = animationTime / animationDuration;
    if (animProgress >= 1.0f) {
        animProgress = 1.0f;  // Clamp to 1.0
        is_spfx_finished = true;  // Mark that the screen is now on
        return;
    }
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw TV turning on effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // TV turn-on animation with central point expanding
    if (animProgress < 0.1f) {
        // First phase: Horizontal line expands from center
        float horizontalWidth = animProgress / 0.1f * 2.0f; // 0 to 2.0
        float lineHeight = 0.01f;
        
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(-horizontalWidth/2, -lineHeight/2);
            glVertex2f(horizontalWidth/2, -lineHeight/2);
            glVertex2f(horizontalWidth/2, lineHeight/2);
            glVertex2f(-horizontalWidth/2, lineHeight/2);
        glEnd();
    } else if (animProgress < 0.2f) {
        // Second phase: Vertical expansion after horizontal is complete
        float normalizedProgress = (animProgress - 0.1f) / 0.1f; // 0 to 1.0
        float verticalHeight = normalizedProgress * 2.0f;  // 0 to 2.0
        
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(-1.0f, -verticalHeight/2);
            glVertex2f(1.0f, -verticalHeight/2);
            glVertex2f(1.0f, verticalHeight/2);
            glVertex2f(-1.0f, verticalHeight/2);
        glEnd();
    } else if (animProgress < 0.3f && animProgress >= 0.5f) {
        // Third phase: Full white screen fading out as noise fades in
        float fadeOutWhite = 1.0f - ((animProgress - 0.5f) / 0.2f);
        
        glColor4f(1.0f, 1.0f, 1.0f, fadeOutWhite);
        glBegin(GL_QUADS);
            glVertex2f(-1.0f, -1.0f);
            glVertex2f(1.0f, -1.0f);
            glVertex2f(1.0f, 1.0f);
            glVertex2f(-1.0f, 1.0f);
        glEnd();
    }
    

    // White noise effect fading out
    if (animProgress >= 0.3f && animProgress < 0.7f) {
        float noiseIntensity = 1.0f - (animProgress / 0.8f);
        
        glColor4f(1.0f, 1.0f, 1.0f, noiseIntensity);
        glBegin(GL_QUADS);
        for (int i = 0; i < 10000; i++) {
            float x = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            float y = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            float size = 0.003f + 0.002f * static_cast<float>(rand()) / RAND_MAX;
            
            glVertex2f(x - size, y - size);
            glVertex2f(x + size, y - size);
            glVertex2f(x + size, y + size);
            glVertex2f(x - size, y + size);
        }
        glEnd();
    }

    // Horizontal line sweeping from center
    if (animProgress > 0.3f && animProgress < 0.8f) {
        float lineProgress = (animProgress - 0.2f) / 0.6f;
        float lineHeight = 0.05f;
        float lineY = (lineProgress * 2.0f - 1.0f);
        
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, lineY - lineHeight);
        glVertex2f(1.0f, lineY - lineHeight);
        glVertex2f(1.0f, lineY + lineHeight);
        glVertex2f(-1.0f, lineY + lineHeight);
        glEnd();
    }

    // Add scanlines and CRT effect
    if (animProgress > 0.5f) {
        float scanlineAlpha = (animProgress - 0.5f) / 0.5f * 0.3f;
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, scanlineTexture);
        glColor4f(1.0f, 1.0f, 1.0f, scanlineAlpha);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
        glTexCoord2f(800.0f, 0.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(800.0f, 600.0f); glVertex2f(1.0f, -1.0f);
        glTexCoord2f(0.0f, 600.0f); glVertex2f(-1.0f, -1.0f);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
    }

    glDisable(GL_BLEND);

}
