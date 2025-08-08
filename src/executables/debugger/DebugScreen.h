//
//  Screen->h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#include "../../engine/RSScreen.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
}
#endif
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <SDL.h>

class DebugScreen: public RSScreen{
    
public:
    
    DebugScreen();
    ~DebugScreen();
    
    void init(int width, int height, bool fullscreen);
    void SetTitle(const char* title);
    void Refresh(void);
private:
    GLuint screen_texture;
    
};
