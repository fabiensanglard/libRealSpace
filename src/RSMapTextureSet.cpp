//
//  RSMapTextureSet.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSMapTextureSet.h"

RSMapTextureSet::RSMapTextureSet(){
    
}

RSMapTextureSet::~RSMapTextureSet(){
    while (!images.empty()){
        RSImage* image = images.back();
        images.pop_back();
        delete image;
    }
}


void RSMapTextureSet::InitFromPAK(PakArchive* archive){
    
    strcpy(name,archive->GetName());
    
    Parse(archive);
}

size_t RSMapTextureSet::GetNumImages(void){
    return images.size();
}

RSImage* RSMapTextureSet::GetImageById(size_t index){
    return images[index];
}


void RSMapTextureSet::Parse(PakArchive* archive){
    
    
    for(size_t i = 0 ; i < archive->GetNumEntries() ; i++){
        
        if (i == 229 || i ==  230)
            printf("Intersest !\n");
        
        PakEntry* entry = archive->GetEntry(i);
        
        if (entry->size == 0)
            continue;
        
        ByteStream stream(entry->data);
        
        uint16_t width = stream.ReadUShort();
        uint16_t height = stream.ReadUShort();
      
        uint32_t size = width*height;
        
        if (entry->size-4 == size){
            //That does look like a map texture !
            RSImage* image = new RSImage();
            
            image->Create("MAP_TEXTURE",width,height);
            image->UpdateContent(stream.GetPosition());
            images.push_back(image);
            //printf("RSMapTextureSet img [%3zu] is %lux%lu.\n",i,image->width,image->height);
        }
        else
            printf("Cannot make sense of entry %lu:\n REASON: (entry size is %lu but advertised is %d).\n",i,entry->size,size);
    }
    
    printf("RSMapTextureSet found %lu textures in %s.\n",archive->GetNumEntries(),archive->GetName());
}

void RSMapTextureSet::List(FILE* output){
    
    printf("Listing all textures from '%s'.\n",name);
    
    for(size_t i = 0 ; i < images.size() ; i++){
        RSImage* image = images[i];
        printf("Image [%3lu] width: %zu height: %zu size: %lu bytes.\n",i,image->width,image->height,image->width*image->height);
    }
    
}

