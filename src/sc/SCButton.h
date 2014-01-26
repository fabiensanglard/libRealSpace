//
//  Button.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Button__
#define __libRealSpace__Button__

typedef void (*ActionFunction)(void);

class SCButton{
    
    SCButton();
    ~SCButton();
    
     
    Point2D position;
    
    void Init(RSImage* upApp, RSImage* downApp,ActionFunction fct);
    
    void OnAction(void);
    
    
private:
    
    enum Appearance { APR_UP, APR_DOWN};

    enum State { STATE_UP, STATE_DOWN };
    State state;
    
    bool clickable ;
    
    RSImage* apperance[2];
    
    ActionFunction onClick;
    
};

#endif /* defined(__libRealSpace__Button__) */
