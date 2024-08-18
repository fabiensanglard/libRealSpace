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
    
    strncpy_s(base,newBase,512);
    
    //If the last character is not '/', add it
    if (base[strlen(base)-1] != '/')
        strcat_s(base,"/");
    
}

#ifdef _WIN32
int Sys_CreateDirectory(const char* path){

    return CreateDirectoryA(path,NULL);
}
#else
    #include <sys/stat.h>
    #include <errno.h>
int Sys_CreateDirectory(const char* path){
    return mkdir(path,S_IRWXU | S_IRUSR | S_IWUSR| S_IXUSR);
}
#endif

void printErrorMessage(int error,const char* subPath){
    
    if (error ==EEXIST || error == ENOENT)
        return;
    
    printf("Error while creating '%s': Reason:\n",subPath);
    switch (error) {
        case EACCES: printf("EACCES: Search permission is denied for a component of the path prefix.| Write permission is denied for the parent directory."); break;
#ifndef _WIN32
		case EDQUOT: printf("EDQUOT: The new directory cannot be created because the user's quota of disk blocks on the file system that will contain the directory has been exhausted.|The user's quota of inodes on the file system on which the directory is being created has been exhausted."); break;
#endif // !_WIN32
        case EEXIST: printf("EEXIST: The named file exists."); break;
        case EFAULT: printf("EFAULT:  Path points outside the process's allocated addressspace."); break;
        case EIO: printf("EIO: An I/O error occurred while making the directory entryor allocating the inode. | An I/O error occurred while reading from or writing to the file system."); break;
        case ELOOP: printf("ELOOP: Too many symbolic links were encountered in translat-ing the pathname."); break;
        case EMLINK: printf("EMLINK: The parent directory already has {LINK_MAX} links."); break;
        case ENAMETOOLONG: printf("ENAMETOOLONG: A component of a pathname exceeded {NAME_MAX} characters, or an entire path name exceeded {PATH_MAX} characters."); break;
        case ENOENT: printf("ENOENT: A component of the path prefix does not exist or pathis an empty string."); break;
        case ENOSPC: printf("ENOSPC: The new directory cannot be created because there is no space left on the file system that would contain it. | There are no free inodes on the file system on which the directory is being created."); break;
        case ENOTDIR: printf("ENOTDIR: A component of the path prefix is not a directory."); break;
        case EROFS: printf("EROFS: The parent directory resides on a read-only file sys-tem."); break;
        default:
            printf("UKNOWN: (error id = %d | Directory could not be created.",error); break;
            
            break;
    }
    printf("\n\n");
}



void CreateDirectories(const char* path){
    
    const char* cursor = path;
    const char* endPath = path+strlen(path);
    

    char subPath[512];
    char* dst=subPath;
    
    while(1){
        
        while (*cursor != '\\' &&
               *cursor!= '/'  &&
               cursor != endPath) {
            *dst=*cursor;
            cursor++;
            dst++;
        }
        
        if (cursor >= endPath)
            return;
        
        
        *dst = 0;
        
        
            
            
            int error = Sys_CreateDirectory(subPath);
            if (error == -1){
                printErrorMessage(errno,subPath);
            }
        else
            printf("Created folder: '%s'.\n",subPath);
        
        *dst=*cursor;
        cursor++;
        dst++;
        
    }
    
}