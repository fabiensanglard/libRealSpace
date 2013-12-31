//
//  RSMapTextureSet.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

RSMapTextureSet::RSMapTextureSet(){
    
}

RSMapTextureSet::~RSMapTextureSet(){
    while (!textures.empty()){
        Texture* texture = textures.back();
        textures.pop_back();
        delete texture;
    }
}


void RSMapTextureSet::InitFromPAK(PakArchive* archive){
    
    strcpy(name,archive->GetName());
    
    Parse(archive);
}

size_t RSMapTextureSet::GetNumTextures(void){
    return textures.size();
}

Texture* RSMapTextureSet::GetTextureById(size_t index){
    return textures[index];
}


void RSMapTextureSet::Parse(PakArchive* archive){
    
    
    for(size_t i = 0 ; i < archive->GetNumEntries() ; i++){
        PakEntry* entry = archive->GetEntry(i);
        
        ByteStream stream(entry->data);
        
        uint16_t width = stream.ReadUShort();
        uint16_t height = stream.ReadUShort();
      
        uint32_t size = width*height;
        
        if (entry->size-4 == size){
            //That does look like a map texture !
            Texture* texture = new Texture();
            texture->Set("MAP_TEXTURE",width,height,stream.GetPosition());
            textures.push_back(texture);
        }
    }
}

void RSMapTextureSet::List(FILE* output){
    
    printf("Listing all textures from '%s'.\n",name);
    
    for(size_t i = 0 ; i < textures.size() ; i++){
        Texture* texture = textures[i];
        printf("Texture [%3lu] width: %d height: %d size: %d bytes.\n",i,texture->width,texture->height,texture->width*texture->height);
    }
    
}

