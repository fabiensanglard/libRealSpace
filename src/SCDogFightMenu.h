//
//  SCDogFightMenu.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCDogFightMenu__
#define __libRealSpace__SCDogFightMenu__

class SCDogFightMenu{
    
public:
    
    SCDogFightMenu();
    ~SCDogFightMenu();
    
    void Init( );
    
    void Run(void);

    static const uint8_t PAK_ID_PALETTE          = 7;
    static const uint8_t PAK_ID_BACKGROUND       = 6;
    static const uint8_t PAK_ID_TITLE            = 1 ;
    static const uint8_t PAK_ID_BUTTONS          = 3 ;
private:
    
    void ParsePalette(PakEntry* entry);
    VGAPalette palette;
};

#endif /* defined(__libRealSpace__SCDogFightMenu__) */
