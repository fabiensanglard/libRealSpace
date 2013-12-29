//
//  Base.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Base.h"

static char base[512];

const char* GetBase(void){
    return base;
}

void SetBase(const char* newBase){
    
    strncpy(base,newBase,512);
    
    //If the last character is not '/', add it
    if (base[strlen(base)-1] != '/')
        strcat(base,"/");
    
}

#ifdef _WIN32
void Sys_CreateDirectory(const char* path){
#include <FileAPI.h>
    CreateDirectoryA(path,NULL);
}
#else
    #include <sys/stat.h>
void Sys_CreateDirectory(const char* path){
    mkdir(path,S_IRWXU | S_IRUSR | S_IWUSR| S_IXUSR);
}
#endif


void CreateDirectories(const char* path){
    
    const char* cursor = path;
    const char* endPath = path+strlen(path);
    char subPath[512];
    
    while(1){
        
        while (*cursor != '\\' &&
               *cursor!= '/'  &&
               cursor != endPath) {
            cursor++;
        }
        
        if (cursor == endPath)
            return;
        
        if (*cursor != '\\' || *cursor!= '/'){
            
            memcpy(subPath, path, cursor-path+1);
            subPath[path-cursor+1] = '\0';
            Sys_CreateDirectory(subPath);
            cursor++;
        }
            else
                return;
        
    }
    
}