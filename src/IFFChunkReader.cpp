//
//  IFFChunkReader.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/27/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

IFFChunkReader::IFFChunkReader(){
    
}

IFFChunkReader::~IFFChunkReader(){
    
}



bool IFFChunkReader::CanRead(char chunkID[4]){
    return true;
}

void IFFChunkReader::Read   (char chunkID[4],uint32_t size,uint8_t* data){
    
}