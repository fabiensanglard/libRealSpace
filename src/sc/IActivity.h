//
//  IActivity.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__IActivity__
#define __libRealSpace__IActivity__


class IActivity{
    
public:
    
    virtual void Init(void) = 0;
    virtual void Start(void) { this->running = true;}
    void Stop(void) { this->running = false;}
    
    virtual void RunFrame (void) = 0;
    
    void SetTitle(const char* title);
    
    inline bool IsRunning(void){ return this->running; }
    
    VGAPalette palette;
    
    virtual ~IActivity();
    
    virtual void Focus(void)  { this->focused = true;}
    virtual void UnFocus(void){ this->focused = false;}
    
    
protected:
    
    IActivity();
    SCButton* CheckButtons(void);
    SCZone* CheckZones(void);
    std::vector<SCButton*> buttons;
    std::vector<SCZone*> zones;
    void DrawButtons(void);
    
private:
    bool running;
    bool focused;
    
    
};


#endif /* defined(__libRealSpace__IActivity__) */
