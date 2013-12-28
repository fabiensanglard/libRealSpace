//
//  test.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"



void testTRE(void){
    
    TreArchive treArchive;
    treArchive.InitFromFile("OBJECTS.TRE");
    treArchive.List(stdout);
    
}

void testJet(void){
    
    const char* trePath = "OBJECTS.TRE";
    const char* jetPath = "..\\..\\DATA\\OBJECTS\\A-10.IFF";
    
    
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    
    
    for (size_t i=0 ; i < treArchive.GetNumEntries() ; i++){
        TreEntry* entry = treArchive.GetEntryByID(i);
        if (strcmp(entry->name, jetPath))
            ;//fprintf(output,"    Entry [%3lu] '%s' size: %lu bytes.\n",i,entry->name,entry->size);
        else
            fprintf(stdout,"    FOUND !Entry [%3lu] '%s' size: %lu bytes.\n",i,entry->name,entry->size);
    }
    
    TreEntry* iffJet = treArchive.GetEntryByName(jetPath);
    
    if (iffJet == NULL){
        printf("Unable to find jet '%s' in TRE archive '%s'.\n",jetPath,trePath);
        return;
    }
        
    
    IffLexer lexer ;
    lexer.InitFromRAM(iffJet->data,iffJet->size);
    
}






int main( int argc,char** argv){
    
    SetBase("/Users/fabiensanglard/Desktop/SC/strikecommander/SC");
    testJet();
}