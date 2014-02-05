//
//  SCFont.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


RSFont::RSFont(){
    
}

RSFont::~RSFont(){
    
}

RLEShape* RSFont::GetShapeForChar(char c){
    return this->letters[c];
}

void RSFont::InitFromPAK(PakArchive* fontArchive){
    for (size_t i = 0 ; i < fontArchive->GetNumEntries(); i++) {
        PakEntry* entry = fontArchive->GetEntry(i);
        RLEShape* s = new RLEShape();
        s->Init(fontArchive->GetEntry(i)->data,fontArchive->GetEntry(i)->size);
        letters.push_back(s);
    }
}