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
    
    // Let's open the TRE archive.
    TreArchive treArchive;
    treArchive.InitFromFile(trePath);
    
    // Let's open the jet IFF file in that archive
    TreEntry* iffJet = treArchive.GetEntryByName(jetPath);
    
    // Oops !
    if (iffJet == NULL){
        printf("Unable to find jet '%s' in TRE archive '%s'.\n",jetPath,trePath);
        return;
    }
        
    // Parse it.
    IffLexer lexer ;
    lexer.InitFromRAM(iffJet->data,iffJet->size);
    
    //Render it !
    
}






int main( int argc,char** argv){
    
    SetBase("/Users/fabiensanglard/Desktop/SC/strikecommander/SC");
    testJet();
}