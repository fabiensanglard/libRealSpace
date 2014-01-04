//
//  NSObjectViewer.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__NSObjectViewer__
#define __libRealSpace__NSObjectViewer__

class SCObjectViewer{
    
public:
    SCObjectViewer();
    ~SCObjectViewer();
    
    void Init( );
    
    void Run(void);
    
    void ListObjects(void);
    
    static const uint8_t PAK_ID_MENU_STATIC       = 0;
    static const uint8_t PAK_ID_MENU_DYNAMC       = 4;
    
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_BACKGROUND_RELEASED   = 0;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_BACKGROUND_PRESSED    = 1;
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW_RELEASED      = 2;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW_PRESSED       = 3;
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWNARROW_RELEASED    = 4;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWNARROW_PRESSED     = 5;
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW2_RELEASED     = 6;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW2_PRESSED      = 7;
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWN2ARROW_RELEASED   = 8;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWN2ARROW_PRESSED    = 9;
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_LEFTARROW_RELEASED    =10;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_LEFTARROW_PRESSED     =11;
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_RIGHTARROW_RELEASED   =12;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_RIGHTARROW_PRESSED    =13;
    
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_EXIT_RELEASED         =14;
    static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_EXIT_PRESSED          =15;
    
    static const uint8_t PAK_ID_BACKGROUND  = 8;
    
private:

    
    typedef struct RSShowCase{
        
        float cameraDist;
        RSEntity* entity;
        
        char displayName[20];
        
    } RSShowCase;
    
    std::vector<RSShowCase> showCases;
    
    
    void ParseObjList(IffLexer* lexer);
    void ParseAssets(PakArchive* archive);
};

#endif /* defined(__libRealSpace__NSObjectViewer__) */
