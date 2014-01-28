//
//  NSObjectViewer.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__NSObjectViewer__
#define __libRealSpace__NSObjectViewer__

class SCObjectViewer : public IActivity{
    
public:
    SCObjectViewer();
    ~SCObjectViewer();
    
    void Init( );
    
    void RunFrame(void);
    
    
    
    void NextObject(void);
    
private:

    
    typedef struct RSShowCase{
        
        float cameraDist;
        RSEntity* entity;
        
        char displayName[20];
        
    } RSShowCase;
    
    std::vector<RSShowCase> showCases;
    
    
    void ParseObjList(IffLexer* lexer);
    void ParseAssets(PakArchive* archive);
    
    RLEShape bluePrint;
    RLEShape title;
    RLEShape board;
    
    uint32_t currentObject;
    
    //For rotating the object
    uint32_t startTime;
};

#endif /* defined(__libRealSpace__NSObjectViewer__) */
