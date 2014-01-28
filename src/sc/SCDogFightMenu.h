//
//  SCDogFightMenu.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCDogFightMenu__
#define __libRealSpace__SCDogFightMenu__

class SCDogFightMenu : public IActivity{
    
public:
    
    SCDogFightMenu();
    ~SCDogFightMenu();
    
    void Init( );
    
    void Run(void);


private:
    
   // void ParsePalette(PakEntry* entry);
    VGAPalette palette;
};

#endif /* defined(__libRealSpace__SCDogFightMenu__) */
