//
//  Screen.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

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
class RSScreen{
    
public:
    
    RSScreen();
    ~RSScreen();
    
    void Init(int32_t zoomFactor);
    void SetTitle(const char* title);
    void Refresh(void);
    
    int32_t width;
    int32_t height;
    int32_t scale;
private:
    
    
};
