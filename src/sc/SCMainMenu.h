//
//  SCMainMenu.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCMainMenu__
#define __libRealSpace__SCMainMenu__

enum ButtonIDS { BUTTON_CONTINUE, BUTTON_LOADGAME, BUTTON_STARTNEWGAME, BUTTON_TRAINING, BUTTON_OBJVIEWER };

#define MAINMENU_PAK_PATH "..\\..\\DATA\\GAMEFLOW\\MAINMENU.PAK"

#define MAINMENU_PAK_BUTTONS_INDICE 0
#define MAINMENU_PAK_BOARD_INDICE 1
#define MAINMENU_PAK_BOARD_PALETTE 2

class SCMainMenu : public IActivity{
    
public:
    SCMainMenu();
    ~SCMainMenu();
    
    virtual void Init(void);
    virtual void RunFrame(void);
    virtual void Focus(void);
    virtual void UnFocus(void);
    
private:
    
    void LoadButtons(void);
    void LoadBoard(void);
    void LoadBackgrounds(void);
    void LoadPalette(void);
    
    
    
    RLEShape board;
    RLEShape sky;
    RLEShape mountain;
    RLEShape cloud;
    
    PakArchive mainMenupak;
    Point2D boardPosition;

    void OnViewObject();
    void OnTrainingMission();
    void OnStartNewGame();
    void OnLoadGame();
    void OnContinue(void);

};

#endif /* defined(__libRealSpace__SCMainMenu__) */