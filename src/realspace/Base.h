//
//  Base.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <string.h>
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
#include <FileAPI.h>
#endif
const char* GetBase(void);
void SetBase(const char* base);
void CreateDirectories(const char* path);


