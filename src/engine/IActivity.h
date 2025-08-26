//
//  IActivity.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__IActivity__
#define __libRealSpace__IActivity__
#include <vector>
#include "../commons/Maths.h"
#include "SCButton.h"
#include "RSVGA.h"
#include "RSScreen.h"
#include "SCMouse.h"

class GameEngine;

extern RSScreen *Screen;
extern GameEngine *Game;
extern SCMouse Mouse;
extern RSVGA VGA;

char* strtoupper(char* dest, const char* src);
bool isPointInQuad(const Point2D &p, const std::vector<Point2D *> *quad);

class IActivity{
private:
    bool running;

protected:
    bool music_playing;
    bool focused;
    int timer{0};
    IActivity();
    SCButton* checkButtons(void);
    std::vector<SCButton*> buttons;
    void drawButtons(void);
    
public:
    VGAPalette palette;

    virtual void init(void) = 0;
    virtual void start(void) { this->running = true;}
    void stop(void);
    virtual void runFrame (void) = 0;
    void setTitle(const char* title);
    inline bool isRunning(void){ return this->running; }
    virtual ~IActivity();
    virtual void focus(void)  { this->focused = true;}
    virtual void unFocus(void){ this->focused = false;}
    virtual void renderMenu(void) {}
    virtual void renderUI(void) {}
    void checkKeyboard(void);
};


#endif /* defined(__libRealSpace__IActivity__) */
