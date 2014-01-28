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
    
public:
    SCButton();
    ~SCButton();
    
     
    Point2D position;
    Point2D dimension;
    
    
    void InitBehavior(ActionFunction fct, Point2D position, Point2D dimension);
    
    
    void OnAction(void);
    
    enum Appearance { APR_UP, APR_DOWN};
    
    enum State { STATE_UP, STATE_DOWN };
    
    
    RLEShape appearance[2];
    
    inline bool IsEnabled(void){ return this->enabled; }
    inline void SetEnable(bool enabled){ this->enabled = enabled;}
    
private:
    
    
    State state;
    
    bool enabled ;
    
    
    
    ActionFunction onClick;
    
};

#endif /* defined(__libRealSpace__Button__) */
