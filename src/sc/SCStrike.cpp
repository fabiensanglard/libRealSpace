//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


SCStrike::SCStrike(){
    
}

SCStrike::~SCStrike(){
    
}

void SCStrike::Init(void ){
    area.InitFromPAKFileName("ARENA.PAK");
    
}

void SCStrike::RunFrame(void){
    
    Renderer.RenderWorldSolid(&area,BLOCK_LOD_MAX,400);
    
}
