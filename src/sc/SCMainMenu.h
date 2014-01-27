//
//  SCMainMenu.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCMainMenu__
#define __libRealSpace__SCMainMenu__


class SCMainMenu : public IActivity{
    
public:
    SCMainMenu();
    ~SCMainMenu();
    
    virtual void Init(void);
    virtual void Run(void);
    
private:
    
    void LoadButtons(void);
    void LoadBoard(void);
    void LoadBackgrounds(void);
    void LoadPalette(void);
    
    void CheckHit(void);
    SCButton buttons[5];
    RLEShape board;
    RLEShape sky;
    RLEShape mountain;
    RLEShape cloud;
    
    PakArchive mainMenupak;
    
    void DrawMenu(void);
};

#endif /* defined(__libRealSpace__SCMainMenu__) */
