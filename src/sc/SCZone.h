//
//  Button.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Zone__
#define __libRealSpace__Zone__

typedef void (*ActionFunction)(void);

class SCZone{
    
public:
    SCZone();
    ~SCZone();
    
     
    Point2D position;
    Point2D dimension;
    
    
    void InitBehavior(ActionFunction fct, Point2D position, Point2D dimension);
    
    
    void OnAction(void);

    void Draw(void);
    
    inline bool IsEnabled(void){ return this->enabled; }
    inline void SetEnable(bool enabled){ this->enabled = enabled;}
    
    
private:
    
    
    bool enabled ;
    
    ActionFunction onClick;
    
};

#endif /* defined(__libRealSpace__Button__) */
